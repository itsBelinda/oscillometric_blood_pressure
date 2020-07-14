
#include <iostream>
#include <unistd.h>
#include <iomanip>

#include <QtCore/QDateTime>

#include "Processing.h"
#include "ComediHandler.h"

#define DEFAULT_MINUTES 2
#define DEFAULT_DATA_SIZE 1024*60*DEFAULT_MINUTES


Processing::Processing() :
        pData(DEFAULT_DATA_SIZE),
        oData(DEFAULT_DATA_SIZE),
        bRunning(false),
        bMeasuring(false) {

    PLOG_VERBOSE << "Processing started";

    currentState = ProcState::Config;
    comedi = new ComediHandler();

    sampling_rate = comedi->getSamplingRate(); //TODO: calculation seems to be wrong, setting manually for now

    // 5Hz mains LP filter
    iirLP = new Iir::Butterworth::LowPass<IIRORDER>;
    assert(iirLP != NULL);
    iirLP->setup(sampling_rate, 10.0); //TODO: parametrise

    // .5Hz mains HP filter
    iirHP = new Iir::Butterworth::HighPass<IIRORDER>;
    assert(iirHP != NULL);
    iirHP->setup(sampling_rate, 0.5); //TODO: parametrise

    pData.clear();
    oData.clear();

    record = new Datarecord(sampling_rate);
}

Processing::~Processing() {
    stopMeasurement();
    stopThread();
}

void Processing::setAmbientVoltage(double voltage){
    ambientVoltage = voltage;
}

void Processing::run() {

    std::cout << "run ..." << std::endl;

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


void Processing::stopThread() {
    // TODO: safely abort measurement
    bRunning = false;
}


/**
 * Starts a new measurment.
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
    if (bMeasuring) {
        bMeasuring = false;
        record->saveAll(Processing::getFilename(), pData);
    }
}

/**
 * Stops the measurement, but without saving the data.
 */
void Processing::resetMeasurement() {
    bMeasuring = false;
}

QString Processing::getFilename() {
    QDateTime dateTime = dateTime.currentDateTime();
    QString dateTimeString = dateTime.toString("yyyy-MM-dd_hh:mm:ss");
    dateTimeString.append("_test.dat");
    return dateTimeString;
}

void Processing::processSample(double newSample) {

    /**
     * Every sample is filtered and sent to the Observers
     */
    double yLP = iirLP->filter(newSample);
    double yHP = iirHP->filter(yLP);
    double ymmHg = getmmHgValue(yLP);

    notifyNewData(ymmHg, yHP);


    switch (currentState) {
        case ProcState::Config:
            //TODO: here I am using the "raw" voltage data, i am missusing the data buffer,
            // this will not cause problems, as long as it is reset afterwards,
            // detect ambient pressure,
            if (checkAmbient()) {
                //enable Start button
                currentState = ProcState::Idle;
                notifyReady();
                pData.clear();

            }
            else {
                pData.push_back(yLP);
            }

            break;

        case ProcState::Idle:

            if( bMeasuring ) {
                currentState = ProcState::Inflate;
                //TODO: should be empty
                pData.clear();
                oData.clear();
            }
            //TODO: only enable start button if successfully detected
            break;
        case ProcState::Inflate:
            pData.push_back(ymmHg);
            oData.push_back(yHP);
            /**
             * Check if the pressure in the cuff is big enough yet.
             * If so, change to the next state.
             */
            if (ymmHg > mmHgInflate) {

                notifySwitchScreen(Screen::deflateScreen);
                // TODO: possibly add entry end exit functions for each state
                // function: switch state: returns new state
                // performs entry and exit operations (notifications)
                // would that work with the state class being a friendly to Processing?
                currentState = ProcState::Deflate;
            }
            break;
        case ProcState::Deflate:
            pData.push_back(ymmHg);
            oData.push_back(yHP);
            /**
             * THIS IS WHERE THE MAGIC HAPPENS:
             * detect max/min in oscillations, possibly more (other algorithms)
             */
            break;
        case ProcState::Calculate:
            pData.push_back(ymmHg);
            oData.push_back(yHP);
            /**
             * Some more magic here:
             * reverse search of ratios in recorded data set since deflate
             */
            break;

    }


}

double Processing::getmmHgValue(double voltageValue) {
    return ((voltageValue - ambientVoltage) * mmHg_per_kPa * kPa_per_V * corrFactor);
}

bool Processing::checkAmbient() {
    bool bAmbientValid = false;

    if (pData.size() == AMBIENT_AV_TIME) {
        double av = 1.0 * std::accumulate(pData.begin(), pData.end(), 0.0) / pData.size();
        double max = *std::max_element(pData.begin(), pData.end());
        auto min = *std::min_element(pData.begin(), pData.end());

        PLOG_VERBOSE << "min: " << min << " max: " << max << " av: " << av << std::endl;
        if (std::abs(av - max) < AMBIENT_DEVIATION) {
            setAmbientVoltage(av);
            bAmbientValid = true;
        } else {
            pData.clear();
            oData.clear();
        }
    }

    return bAmbientValid;
}