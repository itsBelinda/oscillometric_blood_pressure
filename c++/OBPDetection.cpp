#include <iostream>
#include <cmath>
#include <numeric>
#include "OBPDetection.h"


/**
 * Constructor of the OBPDetection class.
 *
 */
OBPDetection::OBPDetection() :
        pData(DEFAULT_DATA_SIZE),
        oData(DEFAULT_DATA_SIZE),
        enoughData(false) {
    reset();
}

/**
 * Destructor of the OBPDetection class.
 */
OBPDetection::~OBPDetection() {

}

/**
 * Gets the last valid heart rate value if there were any.
 * @return The last entry of the heart rate vector.
 */
double OBPDetection::getCurrentHeartRate() {
    double cHR = 0.0;
    if (!hrData.empty()) {
        cHR = hrData.back();
    }
    return cHR;
}

/**
 * Calculates the average over all saved heart rate entries.
 * @return The average heart rate in the current calculations.
 */
double OBPDetection::getAverageHeartRate() {
    return getAverage(hrData);
}

/**
 * Returns the calculated value for the mean arterial pressure (MAP).
 * @return The calculated MAP.
 */
double OBPDetection::getMAP() const {
    return resMAP;
}

/**
 * Returns the calculated value for the systolic blood pressure (SBP).
 * @return The calculated SBP.
 */
double OBPDetection::getSBP() const {
    return resSBP;
}

/**
 * Returns the calculated value for the diastolic blood pressure (DBP).
 * @return The calculated DBP.
 */
double OBPDetection::getDBP() const {
    return resDBP;
}

bool OBPDetection::getIsEnoughData() const {
    return enoughData;
}

/**
 * Processes one data sample pair of pressure and oscillation at a time.
 * Returns true if the process has finished and results might be available.
 * It is possible that the calculations were not successful and true is
 * returned to indicate unsuccessful completion.
 * Check the results getter functions
 *
 * @param pressure The pressure in mmHg for this sample.
 * @param oscillation The oscillation in arbitrary units for this sample.
 * @return True if the calculations are finished.
 */
bool OBPDetection::processSample(double pressure, double oscillation) {
    bool newMax = false;
    pData.push_back(pressure);
    oData.push_back(oscillation);
    if (checkMaxima()) {
        findMinima();
        if (isEnoughData()) {
            //TODO: put calculation in thread so the rest of the application can continue.
            findOWME();
            findMAP();
            enoughData = true;
        }
        newMax = true;
    }
    return newMax;
}


/**
 * Checks the latest samples in oData if there is a local maxima and puts it in a vector to hold all
 * local maxima, together with a reference to the 'time' (sample number) it was recorded.
 * @return true if a local maxima was found.
 */
bool OBPDetection::checkMaxima() {
    bool isValid = false;
    //TODO: prominence as configurable value
    if (oData.size() > minDataSize && *(oData.end() - 2) > prominence) { // ignores the first second or so
        //TODO: idea for more robust algorithm:
        // get a bigger sample size (e.g. 100) and check for minimal "prominence"
        // then take the largest of the values and its position
        auto i = std::max_element((oData.end() - 3), oData.end());

        // is result the middle entry?
        if (std::distance(i, oData.end()) == 2) {
            isValid = isValidMaxima();
        }


    }

    return isValid;
}

/**
 * Checks if the found maximum is acutally valid. If it is found as a valid maxima, the
 * current time and amplitude is saved and the heart rate is calculated from the last valid maximum.
 * @return True if a valid maxima and a new current heart rate was calculated.
 */
bool OBPDetection::isValidMaxima() {
    static int validPulseCnt = 0; // Only for logging purposes.
    bool isValid = false;

    assert(oData.size() >= 2);

    const double testValue = *(oData.end() - 2); // testing the second to last entry
    const auto testSmplNbr = (oData.size() - 1); // NEW: in relation to oData for min-detect!

    if (maxtime.empty()) {
        // Accept any value as a first value, only start testing after the second one
        maxtime.push_back(testSmplNbr);
        maxAmp.push_back(testValue);
        // do not set isValid true, because this would start checking for a minimum between two maxima
    } else {
        assert(!maxtime.empty());
        assert(!maxAmp.empty());

        //time since last max is <minPeakTime (ms) and the new sample is larger: replace the old value
        if ((testSmplNbr - maxtime.back()) < minPeakTime) {
            if (maxAmp.back() < testValue) {
                maxAmp.back() = testValue;
                maxtime.back() = testSmplNbr;
//                std::cout << maxAmp.back() << " replaced\n";
            } else {
                // Skip this maxima, it is too quick after the last one, but smaller.
                if (validPulseCnt > 0) {
                    // No new maxima detected, finish function with old values.
                    // validPulseCnt will be increased again by this.
                    validPulseCnt--;
                }
            }
        } else {
            maxAmp.push_back(testValue);
            maxtime.push_back(testSmplNbr);
        }

        if (maxtime.size() > 1) {
            double newPulse = newPulse = (60.0 * samplingRate) / (double) (maxtime.back() - (*(maxtime.end() - 2)));

            if (isHeartRateValid(newPulse)) {
                hrData.push_back(newPulse);
                validPulseCnt++;
                isValid = true;
            } else {
                PLOG_INFO << "Invalid pulse after " << validPulseCnt << " valid ones" << std::endl;
                validPulseCnt = 0;
                maxAmp.clear();
                maxtime.clear();
                minAmp.clear();
                mintime.clear();
                maxtime.push_back(testSmplNbr);
                maxAmp.push_back(testValue);
                hrData.clear();
                isValid = false;
            }
        }
    }
    return isValid;
}


/**
 * Checks if the heart rate is in between the defined values of maxValidHR and minValidHR.
 * @param heartRate The heart rate to be checked.
 * @return True if the heart rate is within the defined bounds.
 */
bool OBPDetection::isHeartRateValid(double heartRate) {
    return (minValidHR <= heartRate && heartRate <= maxValidHR);
}


/**
 * Finds the minimal value in the oscillation between two maxima.
 */
void OBPDetection::findMinima() {

    if (maxAmp.size() >= 2) {
        //TODO: easier?
        // get sub vector of oData from second last to last max value
        auto firstMax = oData.begin() + *(maxtime.end() - 2);
        auto lastMax = oData.begin() + maxtime.back();
        std::vector<double> newVec(firstMax, lastMax);

        // find minimal value in between
        auto iter = std::min_element(newVec.begin(), newVec.end());
        auto iter2 = std::min_element(firstMax, lastMax);

        if (iter == iter2) {
            std::cout << "same "; //TODO: why is this not the same?
        }
        auto dist = std::distance(newVec.begin(), iter);

        if (mintime.size() == (maxtime.size() - 1)) {
            minAmp.back() = *iter;
            mintime.back() = dist + *(maxtime.end() - 2);
        } else {
            minAmp.push_back(*iter);
            mintime.push_back(dist + *(maxtime.end() - 2));
        }
    }

}

/**
 * Checks, if enough data has been received to
 * @return
 */
bool OBPDetection::isEnoughData() {
    bool bIsEnough = false;
    if (maxAmp.size() > minNbrPeaks) {
        std::max(maxAmp.begin(), maxAmp.end());
        auto iter = std::max_element(maxAmp.begin(), maxAmp.end());
        double cutoff = (*iter) * (ratio_DBP - cutoffHyst);
        if (maxAmp.back() < cutoff) {
            bIsEnough = true;
        }
    }
    return bIsEnough;
}


/**
 * Calculates the Oscillometric Waveform Envelope (OMVE) from the saved min and max values (minAmp and mintime and
 * maxAmp and maxtime) in preparation to find the maximal oscillation and the ratios of it for the systolic and
 * diastolic blood pressure.
 *
 * The calculated values will be stored in omvweTimes and omveData.
 */
void OBPDetection::findOWME() {
    // forward iteration using const iterator, because they should not be touched
    auto timeMax1 = maxtime.cbegin();
    auto ampMin1 = minAmp.cbegin();
    auto ampMax1 = maxAmp.cbegin();

    // The min values are defined between two max values. Therefore, iterate trough them until the second to last value.
    for (auto timeMin1 = mintime.cbegin(); timeMin1 != (mintime.cend() - 1); ++timeMin1) {
        auto timeMin2 = std::next(timeMin1, 1);
        auto timeMax2 = std::next(timeMax1, 1);
        auto ampMin2 = std::next(ampMin1, 1);
        auto ampMax2 = std::next(ampMax1, 1);

        assert(*timeMin1 > *timeMax1);
        assert(*timeMin2 > *timeMax2);
        //TODO: increases processing time A LOT, remove!!!
//        PLOG_VERBOSE << " tmax1: " << *timeMax1 << " tmin1: " << *timeMin1;
//        PLOG_VERBOSE << " tmax2: " << *timeMax2 << " tmin2: " << *timeMin2;
//        PLOG_VERBOSE << " ampMax1: " << *ampMax1 << " ampMin1: " << *ampMin1;
//        PLOG_VERBOSE << " ampMax2: " << *ampMax2 << " ampMin2: " << *ampMin2;

        // Calculate a value interpolated between the two max (resp. min) values at the position (in time)
        // where another min (resp. max) value is to be able to calculate the envelope.
        auto lerpMax = std::lerp(*ampMax1, *ampMax2, getRatio(*timeMax1, *timeMax2, *timeMin1));
        auto lerpMin = std::lerp(*ampMin1, *ampMin2, getRatio(*timeMin1, *timeMin2, *timeMax2));

        // Calculate the envelope, save both time and values.
        omveData.push_back(lerpMax - *ampMin1);
        omveTimes.push_back(*timeMin1);
        omveData.push_back(*ampMax2 - lerpMin);
        omveTimes.push_back(*timeMax2);

        // Increase all the iterators not handled by the for loop:
        timeMax1++;
        ampMin1++;
        ampMax1++;
    }
//    auto finish = std::chrono::high_resolution_clock::now();
//    std::cout << "done " << std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count() << "ns\n";
}

/**
 * Find the Mean Arterial Pressure (MAP) as well as the systolic and diastolic blood pressures (SBP, DBP).
 *
 * The results will be saved in the result variables resMAP, resSBP and resDBP. They are saved as doubled, but this
 * does not represent their precision.
 */
void OBPDetection::findMAP() {

    auto maxOMVE = std::max_element(omveData.begin(), omveData.end());
    auto time = omveTimes[std::distance(omveData.begin(), maxOMVE)];

//    std::cout << "maxOMVE: " << *maxOMVE << std::endl;

    resMAP = pData[time]; //TODO MAP: get it from average heart rate
//    PLOG_DEBUG << "MAP pressure p: " << pData[time] << " time: " << time
//               << "\n pData.size(): " << pData.size() << " oData.size(): " << oData.size();
//      TODO: done for testing. Keeping as comment for now, might be useful for report
//      TODO: size of oData could be used as timeout condition. (normal implementation < 1min == 60000 )
//    Datarecord recordOSC(1.0);
//    recordOSC.saveAll("osc.dat", oData);
//    Datarecord recordP(1.0);
//    recordP.saveAll("p.dat", pData);
//    Datarecord recordOMVE(1.0);
//    recordOMVE.saveAll("omve.dat", omveData);

//    std::for_each(omveTimes.begin(), omveTimes.end(),
//                  [](int time) {
//                      std::cout << "t: " << time << std::endl;
//                  });

    double maxVAL = *maxOMVE;
    double sbpSearch = ratio_SBP * maxVAL;
    double ubSBP;
    double lbSBP;
    int lbTime;
    int ubTime;

    for (auto omveSBP = omveData.begin(); omveSBP != maxOMVE; ++omveSBP) {
        if (*omveSBP > sbpSearch) {
            ubSBP = *omveSBP;
            ubTime = omveTimes[std::distance(omveData.begin(), omveSBP)];
            omveSBP--;
            lbSBP = *omveSBP;
            lbTime = omveTimes[std::distance(omveData.begin(), omveSBP)];
            break;
        }
    }

    int lerpSBPtime = (int) std::lerp(lbTime, ubTime, getRatio(lbSBP, ubSBP, sbpSearch));
    resSBP = pData[lerpSBPtime];

    double dbpSearch = ratio_DBP * maxVAL;
    for (auto omveDBP = maxOMVE; omveDBP != omveData.end(); ++omveDBP) {
        if (*omveDBP < dbpSearch) {
            lbSBP = *omveDBP;
            lbTime = omveTimes[std::distance(omveData.begin(), omveDBP)];
            omveDBP--;
            ubSBP = *omveDBP;
            ubTime = omveTimes[std::distance(omveData.begin(), omveDBP)];
            break;
        }
    }

    // The curve is falling, "upper bound" time is lower than "lower bound" time.
    // The ratio is calculated the same way as before, but to account for the lower
    // value relating to the higher time the ratio is inverted.
    // The interpolation is done from the "upper bound" time (earlier in time) to the
    // "lower bound" time (later in time).
    int lerpDBPtime = (int) std::lerp(ubTime, lbTime, 1.0 - getRatio(lbSBP, ubSBP, dbpSearch));
    resDBP = pData[lerpDBPtime];

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
double OBPDetection::getRatio(double lowerBound, double upperBound, double value) {
    return ((value - lowerBound) / (upperBound - lowerBound));
}

/**
 * Calculates the average value in a given vector and returns it.
 * @param avVector A vector of doubles to take the average from.
 * @return The average of all the values in the vector.
 */
double OBPDetection::getAverage(std::vector<double> avVector) {
    double av = 0.0;
    if (!avVector.empty()) {
        av = std::accumulate(avVector.begin(), avVector.end(), 0.0) / avVector.size();
    }
    return av;
}

/**
 * Resets all variables to start a new measurement.
 */
void OBPDetection::reset() {
    pData.clear();
    oData.clear();
    omveData.clear();
    omveTimes.clear();
    maxAmp.clear();
    maxtime.clear();
    minAmp.clear();
    mintime.clear();
    hrData.clear();

    resMAP = 0.0;
    resSBP = 0.0;
    resDBP = 0.0;
    enoughData = false;
}
