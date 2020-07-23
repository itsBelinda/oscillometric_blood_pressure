#ifndef OBP_PROCESSING_H
#define OBP_PROCESSING_H

#include <vector>
#include <comedilib.h>
#include <Iir.h>

#include "common.h"
#include "CppThread.h"
#include "datarecord.h"
#include "ISubject.h"
#include "ComediHandler.h"
#include "OBPDetection.h"

#define IIRORDER 4

//! The Processing class handles the data acquisition and processing.
/*!
 * What happens here ...
 */
class Processing : public CppThread, public ISubject{

    enum class ProcState {
        Config,
        Idle,
        Inflate, //possilbly: wait for smallest oscillatin
        Deflate,
        Calculate,
    };

public:
    Processing();
    ~Processing() override;

    void stopThread();

    void startMeasurement();
    void stopMeasurement();
    inline ProcState getCurrentState() const;
    void setAmbientVoltage(double voltage);

private:
    void run() override;
    static QString getFilename();
    void processSample(double newSample);
    [[nodiscard]] double getmmHgValue(double voltageValue) const;
    bool checkAmbient();
    std::vector<double> rawData;

    Iir::Butterworth::LowPass<IIRORDER> *iirLP;
    Iir::Butterworth::HighPass<IIRORDER> *iirHP;

    Datarecord *record;
    ComediHandler *comedi;
    OBPDetection *obpDetect;
    std::atomic<bool> bRunning; // process is running and displaying data on screen, but not necessary recording/measuring blood pressure it.
    std::atomic<bool> bMeasuring;
    ProcState currentState;

    /**
     * Important data acquisition values
     */
    const double mmHg_per_kPa = 7.5006157584566; // literature
    const double kPa_per_V = 50; // data sheet

    double sampling_rate = 1000.0;
    double mmHgInflate = 180.0;
    double ambientVoltage = 0.65;
    double corrFactor = 2.5; // due to voltage divider

};


#endif //OBP_PROCESSING_H
