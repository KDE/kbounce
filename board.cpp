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

#include <kdebug.h>
#include <KGlobal>
#include <KLocale>
#include <KRandom>
#include <KStandardDirs>

#include <QTimer>

#include "board.h"
#include "gameobjects.h"

#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3
#define GAME_DELAY 15


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
}

KBounceBoard::~KBounceBoard()
{
    qDeleteAll( m_balls );
    qDeleteAll( m_walls );
    delete m_tilesPix;
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

bool KBounceBoard::checkCollision( const QRectF& rect, bool feedback )
{
    bool result = checkCollisionTiles( rect, feedback );

    foreach( KBounceWall* wall, m_walls )
    {
	if ( wall->visible() && rect.intersects( wall->relativeBoundingRect() ) )
	{
	    result = true;
	    if ( feedback )
		wall->collide( rect );
	}
    }

    return result;
}

bool KBounceBoard::checkCollisionTiles( const QRectF& rect, bool feedback)
{
    QPointF p = rect.topLeft();  
    int ul = m_tiles[static_cast<int>( p.x() )][static_cast<int>( p.y() )];
    p = rect.topRight();
    int ur = m_tiles[static_cast<int>( p.x() )][static_cast<int>( p.y() )];
    p = rect.bottomRight();
    int lr = m_tiles[static_cast<int>( p.x() )][static_cast<int>( p.y() )];
    p = rect.bottomLeft();
    int ll = m_tiles[static_cast<int>( p.x() )][static_cast<int>( p.y() )];

    return (ul != Free ) || ( ur != Free ) || ( lr != Free ) || ( ll != Free );
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

void KBounceBoard::tick()
{
    foreach( KBounceBall* ball, m_balls )
	ball->advance(); 
    foreach( KBounceWall* wall, m_walls )
	wall->advance();

    static int count = 2;
    count--;
    if ( count == 0 )
    {
	count = 2;
	foreach( KBounceBall* ball, m_balls )
	    ball->update();
    }
}

void KBounceBoard::wallFinished( int x1, int y1, int x2, int y2 )
{
    for ( int x = x1; x <= x2; x++ )
	for ( int y = y1; y <= y2; y++ )
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

