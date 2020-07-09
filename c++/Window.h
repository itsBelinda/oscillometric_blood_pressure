#ifndef OBP_WINDOW_H
#define OBP_WINDOW_H


#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QLabel>
#include <qwt/qwt_dial.h>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMainWindow>
#include "IObserver.h"
#include "Plot.h"
#include "common.h"


class Window : public QMainWindow, public IObserver{
Q_OBJECT
public:
    Window(QWidget *parent = 0);
    ~Window();

protected:
    void timerEvent(QTimerEvent *e);
private:
    void eNewData(double pData, double oData) override;

    void setupUi(QMainWindow *window);
    void retranslateUi(QMainWindow *MainWindow);


    // GUI elements, (probably to be moved to separate class)

    double xData[MAX_DATA_LENGTH], yLPData[MAX_DATA_LENGTH], yHPData[MAX_DATA_LENGTH];
    int dataLength;

    QWidget *centralwidget;
    QHBoxLayout *hl;
    QVBoxLayout *vlLeft;
    QVBoxLayout *vlRight;
    QSpacerItem *vSpace1;
    QSpacerItem *vSpace2;
    QSpacerItem *vSpace3;
    QSpacerItem *vSpace4;
    QSpacerItem *vSpace5;
    QTextBrowser *infoBox;
    QLabel *infoLabel;
    QwtDial *meter;
    //QwtDialSimpleNeedle *needle;
    QPushButton *btnStart;
    QLabel *lTitlePlotRaw;
    QLabel *lTitlePlotOsc;
    Plot *pltPre;
    Plot *pltOsc;
    QFrame *line1;
    QFrame *line2;
    QMenuBar *menubar;
    QStatusBar *statusbar;
};


#endif //OBP_WINDOW_H
