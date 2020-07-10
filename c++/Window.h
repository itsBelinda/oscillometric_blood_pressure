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
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStackedWidget>
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
    void eSwitchScreen(Screen eScreen) override;

    void setupUi(QMainWindow *window);
    QWidget * setupPlots(QWidget *parent);
    QWidget * setupStartPage(QWidget *parent);
    QWidget * setupPumpPage(QWidget *parent);
    QWidget * setupReleasePage(QWidget *parent);
    QWidget * setupDeflatePage(QWidget *parent);
    QWidget * setupResultPage(QWidget *parent);

    void retranslateUi(QMainWindow *MainWindow);


    // GUI elements, (probably to be moved to separate class)

    double xData[MAX_DATA_LENGTH], yLPData[MAX_DATA_LENGTH], yHPData[MAX_DATA_LENGTH];
    int dataLength;

    QSplitter *splitter;
    QStackedWidget *lInstructions;
    QWidget *lInstrStart;
    QWidget *lInstrPump;
    QWidget *lInstrRelease;
    QWidget *lInstrDeflate;
    QWidget *lInstrResult;
    QWidget *rWidget;
    QVBoxLayout *vlLeft;
    QVBoxLayout *vlStart;
    QVBoxLayout *vlRelease;
    QVBoxLayout *vlDeflate;
    QVBoxLayout *vlResult;
    QVBoxLayout *vlRight;
    QSpacerItem *vSpace1;
    QSpacerItem *vSpace2;
    QSpacerItem *vSpace3;
    QSpacerItem *vSpace4;
    QSpacerItem *vSpace5;
    QSpacerItem *vSpace6;
    QTextBrowser *ibStart;
    QTextBrowser *infoBox;
    QLabel *infoLabel;
    QwtDial *meter;
    QPushButton *btnStart;
    QLabel *lTitlePlotRaw;
    QLabel *lTitlePlotOsc;
    Plot *pltPre;
    Plot *pltOsc;
    QFrame *line;
    QMenuBar *menubar;
    QStatusBar *statusbar;

private slots:

    void clkBtn1();
    void clkBtn2();
    void clkBtn3();
    void clkBtn4();
    void clkBtn5();
};


#endif //OBP_WINDOW_H
