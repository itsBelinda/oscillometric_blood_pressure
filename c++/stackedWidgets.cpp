

#include <iostream>
#include "stackedWidgets.h"

StackedWidgetWindow::StackedWidgetWindow(QWidget *parent)
        : QMainWindow(parent)
{
    setupUi(this);
}

StackedWidgetWindow::~StackedWidgetWindow()
{
    std::cout << "Stacked widget window deleted." << std::endl;
}

void StackedWidgetWindow::setupUi(QMainWindow *window)
{
    window->resize(538, 425);
    centralwidget = new QWidget(window);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    horizontalLayout = new QHBoxLayout(centralwidget);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    splitter = new QSplitter(centralwidget);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setOrientation(Qt::Horizontal);
    stackedWidget = new QStackedWidget(splitter);
    stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
    page = new QWidget();
    page->setObjectName(QString::fromUtf8("page"));
    verticalLayout = new QVBoxLayout(page);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    pushButton1 = new QPushButton(page);
    pushButton1->setObjectName(QString::fromUtf8("pushButton1"));

    verticalLayout->addWidget(pushButton1);

    stackedWidget->addWidget(page);
    page_2 = new QWidget();
    page_2->setObjectName(QString::fromUtf8("page_2"));
    horizontalLayout_2 = new QHBoxLayout(page_2);
    horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
    pushButton2 = new QPushButton(page_2);
    pushButton2->setObjectName(QString::fromUtf8("pushButton2"));

    horizontalLayout_2->addWidget(pushButton2);

    stackedWidget->addWidget(page_2);
    splitter->addWidget(stackedWidget);
    pushButton3 = new QPushButton(splitter);
    pushButton3->setObjectName(QString::fromUtf8("pushButton3"));
    splitter->addWidget(pushButton3);

    horizontalLayout->addWidget(splitter);

    window->setCentralWidget(centralwidget);
    menubar = new QMenuBar(window);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 538, 22));
    window->setMenuBar(menubar);
    statusbar = new QStatusBar(window);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    window->setStatusBar(statusbar);

    retranslateUi(window);

    stackedWidget->setCurrentIndex(0);

    connect(pushButton1, SIGNAL (clicked()), this, SLOT (clkBtn1()));

    connect(pushButton2, SIGNAL (released()), this, SLOT (clkBtn2()));

    connect(pushButton3, SIGNAL (released()), this, SLOT (clkBtn3()));

    QMetaObject::connectSlotsByName(window);
} // setupUi

void StackedWidgetWindow::retranslateUi(QMainWindow *window)
{
    window->setWindowTitle(QApplication::translate("stackedWindow", "stackedWindow", nullptr));
    pushButton1->setText(QApplication::translate("stackedWindow", "Button 1", nullptr));
    pushButton2->setText(QApplication::translate("stackedWindow", "Button 2", nullptr));
    pushButton3->setText(QApplication::translate("stackedWindow", "Button Always Here", nullptr));
} // retranslateUi

void StackedWidgetWindow::clkBtn1(){
    stackedWidget->setCurrentIndex(1);
    std::cout << "Button 1 pushed" << std::endl;
}
void StackedWidgetWindow::clkBtn2(){
    stackedWidget->setCurrentIndex(0);
    std::cout << "Button 2 pushed" << std::endl;
}

void StackedWidgetWindow::clkBtn3(){

    std::cout << "Button 3 pushed" << std::endl;
    //TODO: deletes, but causes error.. this->deleteLater();
}

void StackedWidgetWindow::eNewData(double pData, double oData) {
//        ambientV = 0.710#0.675 # from calibration
//        mmHg_per_kPa = 7.5006157584566 # from literature
//        kPa_per_V = 50 # 20mV per 1kPa / 0.02 or * 50 - from sensor datasheet
//        corrFact = 2.50 # from calibration
//
//        ymmHg = (y - ambientV)  * mmHg_per_kPa * kPa_per_V * corrFact


}