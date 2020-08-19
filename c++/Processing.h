/**
 * @file        Processing.h
 * @brief       The header file of the Processing class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * Defines the Processing class and contains the general class description.
 */
#ifndef OBP_PROCESSING_H
#define OBP_PROCESSING_H

#include <vector>
#include <comedilib.h>
#include <Iir.h>

#include "common.h"
#include "CppThread.h"
#include "Datarecord.h"
#include "ISubject.h"
#include "ComediHandler.h"
#include "OBPDetection.h"

/**
 * Class dependant configuration values:
 */
#define MAX_PUMPUP 250  //!< Maximal settable pump-up value.
#define IIRORDER 4      //!< IIR filter order.

//! The Processing class handles the data acquisition and processing.
/*!
 * The processing class inherits from the CppThread class and the ISubject class. CppThread is a wrapper to the
 * std::thread class that was written by Bernd Porr to avoid static methods and makes the inheriting class a runnable
 * thread. Processing has an instance of ComediHandler to acquire and two IIR filter instances to pre-process the data.
 * The raw, unfiltered data is stored in a vector that can be handed to the Datarecord instance to save it as a file.
 * The filtered data is sent to the observer(s) to display and passed to the OPDetection instance that performs the
 * algorithm. Data acquisition and filtering are happening whenever the thread is running, the state machine
 * decides when data is passed to the OBPDetection or stored to a file.
 */
class Processing : public CppThread, public ISubject {

    enum class ProcState {
        Config,     //!< Configure the ambient pressure.
        Idle,       //!< Waiting for user to start the measurement.
        Inflate,    //!< Inflate the cuff.
        Deflate,    //!< Deflate the cuff slowly.
        Empty,      //!< Empty the cuff completely
        Results,    //!< Display the results.
    };

public:
    explicit Processing(double fcLP = 10.0, double fcHP = 0.5);
    ~Processing() override;

    void setRatioSBP(double val);
    double getRatioSBP();
    void setRatioDBP(double val);
    double getRatioDBP();
    void setMinNbrPeaks(int val);
    int getMinNbrPeaks();
    void setPumpUpValue(int val);
    int getPumpUpValue();
    double getSamplingRate();

    void resetConfigValues();
    void startMeasurement();
    void stopMeasurement();
    void stopThread();

private:
    void run() override;
    void processSample(double newSample);
    static QString getFilename();
    [[nodiscard]] double getmmHgValue(double voltageValue) const;
    bool checkAmbient();

    std::vector<double> rawData;                 //!< stores the acquired raw data

    Iir::Butterworth::LowPass<IIRORDER> *iirLP;  //!< Low-pass filter instance
    Iir::Butterworth::HighPass<IIRORDER> *iirHP; //!< High-pass filter instance

    Datarecord *record;                         //!< Datarecord instance to store data
    ComediHandler *comedi;                      //!< ComediHandler instance to acquire data
    OBPDetection *obpDetect;                    //!< LOBPDetection instance that implements the algorithm
    std::atomic<bool> bRunning;                 //!< process is running and displaying data on screen.
    std::atomic<bool> bMeasuring;               //!< Boolean to indicate an ongoing measurement.
    ProcState currentState;                     //!< Stores the state of the application

    /**
     * Important data acquisition values:
     */
    const double kPa_per_mmHg = 0.133322;       //!< Value of kPa per 1 mmHg, from literature
    const double kPa_per_V = 50;                //!< Value of kPa per 1 V, from pressure sensor data sheet.

    /**
     * User set configuration values:
     */
    std::atomic<double> mmHgInflate;           //!< Pump-up value used to transition from Inflate to Deflate state.
    double corrFactor;                         //!< Correction factor to account for voltage divider.

    /**
     * Program set configuation values.
     */
    std::atomic<double> sampling_rate;          //!< The sampling rate of the data acquisition
    double ambientVoltage;                      //!< The voltage at ambient pressure, needed for calculations.

};


#endif //OBP_PROCESSING_H
