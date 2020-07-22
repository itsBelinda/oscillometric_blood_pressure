#include <iostream>
#include <unistd.h>
#include <cmath>
#include <QtCore/QDateTime>

#include "Processing.h"
#include "ComediHandler.h"

#define DEFAULT_MINUTES 2
#define DEFAULT_DATA_SIZE 1024*60*DEFAULT_MINUTES

/**
 * The constructor of the Processing thread.
 *
 * Initialises internal objects and prepares the thread for running.
 */
Processing::Processing() :
        rawData(DEFAULT_DATA_SIZE), //TODO: cleanup, the only thing needed here really
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
/**
 * The destructor of the Processing thread.
 *
 * Stops the measurement if not already done and stops the thread.
 */
Processing::~Processing() {
    stopMeasurement();
    stopThread();
}

void Processing::setAmbientVoltage(double voltage) {
    ambientVoltage = voltage;
}

/**
 * The main function of the thread.
 */
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

/**
 * Stops the thread by stopping the data aquisition so the thread teminates and can be joined.
 */
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
                pData.clear();
                oData.clear();
                rawData.clear();
            }

            break;
        case ProcState::Inflate:
            //pData.push_back(ymmHg); //TODO: no need to record this here.
            rawData.push_back(getmmHgValue(newSample)); //record raw data to store later
//            oData.push_back(yHP) //TODO: no need to record this here.
            /**
             * Check if the pressure in the cuff is big enough yet.
             * If so, change to the next state.
             */
            if (ymmHg > mmHgInflate) {

                notifySwitchScreen(Screen::deflateScreen);
                //TODO: possibly add entry end exit functions for each state
                // function: switch state: returns new state
                // performs entry and exit operations (notifications)
                // would that work with the state class being a friendly to Processing?
                currentState = ProcState::Deflate;
            }
            break;
        case ProcState::Deflate:
            //TODO: should the start deflation time be saved? (in terms of raw data)
            // This could avoid the need to store pData at all because we could just
            // average over raw for a heart rate period
            rawData.push_back(getmmHgValue(newSample)); //record raw data to store later
            pData.push_back(ymmHg);
            oData.push_back(yHP);
            /**
             * THIS IS WHERE THE MAGIC HAPPENS:
             * detect max/min in oscillations, possibly more (other algorithms)
             */
            if (checkMaxima(yHP)) {
                findMinima();
                if (isPastDBP()) {
                    currentState = ProcState::Calculate;
                    notifyHeartRate(getAverage(heartRate));
                    //TODO: possibly in a separate thread only for the calculation? or too fast? ca 50ms
                    findOWME();
                    notifySwitchScreen(Screen::emptyCuffScreen);

                }
            }
            break;
        case ProcState::Calculate:

            /**
             * Some more magic here:
             * reverse search of ratios in recorded data set since deflate
             */
            pData.push_back(ymmHg); //keep filling the values until zero is reached //TODO: not needed really
            rawData.push_back(getmmHgValue(newSample)); //record raw data to store later
            if (ymmHg < 1) {
                findMAP();
                stopMeasurement();
                notifySwitchScreen(Screen::resultScreen);
                currentState = ProcState::Idle;
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

void Processing::findMinima() {

    //TODO: set error flag if minima is not found
    if (maxAmp.size() >= 2) {
        //TODO: easier?
        // get sub vector of oData from second last to last max value
        std::vector<double>::const_iterator firstMax = oData.begin() + *(maxtime.end() - 2);
        std::vector<double>::const_iterator lastMax = oData.begin() + maxtime.back();//*(maxtime.end()-1)
        std::vector<double> newVec(firstMax, lastMax);

        // find minimal value in between
        auto iter = std::min_element(newVec.begin(), newVec.end());
        auto iter2 = std::min_element(firstMax, lastMax);

        if (iter == iter2) {
            std::cout << "same "; //TODO: why is this not the same?
        }
        auto dist = std::distance(newVec.begin(), iter);

//        auto iter = std::min_element(&oData[*(maxtime.end()-2)], &oData[maxtime.back()]);
//
//        auto dist = std::distance(oData[*(maxtime.end()-2)], iter);
//        mintime.push_back(*(maxtime.end()-2) + dist );

        if (mintime.size() == (maxtime.size() - 1)) {
            minAmp.back() = *iter;
            mintime.back() = dist + *(maxtime.end() - 2);
//            std::cout << *iter << " replaced\n";
        } else {
            minAmp.push_back(*iter);
            mintime.push_back(dist + *(maxtime.end() - 2));
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
    if (maxAmp.size() > 10) {//&& pData.back() < 40.0) {
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
    const double testingSample = *(oData.end() - 2); // testing the second to last entry
    const auto testingTimeNbr = (oData.size() - 1); // NEW: in relation to oData for min-detect!
    //OLD: in relation to pData, because this is the time since the
    // beginning of the measurement and the time where the pressure
    // I am interested in is stored
    // TODO: sanity test: maxTime and maxAmp are either +1/+0 in size afterwards, or size = 1
    if (maxtime.empty()) {
        // Accept any value as a first value, only start testing after the second one
        maxtime.push_back(testingTimeNbr);
        maxAmp.push_back(testingSample);
    } else {
        assert(!maxtime.empty());
        assert(!maxAmp.empty());
        //time since last max is <0.3s and the new sample is larger: replace the old value
        if ((testingTimeNbr - maxtime.back()) < 300) {
            if (maxAmp.back() < testingSample) {
                maxAmp.back() = testingSample;
                maxtime.back() = testingTimeNbr;
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
            maxtime.push_back(testingTimeNbr);
        }

        if (maxtime.size() > 1) {

            double newPulse = newPulse = 60000.0 / (double) (maxtime.back() - (*(maxtime.end() - 2)));

            if (isPulseValid(newPulse)) {
                heartRate.push_back(newPulse);
                notifyHeartRate(newPulse);
                validPulseCnt++;
                isValid = true;
            } else {
                PLOG_INFO << "invalid pulse after " << validPulseCnt << " valid ones" << std::endl;
                validPulseCnt = 0;
                maxAmp.clear();
                maxtime.clear();
                minAmp.clear();
                mintime.clear();
                maxtime.push_back(testingTimeNbr);
                maxAmp.push_back(testingSample);
                heartRate.clear();
                isValid = false;
            }

            if (validPulseCnt > 5) { //TODO: valid counting not needed anymore
                isValid = true;
            }
        }
    }
    return isValid;
}


void Processing::findOWME() {
//    auto timeMin1 = mintime.cbegin();
    auto timeMax1 = maxtime.cbegin();
    auto ampMin1 = minAmp.cbegin();
    auto ampMax1 = maxAmp.cbegin();
    // forward iteration use const iterator, because they should not be touched
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << "calculating OMVE: mintime size: " << mintime.size() << std::endl;
    // forward iteration
    for (auto timeMin1 = mintime.cbegin(); timeMin1 != (mintime.cend() - 1); ++timeMin1) {
        auto timeMin2 = std::next(timeMin1, 1); //TODO: not needed for last one, might be invalid
        auto timeMax2 = std::next(timeMax1, 1);
        auto ampMin2 = std::next(ampMin1, 1);
        auto ampMax2 = std::next(ampMax1, 1);

//        assert(*timeMin1 < *timeMax1); // something went wrong
//        assert(*timeMin2 < *timeMax2); // something went wrong
        //TODO: increases processing time A LOT, remove!!!
//        PLOG_VERBOSE << " tmax1: " << *timeMax1 << " tmin1: " << *timeMin1;
//        PLOG_VERBOSE << " tmax2: " << *timeMax2 << " tmin2: " << *timeMin2;
//        PLOG_VERBOSE << " ampMax1: " << *ampMax1 << " ampMin1: " << *ampMin1;
//        PLOG_VERBOSE << " ampMax2: " << *ampMax2 << " ampMin2: " << *ampMin2;

        auto lerpMax = std::lerp(*ampMax1, *ampMax2, getRatio(*timeMax1, *timeMax2, *timeMin1));
        auto lerpMin = std::lerp(*ampMin1, *ampMin2, getRatio(*timeMin1, *timeMin2, *timeMax2));
        // TODO: combine all the time & value stuff in one variable (less error prone), but how to use
        // the max_element stuff ect?
        omveData.push_back(lerpMax - *ampMin1);
        omveTimes.push_back(*timeMin1);
        omveData.push_back(*ampMax2 - lerpMin);
        omveTimes.push_back(*timeMax2);
        // Inclreasing all the itterators:c
        timeMax1++;
        ampMin1++;
        ampMax1++;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::cout << "done " << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n";
}

void Processing::findMAP() {

    auto maxOMVE = std::max_element(omveData.begin(), omveData.end());
    auto time = omveTimes[std::distance(omveData.begin(), maxOMVE)];

    std::cout << "maxOMVE: " << *maxOMVE << std::endl;

    double valMAP = pData[time]; //MAP: get it from average heart rate
//    PLOG_DEBUG << "MAP pressure p: " << pData[time] << " time: " << time
//               << "\n pData.size(): " << pData.size() << " oData.size(): " << oData.size();
//      TODO: done for testing. Keeping as comment for now, might be useful for report
//      TODO: size of oData could be used as timeout condition. (normal implementation < 1min == 60000 )
    Datarecord recordOSC(1.0);
    recordOSC.saveAll("osc.dat", oData);
    Datarecord recordP(1.0);
    recordP.saveAll("p.dat", pData);
    Datarecord recordOMVE(1.0);
    recordOMVE.saveAll("omve.dat", omveData);

    std::for_each(omveTimes.begin(), omveTimes.end(),
                  [](int time) {
                      std::cout << "t: " << time << std::endl;
                  });
    // FIND SBP: //TODO: validate
    double maxVAL = *maxOMVE;
    double sbpSearch = ratio_SBP * maxVAL;
    double ubSBP;
    double lbSBP;
    int lbTime;
    int ubTime;

    std::cout << "SBP search: " << sbpSearch << std::endl;
    for (auto omveSBP = omveData.begin(); omveSBP != maxOMVE; ++omveSBP) {
        if (*omveSBP > sbpSearch) {
            ubSBP = *omveSBP;
            ubTime = omveTimes[std::distance(omveData.begin(), omveSBP)];
            omveSBP--;
            lbSBP = *omveSBP;
            lbTime = omveTimes[std::distance(omveData.begin(), omveSBP)];
            std::cout << "ubSBP: " << ubSBP << std::endl;
            std::cout << "ubTime: " << ubTime << std::endl;
            std::cout << "lbSBP: " << lbSBP << std::endl;
            std::cout << "lbTime: " << lbTime << std::endl;
            break; //TODO: do while without break?
        }
    }

    int lerpSBPtime = (int) std::lerp(lbTime, ubTime, getRatio(lbSBP, ubSBP, sbpSearch));
    double valSBP = pData[lerpSBPtime];
    std::cout << "lerpSBPtime: " << lerpSBPtime << std::endl;

    // FIND DBP: // TODO: validate!

    double dbpSearch = ratio_DBP * maxVAL;
    std::cout << "DBP search: " << dbpSearch << std::endl;
    for (auto omveDBP = maxOMVE; omveDBP != omveData.end(); ++omveDBP) {
        if (*omveDBP < dbpSearch) {
            lbSBP = *omveDBP;
            lbTime = omveTimes[std::distance(omveData.begin(), omveDBP)];
            omveDBP--;
            ubSBP = *omveDBP;
            ubTime = omveTimes[std::distance(omveData.begin(), omveDBP)];
            std::cout << "ubDBP: " << ubSBP << std::endl;
            std::cout << "ubTime: " << ubTime << std::endl;
            std::cout << "lbDBP: " << lbSBP << std::endl;
            std::cout << "lbTime: " << lbTime << std::endl;
            break;

        }
    }

    // The curve is falling, "upper bound" time is lower than "lower bound" time.
    // The ratio is calculated the same way as before, but to account for the lower
    // value relating to the higher time the ratio is inverted.
    // The interpolation is done from the "upper bound" time (earlier in time) to the
    // "lower bound" time (later in time).
    int lerpDBPtime = (int) std::lerp(ubTime, lbTime, 1.0 - getRatio(lbSBP, ubSBP, dbpSearch));
    std::cout << "lerpDBPtime: " << lerpDBPtime << std::endl;
    double valDBP = pData[lerpDBPtime];

    notifyResults(valMAP, valSBP, valDBP);
}

/**
 * Helper function that gets the ratio from a value that is in between two others
 * to then calculate the interpolated value between the two with the std::lerp (C++20)
 * function.
 * @param lowerBound    the upper bound value
 * @param upperBound    the lower bound value
 * @param value         the middle value between the lower and upper bound
 * @return
 */
double Processing::getRatio(double lowerBound, double upperBound, double value) {
//    assert( value > lowerBound && value < upperBound);
    return ((value - lowerBound) / (upperBound - lowerBound));
}


double Processing::getAverage(std::vector<double> avVector) {
    return std::accumulate(avVector.begin(), avVector.end(), 0.0) / avVector.size();
}

/**
 * Getter function for the current state of the state machine.
 *
 * @return The current state of the state machine.
 */
Processing::ProcState Processing::getCurrentState() const { return currentState; }

