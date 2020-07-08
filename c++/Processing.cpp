
#include <iostream>
#include <unistd.h>
#include <iomanip>

#include <QtCore/QDateTime>

#include "Processing.h"
#include "ComediHandler.h"

#define DEFAULT_MINUTES 2
#define DEFAULT_DATA_SIZE 1024*60*DEFAULT_MINUTES


Processing::Processing() :
        pData(DEFAULT_DATA_SIZE),
        oData(DEFAULT_DATA_SIZE),
        bRunning(false),
        bMeasuring(false){

    PLOG_VERBOSE << "Processing started";

    comedi = new ComediHandler();

    sampling_rate = comedi->getSamplingRate(); //TODO: calculation seems to be wrong, setting manually for now

    // 5Hz mains LP filter
    iirLP = new Iir::Butterworth::LowPass<IIRORDER>;
    assert(iirLP != NULL);
    iirLP->setup(sampling_rate, 10.0); //TODO: parametrise

    // .5Hz mains HP filter
    iirHP = new Iir::Butterworth::HighPass<IIRORDER>;
    assert(iirHP != NULL);
    iirHP->setup(sampling_rate, 0.5); //TODO: parametrise

    pData.clear();
    oData.clear();

    record = new Datarecord(sampling_rate);
}

Processing::~Processing() {
    stopMeasurement();
    stopThread();
}

bool Processing::bSaveToFile() {
    //TODO: implement saving data to file here.
    return false;
}

void Processing::run() {

    std::cout << "run ..." << std::endl;
    unsigned char buffer[readSize];

    bRunning = true;

    double i = 0.6;
    while (bRunning) {
        if (comedi->getBufferContents() > 0) {

            double y = comedi->getVoltageSample();
            if (bMeasuring) {
                addSample(y);
                //TODO: do not just save data, but "process" it
            }
        } else {
            // If there was no data in the buffer, sleep for 1ms.
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}


void Processing::stopThread() {
    // TODO: safely abort measurement
    bRunning = false;
}

void Processing::addSample(double sample) {
//        ambientV = 0.710#0.675 # from calibration
//        mmHg_per_kPa = 7.5006157584566 # from literature
//        kPa_per_V = 50 # 20mV per 1kPa / 0.02 or * 50 - from sensor datasheet
//        corrFact = 2.50 # from calibration
//
//        ymmHg = (y - ambientV)  * mmHg_per_kPa * kPa_per_V * corrFact
    double yLP = iirLP->filter(sample);
    double yHP = iirHP->filter(yLP);
    pData.push_back(yLP);
    oData.push_back(yHP);

    notifyNewData(yLP, yHP);


}

void Processing::startMeasurement() {
    pData.clear();
    oData.clear();
    bMeasuring = true;
}

void Processing::stopMeasurement() {
    bMeasuring = false;
    record->saveAll(Processing::getFilename(), pData);
}

QString Processing::getFilename() {
    QDateTime dateTime = dateTime.currentDateTime();
    QString dateTimeString = dateTime.toString("testdata_yyyy-MM-dd_hh:mm:ss");
    dateTimeString.append(".dat");
    return dateTimeString;
}