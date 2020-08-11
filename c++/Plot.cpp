#include <iostream>
#include <qwt/qwt_scale_widget.h>
#include <qwt/qwt_legend.h>
#include <qwt/qwt_plot_layout.h>

#include "Plot.h"

int Plot::nextPenColour = (int) Qt::darkRed;

/**
 * Plot constructor, initialises an empty plot curve with no titles.
 * @param xData
 * @param yData
 * @param length
 * @param max
 * @param min
 * @param parent
 */
Plot::Plot(double *xData, double *yData, int length, double yMax, double yMin, QWidget *parent) :
        QwtPlot(parent),
        xData(xData),
        yData(yData) {
    setyAxisScale(yMin, yMax);
    dataCurve = new QwtPlotCurve("");
    dataCurve->setPen(QPen((Qt::GlobalColor) nextPenColour++, 3));
    dataCurve->setRawSamples(xData, yData, length);
    dataLength = length;
    dataCurve->attach(this);

    QwtPlot::setAxisScale(QwtPlot::xBottom, 8, 0);
}

/**
 * Sets the title of the plot.
 * @param title
 */
void Plot::setPlotTitle(const QString &title) {
    QwtText pltTitle( title );
    pltTitle.setRenderFlags( Qt::AlignLeft | Qt::AlignVCenter );
    QwtPlot::setTitle( pltTitle );
}

/**
 * Sets the x- and y-axis title of the plot.
 * @param bottomTitle
 * @param leftTitle
 */
void Plot::setAxisTitles(const QString &bottomTitle, const QString &leftTitle) {
    QwtPlot::setAxisTitle(QwtPlot::xBottom, bottomTitle);
    QwtPlot::setAxisTitle(QwtPlot::yLeft, leftTitle);
//    QwtText pltTitle( title );
//    pltTitle.setRenderFlags( Qt::AlignRight | Qt::AlignVCenter );
//    pltTitle.setFont( plot->axisTitle( QwtPlot::xBottom ).font() );
//    plot->setAxisTitle( QwtPlot::xBottom, axisTitle );
}

/**
 * Adjusts the scaling of the y-axis.
 * @param yMax
 * @param yMin
 */
void Plot::setyAxisScale(double yMin, double yMax) {
    QwtPlot::setAxisScale(QwtPlot::yLeft, yMin, yMax);
}

/**
 * Gets the position of the y-axis. Used to line up vertically stacked plots.
 * @return The position where the y-axis starts.
 */
double Plot::getyAxisExtent() {
    QwtScaleWidget *scaleWidget = this->axisWidget(QwtPlot::yLeft);
    QwtScaleDraw *sd = scaleWidget->scaleDraw();
    double extent = sd->extent(scaleWidget->font());
    if (!scaleWidget->title().isEmpty())
        extent += scaleWidget->title().textSize().height();
    if (this->legend() && QwtPlot::LegendPosition::LeftLegend == this->plotLayout()->legendPosition()) {
        double legendSize = this->legend()->sizeHint().width();
        extent += legendSize;
    }
    return extent;
}

/**
 * Adjusts the position of the y-axis. Used to line up vertically stacked plots.
 * @param extent The position where the y-axis should start.
 */
void Plot::setyAxisExtent(double extent) {

    QwtScaleWidget *scaleWidget = this->axisWidget(QwtPlot::yLeft);
    double extentAdj = extent;
    if (!scaleWidget->title().text().isEmpty()) {
        extentAdj -= scaleWidget->title().textSize().height();
    }
    if (this->legend() && QwtPlot::LegendPosition::LeftLegend == this->plotLayout()->legendPosition()) {
        double legendSize = this->legend()->sizeHint().width();
        extentAdj -= legendSize;
    }
    scaleWidget->scaleDraw()->setMinimumExtent(extentAdj);
}

/**
 * Adds a new data sample to the end of the graph, deleting the oldest one.
 * @param yNew The new data
 */
void Plot::setNewData(double yNew) {
    static int cnt = 0;
    memmove(yData, yData + 1, (dataLength - 1) * sizeof(yData[0]));
    yData[dataLength - 1] = yNew;
}