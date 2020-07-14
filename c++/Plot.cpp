#include "Plot.h"

//TODO: use static declaration
int Plot::nextPenColour = (int) Qt::blue;

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
void Plot::setPlotTitle(const QString& title) {
    QwtPlot::setTitle(title);
}

/**
 * Sets the x- and y-axis title of the plot.
 * @param bottomTitle
 * @param leftTitle
 */
void Plot::setAxisTitles(const QString& bottomTitle, const QString& leftTitle) {
    QwtPlot::setAxisTitle(QwtPlot::xBottom, bottomTitle);
    QwtPlot::setAxisTitle(QwtPlot::yLeft, leftTitle);
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
 * Adds a new data sample to the end of the graph, deleting the oldest one.
 * @param yNew
 */
void Plot::setNewData(double yNew) {
    static int cnt = 0;
    memmove(yData, yData + 1, (dataLength - 1) * sizeof(yData[0]));
    yData[dataLength - 1] = yNew;
    // TODO: data mutex to shift pointers of raw data instead of having two copies?
}