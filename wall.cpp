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

#include "wall.h"

#include <cmath>

#include <kdebug.h>
#include <KRandom>

#include "board.h"
#include "renderer.h"

const qreal KBounceWall::WALL_VELOCITY = 0.125;

KBounceWall::KBounceWall( Direction dir, KBounceRenderer* renderer, KBounceBoard* board )
    : KGameCanvasPixmap( board ), m_renderer( renderer ), m_board( board ), 
    m_dir( dir ), m_tileSize( QSize( 16, 16 ) )
{
}

KBounceWall::~KBounceWall()
{
}

void KBounceWall::collide( KBounceCollision collision )
{
    if ( !visible() )
	return;

    foreach( const KBounceHit &hit, collision )
    {
	if ( hit.type == TILE )
	{
	    finish();
	}
	if ( hit.type == BALL )
	{
	    if ( safeEdgeHit( hit.boundingRect ) )
	    {
		KBounceVector normal = hit.normal;
		bool vertical = qAbs(normal.x) < qAbs(normal.y);

		if ( vertical && ( (m_dir == Up) || (m_dir == Down) ) )
		{
		    finish( true, m_dir );
		}
		if ( !vertical && ( (m_dir == Left) || (m_dir == Right ) ) )
		{
		    finish( true, m_dir );
		}
	    }
	    else
	    {
		emit died();
		hide();
	    }
	}
	if ( hit.type == WALL )
	{
	    if ( safeEdgeHit( hit.boundingRect ) )
	    {
		finish();
	    }
	}
    }
}


void KBounceWall::advance()
{
    if ( !visible() ) 
    {
	return;
    }

    switch( m_dir )
    {
	case Up:
	    m_boundingRect.setTop( m_boundingRect.top() - WALL_VELOCITY );
	    m_nextBoundingRect.setTop( m_boundingRect.top() - WALL_VELOCITY );
	    break;
	case Left:
	    m_boundingRect.setLeft( m_boundingRect.left() - WALL_VELOCITY );
	    m_nextBoundingRect.setLeft( m_boundingRect.left() - WALL_VELOCITY );
	    break;
	case Down:
	    m_boundingRect.setBottom( m_boundingRect.bottom() + WALL_VELOCITY );
	    m_nextBoundingRect.setBottom( m_boundingRect.bottom() + WALL_VELOCITY );
	    break;
	case Right:
	    m_boundingRect.setRight( m_boundingRect.right() + WALL_VELOCITY );
	    m_nextBoundingRect.setRight( m_boundingRect.right() + WALL_VELOCITY );
	    break;
    }
}

void KBounceWall::update()
{
    if ( !visible() )
	return;

    int boundingRectWidth = static_cast<int>
	( std::ceil( m_boundingRect.width() * m_tileSize.width() ) );
    int boundingRectHeight = static_cast<int>
	( std::ceil( m_boundingRect.height() * m_tileSize.height()  ) );

    if ( boundingRectWidth == 0 || boundingRectHeight == 0 )
	return;

    int tileWidth = m_tileSize.width();
    int tileHeight = m_tileSize.height();

    QPixmap px( boundingRectWidth, boundingRectHeight );
    px.fill( QColor( 0, 0, 0, 0 ) );
    QPainter p( &px );

    switch ( m_dir ) 
    {
	case Up:
	    p.drawPixmap( 
		    QRect( 0, 0, tileWidth, qMin( tileHeight, boundingRectHeight ) ),
		    m_renderer->renderElement( "wallEndUp", m_tileSize ),
		    QRect( 0, 0, tileWidth, qMin( tileHeight, boundingRectHeight ) ) );
	    p.drawPixmap( 
		    QRect( 0, tileHeight, tileWidth, boundingRectHeight - tileHeight ),
		    m_renderer->renderElement( "wallV",
			QSize( tileWidth,  18 * tileHeight ) ),
		    QRect( 0, 18 * tileHeight - boundingRectHeight + tileHeight,
		       	tileWidth, boundingRectHeight - tileHeight ) );
	    break;
	case Right:
	    p.drawPixmap(
		    QRect( boundingRectWidth - tileWidth, 0, qMin( tileWidth, boundingRectWidth ), 
			    tileHeight ), 
		    m_renderer->renderElement( "wallEndRight", m_tileSize ),
		    QRect( 0, 0, qMin( tileWidth, boundingRectWidth ), tileHeight ) );
	    p.drawPixmap(
		    QRect( 0, 0, boundingRectWidth - tileWidth, tileHeight ),
		    m_renderer->renderElement( "wallH", 
			QSize( 32 * tileWidth, tileHeight ) ),
		    QRect( 0, 0, boundingRectWidth - tileWidth, tileHeight ) );
	    break;
	case Down:
	    p.drawPixmap(
		    QRect( 0, boundingRectHeight - tileHeight, tileWidth,
			qMin( tileHeight, boundingRectHeight ) ),
		    m_renderer->renderElement( "wallEndDown", m_tileSize ),
		    QRect( 0, qMax( 0, tileHeight - boundingRectHeight ), tileWidth,
			qMin( tileHeight, boundingRectHeight ) ) );
	    p.drawPixmap(
		    QRect( 0, 0, tileWidth, boundingRectHeight - tileHeight ),
		    m_renderer->renderElement( "wallV",
			QSize( tileWidth, 18 * tileHeight ) ),
		    QRect( 0, 0, tileWidth, boundingRectHeight - tileHeight ) );
	    break;
	case Left:
	    p.drawPixmap( 
		    QRect( 0, 0, qMin( boundingRectWidth, tileWidth ), tileHeight ),
		    m_renderer->renderElement( "wallEndLeft", m_tileSize ),
		    QRect( qMax( 0, tileWidth - boundingRectWidth ), 0,
			tileWidth, tileHeight ) );
	    p.drawPixmap( 
		    QRect( tileWidth, 0, boundingRectWidth - tileWidth, tileHeight ),
		    m_renderer->renderElement( "wallH", 
			QSize( 32 * tileWidth, tileHeight ) ),
		    QRect( 32 * tileWidth - boundingRectWidth + tileWidth, 0, 
			boundingRectWidth - tileWidth, tileHeight ) );
    }
    moveTo( m_board->mapPosition( m_boundingRect.topLeft() ) );
    p.end();
    setPixmap( px );
}

void KBounceWall::resize( const QSize& tileSize )
{
    if ( tileSize != m_tileSize )
    {
	m_tileSize = tileSize;
	update();
    }
}

void KBounceWall::build( int x, int y )
{
    if ( visible() )
	return;

    if ( m_dir == Up || m_dir == Down )
    {
	m_boundingRect.setTop( y );
	m_boundingRect.setBottom( y );
	m_boundingRect.setLeft( x );
	m_boundingRect.setRight( x + 1 );
    }
    if ( m_dir == Left || m_dir == Right )
    {
	m_boundingRect.setTop( y );
	m_boundingRect.setBottom( y + 1 );
	m_boundingRect.setLeft( x );
	m_boundingRect.setRight( x );
    }

    m_nextBoundingRect = m_boundingRect;

    setPixmap( QPixmap( 0, 0 ) );
    moveTo( m_board->mapPosition( QPointF( x, y ) ) );
    show();

    m_board->playSound( "wallstart.wav" );
}

QRectF KBounceWall::boundingRect() const
{
    return m_boundingRect;
}

QRectF KBounceWall::nextBoundingRect() const
{
    return m_nextBoundingRect;
}

bool KBounceWall::safeEdgeHit( const QRectF& rect2 ) const
{
	bool safeEdgeHit = false;

	QPointF p1, p2, p3;
	switch ( m_dir ) 
	{
	    case Up:
		p1 = m_nextBoundingRect.topLeft();
		p2 = m_nextBoundingRect.topRight();
		break;
	    case Right:
		p1 = m_nextBoundingRect.topRight();
		p2 = m_nextBoundingRect.bottomRight();
		break;
	    case Down:
		p1 = m_nextBoundingRect.bottomRight();
		p2 = m_nextBoundingRect.bottomLeft();
		break;
	    case Left:
		p1 = m_nextBoundingRect.bottomLeft();
		p2 = m_nextBoundingRect.topLeft();
		break;
		default:
		Q_ASSERT(false);
		break;
	}
	p3.setX( ( p1.x() + p2.x() ) / 2.0 );
	p3.setY( ( p1.y() + p2.y() ) / 2.0 );

	if ( rect2.contains( p1 ) )
	    safeEdgeHit = true;
	else if ( rect2.contains( p2 ) )
	    safeEdgeHit = true;
	else if ( rect2.contains( p3 ) )
	    safeEdgeHit = true;

	return safeEdgeHit;
}

void KBounceWall::finish( bool shorten, Direction dir )
{
    int left = static_cast<int>( m_boundingRect.left() );
    int top = static_cast<int>( m_boundingRect.top() );
    int right = static_cast<int>( m_boundingRect.right() );
    int bottom = static_cast<int>( m_boundingRect.bottom() );

    if ( shorten ) 
    {
	switch ( dir )
	{
	    case Left: left++; break;
	    case Up: top++; break;
	    case Right: right--; break;
	    case Down: bottom--; break;
	}
    }

    emit finished( left, top, right, bottom );
    hide();
    m_board->playSound( "wallend.wav" );
}

#include "wall.moc"

