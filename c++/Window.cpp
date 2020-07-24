#include "Window.h"
#include <qwt/qwt_dial_needle.h>

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

    //TODO: splitter more visible.
    //TODO: minimum width for left and right side
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

    // TODO: menubar working? just missing content?
    menubar = new QMenuBar(window);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 2081, 39));
    window->setMenuBar(menubar);
    statusbar = new QStatusBar(window);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    window->setStatusBar(statusbar);

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

    connect(btnStart, SIGNAL (clicked()), this, SLOT (clkBtnStart()));
    connect(btnCancel, SIGNAL (released()), this, SLOT (clkBtnCancel()));
    connect(btnReset, SIGNAL (released()), this, SLOT (clkBtnReset()));

//    QObject::connect(this, SIGNAL(setMAPText(const QString &)), lMAPval, SLOT(setText(const QString &)), Qt::QueuedConnection);
    connect(but0, SIGNAL (released()), this, SLOT (clkBtn1()));
    connect(but1, SIGNAL (released()), this, SLOT (clkBtn2()));
    connect(but2, SIGNAL (released()), this, SLOT (clkBtn3()));
    connect(but3, SIGNAL (released()), this, SLOT (clkBtn4()));
    connect(but4, SIGNAL (released()), this, SLOT (clkBtn5()));

}

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
    pltOsc = new Plot(xData, yHPData, dataLength, 0.003, -0.003, parent);
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


void Window::retranslateUi(QMainWindow *window) {
    window->setWindowTitle(QApplication::translate("TestWindow", "TestWindow", nullptr));

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

}


void Window::timerEvent(QTimerEvent *) {
    pltMtx.lock();
    pltOsc->replot();
    pltPre->replot();
    pltMtx.unlock();
}

void Window::eNewData(double pData, double oData) {
    pltMtx.lock();
    pltPre->setNewData(pData);
    pltOsc->setNewData(oData);
    pltMtx.unlock();

    QMetaObject::invokeMethod(meter, "setValue", Qt::QueuedConnection, Q_ARG(double, pData));
}

void Window::eSwitchScreen(Screen eNewScreen) {
    switch (eNewScreen) {
        case Screen::startScreen:
            QMetaObject::invokeMethod(btnCancel, "hide", Qt::QueuedConnection);
            QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                      Q_ARG(int, 0));
            btnCancel->hide();
            break;
        case Screen::inflateScreen:
            QMetaObject::invokeMethod(btnCancel, "show", Qt::QueuedConnection);
            QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                      Q_ARG(int, 1));
            break;
        case Screen::deflateScreen:
            QMetaObject::invokeMethod(btnCancel, "show", Qt::QueuedConnection);
            QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                      Q_ARG(int, 2));
            break;
        case Screen::emptyCuffScreen:
            QMetaObject::invokeMethod(btnCancel, "show", Qt::QueuedConnection);
            QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                      Q_ARG(int, 3));
            break;
        case Screen::resultScreen:
            QMetaObject::invokeMethod(btnCancel, "hide", Qt::QueuedConnection);
            QMetaObject::invokeMethod(lInstructions, "setCurrentIndex", Qt::AutoConnection,
                                      Q_ARG(int, 4));
            break;
    }
    currentScreen = eNewScreen;
}

void Window::eResults(double map, double sbp, double dbp) {
    QMetaObject::invokeMethod(lMAPval, "setText", Qt::QueuedConnection,
                              Q_ARG(QString, (QString::number(map, 'f', 0) + " mmHg")));
    QMetaObject::invokeMethod(lSBPval, "setText", Qt::QueuedConnection,
                              Q_ARG(QString, QString::number(sbp, 'f', 0) + " mmHg"));
    QMetaObject::invokeMethod(lDBPval, "setText", Qt::QueuedConnection,
                              Q_ARG(QString, QString::number(dbp, 'f', 0) + " mmHg"));
}

void Window::eHeartRate(double heartRate) {

    QMetaObject::invokeMethod(lheartRate, "setText", Qt::QueuedConnection,
                              Q_ARG(QString, "Current heart rate:<br><b>" +
                                             QString::number(heartRate, 'f', 0) + "</b>"));

    QMetaObject::invokeMethod(lHRvalAV, "setText", Qt::QueuedConnection,
                              Q_ARG(QString, QString::number(heartRate, 'f', 0) + " beats/min"));
}

void Window::eReady() {
    // Instead of :
    // btnStart->setDisabled(false);
    // The invokeMethod is used with a Qt::QueuedConnection.
    // The button is set enabled whenever the UI thread is ready.
    QMetaObject::invokeMethod(btnStart, "setDisabled", Qt::QueuedConnection,
                              Q_ARG(bool, false));
}

void Window::clkBtnStart() {
    process->startMeasurement();
    eSwitchScreen(Screen::inflateScreen);
}

void Window::clkBtnCancel() {
    process->stopMeasurement(); //TODO: make safe
    eSwitchScreen(Screen::startScreen);
}

void Window::clkBtnReset() {
    eSwitchScreen(Screen::startScreen);
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
