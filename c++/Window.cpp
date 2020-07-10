
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
    lWidget = new QWidget();
    lWidget->setMinimumWidth(400);
    rWidget = new QWidget();

    // Layouts
    vlLeft = new QVBoxLayout();
    vlLeft->setObjectName(QString::fromUtf8("vlLeft"));
    vlRight = new QVBoxLayout();
    vlRight->setObjectName(QString::fromUtf8("vlRight"));

    vSpace1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    infoBox = new QTextBrowser(splitter);
    infoBox->setObjectName(QString::fromUtf8("infoBox"));
    infoBox->setFrameShape(QFrame::HLine);
    infoBox->setFrameShadow(QFrame::Plain);
    infoLabel = new QLabel(splitter);
    infoLabel->setObjectName(QString::fromUtf8("infoLabel"));
    infoLabel->setWordWrap(true);

    meter = new QwtDial(splitter);
    meter->setObjectName(QString::fromUtf8("meter"));
    meter->setUpperBound(260.000000000000000);
    meter->setScaleStepSize(20.000000000000000);
    meter->setWrapping(false);
    meter->setInvertedControls(false);
    meter->setLineWidth(4);
    meter->setMode(QwtDial::RotateNeedle);
    meter->setMinScaleArc(20.000000000000000);
    meter->setMaxScaleArc(340.000000000000000);
    QwtDialSimpleNeedle *needle = new QwtDialSimpleNeedle(
            QwtDialSimpleNeedle::Arrow, true, Qt::black,
            QColor(Qt::gray).lighter(130));

    meter->setNeedle(needle);

    btnStart = new QPushButton(splitter);
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

    lTitlePlotRaw = new QLabel(splitter);
    lTitlePlotRaw->setObjectName(QString::fromUtf8("lTitlePlotRaw"));
    lTitlePlotOsc = new QLabel(splitter);
    lTitlePlotOsc->setObjectName(QString::fromUtf8("lTitlePlotOsc"));

    line = new QFrame(splitter);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    pltPre = new Plot(xData, yLPData, dataLength, 1, 0.6, splitter);
    pltPre->setObjectName(QString::fromUtf8("pltPre"));
    pltOsc = new Plot(xData, yHPData, dataLength, 0.5, -0.5, splitter);
    pltOsc->setObjectName(QString::fromUtf8("pltOsc"));

    // build right side of window
    vlRight->addWidget(lTitlePlotRaw);
    vlRight->addWidget(pltPre);
    vlRight->addWidget(line);
    vlRight->addItem(vSpace5);
    vlRight->addWidget(lTitlePlotOsc);
    vlRight->addWidget(pltOsc);

    // build both sides together
    lWidget->setLayout(vlLeft);
    rWidget->setLayout(vlRight);
    splitter->addWidget(lWidget);
    splitter->addWidget(rWidget);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,3);

    // TODO: menubar working? just missing content?
    window->setCentralWidget(splitter);
    menubar = new QMenuBar(window);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 2081, 39));
    window->setMenuBar(menubar);
    statusbar = new QStatusBar(window);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    window->setStatusBar(statusbar);

    retranslateUi(window);

    QMetaObject::connectSlotsByName(window);
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


void Window::eNewData(double pData, double oData) {
    // TODO: calculation does not belong here, just for now
    double ambientV = 0.710; //0.675 # from calibration
    double mmHg_per_kPa = 7.5006157584566; // from literature
    double kPa_per_V = 50.0; // 20mV per 1kPa / 0.02 or * 50 - from sensor datasheet
    double corrFact = 2.50; // from calibration
    double ymmHg = (pData - ambientV) * mmHg_per_kPa * kPa_per_V * corrFact;

    pltPre->setNewData(pData);
    pltOsc->setNewData(oData);
    meter->setValue(ymmHg);
}