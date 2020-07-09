#ifndef OBP_STACKEDWIDGETS_H
#define OBP_STACKEDWIDGETS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QSplitter>
#include "IObserver.h"

QT_BEGIN_NAMESPACE

class StackedWidgetWindow : public QMainWindow, public IObserver
{
    Q_OBJECT
public:
    StackedWidgetWindow(QWidget *parent = nullptr);
    ~StackedWidgetWindow();

private:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QSplitter *splitter;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QVBoxLayout *verticalLayout;
    QPushButton *pushButton1;
    QWidget *page_2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton2;
    QPushButton *pushButton3;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow);
    void retranslateUi(QMainWindow *MainWindow);


    void eNewData(double pData, double oData) override;

    private slots:
    void clkBtn1();
    void clkBtn2();
    void clkBtn3();
};
QT_END_NAMESPACE

#endif //OBP_STACKEDWIDGETS_H
