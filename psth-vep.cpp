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

#include "psth-vep.h"

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
    psthLength(1000),
    psthBinw(20),
    spikeThres(1),
    psthOn(0),
    spikeDetected(false),
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
    timeData[i] = double(i)*psthBinw; // psth time axis
    spikeCountData[i] = 0;
    psthData[i] = 0;
  }

  // the gui, straight forward QT/Qwt
  resize(640,420);
  QHBoxLayout *mainLayout = new QHBoxLayout( this );

  QVBoxLayout *controlLayout = new QVBoxLayout;
  mainLayout->addLayout(controlLayout);

  QVBoxLayout *plotLayout = new QVBoxLayout;
  plotLayout->addStrut(400);
  mainLayout->addLayout(plotLayout);

  mainLayout->setStretchFactor(controlLayout,1);
  mainLayout->setStretchFactor(plotLayout,4);

  // two plots
  RawDataPlot = new DataPlot(xData, yData, psthLength, 
			     crange->max, crange->min, this);
  plotLayout->addWidget(RawDataPlot);
  RawDataPlot->show();

  plotLayout->addSpacing(20);

  MyPsthPlot = new PsthPlot(timeData, psthData, psthLength/psthBinw, this);
  plotLayout->addWidget(MyPsthPlot);
  MyPsthPlot->show();

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
  QGroupBox   *PSTHfunGroup  = new QGroupBox( "Actions", this );
  QVBoxLayout *PSTHfunLayout = new QVBoxLayout;

  PSTHfunGroup->setLayout(PSTHfunLayout);
  PSTHfunGroup->setAlignment(Qt::AlignJustify);
  PSTHfunGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );
  controlLayout->addWidget( PSTHfunGroup );

  averagePsth = new QComboBox(PSTHfunGroup);
  averagePsth->addItem(tr("PSTH"));
  averagePsth->addItem(tr("VEP"));
  PSTHfunLayout->addWidget(averagePsth);
  connect( averagePsth, SIGNAL(currentIndexChanged(int)), SLOT(slotAveragePsth(int)) );

  triggerPsth = new QPushButton(PSTHfunGroup);
  triggerPsth->setText("PSTH on");
  triggerPsth->setCheckable(true);
  PSTHfunLayout->addWidget(triggerPsth);
  connect(triggerPsth, SIGNAL(clicked()), SLOT(slotTriggerPsth()));

  QPushButton *clearPsth = new QPushButton(PSTHfunGroup);
  clearPsth->setText("clear data");
  PSTHfunLayout->addWidget(clearPsth);
  connect(clearPsth, SIGNAL(clicked()), SLOT(slotClearPsth()));

  QPushButton *savePsth = new QPushButton(PSTHfunGroup);
  savePsth->setText("save data");
  PSTHfunLayout->addWidget(savePsth);
  connect(savePsth, SIGNAL(clicked()), SLOT(slotSavePsth()));

  // psth params
  QGroupBox   *PSTHcounterGroup = new QGroupBox( "Parameters", this );
  QVBoxLayout *PSTHcounterLayout = new QVBoxLayout;

  PSTHcounterGroup->setLayout(PSTHcounterLayout);
  PSTHcounterGroup->setAlignment(Qt::AlignJustify);
  PSTHcounterGroup->setSizePolicy( QSizePolicy(QSizePolicy::Fixed,
					       QSizePolicy::Fixed) );
  controlLayout->addWidget( PSTHcounterGroup );

  QLabel *psthLengthLabel = new QLabel("Sweep length", PSTHcounterGroup);
  PSTHcounterLayout->addWidget(psthLengthLabel);

  QwtCounter *cntSLength = new QwtCounter(PSTHcounterGroup);
  cntSLength->setNumButtons(2);
  cntSLength->setIncSteps(QwtCounter::Button1, 10);
  cntSLength->setIncSteps(QwtCounter::Button2, 100);
  cntSLength->setRange(1, MAX_PSTH_LENGTH);
  cntSLength->setValue(psthLength);
  PSTHcounterLayout->addWidget(cntSLength);
  connect(cntSLength, 
	  SIGNAL(valueChanged(double)), 
	  SLOT(slotSetPsthLength(double)));

  QLabel *binwidthLabel = new QLabel("Binwidth", PSTHcounterGroup);
  PSTHcounterLayout->addWidget(binwidthLabel);

  cntBinw = new QwtCounter(PSTHcounterGroup);
  cntBinw->setNumButtons(2);
  cntBinw->setIncSteps(QwtCounter::Button1, 1);
  cntBinw->setIncSteps(QwtCounter::Button2, 10);
  cntBinw->setRange(1, 100);
  cntBinw->setValue(psthBinw);
  PSTHcounterLayout->addWidget(cntBinw);
  connect(cntBinw, SIGNAL(valueChanged(double)), SLOT(slotSetPsthBinw(double)));

  QLabel *thresholdLabel = new QLabel("Spike Threshold", PSTHcounterGroup);
  PSTHcounterLayout->addWidget(thresholdLabel);

  editSpikeT = new QTextEdit("0");
  QFont editFont("Courier",14);
  QFontMetrics editMetrics(editFont);
  editSpikeT->setMaximumHeight ( editMetrics.height()*1.2 );
  editSpikeT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  editSpikeT->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  editSpikeT->setFont(editFont);
  PSTHcounterLayout->addWidget(editSpikeT);
  connect(editSpikeT, SIGNAL(textChanged()), SLOT(slotSetSpikeThres()));

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

void MainWindow::slotSavePsth()
{
  QString fileName = QFileDialog::getSaveFileName();

  if( !fileName.isNull() )
  {
    QFile file(fileName);

    if( file.open(QIODevice::WriteOnly | QFile::Truncate) )
    {
      QTextStream out(&file);

      for(int i=0; i<psthLength/psthBinw; i++)
        out << timeData[i] << "\t" << psthData[i] << "\n";

      file.close();
    }
    else
    {
      // TODO: warning box
    }
  }
}

void MainWindow::slotClearPsth()
{
  time = 0;
  for(int i=0; i<psthLength/psthBinw; i++) {
    psthData[i] = 0;
    spikeCountData[i] = 0;
  }
  spikeDetected = false;
  psthActTrial = 0;
  MyPsthPlot->replot();
}

void MainWindow::slotTriggerPsth()
{
	if(psthOn == 0)
	{
		for(int i=0; i<psthLength/psthBinw; i++) {
			psthData[i] = 0;
			spikeCountData[i] = 0;
		}
		psthActTrial = 0;
		psthOn = 1;
		time = 0;
		MyPsthPlot->startDisplay();
		psthOn = 1;
		psthActTrial = 0;
		time = 0;
	}
	else
	{
		MyPsthPlot->stopDisplay();
		psthOn = 0;
		psthActTrial = 0;
		spikeDetected = false;
	}
}

void MainWindow::slotSetChannel(double c)
{
  adChannel = (int)c;
  spikeDetected = false;
}

void MainWindow::slotSetPsthLength(double l)
{
  psthLength = (int)l;

  for(int i=0; i<psthLength/psthBinw; i++) {
    psthData[i] = 0;
    spikeCountData[i] = 0;
    timeData[i] = double(i)*psthBinw;
  }
  spikeDetected = false;
  psthActTrial = 0;
  time = 0;

  RawDataPlot->setPsthLength((int) l);
  MyPsthPlot->setPsthLength(psthLength/psthBinw);
}

void MainWindow::slotSetPsthBinw(double b)
{
  psthBinw = (int)b;
  for(int i=0; i<psthLength/psthBinw; i++) {
    psthData[i] = 0;
    spikeCountData[i] = 0;
    timeData[i] = double(i)*psthBinw;
  }
  spikeDetected = false;
  psthActTrial = 0;
  time = 0;
  MyPsthPlot->setPsthLength(psthLength/psthBinw);
}

void MainWindow::slotSetSpikeThres()
{
	QString t = editSpikeT->toPlainText();
	spikeThres = t.toFloat();
	thresholdMarker->setValue(0,spikeThres);
	printf("%lf\n",spikeThres);
	spikeDetected = false;
}

void MainWindow::slotAveragePsth(int idx)
{
	linearAverage = (idx>0);
	if ( linearAverage )
	{
		cntBinw->setEnabled(false);
		editSpikeT->setEnabled(false);
		MyPsthPlot->setYaxisLabel("Averaged Data");
		MyPsthPlot->setAxisTitle(QwtPlot::yLeft, "average/V");
		MyPsthPlot->setTitle("VEP");
		triggerPsth->setText("Averaging on");
		psthBinw = 1;
		cntBinw->setValue(psthBinw);
	}
	else
	{
		cntBinw->setEnabled(true);
		editSpikeT->setEnabled(true);
		MyPsthPlot->setYaxisLabel("Spikes/s");
		MyPsthPlot->setAxisTitle(QwtPlot::yLeft, "Spikes/s");
		MyPsthPlot->setTitle("PSTH");
		triggerPsth->setText("PSTH on");
	}
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

    int trialIndex = time % psthLength;

    if( linearAverage && psthOn )
    {
      spikeCountData[trialIndex] += yNew;

      psthData[trialIndex] = spikeCountData[trialIndex] / (time/psthLength + 1);
    }
    else if( !spikeDetected && yNew>spikeThres )
    {
      if(psthOn)
      {
        int psthIndex = trialIndex / psthBinw;

        spikeCountData[psthIndex] += 1;

        psthData[psthIndex] = ( spikeCountData[psthIndex]*1000 ) / 
		( psthBinw * (time/psthLength + 1) );

        spikeDetected = true;
      }
    }
    else if( yNew < spikeThres )
    {
      spikeDetected = false;
    }
    
    if( trialIndex == 0 )
      psthActTrial += 1;
    
    ++time;
  }
  RawDataPlot->replot();
}
