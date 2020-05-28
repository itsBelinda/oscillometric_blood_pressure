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

#include "dataplot.h"
#include <Iir.h>

// maximal length of the data (for memory alloctaion)
#define MAX_DATA_LENGTH 5000

#define SAMPLING_RATE 1000 // 1kHz

#define NOTCH_F 50 // filter out 50Hz noise
#define IIRORDER 6

#define COMEDI_SUB_DEVICE  0
#define COMEDI_RANGE_ID    2    /* +/- 4V */


class MainWindow : public QWidget
{
  Q_OBJECT
    
  // show the raw serial data here
  DataPlot *RawDataPlot;
  
  // channel number for the serial device
  int adChannel;
  // length of the data
  int dataLength;
  
  // data
  double xData[MAX_DATA_LENGTH], yData[MAX_DATA_LENGTH];
  // t is time, p is spike count, psth is spikes/sec
  double timeData[MAX_DATA_LENGTH], spikeCountData[MAX_DATA_LENGTH], psthData[MAX_DATA_LENGTH];
  
  // serial file desc
  int usbFd;
  
  // time counter
  long int time;
  
  comedi_cmd comediCommand;
  
  /**
   * file descriptor for /dev/comedi0
   **/
  comedi_t *dev;
  size_t   readSize;
  bool     sigmaBoard;
  lsampl_t maxdata;
  comedi_range* crange;
  double sampling_rate;

  int numChannels;
  unsigned *chanlist;

  int linearAverage;

  Iir::Butterworth::BandStop<IIRORDER>* iirnotch;

  QCheckBox* filter50HzCheckBox;
private slots:

  // actions:
  void slotClearData();
  void slotSetChannel(double c);
  void slotSaveData();

protected:

  /// timer to read out the data
  virtual void timerEvent(QTimerEvent *e);

public:

  MainWindow( QWidget *parent=0 );
  ~MainWindow();

};

#endif
