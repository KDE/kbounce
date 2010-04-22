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
#include <QHash>

namespace Phonon
{
	class MediaObject;
}

class KBounceSound 
{
public:
	explicit KBounceSound(QObject *parent, const QString& soundPath );
	virtual ~KBounceSound();
	void setSoundsEnabled(bool enabled);
	Phonon::MediaObject* cacheSound(const QString& sound);
	void playSound(const QString& sound);
private:
	void playSoundInternal(const QString& sound);
	QHash<QString,Phonon::MediaObject*> m_hMedia;

	bool m_playSounds;
	QObject *m_parent;
};

#endif // KBOUNCESOUND_H
