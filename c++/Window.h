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
#include <mutex>
#include "common.h"
#include "IObserver.h"
#include "Plot.h"
#include "Processing.h"
#include "SettingsDialog.h"

//! The Window Class handles the user interface (UI).
/*!
 * The UI consists of ... 
 */
class Window : public QMainWindow, public IObserver{
Q_OBJECT
public:
    Window(Processing *process, QWidget *parent = 0);
    ~Window();


protected:
    void timerEvent(QTimerEvent *e) override;
private:
    // Callbacks from observable class, need to be implemented
    // in a thread safe way:
    void eNewData(double pData, double oData) override;
    void eSwitchScreen(Screen eNewScreen) override;
    void eResults(double map, double sbp, double dbp) override;
    void eHeartRate(double map) override;
    void eReady() override;

    // Setting up the UI:
    void setupUi(QMainWindow *window);
    QMenuBar*  setupMenu(QWidget *parent);
    QWidget * setupPlots(QWidget *parent);
    QWidget * setupStartPage(QWidget *parent);
    QWidget * setupInflatePage(QWidget *parent);
    QWidget * setupDeflatePage(QWidget *parent);
    QWidget * setupEmptyCuffPage(QWidget *parent);
    QWidget * setupResultPage(QWidget *parent);
    QWidget * setupSettingsDialog(QWidget *parent);
    void retranslateUi(QMainWindow *MainWindow);

    // Settings:
    void loadSettings();
    Processing *process;
    double xData[MAX_DATA_LENGTH], yLPData[MAX_DATA_LENGTH], yHPData[MAX_DATA_LENGTH];
    int dataLength;

    // Variables that are changed from outside the UI are made
    // atomic, so access to them is thread safe.
    std::atomic<Screen> currentScreen;

    // mutex for thread save access to the plots:
    std::mutex pltMtx;

    // UI components:
    QSplitter *splitter;
    QVBoxLayout *vlLeft;
    QStackedWidget *lInstructions;
    QWidget *lInstrStart;
    QWidget *lInstrPump;
    QWidget *lInstrRelease;
    QWidget *lInstrDeflate;
    QWidget *lInstrResult;
    QWidget *lWidget;
    QWidget *rWidget;
    QVBoxLayout *vlStart;
    QVBoxLayout *vlInflate;
    QVBoxLayout *vlRelease;
    QVBoxLayout *vlDeflate;
    QVBoxLayout *vlResult;
    QFormLayout *flResults;
    QVBoxLayout *vlRight;
    QSpacerItem *vSpace1;
    QSpacerItem *vSpace2;
    QSpacerItem *vSpace4;
    QSpacerItem *vSpace5;
    QSpacerItem *vSpace6;
    QLabel *lMeter;
    QLabel *lInfoStart;
    QLabel *lInfoPump;
    QLabel *lInfoRelease;
    QLabel *lInfoDeflate;
    QLabel *lInfoResult;
    QwtDial *meter;
    QwtDialNeedle *needle;
    QPushButton *btnStart;
    QPushButton *btnReset;
    QPushButton *btnCancel;
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
    QMenu *menuMenu;
    QAction *actionSettings;
    QAction *actionInfo;
    QAction *actionExit;
    SettingsDialog *settingsDialog;


private slots:
    void clkBtnStart();
    void clkBtnCancel();
    void clkBtnReset();
    void on_actionInfo_triggered();
    void on_actionSettings_triggered();
    void on_actionExit_triggered();
    void clkBtn1();
    void clkBtn2();
    void clkBtn3();
    void clkBtn4();
    void clkBtn5();
    void updateValues();
    void resetValuesPerform();
//private signal:
//    void setMAPText(const QString &);
};



#endif //OBP_WINDOW_H
