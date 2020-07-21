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
#include <QtWidgets/QFormLayout>
#include "IObserver.h"
#include "Plot.h"
#include "common.h"
#include "Processing.h"


class Window : public QMainWindow, public IObserver{
Q_OBJECT
public:
    Window(Processing *process, QWidget *parent = 0);
    ~Window();


protected:
    void timerEvent(QTimerEvent *e) override;
private:
    void eNewData(double pData, double oData) override;
    void eSwitchScreen(Screen eScreen) override;
    void eResults(double map, double sbp, double dbp) override;
    void eHeartRate(double map) override;
    void eReady() override;

    void setupUi(QMainWindow *window);
    QWidget * setupPlots(QWidget *parent);
    QWidget * setupStartPage(QWidget *parent);
    QWidget * setupInflatePage(QWidget *parent);
    QWidget * setupDeflatePage(QWidget *parent);
    QWidget * setupEmptyCuffPage(QWidget *parent);
    QWidget * setupResultPage(QWidget *parent);

    void retranslateUi(QMainWindow *MainWindow);

    Processing *process;
    double xData[MAX_DATA_LENGTH], yLPData[MAX_DATA_LENGTH], yHPData[MAX_DATA_LENGTH];
    int dataLength;// TODO: needed ?
    double valHeartRate;
    Screen currentScreen;


    QSplitter *splitter;
    QStackedWidget *lInstructions;
    QWidget *lInstrStart;
    QWidget *lInstrPump;
    QWidget *lInstrRelease;
    QWidget *lInstrDeflate;
    QWidget *lInstrResult;
    QWidget *rWidget;
    QVBoxLayout *vlStart;
    QVBoxLayout *vlLeft;
    QVBoxLayout *vlRelease;
    QVBoxLayout *vlDeflate;
    QVBoxLayout *vlResult;
    QFormLayout *flResults;
    QVBoxLayout *vlRight;
    QSpacerItem *vSpace1;
    QSpacerItem *vSpace2;
    QSpacerItem *vSpace3;
    QSpacerItem *vSpace4;
    QSpacerItem *vSpace5;
    QSpacerItem *vSpace6;
    QLabel *lInfoStart;
    QLabel *lInfoPump;
    QLabel *lInfoRelease;
    QLabel *lInfoDeflate;
    QLabel *lInfoResult;
    QwtDial *meter;
    QPushButton *btnStart;
    QPushButton *btnReset;
    QLabel *lheartRate;
    QLabel *lheartRateAV;
    QLabel *lHRvalAV;
    QLabel *lMAP;
    QLabel *lMAPval;
    QLabel *lSBP;
    QLabel *lSBPval;
    QLabel *lCBP;
    QLabel *lDBPval;
    QLabel *lTitlePlotRaw;
    QLabel *lTitlePlotOsc;
    Plot *pltPre;
    Plot *pltOsc;
    QFrame *line;
    QMenuBar *menubar;
    QStatusBar *statusbar;

private slots:
    void clkBtnStart();
    void clkBtnReset();
    void clkBtn1();
    void clkBtn2();
    void clkBtn3();
    void clkBtn4();
    void clkBtn5();

};


#endif //OBP_WINDOW_H
