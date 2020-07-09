#ifndef OBP_PLOT_H
#define OBP_PLOT_H

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>

/**
 * Plot class as a wrapper to QwtPlot to serve the needs of this particular application.
 */
class Plot : public QwtPlot {
public:
    Plot(double *xData, double *yData, int length,
         double yMax, double yMin,
             QWidget *parent = 0);

    void setPlotTitle(const QString& title);
    void setAxisTitles(const QString& bottomTitle, const QString& leftTitle);
    void setyAxisScale(double yMin, double yMax);
    void setNewData(double yNew);
private:
    static int nextPenColour;   //!< Stores the pen color for the next plot object

    QwtPlotCurve *dataCurve;    //!< The curve object
    double *xData, *yData;      //!< Pointers to the x and y data
    int dataLength;             //!< The length of the data pointers
};




#endif //OBP_PLOT_H
