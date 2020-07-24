#include <iostream>
#include <unistd.h>
#include <cmath>
#include <QtCore/QDateTime>

#include "Processing.h"


/**
 * The constructor of the Processing thread.
 *
 * Initialises internal objects and prepares the thread for running.
 */
Processing::Processing(double fcLP, double fcHP) :
        rawData(DEFAULT_DATA_SIZE),
        bRunning(false),
        bMeasuring(false) {

    PLOG_VERBOSE << "Processing started";

    currentState = ProcState::Config;
    comedi = new ComediHandler();

    sampling_rate = comedi->getSamplingRate();

    // LP filter, default value is 10 Hz, which also takes care of 50 Hz noise.
    if (fcLP > 20.0 || fcLP < 5.0) {
        PLOG_WARNING << "Processing running with low pass filter of: " << fcLP;
    }

    iirLP = new Iir::Butterworth::LowPass<IIRORDER>;
    assert(iirLP != NULL);
    iirLP->setup(sampling_rate, fcLP);

    // HP filter, default value is 0.5 Hz.
    if (fcHP != 0.5) {
        PLOG_WARNING << "Processing running with high pass filter of: " << fcHP;
    }
    iirHP = new Iir::Butterworth::HighPass<IIRORDER>;
    assert(iirHP != NULL);
    iirHP->setup(sampling_rate, fcHP);

    obpDetect = new OBPDetection();
    rawData.clear();

    record = new Datarecord(sampling_rate);

}

/**
 * The destructor of the Processing thread.
 *
 * Stops the measurement if not already done and stops the thread.
 */
Processing::~Processing() {
    stopMeasurement();
    stopThread();
    delete iirHP;
    delete iirLP;
    delete comedi;
    delete record;
    delete obpDetect;
}

void Processing::setAmbientVoltage(double voltage) {
    ambientVoltage = voltage;
}

/**
 * The main function of the thread.
 */
void Processing::run() {
    bRunning = true;

    double i = 0.6;
    while (bRunning) {

        if (comedi->getBufferContents() > 0) {
            processSample(comedi->getVoltageSample());
        } else {
            // If there was no data in the buffer, sleep for 1ms.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

/**
 * Stops the thread by stopping the data acquisition so the thread terminates and can be joined.
 */
void Processing::stopThread() {
    bRunning = false;
}


/**
 * Starts a new measurement.
 */
void Processing::startMeasurement() {
    PLOG_VERBOSE << "start Measurement";
    bMeasuring = true;
    PLOG_VERBOSE << "started";
}

/**
 * Stops the measurement and saves the data to a file.
 */
void Processing::stopMeasurement() {

    bMeasuring = false;

}

/**
 * Gets a file name (string) from the current time.
 * @return The file name as a QString.
 */
QString Processing::getFilename() {
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString("yyyy-MM-dd_hh-mm-ss");
    dateTimeString.append("_test.dat");
    return dateTimeString;
}

/**
 * Processing a single new sample in a state machine.
 * @param newSample
 */
void Processing::processSample(double newSample) {

    /**
     * Every sample is filtered and sent to the Observers
     */
    //TODO: change to work with getmmHgValue(newSample)
    double yLP = iirLP->filter(newSample);
    double yHP = iirHP->filter(yLP);
    double ymmHg = getmmHgValue(yLP);

    notifyNewData(ymmHg, yHP);

    switch (currentState) {
        /**
         * Configure the ambient pressure.
         */
        case ProcState::Config:

            if (checkAmbient()) {
                currentState = ProcState::Idle;
                //enable Start button
                notifyReady();
                rawData.clear();

            } else {
                rawData.push_back(yLP);
            }

            break;

            /**
             * Waiting for user to start the measurement.
             */
        case ProcState::Idle:

            if (bMeasuring) {
                notifyResults(0.0, 0.0, 0.0);
                notifyHeartRate(0.0);
                currentState = ProcState::Inflate;
                notifySwitchScreen(Screen::inflateScreen);
            }

            break;
        case ProcState::Inflate:
            if (!bMeasuring) {
                currentState = ProcState::Idle;
                notifySwitchScreen(Screen::startScreen);
            } else {
                rawData.push_back(getmmHgValue(newSample)); //record raw data to store later

                // Check if pressure in cuff is large enough, so it can be switched to the next state.
                if (ymmHg > mmHgInflate) {
                    obpDetect->reset();
                    notifySwitchScreen(Screen::deflateScreen);
                    //TODO: possibly add entry end exit functions for each state
                    // function: switch state: returns new state
                    // performs entry and exit operations (notifications)
                    // would that work with the state class being a friendly to Processing?
                    currentState = ProcState::Deflate;
                }
            }

            break;
        case ProcState::Deflate:
            if (!bMeasuring) {
                currentState = ProcState::Idle;
                notifySwitchScreen(Screen::startScreen);
            } else {
                rawData.push_back(getmmHgValue(newSample)); //record raw data to store later

                if (obpDetect->processSample(getmmHgValue(yLP), yHP)) {//TODO: heart rate currently not displayed
                    if (obpDetect->getIsEnoughData()) {
                        notifyHeartRate(obpDetect->getAverageHeartRate());
                        notifySwitchScreen(Screen::emptyCuffScreen);
                        currentState = ProcState::Calculate;
                    }
                    notifyHeartRate(obpDetect->getCurrentHeartRate());
                }
                if (ymmHg < 20) {
                    PLOG_WARNING << "Pressure too low to continue algorithm. Cancelled";
                    //TODO: notificataion ?
                    bMeasuring = false;
                    bMeasuring = false;
                }
                if (rawData.size() > DEFAULT_DATA_SIZE) {
                    PLOG_WARNING << "Recording too long to continue algorithm. Cancelled";
                    //TODO: notificataion ?
                    bMeasuring = false;
                }
            }
            break;
        case ProcState::Calculate:
            if (!bMeasuring) {
                currentState = ProcState::Idle;
                notifySwitchScreen(Screen::startScreen);
            } else {
                rawData.push_back(getmmHgValue(newSample)); //record raw data to store later
                if (ymmHg < 1) {
                    notifyResults(obpDetect->getMAP(), obpDetect->getSBP(), obpDetect->getDBP());
                    record->saveAll(Processing::getFilename(), rawData);
                    notifySwitchScreen(Screen::resultScreen);
                    currentState = ProcState::Restults;
                }
            }
            break;
        case ProcState::Restults:
            if (!bMeasuring) {
                currentState = ProcState::Idle;
                notifySwitchScreen(Screen::startScreen);
            }
            break;
    }
}

/**
 * Calculates the mmHg value from the given voltage input.
 * @param voltageValue The voltage input.
 * @return The corresponding value in mmHg.
 */
double Processing::getmmHgValue(double voltageValue) const {
    return ((voltageValue - ambientVoltage) * mmHg_per_kPa * kPa_per_V * corrFactor);
}

bool Processing::checkAmbient() {
    bool bAmbientValid = false;

    if (rawData.size() == AMBIENT_AV_TIME) {
        double av = 1.0 * std::accumulate(rawData.begin(), rawData.end(), 0.0) / rawData.size();
        double max = *std::max_element(rawData.begin(), rawData.end());
        auto min = *std::min_element(rawData.begin(), rawData.end());

        PLOG_VERBOSE << "min: " << min << " max: " << max << " av: " << av << std::endl;
        if (std::abs(av - max) < AMBIENT_DEVIATION) {
            setAmbientVoltage(av);
            bAmbientValid = true;
        } else {
            rawData.clear();
        }
    }

    return bAmbientValid;
}