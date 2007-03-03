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

#ifndef RENDERER_H
#define RENDERER_H

#include <KSvgRenderer>

#include <QString>
#include <QSize>
#include <QHash>
#include <QPixmap>

class KBounceRenderer 
{
    public:
	KBounceRenderer( const QString& fileName );
	~KBounceRenderer();

        bool elementExists( const QString& id );

	void setBackgroundSize( const QSize& size);

	QPixmap renderElement( const QString& id, const QSize& size = QSize( 0, 0 ) );
	QPixmap renderBackground();

    private:
	KSvgRenderer m_svgRenderer;
	QSize m_backgroundSize;
	QPixmap m_cachedBackground;
	QHash<QString, QPixmap> m_tileCache;
};

#endif


