/*
 * Copyright (C) 2000 Stefan Schimanski <schimmi@kde.org>
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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <arts/dispatcher.h>

#include "kbounce.h"

using namespace std;
using namespace Arts;

static const char *description = I18N_NOOP("KDE Bounce Ball Game");
static const char *version = "0.5";

int main(int argc, char **argv)
{
  KAboutData aboutData( "kbounce", I18N_NOOP("KBounce"),
    version, description, KAboutData::License_GPL,
    "(c) 2000, Stefan Schimanski");

  aboutData.addAuthor("Stefan Schimanski", I18N_NOOP("Original author"), "schimmi@kde.org");
  aboutData.addAuthor("Sandro Sigala", I18N_NOOP("Highscore"), "ssigala@globalnet.it");
  aboutData.addAuthor("Benjamin Meyer", I18N_NOOP("Contributions"), "ben-devel@meyerhome.net");
	
  KCmdLineArgs::init( argc, argv, &aboutData );

  QApplication::setColorSpec(QApplication::ManyColor);
  KApplication a;
  KGlobal::locale()->insertCatalogue("libkdegames");
   
  // setup MCOP
  Dispatcher dispatcher;

  KJezzball *top = new KJezzball;
  a.setMainWidget(top);
  top->show();

  return a.exec();
}

// main.cpp

