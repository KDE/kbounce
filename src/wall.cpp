/*
    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <1Stein@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "wall.h"

#include <cmath>


#include <QPainter>
#include <QStandardPaths>

#include "board.h"
#include "renderer.h"
#include "settings.h"

QSize KBounceWall::s_tileSize;
KBounceRenderer * KBounceWall::m_renderer = nullptr;
KBounceWall::Sprites * KBounceWall::s_sprites = nullptr;


KBounceWall::KBounceWall( Direction dir, KBounceRenderer* renderer, KBounceBoard* board )
    : KGameRenderedItem( renderer,QLatin1String(""),board )
    , m_board( board )
      , m_dir( dir )
      , m_soundWallstart( QStandardPaths::locate( QStandardPaths::AppDataLocation, QStringLiteral("sounds/wallstart.wav") ) )
      , m_soundReflect( QStandardPaths::locate( QStandardPaths::AppDataLocation, QStringLiteral("sounds/reflect.wav") ) )
{
    // The wall velocity would initialised on every new level.
    m_wallVelocity = 0.0;

    if (!s_sprites) {
        s_sprites = new Sprites;
    }

    if (!m_renderer) {
        m_renderer = renderer;
    }
}

KBounceWall::~KBounceWall()
{
}

void KBounceWall::collide(const KBounceCollision &collision)
{
    if ( !isVisible() )
        return;

    for (const KBounceHit &hit : collision) {
        switch (hit.type) {
            case ALL:
                break;
            case TILE:
                finish();
                break;
            case BALL:
                if (safeEdgeHit(hit.boundingRect)) {
                    KBounceVector normal = hit.normal;
                    if (qAbs(normal.x) < qAbs(normal.y)) { // vertical
                        if (m_dir == Up || m_dir == Down) {
                            finish( true, m_dir );
                        }
                    }
                    else if (m_dir == Left || m_dir == Right) {
                        finish( true, m_dir );
                    }
                } else {
                    Q_EMIT died();
                    hide();
                }
                break;
            case WALL:
                if (safeEdgeHit(hit.boundingRect)) {
                    finish();
                }
                break;
        }
    }
}


void KBounceWall::goForward()
{
    if (!isVisible()) {
        return;
    }

    switch( m_dir ) {
        case Up:
            m_boundingRect.setTop( m_boundingRect.top() - m_wallVelocity );
            m_nextBoundingRect.setTop( m_boundingRect.top() - m_wallVelocity );
            break;
        case Left:
            m_boundingRect.setLeft( m_boundingRect.left() - m_wallVelocity );
            m_nextBoundingRect.setLeft( m_boundingRect.left() - m_wallVelocity );
            break;
        case Down:
            m_boundingRect.setBottom( m_boundingRect.bottom() + m_wallVelocity );
            m_nextBoundingRect.setBottom( m_boundingRect.bottom() + m_wallVelocity );
            break;
        case Right:
            m_boundingRect.setRight( m_boundingRect.right() + m_wallVelocity );
            m_nextBoundingRect.setRight( m_boundingRect.right() + m_wallVelocity );
            break;
    }
}

void KBounceWall::update()
{
    if (!isVisible())
        return;

    int boundingRectWidth = static_cast<int>(std::ceil(m_boundingRect.width() * s_tileSize.width()));
    int boundingRectHeight = static_cast<int>(std::ceil(m_boundingRect.height() * s_tileSize.height()));

    if (!(boundingRectWidth && boundingRectHeight))
        return;

    int tileWidth = s_tileSize.width();
    int tileHeight = s_tileSize.height();

    QSize pixSize;
    if (m_dir == Left || m_dir == Right) {
        pixSize = QSize(boundingRectWidth + 64 - (boundingRectWidth%64), boundingRectHeight);
    } else {
        pixSize = QSize(boundingRectWidth, boundingRectHeight + 64 - (boundingRectHeight%64));
    }

    QPixmap px;
    if (pixmap().width() < pixSize.width() || pixmap().height() < pixSize.height()) {
        px = QPixmap(pixSize);
    } else {
        px = pixmap(); // already ARGB
    }
    px.fill(Qt::transparent);

    QPainter p(&px);

    QPointF offset = m_board->mapPosition(m_boundingRect.topLeft());

    switch ( m_dir ) {
        case Up: {
                     const int split = qMin(tileHeight, boundingRectHeight);
                     p.drawPixmap(0, 0, s_sprites->wallEndUp, 0, 0, tileWidth, split);
                     p.drawTiledPixmap(0, split, tileWidth, boundingRectHeight - split, s_sprites->wallV, 0, offset.y());
                     break;
                 }
        case Right: {
                        const int split = qMin(tileWidth, boundingRectWidth);
                        p.drawPixmap(boundingRectWidth - tileWidth, 0, split, tileHeight, s_sprites->wallEndRight);
                        p.drawTiledPixmap(0, 0, boundingRectWidth - split, tileHeight, s_sprites->wallH);
                        break;
                    }
        case Down: {
                       const int split = qMin(tileHeight, boundingRectHeight);
                       p.drawPixmap(0, boundingRectHeight - tileHeight, tileWidth, split, s_sprites->wallEndDown);
                       p.drawTiledPixmap(0, 0, tileWidth, boundingRectHeight - split, s_sprites->wallV);
                       break;
                   }
        case Left:
                   const int split = qMin(boundingRectWidth, tileWidth);
                   p.drawPixmap(0, 0, split, tileHeight, s_sprites->wallEndLeft);
                   p.drawTiledPixmap(split, 0, boundingRectWidth - split, tileHeight, s_sprites->wallH, offset.x());
    }
    setPos(offset);
    p.end();
    setPixmap(px);
}

void KBounceWall::loadSprites() {
    s_sprites->wallEndLeft = m_renderer->spritePixmap(QStringLiteral("wallEndLeft"), s_tileSize);
    s_sprites->wallEndUp = m_renderer->spritePixmap(QStringLiteral("wallEndUp"), s_tileSize);
    s_sprites->wallEndRight = m_renderer->spritePixmap(QStringLiteral("wallEndRight"), s_tileSize);
    s_sprites->wallEndDown = m_renderer->spritePixmap(QStringLiteral("wallEndDown"), s_tileSize);

    s_sprites->wallH = m_renderer->spritePixmap(QStringLiteral("wallH"), QSize(32 * s_tileSize.width(), s_tileSize.height()));
    s_sprites->wallV = m_renderer->spritePixmap(QStringLiteral("wallV"), QSize(s_tileSize.width(), 18*s_tileSize.height()));
}

void KBounceWall::resize( const QSize& tileSize )
{
    if ( tileSize != s_tileSize ) {
        s_tileSize = tileSize;
        loadSprites();
        update();
    }
}

void KBounceWall::build( int x, int y )
{
    if (isVisible())
        return;

    if ( m_dir == Up || m_dir == Down ) {
        m_boundingRect.setTop( y );

        if (m_dir == Down) {
            m_boundingRect.setBottom(y + 1);
        } else {
            m_boundingRect.setBottom( y );
        }

        m_boundingRect.setLeft( x );
        m_boundingRect.setRight( x + 1 );
    }
    else if ( m_dir == Left || m_dir == Right ) {
        m_boundingRect.setTop( y );
        m_boundingRect.setBottom( y + 1 );
        m_boundingRect.setLeft( x );

        if (m_dir == Right) {
            m_boundingRect.setRight(x + 1);
        } else {
            m_boundingRect.setRight( x );
        }
    }


    m_nextBoundingRect = m_boundingRect;

    setPixmap(QPixmap());

    setPos(m_board->mapPosition(QPointF( x, y )));
    show();

    if ( KBounceSettings::playSounds() )
        m_soundWallstart.start();
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

    if ( shorten ) {
        switch ( dir )
        {
            case Left: left++; break;
            case Up: top++; break;
            case Right: right--; break;
            case Down: bottom--; break;
        }
    }

    Q_EMIT finished( left, top, right, bottom );
    hide();

    if (KBounceSettings::playSounds())
        m_soundReflect.start();
}

void KBounceWall::setWallVelocity(qreal velocity)
{
    m_wallVelocity = velocity;
}




