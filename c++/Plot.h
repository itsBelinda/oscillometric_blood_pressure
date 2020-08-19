/**
 * @file        Plot.h
 * @brief       The header file of the Plot class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * Defines the Plot class and contains the general class description.
 */
#ifndef OBP_PLOT_H
#define OBP_PLOT_H

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>

//! The Plot class displays a single plot as a Qwt widget.
/*!
 * The Plot class inherits from QwtPlot. It handles setting up plot and axis titles, scaling and changing the
 * displayed data.
 */
class Plot : public QwtPlot {
public:
    Plot(double *xData, double *yData, int length,
         double yMax, double yMin,
             QWidget *parent = 0);

    void setPlotTitle(const QString& title);
    void setAxisTitles(const QString& bottomTitle, const QString& leftTitle);
    double getyAxisExtent();
    void setyAxisExtent(double extent);
    void setyAxisScale(double yMin, double yMax);
    void setNewData(double yNew);
private:
    static int nextPenColour;   //!< Stores the pen color for the next plot object

    QwtPlotCurve *dataCurve;    //!< The curve object
    double *xData, *yData;      //!< Pointers to the x and y data
    int dataLength;             //!< The length of the data pointers
};




#endif //OBP_PLOT_H
