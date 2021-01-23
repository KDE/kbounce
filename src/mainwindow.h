/*
    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <1Stein@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBOUNCE_MAINWINDOW_H
#define KBOUNCE_MAINWINDOW_H

#include <KXmlGuiWindow>

#include <QLabel>
#include <QPointer>

#include "gamewidget.h"

class QAction;
class QStatusBar;
class KToggleAction;


class KBounceMainWindow : public KXmlGuiWindow
{
    Q_OBJECT
    public:
        KBounceMainWindow();
        ~KBounceMainWindow() override;

    protected Q_SLOTS:
        void newGame();
        void pauseGame();
        void closeGame();
        void gameOverNow();
        void configureSettings();
        void readSettings();
        void settingsChanged();
        void setSounds( bool val );
        void showHighscore();
        void displayLevel( int level );
        void displayScore( int score );
        void displayFilled( int filled );
        void displayLives( int lives );
        void displayTime( int time );
        void gameStateChanged( KBounceGameWidget::State state );
    protected:
        void initXMLUI();
        void highscore();

        void focusOutEvent( QFocusEvent * ) override;
        void focusInEvent ( QFocusEvent * ) override;

        KBounceGameWidget* m_gameWidget;

        QStatusBar* m_statusBar;

        KToggleAction *m_pauseAction, *m_backgroundShowAction, *m_soundAction;
        QAction *m_newAction;

        QPointer<QLabel> levelLabel = new QLabel;
        QPointer<QLabel> scoreLabel = new QLabel;
        QPointer<QLabel> filledLabel = new QLabel;
        QPointer<QLabel> livesLabel = new QLabel;
        QPointer<QLabel> timeLabel = new QLabel;
};

#endif // KBOUNCE_MAINWINDOW_H
