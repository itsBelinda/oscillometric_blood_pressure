#ifndef OBP_SETTINGSDIALOG_H
#define OBP_SETTINGSDIALOG_H



#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QDialogButtonBox>

#include "common.h"


class SettingsDialog: public QDialog {
Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    // Getter and setter methods for variables access.
    double getRatioSBP();
    void setRatioSBP(double val);
    double getRatioDBP();
    void setRatioDBP(double val);
    int getMinNbrPeaks();
    void setMinNbrPeaks(int val);
    int getPumpUpValue();
    void setPumpUpValue(int val);

signals:
    void resetValues();
private slots:
    void resetClicked();
private:
    void setupUi(QDialog *SettingsDialog);
    void retranslateUi(QDialog *SettingsDialog);

    QVBoxLayout *vlMain;
    QFormLayout *formL;
    QLabel *lRatioSBP;
    QDoubleSpinBox *dsbRatioSBP;
    QLabel *lRatioDBP;
    QDoubleSpinBox *dsbRatioDBP;
    QLabel *lMinNbrPeaks;
    QSpinBox *sbMinNbrPeaks;
    QLabel *lPumpUpValue;
    QSpinBox *sbPumpUpValue;
    QPushButton *btnReset;
    QDialogButtonBox *buttonBox;



};


#endif //OBP_SETTINGSDIALOG_H
