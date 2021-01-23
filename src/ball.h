/*
    This file is part of the KDE project "KBounce"

    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <1Stein@gmx.de>
    SPDX-FileCopyrightText: 2007 Tomasz Boczkowski <tboczkowski@onet.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALL_H
#define BALL_H

#include <KGameRenderedItem>

#include "gameobject.h"

class KBounceRenderer;
class KBounceBoard;

/**
 * KGameRenderedItem representing a ball
 */
class KBounceBall : public KGameRenderedItem
{
    public:
        static const int BALL_ANIM_DELAY;
        static const qreal BALL_RELATIVE_SIZE;

        /**
         * Constructor
         */
        KBounceBall( KBounceRenderer* renderer, KBounceBoard* board );
        /**
         * Destructor
         */
        ~KBounceBall() override;

        /**
         * Changes ball's state when collisions have been detected
         * Called once per frame before advance()
         */
        void collide( const KBounceCollision& collision );
        /**
         * Performs move calculations
         * This method is called once per frame
         */
        void goForward();
        /**
         * Updates ball position and pixmap.
         * This method is called once per frame.
         */
        void update();

        /*
         * Returns ball's bounding rect in board coordinate system
         * @see relativePos()
         */
        QRectF ballBoundingRect() const;
        /*
         * Returns ball's bounding rect expected in next frame
         * used by colision test
         */
        QRectF nextBoundingRect() const;
        /**
         * Returns ball's position in board coordinate system.
         * Relative board's coordinates are indepentent of actual GameWidget size.
         */
        QPointF relativePos();
        /**
         * Sets ball's position in board coordinate system.
         * @see relativePos()
         */
        void setRelativePos( qreal x, qreal y );
        /**
         * Sets ball's position in board coordinate system
         */
        void setVelocity( qreal vX, qreal vY );
        /**
         * Returns ball's velocity in board coordinate system
         */
        KBounceVector velocity() const;


        /**
         * Sets width and height of ball.
         */
        void resize( const QSize& tileSize );
        /**
         * Rechecks the number of frames of ball animation and sets new pixmaps.
         * This method is useful when changing game theme.
         */
        void resetPixmaps();
        /**
         * Sets a random ball's frame
         */
        void setRandomFrame();

    protected:
        KBounceRenderer* m_renderer;
        KBounceBoard* m_board;
        /**
         * Time after emiting previous sound. If the value is too small,
         * ball will not emit hit sound.
         */
        int m_soundDelay;
        /**
         * Size of a ball in GameWidget depentant coordinate system
         */
        QSize m_size;
        /**
         * Number of frames of ball's animation.
         */
        int m_framesNum;
        qreal m_xPos;
        qreal m_yPos;
        KBounceVector m_velocity;
        bool m_reflectX;
        bool m_reflectY;
        QRectF m_nextBoundingRect;
};

#endif
