
#include <iostream>
#include <unistd.h>
#include <iomanip>

#include <QtCore/QDateTime>

#include "Processing.h"

#define DEFAULT_MINUTES 2
#define DEFAULT_DATA_SIZE 1024*60*DEFAULT_MINUTES






Processing::Processing() :
        pData(DEFAULT_DATA_SIZE),
        oData(DEFAULT_DATA_SIZE),
        adChannel(0) {
    // initialize comedi
    bRunning = false;
    bMeasuring = false;

    std::cout << "processing constructor" << std::endl;
    const char *filename = "/dev/comedi0";

    /* open the device */
    if ((dev = comedi_open(filename)) == 0) {
        comedi_perror(filename);
        exit(1);
    }

    // do not produce NAN for out of range behaviour
    comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

    maxdata = comedi_get_maxdata(dev, COMEDI_SUB_DEVICE, COMEDI_SUB_DEVICE);
    crange = comedi_get_range(dev, COMEDI_SUB_DEVICE, COMEDI_SUB_DEVICE, COMEDI_RANGE_ID);
    //TODO: 1?
    numChannels = comedi_get_n_channels(dev, COMEDI_SUB_DEVICE);
    printf("maxdata: %d \n", maxdata);
    //printf("crange min: %f, max: %f \n", crange->min, crange->max);
    printf("num channels: %d \n", numChannels);

    if( numChannels < COMEDI_NUM_CHANNEL )
    {
        exit(1);
    }
    else
    {
        numChannels = COMEDI_NUM_CHANNEL;
    }

    chanlist = new unsigned[numChannels];

    /* Set up channel list */
    for (int i = 0; i < numChannels; i++)
        chanlist[i] = CR_PACK(i, COMEDI_RANGE_ID, AREF_GROUND);

    int ret = comedi_get_cmd_generic_timed(dev,
                                           COMEDI_SUB_DEVICE,
                                           &comediCommand,
                                           numChannels,
                                           (int) (1e9 / (SAMPLING_RATE)));

    if (ret < 0) {
        printf("comedi_get_cmd_generic_timed failed\n");
        exit(-1);
    }

    /* Modify parts of the command */
    comediCommand.chanlist = chanlist;
    comediCommand.stop_src = TRIG_NONE;
    comediCommand.stop_arg = 0;

    /* comedi_command_test() tests a command to see if the
     * trigger sources and arguments are valid for the subdevice.
     * If a trigger source is invalid, it will be logically ANDed
     * with valid values (trigger sources are actually bitmasks),
     * which may or may not result in a valid trigger source.
     * If an argument is invalid, it will be adjusted to the
     * nearest valid value.  In this way, for many commands, you
     * can test it multiple times until it passes.  Typically,
     * if you can't get a valid command in two tests, the original
     * command wasn't specified very well. */
    ret = comedi_command_test(dev, &comediCommand);
    fprintf(stderr, "first test returned %d\n", ret);
    if (ret < 0) {
        comedi_perror("comedi_command_test");
        exit(-1);
    }

    fprintf(stderr, "first test returned %d\n", ret);

    ret = comedi_command_test(dev, &comediCommand);
    if (ret < 0) {
        comedi_perror("comedi_command_test");
        exit(-1);
    }

    fprintf(stderr, "second test returned %d\n", ret);

    if (ret != 0) {
        fprintf(stderr, "Error preparing command\n");
        exit(-1);
    }

    // the timing is done channel by channel
    // this means that the actual sampling rate is divided by
    // number of channels
    if ((comediCommand.convert_src == TRIG_TIMER) && (comediCommand.convert_arg)) {
        sampling_rate = (((double) 1E9 / comediCommand.convert_arg) / numChannels);
    }
    printf("sampling rate: %f \n", sampling_rate);

    // the timing is done scan by scan (all channels at once)
    // the sampling rate is equivalent of the scan_begin_arg
    if ((comediCommand.scan_begin_src == TRIG_TIMER) && (comediCommand.scan_begin_arg)) {
        sampling_rate = (double) 1E9 / comediCommand.scan_begin_arg;
    }
    printf("sampling rate: %f \n", sampling_rate);
    sampling_rate = SAMPLING_RATE; //TODO: calculation seems to be wrong, setting manually for now

    // 5Hz mains LP filter
    iirLP = new Iir::Butterworth::LowPass<IIRORDER>;
    assert(iirLP != NULL);
    iirLP->setup(sampling_rate, 10.0); //TODO: parametrise

    // .5Hz mains HP filter
    iirHP = new Iir::Butterworth::HighPass<IIRORDER>;
    assert(iirHP != NULL);
    iirHP->setup(sampling_rate, 0.5); //TODO: parametrise

    /* start the command */
    ret = comedi_command(dev, &comediCommand);
    if (ret < 0) {
        comedi_perror("comedi_command");
        exit(1);
    }

    int subdev_flags = comedi_get_subdevice_flags(dev, COMEDI_SUB_DEVICE);

    if ((sigmaBoard = subdev_flags & SDF_LSAMPL))
        readSize = sizeof(lsampl_t) * numChannels;
    else
        readSize = sizeof(sampl_t) * numChannels;

    pData.clear();
    oData.clear();
    record = new Datarecord(SAMPLING_RATE);
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
        if (comedi_get_buffer_contents(dev, COMEDI_SUB_DEVICE) > 0) {
            //TODO: does read sleep while waiting?
            if (read(comedi_fileno(dev), buffer, readSize) == 0) {
                std::cerr << "Error: end of Acquisition!" << std::endl;
                exit(1);
            }

            // TODO: rewrite?
            int v = 0;

            if (sigmaBoard) {
                v = ((lsampl_t *) buffer)[adChannel];
                //std::cout << "raw:" << v << std::endl;
            } else {
                v = ((sampl_t *) buffer)[adChannel];
            }
            double y = comedi_to_phys(v, crange, maxdata);
            //std::cout << "vol:" << y << std::endl; // TODO: debug only
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

    //TODO: remove!!!!
    if(cb) {
        cb->eNewData(yLP, yHP);
//        view->updateRAWPlot(sample);
//        view->updatePressurePlot(yLP);
//        view->updateOscillationPlot(yHP);
    }


}

void Processing::startMeasurement() {
    pData.clear();
    oData.clear();
    bMeasuring = true;
}

void Processing::stopMeasurement() {
    bMeasuring = false;
    record->saveAll(Processing::getFilename(),pData);
}

QString Processing::getFilename() {
    QDateTime dateTime = dateTime.currentDateTime();
    QString dateTimeString = dateTime.toString("yyyy-MM-dd_hh:mm:ss");
    dateTimeString.append(".dat");
    return dateTimeString;
}