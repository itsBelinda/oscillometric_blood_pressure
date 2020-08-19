/**
 * @file        Datarecord.cpp
 * @brief       The implementation of the Datarecord class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 */

#include <QtCore/QTextStream>
#include <QtWidgets/QFileDialog>
#include "Datarecord.h"

/**
 * Constructor to prepare recording of data at a later point.
 * @param samplingRate The sampling rate at which the data will be recorded.
 */
Datarecord::Datarecord(double samplingRate)
{
    nsample = 0;
    this->samplingRate = samplingRate;
    boRecord = false;
}

//
/**
 * Constructor that starts recording immediately
 * @param filename The filename to record data with.
 * @param samplingRate The sampling rate at which the data will be recorded.
 */
Datarecord::Datarecord(QString filename, double samplingRate) // = "default.dat")
{
    rec_filename = filename;
    if (!rec_filename.isNull()) {
        rec_file = new QFile(rec_filename);
        if (rec_file->open(QIODevice::WriteOnly | QFile::Truncate)) {
            outStream = new QTextStream(rec_file);
        }
    }
    nsample = 0;
    this->samplingRate = samplingRate;
    boRecord = true;
}
/**
 * Destructor of Datarecord. Save the file if there is one open.
 */
Datarecord::~Datarecord() {
    if (rec_file) {
        rec_file->close();
    }
};

/**
 * Start storing data to a file. Opens file with the name specified by the parameter.
 * @param filename The name of the file to open.
 */
void Datarecord::startRecording(QString filename) {
    if (!rec_filename.isNull()) {
        rec_file = new QFile(rec_filename);
        if (rec_file->open(QIODevice::WriteOnly)) {
            outStream = new QTextStream(rec_file);
            nsample = 0;
            boRecord = true;
        }
    }
}
/**
 * Stop storing data to a file. Closed the previously declared file.
 */
void Datarecord::stopRecording() {
    nsample = 0;
    boRecord = false;
    if (rec_file) {
        rec_file->close();
    }
}
/**
 * Add a single sample to a file.
 * @param sample The sample to add.
 */
void Datarecord::addSample(double sample) {
    if (!boRecord || !rec_file) {
        return;
    }
    nsample++;
    *outStream << (float) nsample / samplingRate << "\t" << sample << "\n";
}
/**
 * Save the content of a vector to a file.
 * @param fileName The name of the file to store the data to.
 * @param samples  The vector to store to a file.
 */
void Datarecord::saveAll(QString fileName, std::vector<double> samples) {
    startRecording(fileName);
    for (auto sample : samples) {
        addSample(sample);
    }
    stopRecording();
}
