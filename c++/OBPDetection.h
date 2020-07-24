#ifndef OBP_OBPDETECTION_H
#define OBP_OBPDETECTION_H

#include <vector>
#include "common.h"

//! The OBPDetection class handles the implementation of the algorithm to get
//! blood pressure and heart rate from the oscillation data.
/*!
 * After initialisation, where some configurations can be made, the object of
 * this class processes one sample pair at a time. Each sample pair consists
 * of a value for pressure and a value for the oscillation.
 *
 * The object will try to find the maxima and minima in the oscillations and
 * calculate the envelope from this (OMVE). Ultimately, the mean arterial
 * pressure (MAP) is expected where the OMVE is maximal. The algorithm then
 * looks up the corresponding value of pressure at that sample ("time") and
 * stores that value as the MAP.
 *
 * The systolic blood pressure (SBP) is defined as the pressure in time before
 * MAP where the OMVE is a fraction of @ratio_SBP of the value at the MAP.
 *
 * Similarly, the diastolic blood pressure is defined as the pressure in time
 * after the MAP where the OMVE is a fraction of @ratio_DBP of the value at
 * the MAP.
 */
class OBPDetection {
//TODO: add configurable parameters in constructor
//TODO: change osc to use mmHg values
//TODO: MAP, DBP, SBP as average over heart rate
public:
    OBPDetection();
    ~OBPDetection();

    double getCurrentHeartRate();
    double getAverageHeartRate();
    double getMAP();
    double getSBP();
    double getDBP();
    bool getIsEnoughData();

    bool processSample(double pressure, double oscillation);
    void reset();
private:
    // vectors to store values for calculations
    std::vector<double> pData;
    std::vector<double> oData;
    std::vector<double> omveData;
    std::vector<int> omveTimes;
    std::vector<double> maxAmp;
    std::vector<int> maxtime;
    std::vector<double> minAmp;
    std::vector<int> mintime;
    std::vector<double> hrData;

    // variables to store results
    double resMAP{};
    double resSBP{};
    double resDBP{};
    bool enoughData;

    // variables to store configurations
    static double maxValidPulse;
    static double minValidPulse;
    double ratio_SBP = 0.57;    // from literature, might be changed in settings later
    double ratio_DBP = 0.75;    // from literature, might be changed in settings later

    // functions
    bool checkMaxima();
    bool isValidMaxima();
    static bool isHeartRateValid(double heartRate);
    void findMinima();
    bool isEnoughData();
    void findOWME();
    void findMAP();
    static double getRatio(double lowerBound, double upperBound, double value);
    static double getAverage(std::vector<double> avVector);


};


#endif //OBP_OBPDETECTION_H
