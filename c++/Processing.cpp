
#include <iostream>
#include <unistd.h>
#include <iomanip>

#include <QtCore/QDateTime>

#include "Processing.h"
#include "ComediHandler.h"

#define DEFAULT_MINUTES 2
#define DEFAULT_DATA_SIZE 1024*60*DEFAULT_MINUTES


Processing::Processing() :
        nData(DEFAULT_DATA_SIZE),
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

void Processing::setAmbientVoltage(double voltage) {
    ambientVoltage = voltage;
}

void Processing::run() {

    std::cout << "run ..." << std::endl;

    bRunning = true;

    double i = 0.6;
    while (bRunning) {
        if (comedi->getBufferContents() > 0) {
            // TODO: move this into separate "algorithm class" that
            // can process a single sample and make it testable?
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
        /**
         * Configure the ambient pressure.
         */
        case ProcState::Config:
            //TODO: here I am using the "raw" voltage data, i am miss-using the data buffer,
            // this will not cause problems, as long as it is reset afterwards,
            // detect ambient pressure,
            if (checkAmbient()) {
                //enable Start button
                currentState = ProcState::Idle;
                notifyReady();
                pData.clear();

            } else {
                pData.push_back(yLP);
            }

            break;

            /**
             * Waiting for user to start the measurement.
             */
        case ProcState::Idle:

            if (bMeasuring) {
                currentState = ProcState::Inflate;
                //TODO: should be empty
                nData.clear();
                pData.clear();
                oData.clear();
            }
            //TODO: only enable start button if successfully detected
            break;
        case ProcState::Inflate:
            pData.push_back(ymmHg);
//            oData.push_back(yHP) //TODO: no need to record this here.
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
            if(checkMaxima(yHP)) {
                if (isPastDBP()) {
                    currentState = ProcState::Calculate;

                    notifySwitchScreen(Screen::emptyCuffScreen);
                }
            }
            break;
        case ProcState::Calculate:
            pData.push_back(ymmHg); //keep filling the values until zero is reached
            if( ymmHg < 1){
                stopMeasurement();
                notifySwitchScreen(Screen::resultScreen);
            }
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

/**
 * Checks a new sample if it is a local maxima and puts it in a vector to hold all local maxima,
 * together with a reference to the 'time' (sample number) it was recorded.
 * @param newOscData
 */
bool Processing::checkMaxima(double newOscData) {
    static int validPulseCnt = 0;
    bool isValid = false;

    if (oData.size() > 1200) { // ignores the first second or so

//        if (newOscData > 0.000) {
//TODO: calculating each time too much? use FIR filter?
        //double movingAV = std::accumulate(oData.end() - 1000, oData.end(), 0.0) / 1000;
        if (newOscData > 0.0005 ) {
            auto i = std::max_element((oData.end() - 3), oData.end());
            double testingSample = *(oData.end() - 1);

            if ((std::distance(i, oData.end()) == 1)) { // checks if the result is the last entry
                //std::cout << "rising.. " << std::endl;
            } else if (std::distance(i, oData.end()) == 3) {
                //std::cout << "falling.. " << std::endl; // checks if the result is the first entry
            } else { // otherwise, the result is the middle entry

                if( !maxtime.empty() ) {
                    //time since last max is <0.3s and the new sample is larger: replace the old value
                    if(((oData.size() - 1 - maxtime.back()) < 300) && maxAmp.back() < testingSample ){
                        maxAmp.back() = testingSample;
                        maxtime.back() = (oData.size() - 1);
                        std::cout << "replaced: ";
                    }
                } else
                {
                    maxtime.push_back(oData.size() - 1);
                    maxAmp.push_back(testingSample);
                }

                assert( !maxtime.empty() );
                assert( !maxAmp.empty() );
                double newPulse = 60000.0 / (double) (oData.size() - 1 - maxtime.back() );

                std::cout << "pulse: " << newPulse << " time: " << (maxtime.back() / 1000.0) << " osc value: "
                          << maxAmp.back() << std::endl;

                if (isPulseValid(newPulse) ) {
                    currentPulse = newPulse;
                    validPulseCnt++;
                    std::cout << "valid pulse: " << currentPulse << std::endl;
                    maxAmp.push_back( *(oData.end() - 1));
                    maxtime.push_back( oData.size() - 1 );

                } else {
                    std::cout << "invalid pulse after " << validPulseCnt << " valid ones" << std::endl;
                    PLOG_WARNING << "invalid pulse after " << validPulseCnt << " valid ones" << std::endl;
                    validPulseCnt = 0;
                    maxAmp.clear();
                    maxtime.clear();
                    maxtime.push_back(oData.size() - 1);
                    maxAmp.push_back(testingSample);
                }

                if(validPulseCnt > 5){
                    std::cout << "average pulse: " << currentPulse << std::endl;
                    isValid = true;
                }

            }

        }

    }
    return isValid;
}

void Processing::checkMinima(double newOscData) {

}

bool Processing::isPastDBP() {
    bool bIsPast = false;
    if(maxAmp.size() > 10){
        auto iter = std::max_element(maxAmp.begin(), maxAmp.end());
        double cutoff = (*iter) * (ratio_DBP - 0.2);
        if(maxAmp.back() < cutoff) {
            bIsPast = true;
            std::cout << "amp ("<<maxAmp.back()<<") below cutoff: (" << cutoff <<")" << std::endl;
        }
    }
    return bIsPast;
}

int Processing::maxValidPulse{100};
int Processing::minValidPulse{50};
double Processing::maxPulseChange{0.15};

bool Processing::isPulseValid(double pulse) {
    bool isValid = false;

    if (minValidPulse <= pulse && pulse <= maxValidPulse) {
//        if( currentPulse != 0.0) {
//            double diff = ((pulse - currentPulse) * 100) / currentPulse;
//            if (diff < maxPulseChange) {
//                isValid = true;
//            }
//        }
//        else {
        isValid = true;
//        }
    } else {
        // If the detected pulse is out of bounds, reset the current pulse
        //currentPulse = 0.0;
    }

    return isValid;
}
