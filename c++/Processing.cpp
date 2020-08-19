/**
 * @file        Processing.cpp
 * @brief       The implementation of the Processing class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 */
#include <iostream>
#include <unistd.h>
#include <cmath>
#include <QtCore/QDateTime>

#include "Processing.h"


 /**
  * The constructor of the Processing thread.
  *
  * Initialises internal objects and prepares the thread for running.
  * @param fcLP Cutoff frequency for the low-pass filter. Changing the default is not recommended.
  * @param fcHP Cutoff frequency for the high-pass filter. Changing the default might have severe concequences.
  */
Processing::Processing(double fcLP, double fcHP) :
        rawData(DEFAULT_DATA_SIZE),
        bRunning(false),
        bMeasuring(false) {

    PLOG_VERBOSE << "Processing started";

    currentState = ProcState::Config;
    comedi = new ComediHandler();

    sampling_rate = comedi->getSamplingRate();

    /**
     * LP filter, default value is 10 Hz, which also takes care of 50 Hz noise.
     */
    if (fcLP > 20.0 || fcLP < 5.0) {
        PLOG_WARNING << "Processing running with low pass filter of: " << fcLP;
    }

    iirLP = new Iir::Butterworth::LowPass<IIRORDER>;
    assert(iirLP != NULL);
    iirLP->setup(sampling_rate, fcLP);

    /**
     * HP filter, default value is 0.5 Hz.
     */
    if (fcHP != 0.5) {
        PLOG_WARNING << "Processing running with high pass filter of: " << fcHP;
    }
    iirHP = new Iir::Butterworth::HighPass<IIRORDER>;
    assert(iirHP != NULL);
    iirHP->setup(sampling_rate, fcHP);

    obpDetect = new OBPDetection(sampling_rate);
    record = new Datarecord(sampling_rate);

    /**
     * Initialise and reset all values.
     */
    rawData.clear();
    resetConfigValues();

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

/**
 * Set the ratio for SBP calculation.
 *
 * Only possible before the thread is running.
 * @param val The new SBP ratio.
 */
void Processing::setRatioSBP(double val) {
    if (!bRunning) {
        obpDetect->setRatioSBP(val);
    }
}

/**
 * Check the set value for the SBP ratio.
 * @return The SBP ratio stored in the obpDetect instance.
 */
double Processing::getRatioSBP() {
    return obpDetect->getRatioSBP();
}

/**
 * Set the ratio for DBP calculation.
 *
 * Only possible before the thread is running.
 * @param val The new DBP ratio.
 */
void Processing::setRatioDBP(double val) {
    if (!bRunning) {
        obpDetect->setRatioDBP(val);
    }
}

/**
 * Check the set value for the DBP ratio.
 * @return The DBP ratio stored in the obpDetect instance.
 */
double Processing::getRatioDBP() {
    return obpDetect->getRatioDBP();
}

/**
 * Set the minimally necessary number of peaks for a successful BP detection.
 *
 * Only possible before the thread is running.
 * @param val The new minimal number of peaks value.
 */
void Processing::setMinNbrPeaks(int val) {
    if (!bRunning) {
        obpDetect->setMinNbrPeaks(val);
    }
}

/**
 * Check the set value for the minimal number of peaks.
 * @return The minimal number of peaks stored in the obpDetect instance.
 */
int Processing::getMinNbrPeaks() {
    return obpDetect->getMinNbrPeaks();
}

/**
 * Set the pump-up value used to switch from Inflate to Defalte state.
 *
 * Only possible before the thread is running.
 */
void Processing::setPumpUpValue(int val) {
    if (!bRunning && val < MAX_PUMPUP) {
        mmHgInflate = (double) val;
    }
}

/**
 * Gets the currently set pump-up value used to determine when the cuff is sufficiently inflated.
 *
 * This value is used to transition from Inflate to Deflate states.
 * @return mmHgInflate The currently used pump-up value.
 */
int Processing::getPumpUpValue() {
    return mmHgInflate;
}

/**
 * Gets the sampling rate of the data acquisition.
 *
 * @return sampling_rate The sampling rate of the data acquisition.
 */
double Processing::getSamplingRate() {
    return sampling_rate;
}

/**
 * Resets the configuration values to their default.
 *
 * This function should be called once at initialisation of the object and
 * if the default values are to be restored. Non-default values should be set after initialisation.
 *
 * The obpDetect object has to be initialised beforehand.
 */
void Processing::resetConfigValues() {
    bMeasuring = false;
    mmHgInflate = 180.0;
    corrFactor = 2.6;

    obpDetect->resetConfigValues();
}


/**
 * The main running function of the thread.
 */
void Processing::run() {
    bRunning = true;

    while (bRunning) {

        /**
         * Read comedi buffer and process the sample.
         */
        if (comedi->getBufferContents() > 0) {
            processSample(comedi->getVoltageSample());
        } else {
            /**
             * If there was no data in the buffer, sleep for 1ms.
             */
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
    bMeasuring = true;
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
    QString dateTimeString = dateTime.toString("yyyy_MM_dd_hh_mm_ss");
    dateTimeString.append("_data.dat");
    return dateTimeString;
}

/**
 * Processing a single new sample in a state machine.
 * @param newSample
 */
void Processing::processSample(double newSample) {

    /**
     * Every sample is filtered and sent to the Observers
     * after configuration is done.
     * The raw data is stored to save to a file after a successful measurement.
     */
    double ymmHg = 0.0;
    double yLP = 0.0;
    double yHP = 0.0;
    if (currentState != ProcState::Config) {
        ymmHg = getmmHgValue(newSample);
        yLP = iirLP->filter(ymmHg);
        yHP = iirHP->filter(yLP);
        notifyNewData(yLP, yHP);
    }
    if (rawData.size() > DEFAULT_DATA_SIZE) {
        PLOG_WARNING << "Recording too long to continue algorithm. Cancelled";
        // Setting bMeasuring false will ensure return to Idle state.
        bMeasuring = false;
    }
    switch (currentState) {
        case ProcState::Config:

            if (checkAmbient()) {
                currentState = ProcState::Idle;
                // Send ready signal to observers
                notifyReady();
                rawData.clear();

            } else {
                rawData.push_back(newSample);
            }

            break;

        case ProcState::Idle:

            if (bMeasuring) {
                // Reset parameters:
                rawData.clear();
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
                rawData.push_back(ymmHg);

                // Check if pressure in cuff is large enough, so it can be switched to the next state.
                if (ymmHg > mmHgInflate) {
                    obpDetect->reset();
                    notifySwitchScreen(Screen::deflateScreen);
                    currentState = ProcState::Deflate;
                }
            }

            break;
        case ProcState::Deflate:
            if (!bMeasuring) {
                currentState = ProcState::Idle;
                notifySwitchScreen(Screen::startScreen);
            } else {
                rawData.push_back(ymmHg);

                if (obpDetect->processSample(yLP, yHP)) {
                    if (obpDetect->getIsEnoughData()) {
                        notifyHeartRate(obpDetect->getAverageHeartRate());
                        notifySwitchScreen(Screen::emptyCuffScreen);
                        currentState = ProcState::Empty;
                    } else {
                        notifyHeartRate(obpDetect->getCurrentHeartRate());
                    }
                }
                if (ymmHg < 20) {
                    PLOG_WARNING << "Pressure too low to continue algorithm. Cancelled";
                    bMeasuring = false;
                }
            }
            break;
        case ProcState::Empty:
            if (!bMeasuring) {
                currentState = ProcState::Idle;
                notifySwitchScreen(Screen::startScreen);
            } else {
                rawData.push_back(ymmHg);
                if (ymmHg < 2) {
                    notifyResults(obpDetect->getMAP(), obpDetect->getSBP(), obpDetect->getDBP());
                    record->saveAll(Processing::getFilename(), rawData);
                    notifySwitchScreen(Screen::resultScreen);
                    currentState = ProcState::Results;
                }
            }
            break;
        case ProcState::Results:
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
    return ((voltageValue - ambientVoltage) * kPa_per_V * corrFactor) / kPa_per_mmHg;
}

/**
 * Checks the ambient pressure. This method needs to be called repeatedly at startup
 * until it returns true.
 *
 * @return
 */
bool Processing::checkAmbient() {
    bool bAmbientValid = false;

    /**
     * The raw data is observed for the configured amount of time.
     */
    if (rawData.size() == AMBIENT_AV_TIME) {
        double av = 1.0 * std::accumulate(rawData.begin(), rawData.end(), 0.0) / rawData.size();
        double max = *std::max_element(rawData.begin(), rawData.end());
        auto min = *std::min_element(rawData.begin(), rawData.end());

        /**
         * If the pressure is stable for the configured amount of time, it is assumed to be
         * the ambient pressure. The average value is stored.
         */
        PLOG_VERBOSE << "min: " << min << " max: " << max << " av: " << av;
        if (std::abs(av - max) < AMBIENT_DEVIATION) {
            ambientVoltage = av;
            bAmbientValid = true;
        } else {
            rawData.clear();
        }
    }

    return bAmbientValid;
}