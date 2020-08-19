/**
 * @file        OBPDetection.h
 * @brief       The header file of the OBPDetection class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * Defines the OBPDetection class and contains the general class description.
 */
#ifndef OBP_OBPDETECTION_H
#define OBP_OBPDETECTION_H

#include <vector>
#include <atomic>
#include "common.h"

/**
 * Class dependant configuration values:
 * They are absolute maximal vales and should never be set.
 */
#define MIN_RATIO 0.01 //!< A ratio minimum should be larger than 0.
#define MAX_RATIO 0.99 //!< A ratio maximum should be smaller than 1.
#define MIN_PEAKS 5    //!< With less than 5 peaks, the detection is impossible.


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
    std::vector<double> pData;    //!< Stores the pressure data.
    std::vector<double> oData;    //!< Stores the oscillation data.
    std::vector<double> maxAmp;   //!< Stores the detected maxima.
    std::vector<int> maxtime;     //!< Stores the times values where the maxima occurred.
    std::vector<double> minAmp;   //!< Stores the detected minima.
    std::vector<int> mintime;     //!< Stores the times values where the minima occurred.
    std::vector<double> omweData; //!< Stores the calculated values of the OMWE.
    std::vector<int> omweTimes;   //!< Stores the time series where the OMWE was calculated.
    std::vector<double> hrData;   //!< Stores the detected heart rate values.

    // variables to store results
    double resMAP{};    //!< The result of the MAP calculation.
    double resSBP{};    //!< The result of the SBP calculation.
    double resDBP{};    //!< The result of the DBP calculation.
    bool enoughData;    //!< Enough data is available to attempt calculation of the OMWE.

    // variables to store configurations
    // The values that are directly initialised are considered as configuration alues but not implemented as such,
    // they might just as well be constants until then.
    std::atomic<double> ratio_SBP;               //! from literature, might be changed in settings later
    std::atomic<double> ratio_DBP;               //! from literature, might be changed in settings later
    std::atomic<double> maxValidHR = 120.0;      //! The maximal valid heart rate
    std::atomic<double> minValidHR = 50.0;       //! The minimal valid heart rate
    std::atomic<double> prominence = 0.25;       //! The min. prominence of one oscillation to count as a maximum
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

    // private functions:
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
