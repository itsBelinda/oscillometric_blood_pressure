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

#include <QApplication>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  MainWindow   mainWindow;

  mainWindow.show();
  mainWindow.resize(1400,400);
  return app.exec();
}
