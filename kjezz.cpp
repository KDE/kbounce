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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qlayout.h>

#include <kglobal.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kaction.h>
#include <kstdaction.h>

#include "kjezz.h"
#include "game.h"

KJezzball::KJezzball()
   : m_game( 0 )
{
    initGUI();    
}

KJezzball::~KJezzball()
{
}

void KJezzball::initGUI()
{
   KStdAction::openNew( this, SLOT(newGame()), actionCollection());
   KStdAction::close( this, SLOT(closeGame()), actionCollection());
   KStdAction::quit( kapp, SLOT(quit()), actionCollection());
   new KAction(i18n("S&how Highscore"), CTRL+Key_H, 0, 0, actionCollection(), "game_highscore");
   KStdAction::preferences( this, SLOT(showSettings()), actionCollection());
   
   createGUI( "kjezzui.rc" );
}

void KJezzball::newGame()
{
   closeGame();
   if ( !m_game )
   {
      m_game = new JezzGame( 10, this );      
      setView( m_game );
   }
}

void KJezzball::closeGame()
{
   if ( !m_game ) return;
   delete m_game;
   m_game = 0;
}

#include "kjezz.moc"
