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

#include "board.h"

#include <kdebug.h>
#include <KGlobal>
#include <KLocale>
#include <KRandom>
#include <KStandardDirs>
#include <KUrl>
#include <Phonon/MediaObject>

#include <QTimer>
#include "ball.h"
#include "gameobject.h"
#include "wall.h"

#include <cmath>

#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

KBounceBoard::KBounceBoard( KBounceRenderer* renderer, KGameCanvasAbstract* canvas, QWidget* parent )
    : QObject( parent ), KGameCanvasGroup( canvas ), m_renderer( renderer )
{
    m_tilesPix = new KGameCanvasPixmap( this );
    m_tilesPix->moveTo( 0, 0 );
    m_tilesPix->hide();

    m_clock = new QTimer( this );
    m_clock->setInterval( GAME_DELAY );
    connect( m_clock, SIGNAL( timeout() ), this, SLOT( tick() ) );

    m_walls.append( new KBounceWall( KBounceWall::Up, m_renderer, this ) );
    m_walls.append( new KBounceWall( KBounceWall::Right, m_renderer, this ) );
    m_walls.append( new KBounceWall( KBounceWall::Down, m_renderer, this ) );
    m_walls.append( new KBounceWall( KBounceWall::Left, m_renderer, this ) );
    foreach( KBounceWall* wall, m_walls )
    {
	wall->hide();
	connect( wall, SIGNAL( died() ), this, SIGNAL( wallDied() ) );
	connect( wall, SIGNAL( finished( int, int, int, int ) ), this, SLOT( wallFinished( int, int, int, int ) ) );
    }

    clear();

    m_audioPlayer = 0;
    m_playSounds = false;
    m_soundPath = QString();
}

KBounceBoard::~KBounceBoard()
{
    qDeleteAll( m_balls );
    qDeleteAll( m_walls );
    delete m_tilesPix;
    delete m_audioPlayer;
}

void KBounceBoard::resize( QSize& size )
{
    int minTileSize;
    if ( TILE_NUM_H * size.width() - TILE_NUM_W * size.height() > 0 )
	minTileSize = size.height() / TILE_NUM_H;
    else
	minTileSize = size.width() / TILE_NUM_W;

    m_tileSize = QSize( minTileSize, minTileSize );
    foreach( KBounceBall* ball, m_balls )
	ball->resize( m_tileSize );
    foreach( KBounceWall* wall, m_walls )
	wall->resize( m_tileSize );

    redraw();

    size.setWidth( minTileSize * TILE_NUM_W );
    size.setHeight( minTileSize * TILE_NUM_H );
}

void KBounceBoard::redraw()
{
    if ( m_tileSize.isEmpty() )
    {
	m_tilesPix->setPixmap( QPixmap() );
	m_tilesPix->hide();
    }
    else
    {
	
   	QPixmap px( m_tileSize.width() * TILE_NUM_W, m_tileSize.height() * TILE_NUM_H );
	px.fill( QColor( 0, 0, 0, 0 ) );

	QPainter p( &px );

	for ( int i = 0; i < TILE_NUM_W; i++ )
	{
	    for ( int j = 0; j < TILE_NUM_H; j++ )
	    {
		switch ( m_tiles[i][j] )
		{
		    case Free: 
			p.drawPixmap( i * m_tileSize.width(), j* m_tileSize.height(), m_renderer->renderElement( "gridTile", m_tileSize ) );
			break;
		    case Border:
		    case Wall:
			p.drawPixmap( i * m_tileSize.width(), j * m_tileSize.height(), m_renderer->renderElement( "wallTile", m_tileSize ) );
			break;
		    default:
			break;
		}
	    }
	}

	p.end();

	m_tilesPix->setPixmap( px );
	m_tilesPix->show();
    }

    foreach( KBounceBall* ball, m_balls )
    {
	ball->resetPixmaps();
    }
    foreach( KBounceWall* wall, m_walls )
    {
	wall->update();
    }
}

void KBounceBoard::newLevel( int level )
{
    m_clock->stop();
    clear();
    emit fillChanged( m_filled );

    while ( m_balls.count() > level + 1 )
    {
	delete m_balls.back();
	m_balls.removeLast();
    }
    while ( m_balls.count() < level + 1)
    {
	KBounceBall* ball = new KBounceBall( m_renderer, this );
	ball->resize( m_tileSize );
	m_balls.append( ball );
    }
    foreach( KBounceBall* ball, m_balls )
    {
	ball->setRelativePos( 4 + KRandom::random() % ( TILE_NUM_W - 8 ),
	    4 + KRandom::random() % ( TILE_NUM_H - 8 ) );
	ball->setVelocity( ((KRandom::random() & 1)*2-1)*0.125, 
		((KRandom::random() & 1)*2-1)*0.125 );
	ball->raise();
	ball->setRandomFrame();
	ball->show();
    }
    emit ballsChanged( level + 1 );

    foreach( KBounceWall* wall, m_walls )
    {
	wall->hide();
    }
}

void KBounceBoard::setPaused( bool val )
{
    if ( val )
	m_clock->stop();
    else
	m_clock->start();
}

void KBounceBoard::buildWall( const QPoint& pos, bool vertical )
{
    QPointF unmapped = unmapPosition( pos );
    int x = static_cast<int>( unmapped.x() );
    int y = static_cast<int>( unmapped.y() );

    if ( x < 0 || x >= TILE_NUM_W )
	return;
    if ( y < 0 || y >= TILE_NUM_H )
	return;
    if ( m_tiles[x][y] != Free )
	return;

    if ( !vertical )
    {
	m_walls[DIR_LEFT]->build( x, y );
	m_walls[DIR_RIGHT]->build( x, y );
    }
    else
    {
	m_walls[DIR_UP]->build( x, y );
	m_walls[DIR_DOWN]->build( x, y );
    }
}

int KBounceBoard::balls()
{
    return m_balls.count();
}

int KBounceBoard::filled()
{
    return m_filled;
}

KBounceCollision KBounceBoard::checkCollision( void* object, const QRectF& rect, int type )
{
    KBounceCollision result;
    
    if ( (type & TILE) != 0 )
    {
	result += checkCollisionTiles( rect );
    }

    if ( (type & WALL) != 0 )
    {
	foreach( KBounceWall* wall, m_walls )
	{
	    if ( object != wall )
	    {
		if ( wall->visible() && rect.intersects( wall->nextBoundingRect() ) )
		{
		    KBounceHit hit;
		    hit.type = WALL;
		    hit.boundingRect = wall->nextBoundingRect();
		    hit.normal = KBounceVector::normal( rect, hit.boundingRect );
		    result += hit;
		}
	    }
	}
    }

    if ( (type & BALL) != 0 )
    {
	foreach( KBounceBall* ball, m_balls )
	{
	    if ( object != ball )
	    {
		if ( rect.intersects( ball->nextBoundingRect() ) )
		{
		    KBounceHit hit;
		    hit.type = BALL;
		    hit.boundingRect = ball->nextBoundingRect();
		    hit.normal = KBounceVector::normal( rect, hit.boundingRect );
		    result += hit; 
		}
	    }
	}
    }

    return result;
}

KBounceCollision KBounceBoard::checkCollisionTiles( const QRectF& rect)
{
    KBounceVector normal( 0, 0 );

    // This small constant is added to each of the coordinates to 
    // avoid positive collision test result when tested rect lies 
    // on the edge of non-free space
    qreal D = 0.01;

    QPointF p = rect.topLeft();  
    int ul = m_tiles[static_cast<int>( p.x() + D )][static_cast<int>( p.y() + D )];
    if ( ul != Free ) normal += KBounceVector( 1, 1 );

    p = rect.topRight();
    int ur = m_tiles[static_cast<int>( p.x() - D )][static_cast<int>( p.y() + D )];
    if ( ur != Free) normal += KBounceVector( -1, 1 );

    p = rect.bottomRight();
    int lr = m_tiles[static_cast<int>( p.x() - D )][static_cast<int>( p.y() - D )];
    if ( lr != Free ) normal += KBounceVector( -1, -1 );

    p = rect.bottomLeft();
    int ll = m_tiles[static_cast<int>( p.x() + D )][static_cast<int>( p.y() - D )];
    if ( ll != Free ) normal += KBounceVector( 1, -1 );

    KBounceCollision collision;
    if ( (ul != Free ) || ( ur != Free ) || ( lr != Free ) || ( ll != Free ) )
    {
	KBounceHit hit;
	hit.type = TILE;
	hit.normal = normal;
	collision += hit;
    }
    return collision;
}

void KBounceBoard::checkCollisions()
{
    foreach( KBounceWall* wall, m_walls )
    {
	QRectF rect = wall->nextBoundingRect();
	KBounceCollision collision;
	collision = checkCollision( wall, rect, ALL );
	wall->collide( collision );
    }
    foreach( KBounceBall* ball, m_balls )
    {
	QRectF rect = ball->nextBoundingRect();
	KBounceCollision collision;
	collision = checkCollision( ball, rect, ALL );
	ball->collide( collision );
    }
}

QPoint KBounceBoard::mapPosition( const QPointF& pos ) const
{
    return QPoint( static_cast<int>( m_tileSize.width() * pos.x() ),
	   static_cast<int>(  m_tileSize.height() * pos.y() ) );
}

QPointF KBounceBoard::unmapPosition( const QPoint& pos ) const
{
    return QPointF( 1.0 * pos.x() / m_tileSize.width(), 1.0 * pos.y() / m_tileSize.height() );
}

void KBounceBoard::playSound( const QString& name )
{
    if ( m_playSounds == true && m_soundPath != QString() )
    {
	QString fileName = m_soundPath + name;
	m_audioPlayer->setCurrentSource( fileName );
	m_audioPlayer->play();
    }
}

void KBounceBoard::setSounds( bool val )
{
    m_playSounds = val;
    if ( val == true && m_audioPlayer == 0 )
	m_audioPlayer = Phonon::createPlayer( Phonon::GameCategory );
}

void KBounceBoard::setSoundPath( const QString& path )
{
    m_soundPath = path;
}

void KBounceBoard::tick()
{
    checkCollisions();

    foreach( KBounceBall* ball, m_balls )
	ball->advance(); 
    foreach( KBounceWall* wall, m_walls )
	wall->advance();

    foreach( KBounceBall* ball, m_balls )
	ball->update();
    foreach( KBounceWall* wall, m_walls )
	wall->update();
}

void KBounceBoard::wallFinished( int x1, int y1, int x2, int y2 )
{
    for ( int x = x1; x < x2; x++ )
	for ( int y = y1; y < y2; y++ )
	    m_tiles[x][y] = Wall;

    foreach ( KBounceBall* ball, m_balls )
    {
	int x = static_cast<int>( ball->relativePos().x() );
	int y = static_cast<int>( ball->relativePos().y() );
	fill( x, y );
    }

    for ( int x = 0; x < TILE_NUM_W; x++ )
	for ( int y = 0; y < TILE_NUM_H; y++ )
	    if ( m_tiles[x][y] == Free )
		m_tiles[x][y] = Wall;
    for ( int x = 0; x < TILE_NUM_W; x++ )
	for ( int y = 0; y < TILE_NUM_H; y++ )
	    if ( m_tiles[x][y] == Temp )
		m_tiles[x][y] = Free;

    redraw();

    int filled = 0;
    for ( int i = 1; i < TILE_NUM_W - 1; i++ )
	for ( int j = 1; j < TILE_NUM_H - 1; j++ )
	    if ( m_tiles[i][j] == Wall )
		filled++;
    m_filled = filled * 100 / ( ( TILE_NUM_W - 2 ) * ( TILE_NUM_H - 2 ) );

    emit fillChanged( m_filled );
}

void KBounceBoard::clear()
{
    for ( int i = 0; i < TILE_NUM_W; i++ )
	m_tiles[i][0] = m_tiles[i][TILE_NUM_H-1] = Border;
    for ( int j = 0; j < TILE_NUM_H; j++ )
	m_tiles[0][j] = m_tiles[TILE_NUM_W-1][j] = Border;
    for ( int i = 1; i < TILE_NUM_W - 1; i++ )
	for ( int j = 1; j < TILE_NUM_H -1; j++ )
	    m_tiles[i][j] = Free;
    m_filled = 0;
}

void KBounceBoard::fill( int x, int y )
{
    if ( m_tiles[x][y] != Free )
	return;
    m_tiles[x][y] = Temp;

    if ( y > 0 ) fill( x, y - 1 );
    if ( x < TILE_NUM_W - 1 ) fill ( x + 1, y );
    if ( y < TILE_NUM_H - 1 ) fill ( x, y + 1 );
    if ( x > 0 ) fill ( x - 1, y );
}

#include "board.moc"

