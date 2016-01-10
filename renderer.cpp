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
#include "settings.h"
#include "debug.h"

#include <QPainter>
#include <QPalette>
#include <QDir>

#include <krandom.h>
#include <KgThemeProvider>

static KgThemeProvider* provider()
{
    KgThemeProvider* prov = new KgThemeProvider;
    prov->discoverThemes(
            "appdata", QStringLiteral("themes"), //theme file location
            QStringLiteral("default")            //default theme file name
            );
    return prov;
}

KBounceRenderer::KBounceRenderer() : KGameRenderer(provider()), m_backgroundSize( QSize( 0, 0 ) ), m_useRandomBackgrounds(false)
{
}

KBounceRenderer::~KBounceRenderer()
{
}

void KBounceRenderer::setCustomBackgroundPath(const QString& path)
{
    m_useRandomBackgrounds = !path.isEmpty();
    m_customBackgroundPath = path;
    m_cachedBackground = QPixmap();
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
        qCDebug(KBOUNCE_LOG) << "Rendering the background. Size:" << m_backgroundSize;
        if ( m_useRandomBackgrounds && loadNewBackgroundPixmap() )
        {
            return m_cachedBackground;
        }
        // If no valid backgound pixmap found use the original from theme ...
        m_cachedBackground = spritePixmap( QStringLiteral("background"), m_backgroundSize );
    }
    return m_cachedBackground;
}

QPixmap KBounceRenderer::getRandomBackgroundPixmap(const QString& path)
{
    // list directory
    QDir dir( path, QStringLiteral("*.png *.jpg"), QDir::Name|QDir::IgnoreCase, QDir::Files );
    if ( !dir.exists() ) {
        qCDebug(KBOUNCE_LOG) << "CustomBackground Directory not found" << endl;
        return QPixmap();
    }

    if ( dir.count() > 1 )
    {
        QString filename;
        // return random pixmap
        uint pos = KRandom::random() % dir.count();
        if ( pos < dir.count() )
        {
            filename = dir.absoluteFilePath( dir[pos] ); 
        }

        if (!filename.isEmpty() && QFile(filename).exists())
        {
            return QPixmap(filename);	
        }
        else return QPixmap();
    }
    else if ( dir.count() == 1 )
    {
        return QPixmap( dir.absoluteFilePath(dir[0]) );
    }
    else return QPixmap();
}
