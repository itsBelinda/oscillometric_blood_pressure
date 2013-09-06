/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef PSTHPLOT_H
#define PSTHPLOT_H

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>

#include <QTimerEvent>

/** The PSTH plot widget
*/
class PsthPlot : public QwtPlot
{
  ///pointer to the curve widget
  QwtPlotCurve *dataCurve;

  // pointer to the x and y data
  double *xData, *yData;
  
  // PSTH curve
  long cPsthData;
  ///timer id
  int currtimer;

  double max,min;
  int updateCtr;

  int nDatapoints;

protected:
  // replot the data regularly
  virtual void timerEvent(QTimerEvent *e);

public:
  PsthPlot(double *xData, double *yData, int length, QWidget *parent = 0);
  void setPsthLength(int length);
  void startDisplay();
  void stopDisplay();
  void setYaxisLabel(const QString &label) { setAxisTitle(QwtPlot::yLeft, label); }
};

#endif

