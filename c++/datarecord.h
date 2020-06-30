#ifndef OBP_DATARECORD_H
#define OBP_DATARECORD_H


#include <QtCore/QString>
#include <QtCore/QFile>

class Datarecord {

public:
    Datarecord(double samplingRate);
    Datarecord(QString filename, double samplingRate);
    ~Datarecord();

    void addSample(double sample);
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

