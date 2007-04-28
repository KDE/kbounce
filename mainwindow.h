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

#ifndef KBOUNCE_H
#define KBOUNCE_H

#include <KXmlGuiWindow>

#include <QPixmap>

#include "gamewidget.h"

class KAction;
class KStatusBar;
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
   void setSounds( bool val );
   void showHighscore();
   void updateLevel( int level );
   void updateScore( int score );
   void updateFilled( int filled );
   void updateLives( int lives );
   void updateTime( int time );
   void updateState( KBounceGameWidget::State state );

protected:
   void initXMLUI();
   void highscore();

   void focusOutEvent( QFocusEvent * );
   void focusInEvent ( QFocusEvent * );

   KBounceGameWidget* m_gameWidget;

   KStatusBar* m_statusBar;

   KToggleAction *m_pauseButton, *m_backgroundShowAction, *m_soundAction;
   KAction *m_newAction;
};

#endif // KBOUNCE_h

