/*
 * Copyright (C) 2000 Stefan Schimanski <1Stein@gmx.de>
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

#ifndef KBOUNCE_H
#define KBOUNCE_H

#include <kmainwindow.h>

class JezzGame;
class QLCDNumber;
class QGridLayout;
class KToggleAction;
class KAction;

class KJezzball : public KMainWindow
{
   Q_OBJECT

public:
   KJezzball();
   ~KJezzball();

public slots:
   void newGame();
   void pauseGame();
   void closeGame();
   void showHighscore();
   void selectBackground();
   void showBackground();

protected slots:
   void died();
   void newPercent( int percent );
   void second();
   void switchLevel();
   void gameOverNow();
   void configureHighscores();

protected:
   void createLevel( int level );
   void startLevel();
   void stopLevel();
   void nextLevel();
   void highscore();
   void gameOver();
   void initXMLUI();

   void focusOutEvent( QFocusEvent * );
   void focusInEvent ( QFocusEvent * );

   QPixmap getBackgroundPixmap();

   JezzGame *m_gameWidget;
   QWidget *m_view;
   QGridLayout *m_layout;
   QLCDNumber *m_levelLCD;
   QLCDNumber *m_lifesLCD;
   QLCDNumber *m_scoreLCD;
   QLCDNumber *m_percentLCD;
   QLCDNumber *m_timeLCD;
   KToggleAction *m_pauseButton, *m_backgroundShowAction, *m_soundAction;
   KAction *m_newAction;

   QTimer *m_timer;
   QTimer *m_nextLevelTimer;
   QTimer *m_gameOverTimer;

   QString m_backgroundDir;
   bool m_showBackground;
   QPixmap m_background;

   enum { Idle, Running, Paused, Suspend } m_state;

   struct
   {
       int lifes;
       int time;
       int score;
   } m_level;

   struct
   {
       int level;
       int score;
   } m_game;
};

#endif // KBOUNCE_h

