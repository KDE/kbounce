/*
    This file is part of the KDE project "KBounce"

    SPDX-FileCopyrightText: 2006 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2007 Tomasz Boczkowski <tboczkowski@onet.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RENDERER_H
#define RENDERER_H

#include <QSvgRenderer>
#include <KGameRenderer>

#include <QSize>
#include <QHash>
#include <QPixmap>


/**
 * Class for rendering elements of game SVG to QPixmap
 */

class KBounceRenderer : public KGameRenderer
{
    Q_OBJECT
    public:
        /**
         * Constructor.
         * @param fileName path to SVG containing game graphics
         */
        explicit KBounceRenderer();
        /**
         * Destructor.
         */
        ~KBounceRenderer() override;
        /**
         * Sets Background size and invalidates background cache
         */
        void setBackgroundSize(QSize size);
        /**
         * Renders background to QPixmap of size set by setBachgroundSize
         * Background pixmap is cached (setBackgroundSize() invalidates the cache)
         */
        QPixmap renderBackground();
        /**
         * Set s the path were custom background pictures are located.
         */
        void setCustomBackgroundPath(const QString &path);
        /**
         * Returns a random pixmap from the custom background path.
         * If no picture is located in this path the pixmap is null.
         */
        QPixmap getRandomBackgroundPixmap(const QString& path);
        bool loadNewBackgroundPixmap();

    private:
        QSvgRenderer m_svgRenderer;
        QSize m_backgroundSize;
        QPixmap m_cachedBackground;
        QPixmap m_randomBackground;

        QString m_customBackgroundPath;
        bool m_useRandomBackgrounds;
};

#endif //RENDERER_H

