#include "Window.h"
#include <qwt/qwt_dial_needle.h>
#include <iostream>
#include <QtCore/QSettings>

/**
 * The constructor of the Window Class.
 *
 * Needs a Processing object as an argument. It is used to send user input to the Processing Class.
 * Additionally, a QWidget can be passed as an argument to act as the classes parent widget.
 * @param process The Processing class to handle the user inputs.
 * @param parent  The QWidget that is the parent (default 0).
 */
Window::Window(Processing *process, QWidget *parent) :
        dataLength(MAX_DATA_LENGTH),
        process(process),
        QMainWindow(parent) {

    for (int i = 0; i < MAX_DATA_LENGTH; i++) {
        xData[i] = (double) (MAX_DATA_LENGTH - i) / (double) SAMPLING_RATE;     // time axis in seconds
        yLPData[i] = 0;
        yHPData[i] = 0;
    }

    std::lock_guard<std::mutex> guard(pltMtx); //automatically releases mutex when control leaves scope.
    currentScreen = Screen::startScreen;
    setupUi(this);

    // TODO: read settings and potentially set them in Processing.
    // Generate timer event every 50ms to update the window
    (void) startTimer(50);
}

/**
 * The destructor of the Window class.
 *
 * Stops the process thread if window is closed.
 */
Window::~Window() {

    PLOG_VERBOSE << "Cleanup:";
    process->stopThread();
    process->join();
    PLOG_VERBOSE << "Application terminated.";
}

/**
 * This functions is to be called at the initialisation of the object.
 * It builds the whole user interface.
 * @param window  A reference to the parent object.
 */
void Window::setupUi(QMainWindow *window) {
    if (window->objectName().isEmpty())
        window->setObjectName(QString::fromUtf8("MainWindow"));

    QIcon icon(QIcon::fromTheme(QString::fromUtf8("Main")));
    window->resize(1920, 1080);
    window->setAcceptDrops(true);
    window->setWindowIcon(icon);
    window->setTabShape(QTabWidget::Triangular);

    splitter = new QSplitter(window);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setOrientation(Qt::Horizontal);
    splitter->setHandleWidth(5);
    splitter->setChildrenCollapsible(false);

    // The left side is a stacked widget with several pages in a vertical box
    // layout with some permanent information on the bottom.
    lWidget = new QWidget(splitter);
    vlLeft = new QVBoxLayout(lWidget);
    vlLeft->setObjectName(QString::fromUtf8("vlLeft"));
    lInstructions = new QStackedWidget(lWidget);
    lInstructions->setMinimumWidth(500);

    btnCancel = new QPushButton(lWidget);
    btnCancel->setObjectName(QString::fromUtf8("btnCancel"));

    lMeter = new QLabel(lWidget);
    lMeter->setObjectName(QString::fromUtf8("lMeter"));
    lMeter->setAlignment(Qt::AlignCenter);
    meter = new QwtDial(lWidget);
    meter->setObjectName(QString::fromUtf8("meter"));
    meter->setUpperBound(260.0);
    meter->setScaleStepSize(20.0);
    meter->setWrapping(false);
    meter->setInvertedControls(false);
    meter->setLineWidth(4);
    meter->setMode(QwtDial::RotateNeedle);
    meter->setMinScaleArc(20.0);
    meter->setMaxScaleArc(300.0);
    meter->setMinimumSize(400, 400);
    needle = new QwtDialSimpleNeedle(
            QwtDialSimpleNeedle::Arrow, true, Qt::black,
            QColor(Qt::gray).lighter(130));
    meter->setNeedle(needle);

    // Build pages and add them to the instructions panel
    lInstructions->addWidget(setupStartPage(lInstructions));
    lInstructions->addWidget(setupInflatePage(lInstructions));
    lInstructions->addWidget(setupDeflatePage(lInstructions));
    lInstructions->addWidget(setupEmptyCuffPage(lInstructions));
    lInstructions->addWidget(setupResultPage(lInstructions));

    // Add the instructions panel to the splitter
    vlLeft->addWidget(lMeter);
    vlLeft->addWidget(meter);
    vlLeft->addWidget(lInstructions);
    vlLeft->addWidget(btnCancel);
    btnCancel->hide();
    splitter->addWidget(lWidget);

    // Build and add the plot panel to the splitter
    splitter->addWidget(setupPlots(splitter));
    // Set stretch factor of left part to zero so it will not resize
    splitter->setStretchFactor(0, 0);

    // Add splitter to main window.
    window->setCentralWidget(splitter);

    window->setMenuBar(setupMenu(window));

    statusbar = new QStatusBar(window);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    window->setStatusBar(statusbar);

    // Settings Dialog
    setupSettingsDialog(window);

    // Set all text fields in one place.
    retranslateUi(window);

    // Connect UI events (slots)
    QMetaObject::connectSlotsByName(window);

    // Set default look, specify percentage of left side:
    double leftSide = 0.3;
    QList<int> Sizes({(int) (leftSide * width()), (int) ((1.0 - leftSide) * width())});
    splitter->setSizes(Sizes);


    //TODO: only for now to be able to switch between the pages of the stacked widget
    auto *but0 = new QPushButton("0");
    auto *but1 = new QPushButton("1");
    auto *but2 = new QPushButton("2");
    auto *but3 = new QPushButton("3");
    auto *but4 = new QPushButton("4");

    statusbar->addPermanentWidget(but0);
    statusbar->addPermanentWidget(but1);
    statusbar->addPermanentWidget(but2);
    statusbar->addPermanentWidget(but3);
    statusbar->addPermanentWidget(but4);
    auto *spacerbnt = new QLabel(); // fake spacer
    statusbar->addPermanentWidget(spacerbnt, 1);

    connect(btnStart, SIGNAL (released()), this, SLOT (clkBtnStart()));
    connect(btnCancel, SIGNAL (released()), this, SLOT (clkBtnCancel()));
    connect(btnReset, SIGNAL (released()), this, SLOT (clkBtnReset()));

    connect(settingsDialog, SIGNAL(accepted()), this, SLOT(updateValues()));
    connect(settingsDialog, SIGNAL(resetValues()), this, SLOT(resetValuesPerform()));

    //TODO: remove at the end
    connect(but0, SIGNAL (released()), this, SLOT (clkBtn1()));
    connect(but1, SIGNAL (released()), this, SLOT (clkBtn2()));
    connect(but2, SIGNAL (released()), this, SLOT (clkBtn3()));
    connect(but3, SIGNAL (released()), this, SLOT (clkBtn4()));
    connect(but4, SIGNAL (released()), this, SLOT (clkBtn5()));

}

/**
 * Sets up the menu bar to access Settings, Info and Exit.
 * @param parent A reference to the parent widget to set for this page.
 * @return A reference to the generated menu bar.
 */
QMenuBar *Window::setupMenu(QWidget *parent) {
    actionSettings = new QAction(parent);
    actionSettings->setObjectName(QString::fromUtf8("actionSettings"));
    actionInfo = new QAction(parent);
    actionInfo->setObjectName(QString::fromUtf8("actionInfo"));
    actionExit = new QAction(parent);
    actionExit->setObjectName(QString::fromUtf8("actionExit"));

    menubar = new QMenuBar(parent);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menuMenu = new QMenu(menubar);
    menuMenu->setObjectName(QString::fromUtf8("menuMenu"));

    menubar->addAction(menuMenu->menuAction());
    menuMenu->addAction(actionSettings);
    menuMenu->addAction(actionInfo);
    menuMenu->addSeparator();
    menuMenu->addAction(actionExit);
    return menubar;
}
//TODO: make plots a bit nicer.
/**
 * Sets up the page with two plots to display the data.
 *
 * Both plots have time as the x axis.
 * The first plot displays low pass filtered pressure data the y axis is in
 * mmHg pressure.
 * The second plot displays the high pass filtered oscillation data, it is in
 * arbitrary units.
 *
 * @param parent A reference to the parent widget to set for this page.
 * @return A reference to the generated page.
 */
QWidget *Window::setupPlots(QWidget *parent) {
    rWidget = new QWidget(parent);

    vlRight = new QVBoxLayout();
    vlRight->setObjectName(QString::fromUtf8("vlRight"));

    lTitlePlotRaw = new QLabel(parent);
    lTitlePlotRaw->setObjectName(QString::fromUtf8("lTitlePlotRaw"));
    lTitlePlotOsc = new QLabel(parent);
    lTitlePlotOsc->setObjectName(QString::fromUtf8("lTitlePlotOsc"));

    line = new QFrame(parent);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    pltPre = new Plot(xData, yLPData, dataLength, 250, 0.0, parent);
    pltPre->setObjectName(QString::fromUtf8("pltPre"));
    pltOsc = new Plot(xData, yHPData, dataLength, 3, -3, parent);
    pltOsc->setObjectName(QString::fromUtf8("pltOsc"));

    // build right side of window
    vlRight->addWidget(lTitlePlotRaw);
    vlRight->addWidget(pltPre);
    vlRight->addWidget(line);
    vlRight->addItem(vSpace5);
    vlRight->addWidget(lTitlePlotOsc);
    vlRight->addWidget(pltOsc);
    rWidget->setLayout(vlRight);

    return rWidget;

}

/**
 * Sets up the start page with instructions.
 *
 * A button on the button can start the measurements.
 *
 * @param parent A reference to the parent widget to set for this page.
 * @return A reference to the generated page.
 */
QWidget *Window::setupStartPage(QWidget *parent) {
    lInstrStart = new QWidget(parent);

    vlStart = new QVBoxLayout();
    vlStart->setObjectName(QString::fromUtf8("vlStart"));

    lInfoStart = new QLabel(parent);
    lInfoStart->setObjectName(QString::fromUtf8("lInfoStart"));
    lInfoStart->setWordWrap(true);
    lInfoStart->setAlignment(Qt::AlignCenter);

    btnStart = new QPushButton(parent);
    btnStart->setObjectName(QString::fromUtf8("btnStart"));
    btnStart->setDisabled(true);

    vSpace4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlStart->addItem(vSpace4);
    vlStart->addWidget(lInfoStart);
    vlStart->addItem(vSpace6);
    vlStart->addWidget(btnStart);

    lInstrStart->setLayout(vlStart);
    return lInstrStart;

}

/**
 * Sets up the inflation page with instructions.
 *
 * Instructs the user to pump up the cuff.
 *
 * @param parent A reference to the parent widget to set for this page.
 * @return A reference to the generated page.
 */
QWidget *Window::setupInflatePage(QWidget *parent) {

    lInstrPump = new QWidget(parent);

    // Layout for this page:
    vlInflate = new QVBoxLayout();
    vlInflate->setObjectName(QString::fromUtf8("vlInflate"));

    vSpace1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    lInfoPump = new QLabel(parent);
    lInfoPump->setObjectName(QString::fromUtf8("lInfoPump"));
    lInfoPump->setWordWrap(true);
    lInfoPump->setAlignment(Qt::AlignCenter);

    // build left side of window
    vlInflate->addItem(vSpace1);
    vlInflate->addWidget(lInfoPump);
    vlInflate->addItem(vSpace2);

    lInstrPump->setLayout(vlInflate);
    return lInstrPump;
}

/**
 * Sets up the inflation page with instructions.
 *
 * Instructs the user to pump up the cuff.
 *
 * @param parent A reference to the parent widget to set for this page.
 * @return A reference to the generated page.
 */
QWidget *Window::setupDeflatePage(QWidget *parent) {

    lInstrRelease = new QWidget(parent);

    vlRelease = new QVBoxLayout();
    lInfoRelease = new QLabel(parent);
    lInfoRelease->setObjectName(QString::fromUtf8("lInfoRelease"));
    lInfoRelease->setWordWrap(true);
    lInfoRelease->setAlignment(Qt::AlignCenter);
    lheartRate = new QLabel(parent);
    lheartRate->setObjectName(QString::fromUtf8("lheartRate"));
    lheartRate->setAlignment(Qt::AlignCenter);

    vSpace4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace6 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vlRelease->addItem(vSpace6);
    vlRelease->addWidget(lInfoRelease);
    vlRelease->addItem(vSpace4);
    vlRelease->addWidget(lheartRate);

    lInstrRelease->setLayout(vlRelease);
    return lInstrRelease;
}
/**
 * Sets up the empty cup page with instructions.
 *
 * Instructs the user to deflate the cuff completely.
 *
 * @param parent A reference to the parent widget to set for this page.
 * @return A reference to the generated page.
 */
QWidget *Window::setupEmptyCuffPage(QWidget *parent) {
    lInstrDeflate = new QWidget(parent);

    vlDeflate = new QVBoxLayout();
    lInfoDeflate = new QLabel(parent);
    lInfoDeflate->setObjectName(QString::fromUtf8("lInfoDeflate"));
    lInfoDeflate->setWordWrap(true);
    lInfoDeflate->setAlignment(Qt::AlignCenter);

    vlDeflate->addWidget(lInfoDeflate);
    lInstrDeflate->setLayout(vlDeflate);
    return lInstrDeflate;
}
/**
 * Sets up the results page.
 *
 * Displays the calculated results.
 *
 * @param parent A reference to the parent widget to set for this page.
 * @return A reference to the generated page.
 */
QWidget *Window::setupResultPage(QWidget *parent) {
    lInstrResult = new QWidget(parent);

    vlResult = new QVBoxLayout();
    lInfoResult = new QLabel(parent);
    lInfoResult->setObjectName(QString::fromUtf8("lInfoDeflate"));
    lInfoResult->setWordWrap(true);
    lInfoResult->setAlignment(Qt::AlignCenter);

    btnReset = new QPushButton(parent);
    btnReset->setObjectName(QString::fromUtf8("btnReset"));

    flResults = new QFormLayout();
    flResults->setObjectName(QString::fromUtf8("flResults"));
    lMAP = new QLabel(parent);
    lMAP->setObjectName(QString::fromUtf8("lMAP"));
    lMAPval = new QLabel(parent);
    lMAPval->setObjectName(QString::fromUtf8("lMAPval"));
    lSBP = new QLabel(parent);
    lSBP->setObjectName(QString::fromUtf8("lSBP"));
    lSBPval = new QLabel(parent);
    lSBPval->setObjectName(QString::fromUtf8("lSBPval"));
    lCBP = new QLabel(parent);
    lCBP->setObjectName(QString::fromUtf8("lCBP"));
    lDBPval = new QLabel(parent);
    lDBPval->setObjectName(QString::fromUtf8("lDBPval"));
    lheartRateAV = new QLabel(parent);
    lheartRateAV->setObjectName(QString::fromUtf8("lheartRateAV"));
    lheartRateAV->setMinimumWidth(250);
    lHRvalAV = new QLabel(parent);
    lHRvalAV->setObjectName(QString::fromUtf8("lHRvalAV"));
    lHRvalAV->setMinimumWidth(150);

    flResults->setWidget(0, QFormLayout::LabelRole, lMAP);
    flResults->setWidget(0, QFormLayout::FieldRole, lMAPval);
    flResults->setWidget(1, QFormLayout::LabelRole, lSBP);
    flResults->setWidget(1, QFormLayout::FieldRole, lSBPval);
    flResults->setWidget(2, QFormLayout::LabelRole, lCBP);
    flResults->setWidget(2, QFormLayout::FieldRole, lDBPval);
    flResults->setWidget(3, QFormLayout::LabelRole, lheartRateAV);
    flResults->setWidget(3, QFormLayout::FieldRole, lHRvalAV);

    vlResult->addWidget(lInfoResult);
    vlResult->addLayout(flResults);
    vlResult->addWidget(btnReset);
    lInstrResult->setLayout(vlResult);

    return lInstrResult;
}
/**
 * Sets up the dialog for the setting.
 *
 * The user can adjust parameters, they will be saved when the application is closed.
 *
 * @param parent A reference to the parent widget to set for this page.
 * @return A reference to the generated page.
 */
QWidget *Window::setupSettingsDialog(QWidget *parent) {

    settingsDialog = new SettingsDialog(parent);
    loadSettings();
    return settingsDialog;
}
/**
 * Sets all the text in the main window.
 * @param window A pointer to the main window.
 */
void Window::retranslateUi(QMainWindow *window) {
    window->setWindowTitle("Oscillometric Blood Pressure Measurement");

    lInfoStart->setText("<b>Prepare the measurement:</b><br><br>"
                        "1. Put the cuff on your upper arm of your nondominant hand, making sure it is tight.<br>"
                        "2. Rest your arm on a flat surface.<br>"
                        "3. Take the pump into your dominant hand.<br>"
                        "4. Make sure the valve is closed, but you can handle it easily.<br>"
                        "5. Press Start when you are ready.");
    //"<br><br> <i>Picture missing</i><br>"
    lInfoPump->setText("<b>Pump Up to 180 mmHg</b><br><br>"
                       "Using your dominant hand, where your arm is not in the cuff, quickly pump up the cuff to 180 mmHg.<br>"
                       "Make sure the valve is fully closed.<br>"
                       "Use the dial above for reference.");
    lInfoRelease->setText("<b>Slowly release pressure at 3 mmHg/s</b><br><br>"
                          "Open the valve slightly to release pressure at about 3 mmHg per second."
                          "Wait calmly and try not to move. <br><br>");
    //"<i>Add deflation feedback. Possibly have meter here, too.</i>"
    lInfoDeflate->setText("<b>Completely open the valve.</b><br><br>"
                          "Wait for the pressure to go down to 0 mmHg.<br><br>"
                          "You will see the results next.");
    lInfoResult->setText("<b>Results:</b><br><br>"
                         "Click Reset to start a new measurement<br>");

    lMeter->setText("<b>Pressure in mmHg:</b>");
    btnStart->setText("Start");
    btnReset->setText("Reset");
    btnCancel->setText("Cancel");
    lMAP->setText("<b><font color=\"red\">MAP:</font></b>");
    lMAPval->setText("- mmHg");
    lSBP->setText("<font color=\"blue\">SBP:</font>");
    lSBPval->setText("- mmHg");
    lCBP->setText("<font color=\"green\">DBP:</font>");
    lDBPval->setText("- mmHg");
    lheartRate->setText("Current heart rate:<br><b>--</b>");
    lheartRateAV->setText("Average heart rate:");
    lHRvalAV->setText("- beats/min");

    actionSettings->setText("Settings");
    actionInfo->setText("Info");
    actionExit->setText("Exit");
    menuMenu->setTitle("Menu");

}

/**
 * Handles the timer event to update the UI.
 */
void Window::timerEvent(QTimerEvent *) {
    pltMtx.lock();
    pltOsc->replot();
    pltPre->replot();
    pltMtx.unlock();
}

/**
 * Handles notifications about a new data pair.
 * @param pData The newly available pressure data.
 * @param oData The newly available oscillation data.
 */
void Window::eNewData(double pData, double oData) {
    pltMtx.lock();
    pltPre->setNewData(pData);
    pltOsc->setNewData(oData);
    pltMtx.unlock();

    bool bOk = QMetaObject::invokeMethod(meter, "setValue", Qt::QueuedConnection, Q_ARG(double, pData));
    assert(bOk);
}
/**
 * Handles notifications to switch the displayed screen.
 * @param eNewScreen The new screen to display.
 */
void Window::eSwitchScreen(Screen eNewScreen) {
    bool bOk = false;
    switch (eNewScreen) {
        case Screen::startScreen:
            bOk = QMetaObject::invokeMethod(btnCancel, "hide", Qt::QueuedConnection);
            assert(bOk);
            bOk = QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                            Q_ARG(int, 0));
            assert(bOk);
            break;
        case Screen::inflateScreen:
            bOk = QMetaObject::invokeMethod(btnCancel, "show", Qt::QueuedConnection);
            assert(bOk);
            bOk = QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                            Q_ARG(int, 1));
            assert(bOk);
            break;
        case Screen::deflateScreen:
            bOk = QMetaObject::invokeMethod(btnCancel, "show", Qt::QueuedConnection);
            assert(bOk);
            bOk = QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                            Q_ARG(int, 2));
            assert(bOk);
            break;
        case Screen::emptyCuffScreen:
            bOk = QMetaObject::invokeMethod(btnCancel, "show", Qt::QueuedConnection);
            assert(bOk);
            bOk = QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                            Q_ARG(int, 3));
            assert(bOk);
            break;
        case Screen::resultScreen:
            bOk = QMetaObject::invokeMethod(btnCancel, "hide", Qt::QueuedConnection);
            assert(bOk);
            bOk = QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                            Q_ARG(int, 4));
            assert(bOk);
            break;
    }
    currentScreen = eNewScreen;
}

/**
 * Handles notifications for the results.
 * @param map The determined value for mean arterial pressure.
 * @param sbp The determined value for systolic blood pressure.
 * @param dbp The determined value for diastolic blood pressure.
 */
void Window::eResults(double map, double sbp, double dbp) {
    bool bOk = QMetaObject::invokeMethod(lMAPval, "setText", Qt::QueuedConnection,
                                         Q_ARG(QString, (QString::number(map, 'f', 0) + " mmHg")));
    assert(bOk);
    bOk = QMetaObject::invokeMethod(lSBPval, "setText", Qt::QueuedConnection,
                                    Q_ARG(QString, QString::number(sbp, 'f', 0) + " mmHg"));
    assert(bOk);
    bOk = QMetaObject::invokeMethod(lDBPval, "setText", Qt::QueuedConnection,
                                    Q_ARG(QString, QString::number(dbp, 'f', 0) + " mmHg"));
    assert(bOk);
}

/**
 * Handles notificaions about heart rate and displays them in the appropriate location in the UI.
 * @param heartRate The latest heart rate value. Can either be a current or an average heart rate value.
 */
void Window::eHeartRate(double heartRate) {

    bool bOk = QMetaObject::invokeMethod(lheartRate, "setText", Qt::QueuedConnection,
                                         Q_ARG(QString, "Current heart rate:<br><b>" +
                                                        QString::number(heartRate, 'f', 0) + "</b>"));
    assert(bOk);
    bOk = QMetaObject::invokeMethod(lHRvalAV, "setText", Qt::QueuedConnection,
                                    Q_ARG(QString, QString::number(heartRate, 'f', 0) + " beats/min"));
    assert(bOk);
}

/**
 * Handles the notification that the observed class is ready.
 */
void Window::eReady() {
    // Instead of :
    // btnStart->setDisabled(false);
    // The QMetaObject::invokeMethod is used with a Qt::QueuedConnection.
    // The button is set enabled whenever the UI thread is ready.setDisabled
    bool bOk = QMetaObject::invokeMethod(btnStart, "setDisabled", Qt::QueuedConnection,
                                         Q_ARG(bool, false));
    // Checks that function call is valid during development. Do not put function inside assert,
    // because it will be removed in release build!
    assert(bOk);
}

void Window::clkBtnStart() {
    process->startMeasurement();
}

void Window::clkBtnCancel() {
    process->stopMeasurement();
}

void Window::clkBtnReset() {
    process->stopMeasurement();
}

//TODO: remove those after debugging
void Window::clkBtn1() {
    eSwitchScreen(Screen::startScreen);
}

void Window::clkBtn2() {
    eSwitchScreen(Screen::inflateScreen);
}

void Window::clkBtn3() {
    eSwitchScreen(Screen::deflateScreen);
}

void Window::clkBtn4() {
    eSwitchScreen(Screen::emptyCuffScreen);

}

void Window::clkBtn5() {
    eSwitchScreen(Screen::resultScreen);
}

/**
 * This function is called when the menu entry "Info" is pressed.
 *
 * The name of this function (slot) ensures automatic connection with the menu entry
 * actionInfo.
 */
void Window::on_actionInfo_triggered() {

}

/**
 * This function is called when the menu entry "Settings" is pressed.
 *
 * The name of this function (slot) ensures automatic connection with the menu entry
 * actionSettings.
 */
void Window::on_actionSettings_triggered() {
    settingsDialog->setModal(true);
    settingsDialog->show();
}

/**
 * This function is called when the menu entry "Exit" is pressed.
 *
 * The name of this function (slot) ensures automatic connection with the menu entry
 * actionExit.
 */
void Window::on_actionExit_triggered() {
    QApplication::quit();
}

/**
 * loads the settings from the settings file if there is one, otherwise default values are displayed.
 */
void Window::loadSettings() {
    // For every value: get the values from settings and get the default value from Processing.
    // Then set both the value in the settings dialog and in Processing with what was stored in the settings.
    // Changing the values in Processing only works at startup, before the process is running.

    int iVal;
    double dVal;
    QSettings settings; //Saves setting platform independent.
    dVal = settings.value("ratioSBP", process->getRatioSBP()).toDouble();
    settingsDialog->setRatioSBP(dVal);
    process->setRatioSBP(dVal);

    dVal = settings.value("ratioDBP", process->getRatioDBP()).toDouble();
    settingsDialog->setRatioDBP(dVal);
    process->setRatioDBP(dVal);

    iVal = settings.value("minNbrPeaks", process->getMinNbrPeaks()).toInt();
    settingsDialog->setMinNbrPeaks(iVal);
    process->setMinNbrPeaks(iVal);

    iVal = settings.value("pumpUpValue", process->getPumpUpValue()).toInt();
    settingsDialog->setPumpUpValue(iVal);
    process->setPumpUpValue(iVal);

}
/**
 * Does only update the values in the settings file. They will not be applied until the
 * application is restarted.
 */
void Window::updateValues() {
    QSettings settings;
    settings.setValue("ratioSBP", settingsDialog->getRatioSBP());
    settings.setValue("ratioDBP", settingsDialog->getRatioDBP());
    settings.setValue("minNbrPeaks", settingsDialog->getMinNbrPeaks());
    settings.setValue("pumpUpValue", settingsDialog->getPumpUpValue());
}

/**
 * Resets all values both in the application and in the settings file.
 * Changes take effect immediately.
 */
void Window::resetValuesPerform() {
    process->resetConfigValues();
    QSettings settings;
    settings.setValue("ratioSBP", process->getRatioSBP());
    settings.setValue("ratioDBP", process->getRatioDBP());
    settings.setValue("minNbrPeaks", process->getMinNbrPeaks());
    settings.setValue("pumpUpValue", process->getPumpUpValue());
    // Reload the values from settings also writes them to the settings dialog:
    loadSettings();
}