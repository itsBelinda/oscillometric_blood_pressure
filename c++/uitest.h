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
#include <qwt/qwt_dial_needle.h>
#include <qwt/qwt_plot.h>


QT_BEGIN_NAMESPACE

class TestWindow : public QMainWindow
{
Q_OBJECT

public:
    TestWindow(QWidget *parent = nullptr);
    ~TestWindow();
    void setPressure(double pressure);

private:
    QWidget *centralwidget;
    QHBoxLayout *hl;
    QVBoxLayout *vlLeft;
    QVBoxLayout *vlRight;
    QSpacerItem *vSpace1;
    QSpacerItem *vSpace2;
    QSpacerItem *vSpace3;
    QSpacerItem *vSpace4;
    QSpacerItem *vSpace5;
    QTextBrowser *infoBox;
    QLabel *infoLabel;
    QwtDial *meter;
    QPushButton *btnStart;
    QLabel *lTitlePlotRaw;
    QLabel *lTitlePlotOsc;
    QwtPlot *pltRaw;
    QwtPlot *pltOsc;
    QFrame *line1;
    QFrame *line2;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *window);
    void retranslateUi(QMainWindow *MainWindow);

};



QT_END_NAMESPACE

#endif //OBP_UITEST_H
