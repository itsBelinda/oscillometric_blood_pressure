//
// Created by belinda on 25/06/2020.
//

#ifndef OBP_UITEST_H
#define OBP_UITEST_H

#include <QMainWindow>

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <qwt/qwt_dial.h>
#include <qwt/qwt_plot.h>


QT_BEGIN_NAMESPACE

class TestWindow : public QMainWindow
{
Q_OBJECT

public:
    TestWindow(QWidget *parent = nullptr);
    ~TestWindow();

public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_7;
    QTextBrowser *textBrowser;
    QSpacerItem *verticalSpacer_4;
    QLabel *label;
    QSpacerItem *verticalSpacer_3;
    QwtDial *Dial;
    QSpacerItem *verticalSpacer;
    QPushButton *pushButton_2;
    QSpacerItem *verticalSpacer_2;
    QFrame *line_2;
    QVBoxLayout *verticalLayout_9;
    QLabel *label_2;
    QwtPlot *qwtPlot_2;
    QSpacerItem *verticalSpacer_5;
    QFrame *line;
    QLabel *label_3;
    QwtPlot *qwtPlot;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow);
    void retranslateUi(QMainWindow *MainWindow);

};



QT_END_NAMESPACE

#endif //OBP_UITEST_H
