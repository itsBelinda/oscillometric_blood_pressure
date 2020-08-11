#ifndef OBP_OBPDETECTION_H
#define OBP_OBPDETECTION_H

#include <vector>
#include <atomic>
#include "common.h"

// Class dependant configuration values:
#define MIN_RATIO 0.01
#define MAX_RATIO 0.99
#define MIN_PEAKS 5


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
    OBPDetection(double sampling_rate);
    ~OBPDetection();

    // Configuration getter and setters:
    double getRatioSBP();
    void setRatioSBP(double val);
    double getRatioDBP();
    void setRatioDBP(double val);
    int getMinNbrPeaks();
    void setMinNbrPeaks(int val);
    void resetConfigValues();

    // Process values sample by sample:
    bool processSample(double pressure, double oscillation);

    // Getter for results:
    double getCurrentHeartRate();
    double getAverageHeartRate();
    [[nodiscard]] double getMAP() const;
    [[nodiscard]] double getSBP() const;
    [[nodiscard]] double getDBP() const;
    [[nodiscard]] bool getIsEnoughData() const;
    void reset();

private:
    // vectors to store values for calculations
    std::vector<double> pData;
    std::vector<double> oData;
    std::vector<double> omweData;
    std::vector<int> omweTimes;
    std::vector<double> maxAmp;
    std::vector<int> maxtime;
    std::vector<double> minAmp;
    std::vector<int> mintime;
    std::vector<double> hrData;
//    double lastMinima;
X
    // variables to store results
    double resMAP{};
    double resSBP{};
    double resDBP{};
    bool enoughData;

    // variables to store configurations
    std::atomic<double> maxValidHR = 100.0;      //! The maximal valid heart rate
    std::atomic<double> minValidHR = 50.0;       //! The minimal valid heart rate
    std::atomic<double> ratio_SBP;               //! from literature, might be changed in settings later
    std::atomic<double> ratio_DBP;               //! from literature, might be changed in settings later
    std::atomic<double> prominence = 0.25;     //! The min. prominence of one oscillation to count as a maximum
    std::atomic<int> minDataSize = 1200;         //! The min. size of oscillation data. Before this it will not be
    //! analysed.
    std::atomic<int> minPeakTime = 300;          //! The minimal time two peaks should be apart. If there are
    //! multiples, only the larger one will be considered.
    std::atomic<double> samplingRate;            //! The sampling rate needed to calculate the heart rate from samples.
    std::atomic<int> minNbrPeaks = 10;           //! The number of oscillation peaks required to be able to perform
    //! the algorithm.
    std::atomic<double> cutoffHyst = 0.3;        //! The hysteresis below ratio_DBP the oscillations have to be in
    //! order to be able to end the measurement. This is not from the total OMVE, but from the maximal amplitude.
    //! (OMVE calculated afterwards).

    //private functions:
    bool checkMaxima();
    bool isValidMaxima();
    bool isHeartRateValid(double heartRate);
    void findMinima();
    bool isEnoughData();
    void findOWME();
    void findMAP();
    double getPressureAt(int time);

    // Static functions:
    static double getRatio(double lowerBound, double upperBound, double value);
    static double getAverage(std::vector<double> avVector);
};


#endif //OBP_OBPDETECTION_H
