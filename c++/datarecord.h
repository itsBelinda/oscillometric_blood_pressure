#ifndef OBP_DATARECORD_H
#define OBP_DATARECORD_H


#include <QtCore/QString>
#include <QtCore/QFile>

class Datarecord {

public:
    Datarecord(QString filename, double samplingRate);
    ~Datarecord();

    void addSample(double sample);
private:

    QString rec_filename;
    QFile*  rec_file;
    QTextStream* outStream;
    long int nsample;
    double samplingRate;
};


#endif //OBP_DATARECORD_H

