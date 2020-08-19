/**
 * @file        Window.h
 * @brief       The header file of the Window class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * Defines the Window class and contains the general class description.
 */

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
#include "InfoDialog.h"
/**
 * Class dependant configuration values:
 */
#define SCREEN_UPDATE_MS 50  //!< Screen update rate in ms.


//! The Window class is the implementation of the graphical user interface (GUI).
/*!
 * The Window class inherits from QMainWindow and IObserver. The QMainWindow makes the class an executable Qt window,
 * with Qt taking care of updating the GUI, generating events for button clicks and so on. The IObserver makes the
 * class able to be registered as an observer for a subject that will send notifications to the observer. All GUI
 * elements are set up in the constructor of Window, including the settings and info plane. However, they will only
 * be shown if necessary. I.e. what page of the user instructions is displayed is defined by the state of the screen
 * enum (currentScreen).
 *
 * The Window object is instantiated with a reference to a Processing object in the constructor. This is necessary
 * so the user inputs can be relayed and the Processing thread can be stopped when the window is closed, and the
 * application exited.
 */
class Window : public QMainWindow, public IObserver
{
Q_OBJECT
public:
    Window(Processing *process, QWidget *parent = 0);
    ~Window();


protected:
    void timerEvent(QTimerEvent *e) override;
private:
    // Callbacks from observable class, need to be implemented in a thread safe way:
    void eNewData(double pData, double oData) override;
    void eSwitchScreen(Screen eNewScreen) override;
    void eResults(double map, double sbp, double dbp) override;
    void eHeartRate(double map) override;
    void eReady() override;

    // Setting up the UI:
    void setupUi(QMainWindow *window);
    QMenuBar *setupMenu(QWidget *parent);
    QWidget *setupPlots(QWidget *parent);
    QWidget *setupStartPage(QWidget *parent);
    QWidget *setupInflatePage(QWidget *parent);
    QWidget *setupDeflatePage(QWidget *parent);
    QWidget *setupEmptyCuffPage(QWidget *parent);
    QWidget *setupResultPage(QWidget *parent);
    QWidget *setupSettingsDialog(QWidget *parent);
    QWidget *setupInfoDialogue(QWidget *parent);
    void retranslateUi(QMainWindow *MainWindow);

    // Settings:
    void loadSettings();
    Processing *process;
    double xData[MAX_DATA_LENGTH],    //!< X-axis of the plot data (time)
    yLPData[MAX_DATA_LENGTH],         //!< Y-axis of the low-pass filtered pressure data.
    yHPData[MAX_DATA_LENGTH];         //!< Y-axis of the high-pass filtered data.
    int dataLength;                   //!< Length of the shown data. Possibility to change zoom.
    int pumpUpVal;                    //!< Pump-up value used to display required pressure.

    // Variables that are changed from outside the UI are made
    // atomic, so access to them is thread safe.
    std::atomic<Screen> currentScreen;

    std::mutex mtxPlt;                //!< mutex for thread save access to the plots:

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
    QSpacerItem *vSpace7;
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
    QLabel *lMeasured;
    QLabel *lMAP;
    QLabel *lMAPval;
    QLabel *lEstimated;
    QLabel *lSBP;
    QLabel *lSBPval;
    QLabel *lDBP;
    QLabel *lDBPval;
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
    InfoDialog *infoDialogue;

private slots:
    void clkBtnStart();
    void clkBtnCancel();
    void clkBtnReset();
    void on_actionInfo_triggered();
    void on_actionSettings_triggered();
    void on_actionExit_triggered();
    void updateValues();
    void resetValuesPerform();
};


#endif //OBP_WINDOW_H
