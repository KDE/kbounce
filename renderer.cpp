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
#include <QDir>
#include <krandom.h>

KBounceRenderer::KBounceRenderer()
	: m_svgRenderer(), m_backgroundSize( QSize( 0, 0 ) ) ,m_tileCache("KBounceTileCache"),m_useRandomBackgrounds(false)
{
}

KBounceRenderer::~KBounceRenderer()
{
}

bool KBounceRenderer::load( const QString& fileName )
{
    kDebug() << "File name:" << fileName;
	if ( m_tileCache.isEnabled() )
	{
		m_tileCache.discard();
	}
    m_cachedBackground = QPixmap();
    return m_svgRenderer.load( fileName );
}

void KBounceRenderer::setCustomBackgroundPath(const QString& path)
{
    if (path.isEmpty())
    {
        m_useRandomBackgrounds = false;
    }
    else
    {
        m_useRandomBackgrounds = true;
    }
    m_customBackgroundPath=path;
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
	if (size != m_backgroundSize )
    {
        m_backgroundSize = size;
		if ( m_useRandomBackgrounds && !m_cachedBackground.isNull() )
        {
            m_cachedBackground = m_randomBackground.scaled(m_backgroundSize,Qt::IgnoreAspectRatio);
        }
        else
        {
            m_cachedBackground = QPixmap();
        }    
    }
}

bool KBounceRenderer::loadNewBackgroundPixmap()
{
    bool backgroundFound = false;
    if ( !m_customBackgroundPath.isEmpty() )
    {
        m_randomBackground = getRandomBackgroundPixmap(m_customBackgroundPath);
        if ( !m_randomBackground.isNull() )
        {
            m_cachedBackground = m_randomBackground.scaled(m_backgroundSize,Qt::IgnoreAspectRatio);
            backgroundFound = true;
        }
    }
    return backgroundFound;
}


QPixmap KBounceRenderer::renderBackground()
{
    if (m_cachedBackground.isNull() && !m_backgroundSize.isNull())
    {
		//This is a dirty fix to the qt's m_svgRenderer.render() method that
		//leaves an garbage-filled border of a pixmap
		kDebug() << "Rendering the background. Size:" << m_backgroundSize;
		if ( m_useRandomBackgrounds && loadNewBackgroundPixmap() )
		{
		    return m_cachedBackground;
		}

		// If no valid backgound pixmap found use the original from theme ...
		m_cachedBackground = QPixmap( m_backgroundSize );
		m_cachedBackground.fill( QApplication::palette().window().color() );
		QPainter p( &m_cachedBackground );

		m_svgRenderer.render( &p, "background" );
    }
    return m_cachedBackground;
}

QPixmap KBounceRenderer::getRandomBackgroundPixmap(const QString& path)
{
    // list directory
    QDir dir( path, "*.png *.jpg", QDir::Name|QDir::IgnoreCase, QDir::Files );
    if ( !dir.exists() ) {
        kDebug() << "CustomBackground Directory not found" << endl;
        return QPixmap();
    }

    if (dir.count() > 1)
    {
        // return random pixmap
        int num = KRandom::random() % dir.count();
        return QPixmap( dir.absoluteFilePath( dir[num] ) );
    }
    else if (dir.count()==1)
    {
        return QPixmap( dir.absoluteFilePath(dir[0]) );
    }
    else return QPixmap();
}


QPixmap KBounceRenderer::renderElement( const QString& id, const QSize& size )
{
	
	QPixmap renderedTile;
	bool elementFound = m_tileCache.find( id, renderedTile );
	if ( !elementFound && size.isEmpty() )
    {
        kDebug() << "Rendering element of no size id:" << id;
        return QPixmap();
    }

    if ( !elementFound || ( !size.isNull() && size != renderedTile.size() ) )
    {
        kDebug() << "Rendering" << id << "size:" << size;
        QImage baseImage( size, QImage::Format_ARGB32_Premultiplied );
        baseImage.fill( 0 );
        QPainter p( &baseImage );
        m_svgRenderer.render( &p, id );
        p.end();
        renderedTile = QPixmap::fromImage( baseImage );
        m_tileCache.insert( id, renderedTile );
    }
    return renderedTile;
}

QPixmap KBounceRenderer::renderElement( const QString& id, int frame, const QSize& size )
{
    QString name = id + '_' + QString::number( frame );
    return renderElement( name, size );
}

