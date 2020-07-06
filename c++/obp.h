/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005 by Bernd Porr                                      *
 *   mail@berndporr.me.uk                                                  *
 *   Copyright (C) 2020 by Belinda Kneubühler                              *
 *   belinda.kneubuehler@outlook.com                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef OBP_H
#define OBP_H

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>

#include <comedilib.h>
#include <qwt/qwt_counter.h>
#include <qwt/qwt_plot_marker.h>

#include <Iir.h>
#include "dataplot.h"
#include "datarecord.h"

// maximal length of the data (for memory allocation)
#define MAX_DATA_LENGTH 8000

#define SAMPLING_RATE 1000 // 1kHz

#define NOTCH_F 50 // filter out 50Hz noise
#define IIRORDER 6
#define IIRORDER_HIGH 10

#define COMEDI_SUB_DEVICE  0
#define COMEDI_RANGE_ID    0   /* +/- 1.325V  for sigma device*/
#define COMEDI_NUM_CHANNEL 1

class MainWindow : public QWidget {
Q_OBJECT

    // show the raw serial data here
    DataPlot *RawDataPlot;
    DataPlot *LPPlot;
    DataPlot *HPPlot;

    // channel number for the serial device
    int adChannel;
    // length of the data
    int dataLength;

    // data
    double xData[MAX_DATA_LENGTH], yData[MAX_DATA_LENGTH], yLPData[MAX_DATA_LENGTH], yHPData[MAX_DATA_LENGTH];
    // t is time, p is spike count, psth is spikes/sec
    double timeData[MAX_DATA_LENGTH], spikeCountData[MAX_DATA_LENGTH], psthData[MAX_DATA_LENGTH];

    QCheckBox *filter50HzCheckBox;
private slots:

    // actions:

    void slotSetChannel(double c);

protected:

    /// timer to read out the data
    virtual void timerEvent(QTimerEvent *e);

public:

    MainWindow(QWidget *parent = 0);

    ~MainWindow();

    // update functions, to be used as callbacks,
    // TODO: change implementation to events?
    // potentially model based, view has access to data,
    // just needs input on when to access it

    //TODO: update: memory location for data for plot
    // cannot change!
    void updateRAWPlot(double yNew);
    void updatePressurePlot(double yNew);
    void updateOscillationPlot(double yNew);

    void updatePressurePlot(double *pData, int length);
};

#endif
