/**
 * @file        Datarecord.h
 * @brief
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 *
 * @details
 */

#ifndef OBP_DATARECORD_H
#define OBP_DATARECORD_H


#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

class Datarecord {

public:
    Datarecord(double samplingRate);
    Datarecord(QString filename, double samplingRate);
    ~Datarecord();

    void addSample(double sample);
    void saveAll(QString fileName, std::vector<double> samples);
    void saveAll(QString fileName, std::vector<int> times, std::vector<double> samples);
    void startRecording(QString filename);
    void startRecording();
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

