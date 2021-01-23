/*
    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <1Stein@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "board.h"

#include <QGraphicsScene>
#include <QTimer>
#include <QPainter>
#include <QRandomGenerator>

#include "ball.h"
#include "wall.h"
#include "debug.h"

#include <cmath>

#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3


KBounceBoard::KBounceBoard( KBounceRenderer* renderer )
    : QGraphicsObject()
    , m_renderer( renderer )
{
    m_clock = new QTimer( this );
    m_clock->setInterval( GAME_DELAY );
    connect(m_clock, &QTimer::timeout, this, &KBounceBoard::tick);

    m_walls.append( new KBounceWall( KBounceWall::Up, m_renderer, this ) );
    m_walls.append( new KBounceWall( KBounceWall::Right, m_renderer, this ) );
    m_walls.append( new KBounceWall( KBounceWall::Down, m_renderer, this ) );
    m_walls.append( new KBounceWall( KBounceWall::Left, m_renderer, this ) );
    for (KBounceWall* wall : qAsConst(m_walls)) {
        wall->hide();
        connect( wall, &KBounceWall::died, this, &KBounceBoard::wallDied );
        connect( wall, &KBounceWall::finished, this, &KBounceBoard::wallFinished );
    }

    clear();

    // Initialize this members with the old default values.
    m_ballVelocity = 0.125;
    m_wallVelocity = 0.125;
}

KBounceBoard::~KBounceBoard()
{
    qDeleteAll( m_balls );
    qDeleteAll( m_walls );
}

void KBounceBoard::resize( QSize& size )
{
    //Pause the clock to prevent ticks during resize ...
    bool alreadyPaused = false;
    if (!m_clock->isActive())
    {
        // ... but only when we are not already paused
        alreadyPaused = true;
    }
    else
    {
        setPaused(true);
    }

    int minTileSize;
    if ( TILE_NUM_H * size.width() - TILE_NUM_W * size.height() > 0 ) {
        minTileSize = size.height() / TILE_NUM_H;
    } else {
        minTileSize = size.width() / TILE_NUM_W;
    }

    m_tileSize = QSize( minTileSize, minTileSize );

    for (KBounceBall* ball : qAsConst(m_balls)) {
        ball->resize( m_tileSize );
    }

    for (KBounceWall* wall : qAsConst(m_walls)) {
        wall->resize( m_tileSize );
    }

    size.setWidth( minTileSize * TILE_NUM_W );
    size.setHeight( minTileSize * TILE_NUM_H );
    if (!alreadyPaused)
    {
        setPaused(false);
    }
}

void KBounceBoard::newLevel( int level )
{
    m_clock->stop();
    clear();
    Q_EMIT fillChanged( m_filled );

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
    for (KBounceBall* ball : qAsConst(m_balls)) {
        ball->setRelativePos( 4 + QRandomGenerator::global()->bounded( TILE_NUM_W - 8 ),
                4 + QRandomGenerator::global()->bounded( TILE_NUM_H - 8 ) );

        ball->setVelocity( (QRandomGenerator::global()->bounded(2)*2-1)*m_ballVelocity,
                (QRandomGenerator::global()->bounded(2)*2-1)*m_ballVelocity );
        ball->setRandomFrame();
        ball->show();
    }
    Q_EMIT ballsChanged( level + 1 );

    for (KBounceWall* wall : qAsConst(m_walls)) {
        wall->setWallVelocity(m_wallVelocity);
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

void KBounceBoard::setBallVelocity(qreal vel)
{
    m_ballVelocity = vel;
}

void KBounceBoard::setWallVelocity(qreal vel)
{
    m_wallVelocity = vel;
}

void KBounceBoard::buildWall( const QPointF& pos, bool vertical )
{
    QPointF unmapped( pos.x() - x(), pos.y() - y());
    int x = static_cast<int>( unmapped.x() / m_tileSize.width() );
    int y = static_cast<int>( unmapped.y() / m_tileSize.height() );

    if ( x < 0 || x >= TILE_NUM_W )
    {
        qCDebug(KBOUNCE_LOG) << "Wall x position out of board.";
        return;
    }
    if ( y < 0 || y >= TILE_NUM_H )
    {
        qCDebug(KBOUNCE_LOG) << "Wall y position out of board.";
        return;
    }
    if ( m_tiles[x][y] != Free )
    {
        qCDebug(KBOUNCE_LOG) << "Wall could not be build in a field which is not free.";
        return;
    }

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
        for (KBounceWall* wall : qAsConst(m_walls)) {
            if ( object != wall )
            {
                if ( wall->isVisible() && rect.intersects( wall->nextBoundingRect() ) )
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
        for (KBounceBall* ball : qAsConst(m_balls)) {
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
    for (KBounceWall* wall : qAsConst(m_walls)){
        QRectF rect = wall->nextBoundingRect();
        KBounceCollision collision;
        collision = checkCollision( wall, rect, ALL );
        wall->collide( collision );
    }
    for (KBounceBall* ball : qAsConst(m_balls)) {
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

QRectF KBounceBoard::boundingRect() const
{
    return QRectF( x(), y(),
            TILE_NUM_W * m_tileSize.width(),
            TILE_NUM_H * m_tileSize.height() );
}

void KBounceBoard::tick()
{
    checkCollisions();

    for (KBounceBall* ball : qAsConst(m_balls)) {
        ball->goForward();
    }
    for (KBounceWall* wall : qAsConst(m_walls)) {
        wall->goForward();
    }

    for (KBounceBall* ball : qAsConst(m_balls)) {
        ball->update();
    }

    for (KBounceWall* wall : qAsConst(m_walls)) {
        wall->update();
    }
}

QPixmap KBounceBoard::applyWallsOn(const QPixmap &background) const
{
    if (m_tileSize.isEmpty())
        return background;

    QPixmap walledBackground = background;
    const QPixmap gridTile = m_renderer->spritePixmap(QStringLiteral("gridTile"), m_tileSize);
    const QPixmap wallTile = m_renderer->spritePixmap(QStringLiteral("wallTile"), m_tileSize);
    QPainter p(&walledBackground);
    for (int i = 0; i < TILE_NUM_W; ++i) {
        for (int j = 0; j < TILE_NUM_H; ++j) {
            switch (m_tiles[i][j]) {
                case Free:
                    p.drawPixmap(x() + i * m_tileSize.width(), y() + j * m_tileSize.height(), gridTile);
                    break;

                case Border:
                case Wall:
                    p.drawPixmap(x() + i * m_tileSize.width(), y() + j * m_tileSize.height(), wallTile);
                    break;

                default:
                    break;
            }
        }
    }
    return walledBackground;
}

void KBounceBoard::wallFinished( int x1, int y1, int x2, int y2 )
{
    for ( int x = x1; x < x2; x++ )
        for ( int y = y1; y < y2; y++ )
            m_tiles[x][y] = Wall;

    for (KBounceBall* ball : qAsConst(m_balls)) {
        int x1 = static_cast<int>( ball->ballBoundingRect().x() );
        int y1 = static_cast<int>( ball->ballBoundingRect().y() );
        int x2 = static_cast<int>( ball->ballBoundingRect().right() );
        int y2 = static_cast<int>( ball->ballBoundingRect().bottom() );
        // try to fill from all edges
        // this way we can avoid most precision-related issues
        fill(x1, y1);
        fill(x1, y2);
        fill(x2, y1);
        fill(x2, y2);
    }

    for ( int x = 0; x < TILE_NUM_W; x++ )
        for ( int y = 0; y < TILE_NUM_H; y++ )
            if ( m_tiles[x][y] == Free )
                m_tiles[x][y] = Wall;
    for ( int x = 0; x < TILE_NUM_W; x++ )
        for ( int y = 0; y < TILE_NUM_H; y++ )
            if ( m_tiles[x][y] == Temp )
                m_tiles[x][y] = Free;

    int filled = 0;
    for ( int i = 1; i < TILE_NUM_W - 1; i++ )
        for ( int j = 1; j < TILE_NUM_H - 1; j++ )
            if ( m_tiles[i][j] == Wall )
                filled++;
    m_filled = filled * 100 / ( ( TILE_NUM_W - 2 ) * ( TILE_NUM_H - 2 ) );

    scene()->setBackgroundBrush(applyWallsOn(m_renderer->renderBackground()));

    Q_EMIT fillChanged( m_filled );
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



