/**
 * @file        InfoDialog.h
 * @brief       The header file of the InfoDialog class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 * Defines the InfoDialog class and contains the general class description.
 */
#ifndef OBP_INFODIALOG_H
#define OBP_INFODIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

//! The InfoDialog Class displays a pup-up window with some information.
/*!
 * The information pane shows the application version number, the licence and provides a link to the project GitHub
 * page.
 */
class InfoDialog : public QDialog
{
Q_OBJECT
public:
    InfoDialog(QWidget *parent);
private:
    void setupUi(QDialog *Information);
    void retranslateUi(QDialog *Information);

    // The GUI components:
    QVBoxLayout *vlMain;
    QLabel *lTilte;
    QTextBrowser *textBrowser;
    QLabel *lLicense;
};


#endif //OBP_INFODIALOG_H
