//
// Created by belinda on 25/06/2020.
//
#include <QtWidgets>
#include "uitest.h"

TestWindow::TestWindow(QWidget *parent)
          : QMainWindow(parent)
//        , ui(new Ui::MainWindow)
{
    setupUi(this);
}

TestWindow::~TestWindow()
{

}

void TestWindow::setupUi(QMainWindow *MainWindow)
{
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->resize(2081, 1006);
    MainWindow->setAcceptDrops(true);
    QIcon icon(QIcon::fromTheme(QString::fromUtf8("Main")));
    MainWindow->setWindowIcon(icon);
    MainWindow->setTabShape(QTabWidget::Triangular);
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    horizontalLayout_3 = new QHBoxLayout(centralwidget);
    horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
    verticalLayout_7 = new QVBoxLayout();
    verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
    textBrowser = new QTextBrowser(centralwidget);
    textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
    textBrowser->setFrameShape(QFrame::HLine);
    textBrowser->setFrameShadow(QFrame::Plain);

    verticalLayout_7->addWidget(textBrowser);

    verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_7->addItem(verticalSpacer_4);

    label = new QLabel(centralwidget);
    label->setObjectName(QString::fromUtf8("label"));
    label->setWordWrap(true);

    verticalLayout_7->addWidget(label);

    verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_7->addItem(verticalSpacer_3);

    Dial = new QwtDial(centralwidget);
    Dial->setObjectName(QString::fromUtf8("Dial"));
    Dial->setUpperBound(260.000000000000000);
    Dial->setScaleStepSize(20.000000000000000);
    Dial->setWrapping(false);
    Dial->setInvertedControls(false);
    Dial->setLineWidth(4);
    Dial->setMode(QwtDial::RotateNeedle);
    Dial->setMinScaleArc(20.000000000000000);
    Dial->setMaxScaleArc(340.000000000000000);

    verticalLayout_7->addWidget(Dial);

    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_7->addItem(verticalSpacer);

    pushButton_2 = new QPushButton(centralwidget);
    pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

    verticalLayout_7->addWidget(pushButton_2);

    verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_7->addItem(verticalSpacer_2);


    horizontalLayout_3->addLayout(verticalLayout_7);

    line_2 = new QFrame(centralwidget);
    line_2->setObjectName(QString::fromUtf8("line_2"));
    line_2->setFrameShape(QFrame::VLine);
    line_2->setFrameShadow(QFrame::Sunken);

    horizontalLayout_3->addWidget(line_2);

    verticalLayout_9 = new QVBoxLayout();
    verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
    label_2 = new QLabel(centralwidget);
    label_2->setObjectName(QString::fromUtf8("label_2"));

    verticalLayout_9->addWidget(label_2);

    qwtPlot_2 = new QwtPlot(centralwidget);
    qwtPlot_2->setObjectName(QString::fromUtf8("qwtPlot_2"));

    verticalLayout_9->addWidget(qwtPlot_2);

    verticalSpacer_5 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout_9->addItem(verticalSpacer_5);

    line = new QFrame(centralwidget);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    verticalLayout_9->addWidget(line);

    label_3 = new QLabel(centralwidget);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    verticalLayout_9->addWidget(label_3);

    qwtPlot = new QwtPlot(centralwidget);
    qwtPlot->setObjectName(QString::fromUtf8("qwtPlot"));

    verticalLayout_9->addWidget(qwtPlot);


    horizontalLayout_3->addLayout(verticalLayout_9);

    MainWindow->setCentralWidget(centralwidget);
    menubar = new QMenuBar(MainWindow);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 2081, 39));
    MainWindow->setMenuBar(menubar);
    statusbar = new QStatusBar(MainWindow);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    MainWindow->setStatusBar(statusbar);

    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);
} // setupUi

void TestWindow::retranslateUi(QMainWindow *MainWindow)
{
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
    textBrowser->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                                               "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                                               "p, li { white-space: pre-wrap; }\n"
                                                               "</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
                                                               "<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Instructions about what to do come here. Explaining what to do.</p></body></html>", nullptr));
    label->setText(QApplication::translate("MainWindow", "Alternatively, instructions in a label. Explaining what to do. However, are these wrapping? \n"
                                                         "Below is some sort of visual feedback (depending on the state of the application).", nullptr));
    pushButton_2->setText(QApplication::translate("MainWindow", "Start ", nullptr));
    label_2->setText(QApplication::translate("MainWindow", "Title of Plot 1: RawData", nullptr));
    label_3->setText(QApplication::translate("MainWindow", "Title of Plot 2: Oscillogram", nullptr));
} // retranslateUi
