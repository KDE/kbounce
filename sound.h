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


#ifndef KBOUNCESOUND_H
#define KBOUNCESOUND_H

#include <QString>

namespace Phonon
{
	class MediaObject;
}

class KBounceSound
{
public:
	explicit KBounceSound( const QString& soundPath );
	virtual ~KBounceSound();
	void setSoundsEnabled(bool enabled);
	void playSound(const QString& sound);
private:
	Phonon::MediaObject* m_audioPlayer1;
	Phonon::MediaObject* m_audioPlayer2;

	bool m_playSounds;	
};

#endif // KBOUNCESOUND_H
