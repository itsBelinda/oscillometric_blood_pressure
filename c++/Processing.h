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

#define IIRORDER 4

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

    static int maxValidPulse;
    static int minValidPulse;
    static double maxPulseChange;
private:
    void run() override;
    void resetMeasurement();
    static QString getFilename();
    void processSample(double newSample);
    [[nodiscard]] double getmmHgValue(double voltageValue) const;
    bool checkAmbient();
    bool checkMaxima(double newOscData);
    void findMinima();
    void findOWME();
    void findMAP();
    bool isPastDBP();
    static double getRatio(double lowerBound, double upperBound, double value);
    static double getAverage(std::vector<double> avVector);
    static bool isPulseValid( double pulse );
    bool isValidMaxima();
    std::vector<double> rawData;
    std::vector<double> pData;
    std::vector<double> oData;
    std::vector<double> omveData;
    std::vector<double> heartRate;
    std::vector<int> omveTimes;
    unsigned long lastTimeMax;
    double lastDataMax;
    double avPulse;
    //TODO: std::map?
    std::vector<int> maxtime;
    std::vector<int> mintime;
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
