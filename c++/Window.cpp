
#include <qwt/qwt_dial_needle.h>
#include <iostream>
#include "Window.h"

Window::Window(QWidget *parent) :
        dataLength(MAX_DATA_LENGTH),
        QMainWindow(parent) {

    for (int i = 0; i < MAX_DATA_LENGTH; i++) {
        xData[i] = (double) (MAX_DATA_LENGTH - i) / (double) SAMPLING_RATE;     // time axis in seconds
        yLPData[i] = 0;
        yHPData[i] = 0;
    }

    setupUi(this);

    // Generate timer event every 50ms to update the window
    (void) startTimer(50);
}

Window::~Window() {

}


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

    // the left side is a stacked widget with several pages
    lInstructions = new QStackedWidget(window);
    lInstructions->setMinimumWidth(400);

    // Build pages and add them to the instructions panel
    lInstructions->addWidget(setupStartPage(lInstructions));
    lInstructions->addWidget(setupPumpPage(lInstructions));
    lInstructions->addWidget(setupReleasePage(lInstructions));
    lInstructions->addWidget(setupDeflatePage(lInstructions));
    lInstructions->addWidget(setupResultPage(lInstructions));

    // Add the instructions panel to the splitter
    splitter->addWidget(lInstructions);

    // Build and add the plot panel to the splitter
    splitter->addWidget(setupPlots(splitter));

    // Set default stretch factors
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);

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

    // Update Text.
    retranslateUi(window);

    //Screen::startScreen
    // Set start page for instructions
    lInstructions->setCurrentIndex(1);
    QMetaObject::connectSlotsByName(window);


    //TODO: only for now to be able to switch between the pages of the stacked widget
    auto * widget = new QWidget();
    auto  * but0 = new QPushButton("0");
    auto  * but1 = new QPushButton("1");
    auto  * but2 = new QPushButton("2");
    auto  * but3 = new QPushButton("3");
    auto  * but4 = new QPushButton("4");

    auto *spacer = new QLabel(); // fake spacer
    statusbar->addPermanentWidget(but0);
    statusbar->addPermanentWidget(but1);
    statusbar->addPermanentWidget(but2);
    statusbar->addPermanentWidget(but3);
    statusbar->addPermanentWidget(but4);
    auto *spacerbnt = new QLabel(); // fake spacer
    statusbar->addPermanentWidget(spacerbnt, 1);

    connect(but0, SIGNAL (released()), this, SLOT (clkBtn1()));
    connect(but1, SIGNAL (released()), this, SLOT (clkBtn2()));
    connect(but2, SIGNAL (released()), this, SLOT (clkBtn3()));
    connect(but3, SIGNAL (released()), this, SLOT (clkBtn4()));
    connect(but4, SIGNAL (released()), this, SLOT (clkBtn5()));

}


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

    pltPre = new Plot(xData, yLPData, dataLength, 1, 0.6, parent);
    pltPre->setObjectName(QString::fromUtf8("pltPre"));
    pltOsc = new Plot(xData, yHPData, dataLength, 0.5, -0.5, parent);
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


QWidget *Window::setupStartPage(QWidget *parent) {
    return lInstrStart = new QWidget(parent);

}


QWidget *Window::setupPumpPage(QWidget *parent) {

    lInstrPump = new QWidget(parent);

    // Layout for this page:
    vlLeft = new QVBoxLayout();
    vlLeft->setObjectName(QString::fromUtf8("vlLeft"));


    vSpace1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    infoBox = new QTextBrowser(parent);
    infoBox->setObjectName(QString::fromUtf8("infoBox"));
    infoBox->setFrameShape(QFrame::HLine);
    infoBox->setFrameShadow(QFrame::Plain);
    infoLabel = new QLabel(parent);
    infoLabel->setObjectName(QString::fromUtf8("infoLabel"));
    infoLabel->setWordWrap(true);

    meter = new QwtDial(parent);
    meter->setObjectName(QString::fromUtf8("meter"));
    meter->setUpperBound(260.000000000000000);
    meter->setScaleStepSize(20.000000000000000);
    meter->setWrapping(false);
    meter->setInvertedControls(false);
    meter->setLineWidth(4);
    meter->setMode(QwtDial::RotateNeedle);
    meter->setMinScaleArc(20.000000000000000);
    meter->setMaxScaleArc(340.000000000000000);
    auto *needle = new QwtDialSimpleNeedle(
            QwtDialSimpleNeedle::Arrow, true, Qt::black,
            QColor(Qt::gray).lighter(130));

    meter->setNeedle(needle);

    btnStart = new QPushButton(parent);
    btnStart->setObjectName(QString::fromUtf8("btnStart"));

    // build left side of window
    vlLeft->addWidget(infoBox);
    vlLeft->addItem(vSpace1);
    vlLeft->addWidget(infoLabel);
    vlLeft->addItem(vSpace2);
    vlLeft->addWidget(meter);
    vlLeft->addItem(vSpace3);
    vlLeft->addWidget(btnStart);
    vlLeft->addItem(vSpace4);

    lInstrPump->setLayout(vlLeft);
    return lInstrPump;
}


QWidget *Window::setupReleasePage(QWidget *parent) {
    return lInstrRelease = new QWidget(parent);
}

QWidget *Window::setupDeflatePage(QWidget *parent) {
    return lInstrResult = new QWidget(parent);
}

QWidget *Window::setupResultPage(QWidget *parent) {
    return lInstrResult = new QWidget(parent);
}


void Window::retranslateUi(QMainWindow *window) {
    window->setWindowTitle(QApplication::translate("TestWindow", "TestWindow", nullptr));
    infoBox->setHtml(QApplication::translate("TestWindow",
                                             "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                             "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                             "p, li { white-space: pre-wrap; }\n"
                                             "</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
                                             "<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Instructions about what to do come here. Explaining what to do.</p></body></html>",
                                             nullptr));
    infoLabel->setText(QApplication::translate("TestWindow",
                                               "Alternatively, instructions in a infoLabel. Explaining what to do. However, are these wrapping? \n"
                                               "Below is some sort of visual feedback (depending on the state of the application).",
                                               nullptr));
    btnStart->setText(QApplication::translate("MainWindow", "Start ", nullptr));
    lTitlePlotRaw->setText(QApplication::translate("TestWindow", "Title of Plot 1: Pressure", nullptr));
    lTitlePlotOsc->setText(QApplication::translate("TestWindow", "Title of Plot 2: Oscillogram", nullptr));
}


void Window::timerEvent(QTimerEvent *) {
    pltOsc->replot();
    pltPre->replot();
    meter->repaint();
}



void Window::clkBtn1(){
    eSwitchScreen(Screen::startScreen);
}
void Window::clkBtn2(){
    eSwitchScreen(Screen::inflateScreen);
}
void Window::clkBtn3(){
    eSwitchScreen(Screen::releaseScreen);
}
void Window::clkBtn4(){
    eSwitchScreen(Screen::deflateScreen);

}
void Window::clkBtn5(){
    eSwitchScreen(Screen::resultScreen);
}


void Window::eNewData(double pData, double oData) {
    // TODO: calculation does not belong here, just for now
    double ambientV = 0.710; //0.675 # from calibration
    double mmHg_per_kPa = 7.5006157584566; // from literature
    double kPa_per_V = 50.0; // 20mV per 1kPa / 0.02 or * 50 - from sensor datasheet
    double corrFact = 2.50; // from calibration
    double ymmHg = (pData - ambientV) * mmHg_per_kPa * kPa_per_V * corrFact;

    //TODO: depending on screen state?
    pltPre->setNewData(pData);
    pltOsc->setNewData(oData);
    meter->setValue(ymmHg);
}

void Window::eSwitchScreen(Screen eScreen) {
    // TODO: add more
    switch (eScreen) {
        case Screen::startScreen:
            lInstructions->setCurrentIndex(0);
            break;
        case Screen::inflateScreen:
            lInstructions->setCurrentIndex(1);
            break;
        case Screen::releaseScreen:
            lInstructions->setCurrentIndex(2);
            break;
        case Screen::deflateScreen:
            lInstructions->setCurrentIndex(3);
            break;
        case Screen::resultScreen:
            lInstructions->setCurrentIndex(4);
            break;
    }
}
