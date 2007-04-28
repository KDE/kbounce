/*
 * Copyright (C) 2000-2005 Stefan Schimanski <schimmi@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>

#include "mainwindow.h"

#include <klocale.h>
#include <kglobal.h>

using namespace std;

static const char description[] = I18N_NOOP("KDE Bounce Ball Game");
static const char version[] = "0.5";

int main(int argc, char **argv)
{
  KAboutData aboutData( "kbounce", I18N_NOOP("KBounce"),
    version, description, KAboutData::License_GPL,
    "(c) 2000-2005, Stefan Schimanski");

  aboutData.addAuthor("Stefan Schimanski", I18N_NOOP("Original author"), "schimmi@kde.org");
  aboutData.addAuthor("Sandro Sigala", I18N_NOOP("Highscore"), "ssigala@globalnet.it");
  aboutData.addAuthor("Benjamin Meyer", I18N_NOOP("Contributions"), "ben+kbounce@meyerhome.net");

  aboutData.addCredit("Dmitry Suzdalev", I18N_NOOP("Port to QGraphicsView framework"), "dimsuz@gmail.com");
  aboutData.addCredit("Tomasz Boczkowski", 0, "tboczkowski@onet.pl");

  KCmdLineArgs::init( argc, argv, &aboutData );

  QApplication::setColorSpec(QApplication::ManyColor);
  KApplication application;
  KGlobal::locale()->insertCatalog("libkdegames");

  if (application.isSessionRestored())
      RESTORE(KBounceMainWindow)
  else {
      KBounceMainWindow *w = new KBounceMainWindow;
      w->show();
  }
  return application.exec();
}

