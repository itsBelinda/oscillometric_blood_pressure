//
// Created by belinda on 30/06/2020.
//

#include <QtCore/QTextStream>
#include <QtWidgets/QFileDialog>
#include "datarecord.h"

// Constructor for preparation
Datarecord::Datarecord(double samplingRate) // = "default.dat")
{
    nsample = 0;
    this->samplingRate = samplingRate;
    boRecord = false;
}

// Constructor that starts recording immediately
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

Datarecord::~Datarecord() {
    if (rec_file) {
        rec_file->close();
    }
};


void Datarecord::startRecording() {
    rec_filename = QFileDialog::getSaveFileName();
    if (!rec_filename.isNull()) {
        rec_file = new QFile(rec_filename);
        if (rec_file->open(QIODevice::WriteOnly)) {
            outStream = new QTextStream(rec_file);
            nsample = 0;
            boRecord = true;
        }
    }
}

void Datarecord::startRecording(QString filename) {
    rec_filename = filename; //TODO: filename does not have to be saved?
    if (!rec_filename.isNull()) {
        rec_file = new QFile(rec_filename);
        if (rec_file->open(QIODevice::WriteOnly)) {
            outStream = new QTextStream(rec_file);
            nsample = 0;
            boRecord = true;
        }
    }
}

void Datarecord::stopRecording() {
    nsample = 0;
    boRecord = false;
    if (rec_file) {
        rec_file->close();
    }
}

//TODO: single samples for now, maybe a whole buffer would be better? Thread?
//
void Datarecord::addSample(double sample) {
    if (!boRecord || !rec_file) {
        return;
    }
    nsample++;
    // TODO: local outstream?
    // TODO: custom separator?
    *outStream << (float) nsample / samplingRate << "\t" << sample << "\n";
}

void Datarecord::saveAll(QString fileName, std::vector<double> samples) {
    startRecording(fileName);
    for (auto sample : samples) {
        addSample(sample);
    }
    stopRecording();
}


void Datarecord::saveAll(QString fileName, std::vector<int> times, std::vector<double> samples) {
    startRecording(fileName);
    auto time = times.begin();
    for (auto sample : samples) {
        *outStream << (float) *time / samplingRate << "\t" << sample << "\n";
        time = std::next(time,1);
    }
    stopRecording();
}