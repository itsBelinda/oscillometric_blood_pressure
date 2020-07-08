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


public:
    Processing();
    ~Processing() override;

    bool bSaveToFile();
    void stopThread();

    void startMeasurement();
    void stopMeasurement();
private:
    void run() override;
    std::vector<double> pData;// = std::vector<double>(122880);
    std::vector<double> oData;// = std::vector<double>(122880);

    Iir::Butterworth::LowPass<IIRORDER> *iirLP;
    Iir::Butterworth::HighPass<IIRORDER> *iirHP;

    Datarecord *record;
    ComediHandler *comedi;
    bool bRunning; // process is running and displaying data on screen, but not necessary recording/measuring blood pressure it.
    bool bMeasuring;


//TODO: move to comedi class?

    /**
   * file descriptor for /dev/comedi0
   **/
    comedi_cmd comediCommand;
    comedi_t *dev;
    size_t readSize;
    bool sigmaBoard;//TODO: warning, if not sigma board
    lsampl_t maxdata;
    comedi_range *crange;
    double sampling_rate;

    int numChannels;
    unsigned *chanlist;

    void addSample(double sample);

    static QString getFilename();
};


#endif //OBP_PROCESSING_H
