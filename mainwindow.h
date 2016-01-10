/*
 * Copyright (C) 2000-2005 Stefan Schimanski <1Stein@gmx.de>
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
        ~KBounceMainWindow();

    protected slots:
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

        void focusOutEvent( QFocusEvent * );
        void focusInEvent ( QFocusEvent * );

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
