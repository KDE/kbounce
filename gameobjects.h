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

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#include "kgamecanvas.h"

#include <QList>
#include <QObject>

class KBounceRenderer;
class KBounceBoard;

/**
 * KGameCanvasPixmap that displays balls
 */
class KBounceBall : public KGameCanvasPixmap
{
    public:
	/**
	 * Constructor
	 */
	KBounceBall( KBounceRenderer* renderer, KBounceBoard* board );
	/**
	 * Destructor
	 */
	~KBounceBall();
	/**
	 * Performs move and collision calculations.
	 * This method is called once per frame
	 */
	void advance();
	/**
	 * Updates ball position and pixmap.
	 * This method is called once per frame.
	 */
	void update();
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
	/*
	 * Returns ball's bounding rect in board coordinate system
	 * @see relativePos()
	 */
	QRectF relativeBoundingRect() const;
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
	qreal m_xVelocity;
	qreal m_yVelocity;
};


class KBounceWall : public QObject, public KGameCanvasPixmap
{
    Q_OBJECT

    public:
	enum Direction { Up = 0, Down, Left, Right };

	KBounceWall( Direction dir, KBounceRenderer* renderer, KBounceBoard* board );
	~KBounceWall();

	void advance();
    	void update();

	void resize( const QSize& tileSize );

	void collide( const QRectF& rect );
	void build( int x, int y );
	QRectF relativeBoundingRect() const;

    signals:
	void finished( int x1, int y1, int x2, int y2 );
	void died();

    private:
	KBounceRenderer *m_renderer;
	KBounceBoard *m_board;
	QSize m_tileSize;
	Direction m_dir;
	qreal m_x1, m_y1;
	qreal m_x2, m_y2;
	qreal m_velocityX, m_velocityY;
};

#endif //GAMEOBJECTS_H

