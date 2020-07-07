#include <QtWidgets>
#include "uitest.h"

TestWindow::TestWindow(QWidget *parent)
          : QMainWindow(parent)
{
    setupUi(this);
}

TestWindow::~TestWindow()
{

}

void TestWindow::setupUi(QMainWindow *window)
{
    if (window->objectName().isEmpty())
        window->setObjectName(QString::fromUtf8("TestWindow"));

    QIcon icon(QIcon::fromTheme(QString::fromUtf8("Main")));
    window->resize(2081, 1006);
    window->setAcceptDrops(true);
    window->setWindowIcon(icon);
    window->setTabShape(QTabWidget::Triangular);
    centralwidget = new QWidget(window);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));

    // Layouts
    hl = new QHBoxLayout(centralwidget);
    hl->setObjectName(QString::fromUtf8("hl"));
    vlLeft = new QVBoxLayout();
    vlLeft->setObjectName(QString::fromUtf8("vlLeft"));
    vlRight = new QVBoxLayout();
    vlRight->setObjectName(QString::fromUtf8("vlRight"));

    vSpace1 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vSpace5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    line1 = new QFrame(centralwidget);
    line1->setObjectName(QString::fromUtf8("line1"));
    line1->setFrameShape(QFrame::VLine);
    line1->setFrameShadow(QFrame::Sunken);

    infoBox = new QTextBrowser(centralwidget);
    infoBox->setObjectName(QString::fromUtf8("infoBox"));
    infoBox->setFrameShape(QFrame::HLine);
    infoBox->setFrameShadow(QFrame::Plain);
    infoLabel = new QLabel(centralwidget);
    infoLabel->setObjectName(QString::fromUtf8("infoLabel"));
    infoLabel->setWordWrap(true);

    // TODO: needs a separate class to support setting needle directly.
    meter = new QwtDial(centralwidget);
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
            QColor( Qt::gray ).lighter( 130 ) );

    meter->setNeedle(needle);

    btnStart = new QPushButton(centralwidget);
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

    lTitlePlotRaw = new QLabel(centralwidget);
    lTitlePlotRaw->setObjectName(QString::fromUtf8("lTitlePlotRaw"));
    lTitlePlotOsc = new QLabel(centralwidget);
    lTitlePlotOsc->setObjectName(QString::fromUtf8("lTitlePlotOsc"));

    line2 = new QFrame(centralwidget);
    line2->setObjectName(QString::fromUtf8("line2"));
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);

    pltRaw = new QwtPlot(centralwidget);
    pltRaw->setObjectName(QString::fromUtf8("pltRaw"));
    pltOsc = new QwtPlot(centralwidget);
    pltOsc->setObjectName(QString::fromUtf8("pltOsc"));

    // build right side of window
    vlRight->addWidget(lTitlePlotRaw);
    vlRight->addWidget(pltRaw);
    vlRight->addWidget(line2);
    vlRight->addItem(vSpace5);
    vlRight->addWidget(lTitlePlotOsc);
    vlRight->addWidget(pltOsc);

    // build both sides together
    hl->addLayout(vlLeft);
    hl->addWidget(line1);
    hl->addLayout(vlRight);

    // TODO: menubar working? just missing content?
    window->setCentralWidget(centralwidget);
    menubar = new QMenuBar(window);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 2081, 39));
    window->setMenuBar(menubar);
    statusbar = new QStatusBar(window);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    window->setStatusBar(statusbar);

    retranslateUi(window);

    QMetaObject::connectSlotsByName(window);
} // setupUi

void TestWindow::retranslateUi(QMainWindow *window)
{
    window->setWindowTitle(QApplication::translate("TestWindow", "TestWindow", nullptr));
    infoBox->setHtml(QApplication::translate("TestWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                                               "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                                               "p, li { white-space: pre-wrap; }\n"
                                                               "</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
                                                               "<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Instructions about what to do come here. Explaining what to do.</p></body></html>", nullptr));
    infoLabel->setText(QApplication::translate("TestWindow", "Alternatively, instructions in a infoLabel. Explaining what to do. However, are these wrapping? \n"
                                                         "Below is some sort of visual feedback (depending on the state of the application).", nullptr));
    btnStart->setText(QApplication::translate("MainWindow", "Start ", nullptr));
    lTitlePlotRaw->setText(QApplication::translate("TestWindow", "Title of Plot 1: RawData", nullptr));
    lTitlePlotOsc->setText(QApplication::translate("TestWindow", "Title of Plot 2: Oscillogram", nullptr));
} // retranslateUi

void TestWindow::setPressure(double pressure)
{
    meter->setValue(pressure);
}