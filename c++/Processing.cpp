
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
        // TODO: make dependant on user selection
        record->saveAll(Processing::getFilename(), rawData);
    }
}

/**
 * Stops the measurement, but without saving the data.
 */
void Processing::resetMeasurement() {
    bMeasuring = false;
}

QString Processing::getFilename() {
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = dateTime.toString("yyyy-MM-dd_hh-mm-ss");
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
                rawData.clear();
            }
            //TODO: only enable start button if successfully detected
            break;
        case ProcState::Inflate:
            pData.push_back(ymmHg);
            rawData.push_back(getmmHgValue(newSample)); //record raw data to store later
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
            rawData.push_back(getmmHgValue(newSample)); //record raw data to store later
            oData.push_back(yHP);
            /**
             * THIS IS WHERE THE MAGIC HAPPENS:
             * detect max/min in oscillations, possibly more (other algorithms)
             */
            if (checkMaxima(yHP)) {
                checkMinima();
                if (isPastDBP()) {
                    currentState = ProcState::Calculate;
                    notifySwitchScreen(Screen::emptyCuffScreen);

                }
            }
            break;
        case ProcState::Calculate:

            pData.push_back(ymmHg); //keep filling the values until zero is reached
            rawData.push_back(getmmHgValue(newSample)); //record raw data to store later
            if (ymmHg < 1) {
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
            rawData.clear();
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

    if (oData.size() > 1200 && newOscData > 0.0005) { // ignores the first second or so

        auto i = std::max_element((oData.end() - 3), oData.end());

        // is result the middle entry?
        if (std::distance(i, oData.end()) == 2) {
            isValid = isValidMaxima();
        }


    }

    return isValid;
}

void Processing::checkMinima() {
    if(maxAmp.size() >= 2 ){
        //TODO: easier?
        // get sub vector of oData from second last to last max value
        std::vector<double>::const_iterator firstMax = oData.begin() + *(maxtime.end()-2);
        std::vector<double>::const_iterator lastMax = oData.begin() + maxtime.back();//*(maxtime.end()-1)
        std::vector<double> newVec(firstMax, lastMax);

        // find minimal value in between
        auto iter = std::min_element(newVec.begin(),newVec.end());

        auto dist = std::distance(oData.begin(), iter);
//        auto iter = std::min_element(&oData[*(maxtime.end()-2)], &oData[maxtime.back()]);
//
//        auto dist = std::distance(oData[*(maxtime.end()-2)], iter);
//        mintime.push_back(*(maxtime.end()-2) + dist );

        if( mintime.size() == maxtime.size()){
            minAmp.back() =  *iter ;
            mintime.back() =  dist ;
//            std::cout << *iter << " replaced\n";
        }else{
            minAmp.push_back( *iter );
            mintime.push_back( dist );
//            std::cout << *iter << " appended\n";
        }
        //check:
//        if( oData[mintime.back()] == *iter ){
//            std::cout << *iter << " the same\n";
//        }
//        else {
//            std::cout << *iter << " iter " << oData[mintime.back()] << "\n";
//
//        }
    }

}

bool Processing::isPastDBP() {
    bool bIsPast = false;
    if (maxAmp.size() > 10) {
        auto iter = std::max_element(maxAmp.begin(), maxAmp.end());
        double cutoff = (*iter) * (ratio_DBP - 0.2);
        if (maxAmp.back() < cutoff) {
            bIsPast = true;
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
        isValid = true;
    }

    return isValid;
}


bool Processing::isValidMaxima() {
    static int validPulseCnt = 0;
    bool isValid = false;
    const double testingSample = *(oData.end() - 1);
    const auto testingTime = (oData.size() - 1);
    // TODO: sanity test: maxTime and maxAmp are either +1/+0 in size afterwards, or size = 1
    if (maxtime.empty()) {
        // Accept any value as a first value, only start testing after the second one
        maxtime.push_back(oData.size() - 1);
        maxAmp.push_back(testingSample);
    } else {
        assert(!maxtime.empty());
        assert(!maxAmp.empty());
        //time since last max is <0.3s and the new sample is larger: replace the old value
        if ((testingTime - maxtime.back()) < 300) {
            if (maxAmp.back() < testingSample) {
                maxAmp.back() = testingSample;
                maxtime.back() = testingTime;
//                std::cout << maxAmp.back() << " replaced\n";
            } else {
                // skip this maxima, it's too quick after the last one, but smaller
                // no new maxima detected, finish function with old values.
                if (validPulseCnt > 0) {
                    validPulseCnt--;
                }
            }
        } else {
            maxAmp.push_back(testingSample);
            maxtime.push_back(testingTime);
        }

        if (maxtime.size() > 1) {

            double newPulse = newPulse = 60000.0 / (double) (maxtime.back() - (*(maxtime.end() - 2)));

            if (isPulseValid(newPulse)) {
                currentPulse = newPulse;
                validPulseCnt++;
                isValid = true;
            } else {
                PLOG_INFO << "invalid pulse after " << validPulseCnt << " valid ones" << std::endl;
                validPulseCnt = 0;
                maxAmp.clear();
                maxtime.clear();
                maxtime.push_back(testingTime);
                maxAmp.push_back(testingSample);
                isValid = false;
            }

            if (validPulseCnt > 5) { //TODO: valid counting not needed anymore
                isValid = true;
            }
        }
    }
    return isValid;
}

