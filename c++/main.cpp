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

#include <iostream>
#include <QApplication>

#include "common.h"
#include "Processing.h"
#include "Window.h"
#include <plog/Initializers/RollingFileInitializer.h>

int main(int argc, char **argv) {

    plog::init(plog::verbose, "log.csv", 1000000, 5);

    PLOG_VERBOSE << "Application started.";
    QApplication app(argc, argv);
    app.setOrganizationName("UofG");
    app.setApplicationName("Oscillometric Blood Pressure Measurement");

    Processing procThread;

    Window mainW(&procThread);
    mainW.show();

    procThread.attach(&mainW);
    procThread.start();

    return app.exec();

}


