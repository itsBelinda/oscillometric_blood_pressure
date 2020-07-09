/***************************************************************************
 *   Copyright (C) 2003 by Matthias H. Hennig                              *
 *             (C) 2013 by Bernd Porr                                      *
 *             (C) 2020 by Belinda Kneubühler                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "obp.h"
#include <iostream>
#include <QApplication>

#include "Processing.h"
#include "stackedWidgets.h"
#include "uitest.h"


#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

int main(int argc, char **argv) {

    //TODO: possibly add time to log file, so no overwrites happen, for now overwrites are preferred.
    plog::init(plog::verbose, "log.csv", 1000000, 5);

    PLOG_VERBOSE << "Application started.";
    QApplication app(argc, argv);

    Processing procThread;

    MainWindow w;
    w.show();
    w.resize(1400, 400);

    StackedWidgetWindow stackedW;
    stackedW.show();

    TestWindow testW;
    testW.show();
    //Todo: maybe have procthread in window (model in view?)
    procThread.attach(&w);
    procThread.attach(&testW);
    std::cout << "create thread" << std::endl;
    procThread.start();
    std::cout << "process started" << std::endl;
    //todo: should be done by algorithm
    procThread.startMeasurement();

//    std::cout << "user input stops program (press ENTER)..." << std::endl;
//    std::cin.get();
    int appReturn = app.exec();

    PLOG_VERBOSE << "Application terminated.";
    // TODO: stop properly from program/algo
    procThread.stopMeasurement();
    procThread.stopThread();
    procThread.join();
    return appReturn;

}


