/*
    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <1Stein@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BOARD_H
#define BOARD_H

#include <QGraphicsItemGroup>

#include <QList>
#include <QSize>

#include "gameobject.h"
#include "renderer.h"

#define TILE_NUM_H 20
#define TILE_NUM_W 32


class KBounceBall;
class KBounceWall;

class KBounceBoard: public QGraphicsObject
{
    Q_OBJECT

    public:
        enum TileType{ Empty, Free, Border, Wall, Temp };

        explicit KBounceBoard( KBounceRenderer *renderer );
        ~KBounceBoard() override;

        QPixmap applyWallsOn(const QPixmap &background) const;
        void resize( QSize& size );
        void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}

        void newLevel( int level );
        void setPaused( bool );

        void buildWall( const QPointF& pos, bool vertical );

        int balls();
        int filled();

        KBounceCollision checkCollision( void* object, const QRectF& rect, int type );
        KBounceCollision checkCollisionTiles( const QRectF& rect );
        void checkCollisions();

        QPoint mapPosition( const QPointF& pos ) const;
        QRectF boundingRect() const override;

        void setBallVelocity(qreal velocity);
        void setWallVelocity(qreal velocity);
    Q_SIGNALS:
        void ballsChanged( int balls );
        void fillChanged( int fill );
        void wallDied();

    protected Q_SLOTS:
        void tick();
        void wallFinished( int x1, int y1, int x2, int y2 );

    private:
        void clear();
        void fill( int x, int y );

        KBounceRenderer* m_renderer;

        TileType m_tiles[TILE_NUM_W][TILE_NUM_H];
        QSize m_tileSize;
        int m_filled;

        QList<KBounceBall*> m_balls;
        QList<KBounceWall*> m_walls;

        QTimer* m_clock;


        qreal m_ballVelocity;
        qreal m_wallVelocity;
};

#endif // BOARD_H
