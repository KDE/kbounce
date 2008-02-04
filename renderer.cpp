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
 * License along with KBounce; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "renderer.h"

#include <kdebug.h>

#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPalette>

KBounceRenderer::KBounceRenderer()
	: m_svgRenderer(), m_backgroundSize( QSize( 0, 0 ) )
{
}

KBounceRenderer::~KBounceRenderer()
{
}

bool KBounceRenderer::load( const QString& fileName )
{
    kDebug() << "File name:" << fileName;
    m_tileCache.clear();
    m_cachedBackground = QPixmap();
    return m_svgRenderer.load( fileName );
}

bool KBounceRenderer::elementExists( const QString& id )
{
    return m_svgRenderer.elementExists( id );
}

int KBounceRenderer::frames( const QString& id )
{
    int frame = 0;
    while ( elementExists( id + '_' + QString::number( frame ) ) )
	frame++;
    return frame;
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
	//This is a dirty fix to the qt's m_svgRenderer.render() method that
	//leaves an garbage-filled border of a pixmap
	kDebug() << "Rendering the background. Size:" << m_backgroundSize;
	m_cachedBackground = QPixmap( m_backgroundSize );
	m_cachedBackground.fill(QApplication::palette().window().color());
	QPainter p( &m_cachedBackground );
	m_svgRenderer.render( &p, "background" );
    }
    return m_cachedBackground;
}

QPixmap KBounceRenderer::renderElement( const QString& id, const QSize& size )
{
    QHash<QString, QPixmap>::const_iterator elementIt = m_tileCache.constFind(id);
    QHash<QString, QPixmap>::const_iterator itEnd = m_tileCache.constEnd();
    if ( elementIt == itEnd && size.isEmpty() )
    {
	kDebug() << "Rendering element of no size id:" << id;
	return QPixmap();
    }

    if ( elementIt == itEnd || ( size != QSize( 0, 0 ) && size != elementIt.value().size() ) )
    {
	kDebug() << "Rendering" << id << "size:" << size;
	QImage baseImage( size, QImage::Format_ARGB32_Premultiplied );
	baseImage.fill( 0 );
	QPainter p( &baseImage );
	m_svgRenderer.render( &p, id );
	p.end();
	QPixmap renderedTile = QPixmap::fromImage( baseImage );
	elementIt = m_tileCache.insert(id, renderedTile);
    }
    return elementIt.value();
}

QPixmap KBounceRenderer::renderElement( const QString& id, int frame, const QSize& size )
{
    QString name = id + '_' + QString::number( frame );
    return renderElement( name, size );
}

