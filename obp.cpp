/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005-2017 by Bernd Porr                                 *
 *   mail@berndporr.me.uk                                                  *
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

MainWindow::MainWindow( QWidget *parent ) :
    QWidget(parent),
    adChannel(0),
    dataLength(1000),
    //psthBinw(20),
    //spikeThres(1),
    //psthOn(0),
    //spikeDetected(false),
    time(0),
    linearAverage(0)
{
  // initialize comedi
  const char *filename = "/dev/comedi0";

  /* open the device */
  if( (dev = comedi_open(filename)) == 0 )
  {
    comedi_perror(filename);
    exit(1);
  }

  // do not produce NAN for out of range behaviour
  comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER); 

  maxdata = comedi_get_maxdata(dev, COMEDI_SUB_DEVICE, 0);
  crange = comedi_get_range(dev,COMEDI_SUB_DEVICE,0,0);
  numChannels = comedi_get_n_channels(dev, COMEDI_SUB_DEVICE);

  chanlist = new unsigned[numChannels];

  /* Set up channel list */
  for( int i=0; i<numChannels; i++ )
    chanlist[i] = CR_PACK(i, COMEDI_RANGE_ID, AREF_GROUND);

  int ret = comedi_get_cmd_generic_timed( dev,
                                          COMEDI_SUB_DEVICE,
                                          &comediCommand,
                                          numChannels,
                                          (int)(1e9/(SAMPLING_RATE)) );

  if(ret < 0)
  {
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

  if(ret < 0)
  {
    comedi_perror("comedi_command_test");
    exit(-1);
  }

  fprintf(stderr, "first test returned %d\n", ret);

  ret = comedi_command_test(dev, &comediCommand);
  if(ret < 0)
  {
    comedi_perror("comedi_command_test");
    exit(-1);
  }

  fprintf(stderr, "second test returned %d\n", ret);

  if(ret != 0)
  {
    fprintf(stderr,"Error preparing command\n");
    exit(-1);
  }

  // the timing is done channel by channel
  // this means that the actual sampling rate is divided by
  // number of channels
  if ((comediCommand.convert_src ==  TRIG_TIMER)&&(comediCommand.convert_arg)) {
	  sampling_rate=(((double)1E9 / comediCommand.convert_arg)/numChannels);
  }
  
  // the timing is done scan by scan (all channels at once)
  // the sampling rate is equivalent of the scan_begin_arg
  if ((comediCommand.scan_begin_src ==  TRIG_TIMER)&&(comediCommand.scan_begin_arg)) {
	  sampling_rate=(double)1E9 / comediCommand.scan_begin_arg;
  }

  // 50Hz or 60Hz mains notch filter
  iirnotch = new Iir::Butterworth::BandStop<IIRORDER>;
  assert( iirnotch != NULL );
  iirnotch->setup (IIRORDER, sampling_rate, NOTCH_F, NOTCH_F/10.0);

  /* start the command */
  ret = comedi_command(dev, &comediCommand);
  if(ret < 0)
  {
    comedi_perror("comedi_command");
    exit(1);
  }

  int subdev_flags = comedi_get_subdevice_flags(dev, COMEDI_SUB_DEVICE);

  if( (sigmaBoard = subdev_flags & SDF_LSAMPL) )
    readSize = sizeof(lsampl_t) * numChannels;
  else
    readSize = sizeof(sampl_t) * numChannels;

  //  Initialize data for plots
  for(int i=0; i<MAX_PSTH_LENGTH; i++)
  {
    xData[i] = i;     // time axis
    yData[i] = 0;
    //timeData[i] = double(i)*psthBinw; // psth time axis
    spikeCountData[i] = 0;
    psthData[i] = 0;
  }

  // the gui, straight forward QT/Qwt
  resize(640,800); //TODO: currently has no effect?
  QHBoxLayout *mainLayout = new QHBoxLayout( this );

  QVBoxLayout *controlLayout = new QVBoxLayout;
  mainLayout->addLayout(controlLayout);

  QVBoxLayout *plotLayout = new QVBoxLayout;
  plotLayout->addStrut(400);
  mainLayout->addLayout(plotLayout);

  mainLayout->setStretchFactor(controlLayout,1);
  mainLayout->setStretchFactor(plotLayout,4);

  // just one plot
  RawDataPlot = new DataPlot(xData, yData, dataLength,
			     crange->max, crange->min, this);
  plotLayout->addWidget(RawDataPlot);
  RawDataPlot->show();

  plotLayout->addSpacing(20);

  /*---- Buttons ----*/

  // AD group
  QGroupBox   *ADcounterGroup = new QGroupBox( "A/D Channel", this );
  QVBoxLayout *ADcounterLayout = new QVBoxLayout;

  ADcounterGroup->setLayout(ADcounterLayout);
  ADcounterGroup->setAlignment(Qt::AlignJustify);
  ADcounterGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
  controlLayout->addWidget( ADcounterGroup );

  QwtCounter *cntChannel = new QwtCounter(ADcounterGroup);
  cntChannel->setRange(0, numChannels-1);
  cntChannel->setValue(adChannel);
  ADcounterLayout->addWidget(cntChannel);
  connect(cntChannel, SIGNAL(valueChanged(double)), SLOT(slotSetChannel(double)));

  filter50HzCheckBox = new QCheckBox( "50Hz filter" );
  filter50HzCheckBox->setEnabled( true );
  ADcounterLayout->addWidget(filter50HzCheckBox);

  // psth functions
  QGroupBox   *AcitonsGroup  = new QGroupBox( "Actions", this );
  QVBoxLayout *ActionsLayout = new QVBoxLayout;

  AcitonsGroup->setLayout(ActionsLayout);
  AcitonsGroup->setAlignment(Qt::AlignJustify);
  AcitonsGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
  controlLayout->addWidget( AcitonsGroup );

  QPushButton *clearData = new QPushButton(AcitonsGroup);
  clearData->setText("clear data");
  ActionsLayout->addWidget(clearData);
  connect(clearData, SIGNAL(clicked()), SLOT(slotClearPsth()));

  QPushButton *saveData = new QPushButton(AcitonsGroup);
  saveData->setText("save data");
  ActionsLayout->addWidget(saveData);
  connect(saveData, SIGNAL(clicked()), SLOT(slotSavePsth()));

  thresholdMarker = new QwtPlotMarker();
  thresholdMarker->setValue(0,0);
  thresholdMarker->attach(RawDataPlot);
  thresholdMarker->setLineStyle(QwtPlotMarker::HLine);

  // Generate timer event every 50ms
  (void)startTimer(50);

}

MainWindow::~MainWindow()
{
  delete[] chanlist;
}

void MainWindow::slotSaveData()
{
  QString fileName = QFileDialog::getSaveFileName();

  if( !fileName.isNull() )
  {
    QFile file(fileName);

    if( file.open(QIODevice::WriteOnly | QFile::Truncate) )
    {
      QTextStream out(&file);

//      for(int i=0; i<dataLength/psthBinw; i++)
//        out << timeData[i] << "\t" << psthData[i] << "\n";

      file.close();
    }
    else
    {
      // TODO: warning box
    }
  }
}

void MainWindow::slotClearData()
{
  time = 0;
  // TODO kept from template.
}

void MainWindow::slotSetChannel(double c)
{
  adChannel = (int)c;
}

void MainWindow::timerEvent(QTimerEvent *)
{
  unsigned char buffer[readSize];

  while( comedi_get_buffer_contents(dev, COMEDI_SUB_DEVICE) > 0 )
  {
    if( read(comedi_fileno(dev), buffer, readSize) == 0 )
    {
      printf("Error: end of Aquisition\n");
      exit(1);
    }

    int v;

    if( sigmaBoard )
	    v = ((lsampl_t *)buffer)[adChannel];
    else
	    v = ((sampl_t *)buffer)[adChannel];

    double yNew = comedi_to_phys(v,
				 crange,
				 maxdata) / ((double)PREAMP_GAIN);

    if (filter50HzCheckBox->checkState()==Qt::Checked) {
	    yNew=iirnotch->filter(yNew);
    }                                

    RawDataPlot->setNewData(yNew);

    ++time;
  }
  RawDataPlot->replot();
}
