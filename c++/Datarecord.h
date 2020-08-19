/**
 * @file        Datarecord.h
 * @brief       The header file of the Datarecord class.
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 *
 * @details
 * Defines the Datarecord class and contains the general class description.
 */

#ifndef OBP_DATARECORD_H
#define OBP_DATARECORD_H


#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

//! The Datarecord Class
/*!
 * The class Datarecord is used to store data in a file. There are two options. One is to store it sample by sample,
 * the other by handing it a vector of doubles to store. If the sampling rate is supplied, it will save the values
 * with the corresponding time. Otherwise, data will be numbered with the sample. In this application, the data is
 * stored at the end of a measurement. A vector is handed to the object together with a file name that represents the
 * current date and time.
 */
class Datarecord {

public:
    Datarecord(double samplingRate);
    Datarecord(QString filename, double samplingRate);
    ~Datarecord();

    void addSample(double sample);
    void saveAll(QString fileName, std::vector<double> samples);
    void startRecording(QString filename);
    void stopRecording();
private:

    QString rec_filename;
    QFile*  rec_file;
    QTextStream* outStream;
    bool boRecord;
    long int nsample;
    double samplingRate;

};


#endif //OBP_DATARECORD_H

