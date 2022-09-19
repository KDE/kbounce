/*
    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <schimmi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mainwindow.h"

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#include <KDBusService>

#include <QApplication>
#include <QCommandLineParser>

#include "debug.h"
#include "kbounce_version.h"

int main(int argc, char **argv)
{
    // Fixes blurry icons with fractional scaling
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kbounce"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kbouncerc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("kbounceui.rc"));
    migrate.migrate();
#endif
    KLocalizedString::setApplicationDomain("kbounce");
    KAboutData aboutData(QStringLiteral("kbounce"), i18n("KBounce"),
            QStringLiteral(KBOUNCE_VERSION_STRING),
            i18n("Bounce Ball Game"),
            KAboutLicense::GPL,
            i18n("(c) 2000-2005, Stefan Schimanski\n(c) 2007, Tomasz Boczkowski"),
            QString(),
            QStringLiteral("https://apps.kde.org/kbounce"));

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
    KCrash::initialize();
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kbounce")));

    KDBusService service;

    if (app.isSessionRestored())
        kRestoreMainWindows<KBounceMainWindow>();
    else {
        KBounceMainWindow *w = new KBounceMainWindow;
        w->show();
    }
    return app.exec();
}

