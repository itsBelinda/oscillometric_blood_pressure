/**
 * @file        SettingsDialog.cpp
 * @brief       The implementaion of the SettingsDialog class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 */
#include <iostream>
#include "SettingsDialog.h"
/**
 * The constructor of the SettingsDialog class.
 * @param parent A reference to the parent object.
 */
SettingsDialog::SettingsDialog(QWidget *parent) :
        QDialog(parent) {
    setupUi(this);
}

/**
 * Gets the value of the set value in the double spin box for the SBP ratio.
 * @return The value of the double spin box.
 */
double SettingsDialog::getRatioSBP() {
    return dsbRatioSBP->value();
}
/**
 * Sets the value of the set value in the double spin box for the SBP ratio.
 * @param val The value to set.
 */
void SettingsDialog::setRatioSBP(double val) {
    dsbRatioSBP->setValue(val);
}

/**
 * Gets the value of the set value in the double spin box for the DBP ratio.
 * @return The value of the double spin box.
 */
double SettingsDialog::getRatioDBP() {
    return dsbRatioDBP->value();
}

/**
 * Sets the value of the set value in the double spin box for the DBP ratio.
 * @param val The value to set.
 */
void SettingsDialog::setRatioDBP(double val) {
    dsbRatioDBP->setValue(val);
}

/**
 * Gets the value of the set value in the spin box for the minimal number of peaks.
 * @return The value of the spin box.
 */
int SettingsDialog::getMinNbrPeaks() {
    return sbMinNbrPeaks->value();
}

/**
 * Sets the value of the set value in the spin box for the minimal number of peaks.
 * @param val The value to set.
 */
void SettingsDialog::setMinNbrPeaks(int val) {
    sbMinNbrPeaks->setValue(val);
}

/**
 * Gets the value of the set value in the spin box for the pump-up pressure.
 * @return The value of the spin box.
 */
int SettingsDialog::getPumpUpValue() {
    return sbPumpUpValue->value();
}

/**
 * Sets the value of the set value in the spin box for the pump-up pressure.
 * @param val The value to set.
 */
void SettingsDialog::setPumpUpValue(int val) {
    sbPumpUpValue->setValue(val);
}

/**
 * This method is to be called at the initialisation of the object. It builds the user interface.
 * @param SettingsDialog A pointer to the object itself.
 */
void SettingsDialog::setupUi(QDialog *SettingsDialog) {
    if (SettingsDialog->objectName().isEmpty())
        SettingsDialog->setObjectName(QString::fromUtf8("SettingsDialog"));
    SettingsDialog->resize(400, 300);
    vlMain = new QVBoxLayout(SettingsDialog);
    vlMain->setObjectName(QString::fromUtf8("vlMain"));
    formL = new QFormLayout();
    formL->setObjectName(QString::fromUtf8("formL"));
    formL->setFormAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);

    lRatioSBP = new QLabel(SettingsDialog);
    lRatioSBP->setObjectName(QString::fromUtf8("lRatioSBP"));
    dsbRatioSBP = new QDoubleSpinBox(SettingsDialog);
    dsbRatioSBP->setObjectName(QString::fromUtf8("dsbRatioSBP"));
    dsbRatioSBP->setRange(RATIO_MIN, RATIO_MAX);
    dsbRatioSBP->setSingleStep(0.1);
    lRatioDBP = new QLabel(SettingsDialog);
    lRatioDBP->setObjectName(QString::fromUtf8("lRatioDBP"));
    dsbRatioDBP = new QDoubleSpinBox(SettingsDialog);
    dsbRatioDBP->setObjectName(QString::fromUtf8("dsbRatioDBP"));
    dsbRatioDBP->setRange(RATIO_MIN, RATIO_MAX);
    dsbRatioDBP->setSingleStep(0.1);
    lMinNbrPeaks = new QLabel(SettingsDialog);
    lMinNbrPeaks->setObjectName(QString::fromUtf8("lMinNbrPeaks"));
    sbMinNbrPeaks = new QSpinBox(SettingsDialog);
    sbMinNbrPeaks->setObjectName(QString::fromUtf8("sbMinNbrPeaks"));
    sbMinNbrPeaks->setRange(NBR_PEAKS_MIN, NBR_PEAKS_MAX);
    lPumpUpValue = new QLabel(SettingsDialog);
    lPumpUpValue->setObjectName(QString::fromUtf8("lPumpUpValue"));
    sbPumpUpValue = new QSpinBox(SettingsDialog);
    sbPumpUpValue->setObjectName(QString::fromUtf8("sbPumpUpValue"));
    sbPumpUpValue->setRange(PUMP_UP_VALUE_MIN, PUMP_UP_VALUE_MAX);
    sbPumpUpValue->setSingleStep(10);

    formL->setWidget(0, QFormLayout::LabelRole, lRatioSBP);
    formL->setWidget(0, QFormLayout::FieldRole, dsbRatioSBP);
    formL->setWidget(1, QFormLayout::LabelRole, lRatioDBP);
    formL->setWidget(1, QFormLayout::FieldRole, dsbRatioDBP);
    formL->setWidget(2, QFormLayout::LabelRole, lMinNbrPeaks);
    formL->setWidget(2, QFormLayout::FieldRole, sbMinNbrPeaks);
    formL->setWidget(3, QFormLayout::LabelRole, lPumpUpValue);
    formL->setWidget(3, QFormLayout::FieldRole, sbPumpUpValue);

    vlMain->addLayout(formL);

    btnReset = new QPushButton(SettingsDialog);
    btnReset->setObjectName(QString::fromUtf8("btnReset"));
    vlMain->addWidget(btnReset);

    lDescription = new QLabel(SettingsDialog);
    lDescription->setObjectName(QString::fromUtf8("lDescription"));
    lDescription->setWordWrap(true);
    vlMain->addWidget(lDescription);

    buttonBox = new QDialogButtonBox(SettingsDialog);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    vlMain->addWidget(buttonBox);

    retranslateUi(SettingsDialog);
    QObject::connect(buttonBox, SIGNAL(accepted()), SettingsDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), SettingsDialog, SLOT(reject()));

    QObject::connect(btnReset, SIGNAL(clicked()), SettingsDialog, SLOT(resetClicked()));

    QMetaObject::connectSlotsByName(SettingsDialog);
}

/**
 * Sets all the text in the SettingsDialog window.
 * @param SettingsDialog A pointer to the object itself.
 */
void SettingsDialog::retranslateUi(QDialog *SettingsDialog) {
    SettingsDialog->setWindowTitle("Application Settings");
    btnReset->setText("Reset Values");
    lRatioSBP->setText("SBP ratio:");
    lRatioDBP->setText("DBP ratio:");
    lMinNbrPeaks->setText("Min detected peaks:");
    lPumpUpValue->setText("Pump-up value (mmHg):");
    lDescription->setText("The application is not guaranteed to work reliably if the values are changed.<br>"
                          "<b>Changes take effect after restarting the application.</b><br>"
                          "Resetting the values takes effect immediately.");
}

/**
 * Handles the reset values button event.
 *
 * A signal is emitted to be processed by the parent widget.
 */
void SettingsDialog::resetClicked(){
    emit resetValues();
}

