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

#define IIRORDER 6

class Processing : public CppThread, public ISubject{

    enum class ProcState {
        Config,
        Idle,
        Inflate,
        Deflate,
        Calculate,
    };

public:
    Processing();
    ~Processing() override;

    void stopThread();

    void startMeasurement();
    void stopMeasurement();
    inline ProcState getCurrentState() const { return currentState; }

    void setAmbientVoltage(double voltage);

private:
    void run() override;
    void resetMeasurement();
    static QString getFilename();
    void processSample(double newSample);
    double getmmHgValue(double voltageValue);
    bool checkAmbient();
    void checkMaxima(double newOscData);
    void checkMinima(double newOscData);
    std::vector<double> nData;
    std::vector<double> pData;
    std::vector<double> oData;
    double lastTimeMax;
    double lastDataMax;
    //TODO: std::map?
    std::vector<double> maxtime;
    std::vector<double> mintime;
    std::vector<double> maxAmp;
    std::vector<double> minAmp;

    Iir::Butterworth::LowPass<IIRORDER> *iirLP;
    Iir::Butterworth::HighPass<IIRORDER> *iirHP;

    Datarecord *record;
    ComediHandler *comedi;
    bool bRunning; // process is running and displaying data on screen, but not necessary recording/measuring blood pressure it.
    bool bMeasuring;
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

    double ratio_SBP = 0.57;    // from literature, might be changed in settings later
    double ratio_DBP = 0.75;    // from literature, might be changed in settings later

};


#endif //OBP_PROCESSING_H
