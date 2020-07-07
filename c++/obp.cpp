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
#include "uitest.h"
#include "stackedWidgets.h"

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



MainWindow::MainWindow(QWidget *parent):
        QWidget(parent),
        dataLength(MAX_DATA_LENGTH){

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
                               1,0.6,this);//crange->max, crange->min, this);
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
    //connect(startRecord, SIGNAL(clicked()), SLOT(slotStartRecord()));

    QPushButton *stopRecord = new QPushButton(AcitonsGroup);
    stopRecord->setText("stop recording data");
    ActionsLayout->addWidget(stopRecord);
    //connect(stopRecord, SIGNAL(clicked()), SLOT(slotStopRecord()));


    // Generate timer event every 50ms
    (void) startTimer(50);



}

MainWindow::~MainWindow() {

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
// TODO: repaint directly in update functions leeds to "recursive repaint"
    RawDataPlot->replot();
    LPPlot->replot();
    HPPlot->replot();
}
//
// TODO: changing where the memory address points is not allowed with qwt!
// copying seems to be the only possibility for now.
void MainWindow::updatePressurePlot(double *pData, int length){
    LPPlot->setNewData(pData, length);
}

//void MainWindow::updateOscillationPlot(double *pData, int length){
//    HPPlot->setNewData(pData, length);
//}

void MainWindow::updateRAWPlot(double yNew){
    RawDataPlot->setNewData(yNew);
}
void MainWindow::updatePressurePlot(double yNew){
    LPPlot->setNewData(yNew);
}
void MainWindow::updateOscillationPlot(double yNew){
    HPPlot->setNewData(yNew);
}

void MainWindow::eNewData(double pData, double oData) {
    LPPlot->setNewData(pData);
    HPPlot->setNewData(oData);
}
