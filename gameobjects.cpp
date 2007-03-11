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

#include <cmath>

#include <kdebug.h>
#include <KRandom>

#include <QString>

#include "board.h"
#include "gameobjects.h"
#include "renderer.h"

#define BALL_ANIM_DELAY 60
#define BALL_RELATIVE_SIZE 0.8
#define GAME_DELAY 15
#define WALL_VELOCITY 0.125
#define MS2TICKS( ms ) ((ms)/GAME_DELAY)

KBounceBall::KBounceBall( KBounceRenderer* renderer, KBounceBoard* board )
    : KGameCanvasPixmap( board ), m_renderer( renderer ), m_board( board ),
    m_soundDelay( MS2TICKS(BALL_ANIM_DELAY)/2 ), m_size( QSize( 16, 16 ) ), m_frame( 0 ),
    m_xPos( 0 ), m_yPos( 0 ), m_xVelocity( 0 ), m_yVelocity( 0 )
{
    resetPixmaps();
}


KBounceBall::~KBounceBall()
{
}

void KBounceBall::advance()
{
   bool reflectX = false;
   bool reflectY = false;
   QRectF r;

   // ball already on a wall? (should normally never happen)
   // commented out to stop bug which causes balls to
   // sometimes stop when clicked on
   r = relativeBoundingRect();
   if ( m_board->checkCollision( r, false ) ) 
       kDebug() << k_funcinfo << "On wall " << endl;

   r = relativeBoundingRect();
   r.translate( m_xVelocity, 0.0 );
   if ( m_board->checkCollision( r, false ) )
   {
       reflectX = true;
   }

   r = relativeBoundingRect();
   r.translate( 0.0, m_yVelocity );
   if ( m_board->checkCollision( r, false ) )
   {
       reflectY = true;
   }

   r = relativeBoundingRect();
   r.translate( m_xVelocity , m_yVelocity );
   if ( m_board->checkCollision( r, true ) )
       if ( !reflectX && !reflectY )
       {
           reflectX = reflectY = true;
       }

   // apply reflection
   if ( reflectX ) m_xVelocity = -m_xVelocity;
   if ( reflectY ) m_yVelocity = -m_yVelocity;

   // play collision sound
   m_soundDelay++;
   if ( reflectX || reflectY )
   {
     //TODO: readd sound support  if ( m_soundDelay>50 ) JezzGame::playSound( "reflect.au" );
       m_soundDelay = 0;
   }

   m_xPos += m_xVelocity;
   m_yPos += m_yVelocity;

   update();

}

void KBounceBall::update()
{
    m_frame++;
    if ( m_frame >= m_frames.count() )
	m_frame = 0;

    setFrame( m_frame );
   moveTo( m_board->mapPosition( QPointF( m_xPos, m_yPos ) ) );
}

void KBounceBall::resize( const QSize& tileSize )
{
    kDebug() << k_funcinfo << " New size: " << tileSize << endl;

    m_size.setWidth( static_cast<int>( BALL_RELATIVE_SIZE * tileSize.width() ) );
    m_size.setHeight( static_cast<int> ( BALL_RELATIVE_SIZE * tileSize.height() ) );
    moveTo( m_board->mapPosition( QPointF( m_xPos, m_yPos ) ) ); 
    if( !m_frames.isEmpty() )
    {
        setPixmap( m_renderer->renderElement( m_frames.at( m_frame ), m_size ) );
    }
} 

void KBounceBall::resetPixmaps()
{
    m_frames.clear();

    QString name = "ball";
    int frames = 0;
    while ( m_renderer->elementExists( QString( "%1_%2" ).arg( name ).arg( frames )  ) )
    {
       m_frames.append( QString( "%1_%2" ).arg( name ).arg( frames ) );
       frames++;
    }

    kDebug() << k_funcinfo << ". Frames: " << frames << endl;
	
    m_frame = 0;
    setFrame( m_frame );
}

void KBounceBall::setFrame(int frame)
{
    if( !m_frames.isEmpty() )
    {
        setPixmap( m_renderer->renderElement( m_frames.at( frame ), m_size ) );
    }
}

void KBounceBall::setRandomFrame()
{
    if ( !m_frames.empty() )
    {
	int frame = KRandom::random() % m_frames.count();
    	kDebug() << k_funcinfo << endl;
	setFrame( frame );
    }
}

QRectF KBounceBall::relativeBoundingRect() const
{
    return QRectF( m_xPos, m_yPos, BALL_RELATIVE_SIZE, BALL_RELATIVE_SIZE );
}

QPointF KBounceBall::relativePos()
{
    return QPointF( m_xPos, m_yPos );
}

void KBounceBall::setRelativePos( qreal x, qreal y )
{
    m_xPos = x;
    m_yPos = y;
    moveTo( m_board->mapPosition( QPointF( m_xPos, m_yPos ) ) );
}

void KBounceBall::setVelocity( qreal vX, qreal vY )
{
    m_xVelocity = vX;
    m_yVelocity = vY;
}


KBounceWall::KBounceWall( Direction dir, KBounceRenderer* renderer, KBounceBoard* board )
    : KGameCanvasPixmap( board ), m_renderer( renderer ), m_board( board ), 
    m_tileSize( QSize( 16, 16 ) ), m_dir( dir )
{
    switch ( m_dir )
    {
	case Up:
	    m_velocityX = 0; m_velocityY = -WALL_VELOCITY; break;
	case Right:
	    m_velocityX = WALL_VELOCITY; m_velocityY = 0; break;
	case Down:
	    m_velocityX = 0; m_velocityY = WALL_VELOCITY; break;
	case Left:
	    m_velocityX = -WALL_VELOCITY; m_velocityY = 0; break;
    }
}

KBounceWall::~KBounceWall()
{
}

void KBounceWall::advance()
{
    if ( !visible() )
	return;

    QRectF rect = relativeBoundingRect();
    rect.translate( m_velocityX, m_velocityY );
    if ( m_board->checkCollisionTiles( rect ) )
    {
	if ( m_dir == Up || m_dir == Down )
    	    emit finished( static_cast<int>( m_x1 ), static_cast<int>( m_y1 ),
		    static_cast<int>( m_x2 ), static_cast<int>( m_y2 - 0.1 ) );
	else
	    emit finished( static_cast<int>( m_x1 ), static_cast<int>( m_y1 ),
		    static_cast<int>( m_x2 - 0.1 ), static_cast<int>( m_y2 ) );
	hide();
    }
    else
    {
	switch( m_dir )
	{
	    case Up:
	    case Left:
		m_x1 += m_velocityX;
		m_y1 += m_velocityY;
		break;
	    case Right:
	    case Down:
		m_x2 += m_velocityX;
		m_y2 += m_velocityY;
		break;
	}
    }

    update();
}

void KBounceWall::update()
{
    qreal dx = m_x2 > m_x1 ? m_x2 - m_x1 : m_x1 - m_x2;
    qreal dy = m_y2 > m_y1 ? m_y2 - m_y1 : m_y1 - m_y2;
    if ( dx == 0 && dy != 0 ) dx = 1;
    if ( dy == 0 && dx != 0 ) dy = 1;

    int width = static_cast<int>( ceil( dx * m_tileSize.width() ) );
    int height = static_cast<int>( ceil( dy * m_tileSize.height()  ) );
    int tileWidth = m_tileSize.width();
    int tileHeight = m_tileSize.height();
	
    if ( width == 0 && height != 0 ) width = m_tileSize.width();
    if ( height == 0 && width != 0 ) height = m_tileSize.height();
    if ( width == 0 || height == 0 )
    {
	QPixmap px( m_tileSize );
	px.fill( QColor( 0, 0, 0 , 0 ) );
	setPixmap( px );
	return;
    }

    QPixmap px( width, height );
    px.fill( QColor( 0, 0, 0, 0 ) );

    QPainter p( &px );

    switch( m_dir )
    {
	case Up:
	    if ( dy <= 1 )
	    {
		p.drawPixmap( QRect( 0, 0, tileWidth, height ),
		    m_renderer->renderElement( "wallEndUp", m_tileSize ),
		    QRect( 0, 0, tileWidth, height ) );
	    }
	    else
	    {
		p.drawPixmap( QRect( 0, tileHeight, width, height - tileHeight ),
			m_renderer->renderElement( "wallV", QSize( tileWidth, 18 * tileHeight ) ),
			QRect( 0, 18 * tileHeight - height, width, height - tileHeight ) );
		p.drawPixmap( QRect( 0, 0, tileWidth, tileHeight ),
			m_renderer->renderElement( "wallEndUp", m_tileSize ),
			QRect( 0, 0, tileWidth, tileHeight ) );
	    }
	    break;
	case Right:
	    if ( dx <= 1 )
	    {
		p.drawPixmap( QRect( 0, 0, width, tileHeight ),
			m_renderer->renderElement( "wallEndRight", m_tileSize ),
			QRect( tileWidth - width, 0, width, tileHeight ) );
	    }
	    else
	    {
		p.drawPixmap( QRect( 0, 0, width - tileWidth, height ),
			m_renderer->renderElement( "wallH", QSize( 32 * tileWidth, tileHeight ) ),
			QRect( 0, 0,width - tileWidth, height ) );
		p.drawPixmap( QRect( width - tileWidth, 0, tileWidth, tileHeight ),
			m_renderer->renderElement( "wallEndRight", m_tileSize ),
		       QRect( 0, 0, tileWidth, tileHeight ) );
	    }
	    break;
	case Down:
	    if ( dy <= 1 )
	    {
		p.drawPixmap( QRect( 0, 0, tileWidth, height ),
			m_renderer->renderElement( "wallEndDown", m_tileSize ),
			QRect( 0, tileHeight - height, tileWidth, height ) );
	    }
	    else
	    {
		p.drawPixmap( QRect( 0, 0, width, height - tileHeight ),
		    m_renderer->renderElement( "wallV", QSize( tileWidth, 18 * tileHeight ) ),
		    QRect( 0, 0, width, height - tileHeight ) );
		p.drawPixmap( QRect( 0, height - tileHeight, tileWidth, tileHeight ),
			m_renderer->renderElement( "wallEndDown", m_tileSize ),
			QRect( 0, 0, tileWidth, tileHeight ) );
	    }
	    break;
	case Left:
	    if ( dx <= 1 )
	    {
		p.drawPixmap( QRect( 0, 0, width, tileHeight ),
			m_renderer->renderElement( "wallEndLeft", m_tileSize ),
			QRect( 0, 0, width, tileHeight ) );
	    }
	    else
	    {
		p.drawPixmap( QRect( tileWidth, 0, width - tileWidth, height ),
			m_renderer->renderElement( "wallH", QSize( 32 * tileWidth, tileHeight ) ),
			QRect( 32 * tileWidth - width, 0, width - tileWidth, height ) );
		p.drawPixmap( QRect( 0, 0, tileWidth, tileHeight ),
			m_renderer->renderElement( "wallEndLeft", m_tileSize ),
			QRect( 0, 0, tileWidth, tileHeight ) );
	    }
	    break;
    }

    moveTo( m_board->mapPosition( QPointF( m_x1, m_y1 ) ) );
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

void KBounceWall::collide( const QRectF& )
{
    emit died();
    hide();
}

void KBounceWall::build( int x, int y )
{
    if ( visible() )
	return;

    m_x1 = m_x2 = x;
    m_y1 = m_y2 = y;

    update();
    moveTo( m_board->mapPosition( QPointF( x, y ) ) );
    show();
}

QRectF KBounceWall::relativeBoundingRect() const
{
    qreal dx = m_x1 > m_x2 ? m_x1 - m_x2 : m_x2 - m_x1;
    qreal dy = m_y1 > m_y2 ? m_y1 - m_y2 : m_y2 - m_y1;
    if ( dx == 0 && dy != 0 ) dx = 0.98;
    if ( dy == 0 && dx != 0 ) dy = 0.98;
    QRectF rect = QRectF( m_x1 + 0.1 , m_y1 + 0.1 , dx, dy );
    return rect;
}

#include "gameobjects.moc"

