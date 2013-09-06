/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005 by Bernd Porr                                      *
 *   mail@berndporr.me.uk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef PHYSIO_PSTH_H
#define PHYSIO_PSTH_H

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>

#include <comedilib.h>
#include <qwt/qwt_counter.h>
#include <qwt/qwt_plot_marker.h>

#include "psthplot.h"
#include "dataplot.h"
#include <Iir.h>

// maximal length of the PSTH (for memory alloctaion)
#define MAX_PSTH_LENGTH 5000

#define SAMPLING_RATE 1000 // 1kHz

#define NOTCH_F 50 // filter out 50Hz noise
#define IIRORDER 6

#define COMEDI_SUB_DEVICE  0
#define COMEDI_RANGE_ID    0    /* +/- 4V */


class MainWindow : public QWidget
{
  Q_OBJECT
    
  // show the raw serai data here
  DataPlot *RawDataPlot;
  // here the PSTH will be shown
  PsthPlot *MyPsthPlot;
  
  // channel number for the seari device
  int adChannel;
  // length of the PSTH, this is the length on one trial
  int psthLength;
  // bin width for the PSTH
  int psthBinw;
  // treshold for a spike
  double spikeThres;

  // boo, activate/deactivate the psth plot
  int psthOn;

  // count trials while recording
  int psthActTrial;
  
  // bool, set when a spike is detected and the activity has not
  // gone back to resting potential
  bool spikeDetected;
  
  // data
  double xData[MAX_PSTH_LENGTH], yData[MAX_PSTH_LENGTH];
  // PSTH, t is time, p is spike count, psth is spikes/sec
  double timeData[MAX_PSTH_LENGTH], spikeCountData[MAX_PSTH_LENGTH], psthData[MAX_PSTH_LENGTH];
  
  // serai file desc
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

  QComboBox *averagePsth;
  QwtCounter *cntBinw;
  QTextEdit *editSpikeT;
  QPushButton *triggerPsth;
  QCheckBox* filter50HzCheckBox;
  QwtPlotMarker *thresholdMarker;

private slots:

  // actions:
  void slotClearPsth();
  void slotTriggerPsth();
  void slotSetChannel(double c);
  void slotSetPsthLength(double l);
  void slotSetPsthBinw(double b);
  void slotSetSpikeThres();
  void slotSavePsth();
  void slotAveragePsth(int idx);

protected:

  /// timer to read out the data
  virtual void timerEvent(QTimerEvent *e);

public:

  MainWindow( QWidget *parent=0 );
  ~MainWindow();

};

#endif
