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

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#include "kgamecanvas.h"

#include <QList>
#include <QObject>
#include <QString>

class KBounceRenderer;
class KBounceBoard;

class KBounceBall : public KGameCanvasPixmap
{
 public:
    KBounceBall( KBounceRenderer* renderer, KBounceBoard* board );
    ~KBounceBall();

    void advance();
    void update();

    void resize( const QSize& tileSize );
    void resetPixmaps();
    void setFrame(int frame);
    void setRandomFrame();

    QRectF relativeBoundingRect() const;
    QPointF relativePos();
    void setRelativePos( qreal x, qreal y );
    void setVelocity( qreal vX, qreal vY );

 protected:
    KBounceRenderer* m_renderer;
    KBounceBoard* m_board;
    int m_soundDelay;
    QSize m_size;
    QList<QString> m_frames;
    int m_frame;
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

