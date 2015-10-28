/*
 * Copyright (C) 2007 Tomasz Boczkowski <tboczkowski@onet.pl>
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


#include "ball.h"

#include <cmath>

#include <QDebug>

#include <KRandom>

#include "board.h"
#include "renderer.h"

const int KBounceBall::BALL_ANIM_DELAY = 50;
const qreal KBounceBall::BALL_RELATIVE_SIZE = 0.8;

KBounceBall::KBounceBall( KBounceRenderer* renderer, KBounceBoard* board )
: KGameRenderedItem(renderer,QLatin1String(""), board ), m_renderer( renderer ), m_board( board ),
    m_soundDelay( 0 ), m_size( QSize( 16, 16 ) ), m_framesNum( 0 ), m_xPos( 0 ), m_yPos( 0 ) 
{
	setSpriteKey(QStringLiteral("ball"));
	resetPixmaps();
	m_nextBoundingRect.setSize( QSizeF( BALL_RELATIVE_SIZE, BALL_RELATIVE_SIZE ) );
}


KBounceBall::~KBounceBall()
{
}

void KBounceBall::goForward()
{
   if ( m_reflectX )
   {
       m_velocity.x = -m_velocity.x;
       m_reflectX = false;
   }
   
   if ( m_reflectY )
   {
       qDebug() << "Reflecting ball Y";
       m_velocity.y = -m_velocity.y;
       m_reflectY = false;
   }

   m_xPos += m_velocity.x;
   m_yPos += m_velocity.y;

   m_nextBoundingRect.moveTo( m_xPos + m_velocity.x, m_yPos + m_velocity.y );
}

void KBounceBall::collide( const KBounceCollision& collision )
{
    foreach ( const KBounceHit &hit, collision )
    {
		if ( hit.type == TILE || hit.type == WALL )
		{
		    if ( hit.normal.x > 0 && m_velocity.x < 0 )
			m_reflectX = true;
		    if ( hit.normal.x < 0 && m_velocity.x > 0 )
			m_reflectX = true;
		    if ( hit.normal.y > 0 && m_velocity.y < 0 )
			m_reflectY = true;
		    if ( hit.normal.y < 0 && m_velocity.y > 0 )
			m_reflectY = true;
		}
    }
}

void KBounceBall::update()
{
	setFrame( frame()+1 );
	setPos( m_board->mapPosition( QPointF( m_xPos, m_yPos ) ) );
}

void KBounceBall::resize( const QSize& tileSize )
{
    qDebug() << "New size:" << tileSize;

    m_size.setWidth( static_cast<int>( BALL_RELATIVE_SIZE * tileSize.width() ) );
    m_size.setHeight( static_cast<int> ( BALL_RELATIVE_SIZE * tileSize.height() ) );
	setRenderSize(m_size);
    setPos( m_board->mapPosition( QPointF( m_xPos, m_yPos ) ) );
}

void KBounceBall::resetPixmaps()
{
	m_framesNum = frameCount();
	setFrame( 1 );
}

void KBounceBall::setRandomFrame()
{
    int frame = 1;
    if ( m_framesNum > 1 )
	{
		frame = KRandom::random() % m_framesNum;
	}
    setFrame( frame );
}

QRectF KBounceBall::ballBoundingRect() const
{
    return QRectF( m_xPos, m_yPos, BALL_RELATIVE_SIZE, BALL_RELATIVE_SIZE );
}

QRectF KBounceBall::nextBoundingRect() const
{
    return m_nextBoundingRect;
}

QPointF KBounceBall::relativePos()
{
    return QPointF( m_xPos, m_yPos );
}

void KBounceBall::setRelativePos( qreal x, qreal y )
{
    m_xPos = x;
    m_yPos = y;
    setPos( m_board->mapPosition( QPointF( m_xPos, m_yPos ) ) );
}

void KBounceBall::setVelocity( qreal vX, qreal vY )
{
    m_velocity.x = vX;
    m_velocity.y = vY;
}

KBounceVector KBounceBall::velocity() const
{
    return KBounceVector( m_velocity.x, m_velocity.y );
}

