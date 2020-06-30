//
// Created by belinda on 30/06/2020.
//

#include <QtCore/QTextStream>
#include "datarecord.h"

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

}

Datarecord::~Datarecord(){
    if (rec_file) {
        rec_file->close();
    }
};

//TODO: single samples for now, maybe a whole buffer would be better? Thread?
//
void Datarecord::addSample(double sample)
{
    if(!rec_file->isOpen())
    {
        return;
    }
    nsample++;
    // TODO: local outstream?
    // TODO: custom separator?
    *outStream << nsample << "\t" << sample << "\n";
}