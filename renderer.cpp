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

#include <kdebug.h>
#include <KPixmapEffect>

#include <QPainter>

#include "renderer.h"

KBounceRenderer::KBounceRenderer( const QString& fileName )
	: m_svgRenderer( fileName ), m_backgroundSize( QSize( 0, 0 ) )
{
}

KBounceRenderer::~KBounceRenderer()
{
}

bool KBounceRenderer::elementExists( const QString& id )
{
    return m_svgRenderer.elementExists( id );
}

void KBounceRenderer::setBackgroundSize( const QSize& size )
{
    if (size != m_backgroundSize)
    {
	m_cachedBackground = QPixmap();
	m_backgroundSize = size;
    }
}

QPixmap KBounceRenderer::renderBackground()
{
    if (m_cachedBackground.isNull())
    {
	kDebug() << "Rendering the background. Size: " << m_backgroundSize << endl;
	m_cachedBackground = QPixmap( m_backgroundSize );
	QPainter p( &m_cachedBackground );
	m_svgRenderer.render( &p, "background" );
    }
    return m_cachedBackground;
}

QPixmap KBounceRenderer::renderElement( const QString& id, const QSize& size )
{
    if ( !m_tileCache.contains( id ) || ( size != QSize( 0, 0 ) && size != m_tileCache[id].size() ) )
    {
	kDebug() << k_funcinfo << " Rendering " << id << endl;
	QImage baseImage( size, QImage::Format_ARGB32_Premultiplied );
	baseImage.fill( 0 );
	QPainter p( &baseImage );
	m_svgRenderer.render( &p, id );
	QPixmap renderedTile = QPixmap::fromImage( baseImage );
	m_tileCache[id] = renderedTile;
    }
    return m_tileCache[id];
}

