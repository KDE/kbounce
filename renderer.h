/*
 * Copyright (C) 2006 Dmitry Suzdalev <dimsuz@gmail.com>
 * Copyright (C) 2007 Tomasz Boczkowski <tboczkowski@onet.pl>
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
 * License along with KBounce; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <KSvgRenderer>


#include <QSize>
#include <QHash>
#include <QPixmap>

/**
 * Class for rendering elements of game SVG to QPixmap
 */
class KBounceRenderer 
{
    public:
	/**
	 * Constructor.
	 * @param fileName path to SVG containing game graphics
	 */
	explicit KBounceRenderer();
	/**
	 * Destructor.
	 */
	~KBounceRenderer();
	/**
	 * Loads SVG file and invalidates pixmap cache
	 */
	bool load( const QString& fileName );
	/**
	 * Checks if specified element is included in loaded SVG.
	 */
        bool elementExists( const QString& id );
	/**
	 * Returns number of frames for element in SVG with specified ID
	 */
	int frames( const QString& id );
	/**
	 * Sets Background size and invalidates background cache
	 */
	void setBackgroundSize( const QSize& size);
	/**
	 * Renders elements to pixmap.
	 * @param size if == QSize( 0, 0 ) will return rendered element
	 * of former size.
	 */
	QPixmap renderElement( const QString& id, const QSize& size = QSize( 0, 0 ) );
	/**
	 * Renders frame of an element to pixmap
	 * @param size if == QSize( 0, 0 ) will return rendered element
	 * of former size
	 */
	QPixmap renderElement( const QString& id, int frame, const QSize& size = QSize( 0, 0 ) );
	/**
	 * Renders background to QPixmap of size set by setBachgroundSize
	 * Background pixmap is cached (setBackgroundSize() invalidates the cache)
	 */
	QPixmap renderBackground();

    private:
	KSvgRenderer m_svgRenderer;
	QSize m_backgroundSize;
	QPixmap m_cachedBackground;
	QHash<QString, QPixmap> m_tileCache;
};

#endif //RENDERER_H

