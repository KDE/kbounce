/*
* Copyright (C) 2010 Andreas Scherf <ascherfy@gmail.com>
*
* This file is part of the KDE project "KBounce"
*
* KBounce is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* KBounce is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with KBounce; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
*/

#include "sound.h"

#include <QDir>
#include <Phonon/MediaObject>

#include <kdebug.h>

KBounceSound::KBounceSound(const QString& soundPath) : m_playSounds(false)
{
	m_audioPlayer1 = 0L;
	m_audioPlayer2 = 0L;
	QDir::addSearchPath( "sounds", soundPath );
}

KBounceSound::~KBounceSound()
{
	delete m_audioPlayer1;
	m_audioPlayer1 = 0L;
	delete m_audioPlayer2;
	m_audioPlayer2 = 0L;
}


void KBounceSound::setSoundsEnabled(bool enabled)
{
	m_playSounds = enabled;
	if ( enabled )
	{
		if (!m_audioPlayer1)
		{
			m_audioPlayer1 = Phonon::createPlayer(Phonon::GameCategory);
		}
		if (!m_audioPlayer2)
		{
			m_audioPlayer2 = Phonon::createPlayer(Phonon::GameCategory);
		}
	}
	else
	{
		delete m_audioPlayer1;
		delete m_audioPlayer2;
		m_audioPlayer1 = 0;
		m_audioPlayer2 = 0;
	}
}

void KBounceSound::playSound(const QString& sound)
{
	Phonon::MediaObject* m_usedMedia = 0L;
	
	kDebug() << "Playing sound:"+ sound;
	if (m_playSounds)
	{
		// Choose the media object with the smallest remaining time
		if ( m_audioPlayer1->remainingTime() <= m_audioPlayer2->remainingTime() )
		{
			m_usedMedia = m_audioPlayer1;
		}
		else
		{
			m_usedMedia = m_audioPlayer2;
		}
		m_usedMedia->setCurrentSource( "sounds:" + sound );
		m_usedMedia->play();
	}
}


