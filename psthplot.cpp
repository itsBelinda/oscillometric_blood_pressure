/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "psthplot.h"

#include <QTimerEvent>

PsthPlot::PsthPlot(double *xData, double *yData, int length, QWidget *parent) :
    QwtPlot(parent),
    xData(xData),
    yData(yData)
{
  // Assign a title
  setTitle("PSTH");
  setAxisTitle(QwtPlot::xBottom, "Time/ms");
  setAxisTitle(QwtPlot::yLeft, "Spikes/s");

  dataCurve = new QwtPlotCurve("PSTH");
  dataCurve->setRawSamples(xData, yData, length);
  dataCurve->attach(this);
  dataCurve->setPen( QPen(Qt::blue, 2) );
  dataCurve->setStyle(QwtPlotCurve::Steps);

  max = 0;
  min = 0;
  nDatapoints = length;
  updateCtr = 1;

  setAutoReplot(false);
}

void PsthPlot::setPsthLength(int length)
{
	nDatapoints = length;	
	dataCurve->setRawSamples(xData, yData, length);
}

void PsthPlot::startDisplay()
{
	currtimer=startTimer(150);
}

void PsthPlot::stopDisplay()
{
	killTimer(currtimer);
}

void PsthPlot::timerEvent(QTimerEvent *)
{
	updateCtr--;
	if (updateCtr==0) {
		min = INT_MAX;
		max = INT_MIN;
		for(int i=0;i<nDatapoints;i++) {
			float y = yData[i];
			if (y>max) max = y;
			if (y<min) min = y;
		}
		double d = max - min;
		setAxisScale(QwtPlot::yLeft,min-d/10,max+d/10);
		updateCtr = 10;
	}

  replot();
}
