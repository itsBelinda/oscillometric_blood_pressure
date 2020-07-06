#ifndef OBP_PROCESSING_H
#define OBP_PROCESSING_H

#include <vector>

#include <comedilib.h>
#include <Iir.h>
#include "CppThread.h"
#include "datarecord.h"


//#define SAMPLING_RATE 1000.0 // 1kHz
#define IIRORDER 6

#define COMEDI_SUB_DEVICE  0
#define COMEDI_RANGE_ID    0   /* +/- 1.325V  for sigma device*/
#define COMEDI_NUM_CHANNEL 1
#define SAMPLING_RATE 1000 // 1kHz





class IEventListener { // "Observed" - Listener needs to implement these functions
public:
    virtual void eNewData(double pData, double oData) {};
    virtual void eSwitchScreen() {}; //TBD

};

//TODO: Base class "EventDriver" to extend for Processing:
//class IEventDriver {
// from :https://github.com/embeddedartistry/embedded-resources/blob/master/examples/cpp/callbacks.cpp
//    // Register a callback.
//    void register_callback(const cb_t &cb, const my_events_t event)
//    {
//        // add callback to end of callback list
//        callbacks_.push_back({cb, event});
//    }
//
//    /// Call all the registered callbacks.
//    void callback() const
//    {
//        my_events_t event = VIDEO_START;
//        // iterate through callback list and call each one
//        for (const auto &cb : callbacks_)
//        {
//            if(cb.event == event)
//            {
//                cb.cb(val_);
//            }
//        }
//    }
//
//private:
//    /// List of callback functions.
//    std::vector<cb_event_t> callbacks_;
//
//};

class Processing : public CppThread {


public:
    Processing();
    ~Processing() override;


    bool bSaveToFile();
    void stopThread();

    //TODO:should ultimately be handled by the algorithm:
    void startMeasurement();
    void stopMeasurement();

    //callback registrations (event handlers?):
   // void registerListener(IProcessEvent )
    //void regDataCallback();
    void registerListener(IEventListener* cb) {this->cb =cb;}

private:
    IEventListener* cb = NULL;
    void run() override;
    std::vector<double> pData;// = std::vector<double>(122880);
    std::vector<double> oData;// = std::vector<double>(122880);

    Iir::Butterworth::LowPass<IIRORDER> *iirLP;
    Iir::Butterworth::HighPass<IIRORDER> *iirHP;

    Datarecord *record;
    bool bRunning; // process is running and displaying data on screen, but not necessary recording/measuring blood pressure it.
    bool bMeasuring;


//TODO: move to comedi class?

    int adChannel;
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
