/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005-2017 by Bernd Porr                                 *
 *   mail@berndporr.me.uk                                                  *
 *   Copyright (C) 2020 by Belinda Kneubühler                              *
 *   belinda.kneubuehler@outlook.com                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "obp.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QTextStream>
#include <QComboBox>

#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent),
        adChannel(0),
        dataLength(MAX_DATA_LENGTH),
        time(0),
        linearAverage(0) {
    // initialize comedi
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
    numChannels = comedi_get_n_channels(dev, COMEDI_SUB_DEVICE);
    printf("maxdata: %d \n", maxdata);
    //printf("crange min: %f, max: %f \n", crange->min, crange->max);
    printf("num channels: %d \n", numChannels);
    printf("dataLength: %d \n", dataLength);

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
    //sampling_rate = SAMPLING_RATE; //TODO: calculation seems to be wrong, setting manually for now

    // 50Hz or 60Hz mains notch filter
    iirnotch = new Iir::Butterworth::BandStop<IIRORDER>;
    assert(iirnotch != NULL);
    iirnotch->setup(IIRORDER, sampling_rate, NOTCH_F, NOTCH_F / 10.0);

    // 5Hz mains LP filter
    iirLP = new Iir::Butterworth::LowPass<IIRORDER_HIGH>;
    assert(iirLP != NULL);
    iirLP->setup(sampling_rate, 5.0);

    // .5Hz mains HP filter
    iirHP = new Iir::Butterworth::HighPass<IIRORDER>;
    assert(iirHP != NULL);
    iirHP->setup(sampling_rate, 0.5);

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

    //  Initialize data for plots
    for (int i = 0; i < MAX_DATA_LENGTH; i++) {
        xData[i] = (double)i/(double)SAMPLING_RATE;     // time axis in seconds
        yData[i] = 0;
        yLPData[i] = 0;
        yHPData[i] = i;
    }

    // the gui, straight forward QT/Qwt
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    QVBoxLayout *controlLayout = new QVBoxLayout;
    mainLayout->addLayout(controlLayout);

    QVBoxLayout *plotLayout = new QVBoxLayout;
    plotLayout->addStrut(400);
    mainLayout->addLayout(plotLayout);

    mainLayout->setStretchFactor(controlLayout, 1);
    mainLayout->setStretchFactor(plotLayout, 4);

    // Raw data plot (50Hz notch filter option)
    RawDataPlot = new DataPlot(xData, yData, dataLength,
                               200,0,this);//crange->max, crange->min, this);
    RawDataPlot->setAxisTitles("Time/ms", "Pressure/mmHg");

    plotLayout->addWidget(RawDataPlot);
    RawDataPlot->show();
    plotLayout->addSpacing(20);

    // LP data plot
    LPPlot = new DataPlot(xData, yLPData, dataLength,
                               1, 0.6, this);

    plotLayout->addWidget(LPPlot);
    LPPlot->setPlotTitle("5 Hz LP filtered");
    LPPlot->show();
    plotLayout->addSpacing(20);

    // HP data plot
    HPPlot = new DataPlot(xData, yHPData, dataLength,
                          0.5, -0.5, this);

    plotLayout->addWidget(HPPlot);
    HPPlot->setPlotTitle("0.5 Hz HP filtered (x5)");
    HPPlot->show();
    plotLayout->addSpacing(20);

    /*---- Buttons ----*/

    // Filter group
    QGroupBox *ADcounterGroup = new QGroupBox("Filter", this);
    QVBoxLayout *ADcounterLayout = new QVBoxLayout;

    ADcounterGroup->setLayout(ADcounterLayout);
    ADcounterGroup->setAlignment(Qt::AlignJustify);
    ADcounterGroup->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    controlLayout->addWidget(ADcounterGroup);

    filter50HzCheckBox = new QCheckBox("50Hz filter");
    filter50HzCheckBox->setEnabled(true);
    ADcounterLayout->addWidget(filter50HzCheckBox);

    // Actions
    QGroupBox *AcitonsGroup = new QGroupBox("Actions", this);
    QVBoxLayout *ActionsLayout = new QVBoxLayout;

    AcitonsGroup->setLayout(ActionsLayout);
    AcitonsGroup->setAlignment(Qt::AlignJustify);
    AcitonsGroup->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    controlLayout->addWidget(AcitonsGroup);

    QPushButton *startRecord = new QPushButton(AcitonsGroup);
    startRecord->setText("start recording data");
    ActionsLayout->addWidget(startRecord);
    connect(startRecord, SIGNAL(clicked()), SLOT(slotStartRecord()));

    QPushButton *stopRecord = new QPushButton(AcitonsGroup);
    stopRecord->setText("stop recording data");
    ActionsLayout->addWidget(stopRecord);
    connect(stopRecord, SIGNAL(clicked()), SLOT(slotStopRecord()));

    record = new Datarecord(1000.0);

    // Generate timer event every 50ms
    (void) startTimer(50);

}

MainWindow::~MainWindow() {
    delete[] chanlist;
}

void MainWindow::slotStartRecord() {
    record->startRecording();
}

void MainWindow::slotStopRecord() {
    record->stopRecording();
    time = 0;
    // TODO kept from template.
}

void MainWindow::slotSetChannel(double c) {
    adChannel = (int) c;
}

/**
 * timerEvent
 *
 * Whenever a timer event occurs (every 50ms) the comedi device is read until there is no data
 * available.
 * Processing of the data is also done.
 *
 * @param QTimerEvent
 */
void MainWindow::timerEvent(QTimerEvent *) {
    static int count = 0;
    unsigned char buffer[readSize];

    while (comedi_get_buffer_contents(dev, COMEDI_SUB_DEVICE) > 0) {
        if (read(comedi_fileno(dev), buffer, readSize) == 0) {
            printf("Error: end of Acquisition\n");
            exit(1);
        }

        int v;

        if (sigmaBoard) {
            v = ((lsampl_t *) buffer)[adChannel];
        } else {
            v = ((sampl_t *) buffer)[adChannel];
        }

        double yNew = comedi_to_phys(v,
                                     crange,
                                     maxdata);

        if (filter50HzCheckBox->checkState() == Qt::Checked) {
            yNew = iirnotch->filter(yNew);
        }

        double yLP = iirLP->filter(yNew);
        double yHP = iirHP->filter(yLP);

//        ambientV = 0.710#0.675 # from calibration
//        mmHg_per_kPa = 7.5006157584566 # from literature
//        kPa_per_V = 50 # 20mV per 1kPa / 0.02 or * 50 - from sensor datasheet
//        corrFact = 2.50 # from calibration
//
//        ymmHg = (y - ambientV)  * mmHg_per_kPa * kPa_per_V * corrFact
        RawDataPlot->setNewData((yNew-.71)* 7.5006157584566*50*2.5);
        LPPlot->setNewData(yLP);
        HPPlot->setNewData(yHP*5);
        record->addSample(yNew);
        ++time;
    }
    RawDataPlot->replot();
    LPPlot->replot();
    HPPlot->replot();
}
