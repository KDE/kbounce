/*
    This file is part of the KDE project "KBounce"

    SPDX-FileCopyrightText: 2006 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2007 Tomasz Boczkowski <tboczkowski@onet.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "renderer.h"
#include "settings.h"
#include "debug.h"

#include <QDir>
#include <QRandomGenerator>

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

void KBounceRenderer::setBackgroundSize( QSize size )
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
        qCDebug(KBOUNCE_LOG) << "CustomBackground Directory not found";
        return QPixmap();
    }

    if ( dir.count() > 1 )
    {
        QString filename;
        // return random pixmap
        const uint pos = QRandomGenerator::global()->bounded(dir.count());
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
