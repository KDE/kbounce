/*
 * Copyright (C) 2000-2005 Stefan Schimanski <1Stein@gmx.de>
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

#ifndef BALL_H
#define BALL_H

#include "kgamecanvas.h"


#include "gameobject.h"

class KBounceRenderer;
class KBounceBoard;

/**
 * KGameCanvasPixmap representing a ball
 */
class KBounceBall : public KGameCanvasPixmap
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
	~KBounceBall();

	/**
	 * Changes ball's state when collisions have been detected
	 * Called once per frame before advance()
	 */
	void collide( const KBounceCollision& collision );
	/**
	 * Performs move calculations
	 * This method is called once per frame
	 */
	void advance();
	/**
	 * Updates ball position and pixmap.
	 * This method is called once per frame.
	 */
	void update();

	/*
	 * Returns ball's bounding rect in board coordinate system
	 * @see relativePos()
	 */
	QRectF boundingRect() const;
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
	 * Sets ball's current frame
	 */
	void setFrame(int frame);
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
	 * Current frame of ball's animation.
	 */
	int m_frame;
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

