/**
 * @file        main.cpp
 * @brief       Main program
 * @version     1.0.0
 * @author      Belinda Kneubühler
 * @date        2020-08-18
 * @copyright   GNU General Public License v2.0
 * @author      Bernd Porr
 * @date        2013
 * @author      Matthias H. Hennig
 * @date        2003
 */



#include <QApplication>
#include "common.h"
#include "Processing.h"
#include "Window.h"
#include <plog/Initializers/RollingFileInitializer.h>

int main(int argc, char **argv) {

    plog::init(plog::warning, "obp_log.csv", 1000000, 5);

    PLOG_VERBOSE << "Application started.";
    QApplication app(argc, argv);
    // The following values are set to make storing settings simple.
    app.setOrganizationName("UofG");
    app.setApplicationName("Oscillometric Blood Pressure Measurement");

    Processing procThread;

    Window mainW(&procThread);
    mainW.show();

    procThread.attach(&mainW);
    procThread.start();

    return app.exec();

}

