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

KBounceSound::KBounceSound(QObject *parent, const QString& soundPath) : m_playSounds(false), m_parent(parent)
{
	QDir::addSearchPath( "sounds", soundPath );
}

KBounceSound::~KBounceSound()
{
	qDeleteAll(m_hMedia);
}


void KBounceSound::setSoundsEnabled(bool enabled)
{
	m_playSounds = enabled;
}

void KBounceSound::playSound(const QString& sound)
{
	 playSoundInternal( sound );
}

Phonon::MediaObject* KBounceSound::cacheSound(const QString& sound)
{
	Phonon::MediaObject* usedMedia = 0L;
	if ( !m_hMedia.contains(sound) )
	{
		usedMedia = Phonon::createPlayer(Phonon::GameCategory);
		usedMedia->setParent( m_parent );
		usedMedia->setCurrentSource( QString(QLatin1String("sounds:") + sound) );
		m_hMedia.insert( sound, usedMedia );
	}
	else
	{
		QHash<QString,Phonon::MediaObject*>::iterator element = m_hMedia.find(sound);
		usedMedia = element.value();
	}
	return usedMedia;
}

void KBounceSound::playSoundInternal(const QString& sound)
{
	if (m_playSounds)
	{
		Phonon::MediaObject* usedMedia = 0L;
		kDebug() << "Playing sound:"+ sound;
		if ( m_hMedia.contains(sound) )
		{
			QHash<QString,Phonon::MediaObject*>::iterator element = m_hMedia.find(sound);
			usedMedia = element.value();
		}
		else
		{
			usedMedia = cacheSound( sound );
		}
		usedMedia->play();
	}
}




