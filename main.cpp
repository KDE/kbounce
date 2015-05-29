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

#include "mainwindow.h"

#include <KAboutData>
#include <KLocalizedString>
#include <kdelibs4configmigrator.h>
#include <KDBusService>

#include <QApplication>
#include <QCommandLineParser>

static const char description[] = I18N_NOOP("KDE Bounce Ball Game");
static const char copyleft[] = I18N_NOOP("(c) 2000-2005, Stefan Schimanski\n(c) 2007, Tomasz Boczkowski");

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  Kdelibs4ConfigMigrator migrate(QLatin1String("kbounce"));
  migrate.setConfigFiles(QStringList() << QLatin1String("kbouncerc"));
  migrate.setUiFiles(QStringList() << QLatin1String("kbounceui.rc"));
  migrate.migrate();
  KLocalizedString::setApplicationDomain("kbounce");
  KAboutData aboutData(QStringLiteral("kbounce"), i18n("KBounce"),
                       QStringLiteral("0.11"), i18n(description),
                       KAboutLicense::GPL, i18n(copyleft), QString(),
                       QStringLiteral("http://games.kde.org/kbounce"));

  aboutData.addAuthor(i18n("Stefan Schimanski"),
                      i18n("Original author"),
                      QStringLiteral("schimmi@kde.org"));

  aboutData.addAuthor(i18n("Sandro Sigala"),
                      i18n("Highscore"),
                      QStringLiteral("ssigala@globalnet.it"));

  aboutData.addAuthor(i18n("Benjamin Meyer"),
                      i18n("Contributions"),
                      QStringLiteral("ben+kbounce@meyerhome.net"));

  aboutData.addAuthor(i18n("Tomasz Boczkowski"),
                      i18n("Port to KDE4. Current maintainer"),
                      QStringLiteral("tboczkowski@onet.pl"));

  aboutData.addCredit(i18n("Dmitry Suzdalev"),
                      i18n("Port to QGraphicsView framework"),
                      QStringLiteral("dimsuz@gmail.com"));

  aboutData.addCredit(i18n("Andreas Scherf"),
                      i18n("Image Background and Fixes"),
                      QStringLiteral("ascherfy@gmail.com"));

  KAboutData::setApplicationData(aboutData);
  QCommandLineParser parser;
  parser.addVersionOption();
  parser.addHelpOption();
  aboutData.setupCommandLine(&parser);
  parser.process(app);
  aboutData.processCommandLine(&parser);

  app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kbounce")));

  KDBusService service;

  if (app.isSessionRestored())
      RESTORE(KBounceMainWindow)
  else {
      KBounceMainWindow *w = new KBounceMainWindow;
      w->show();
  }
  return app.exec();
}

