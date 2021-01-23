/*
    This file is part of the KDE project "KBounce"

    SPDX-FileCopyrightText: 2007 Tomasz Boczkowski <tboczkowski@onet.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WALL_H
#define WALL_H

#include <KGameRenderedItem>

#include <QObject>
#include <KgSound>
#include "gameobject.h"

class KBounceRenderer;
class KBounceBoard;

/**
 * KGameRenderedItem representing a wall "under-construction"
 *
 * There are four walls in a board, each of which extends in
 * other direction: up, right, down or left
 */ 
class KBounceWall : public QObject, public KGameRenderedItem
{
    Q_OBJECT

    public:
        enum Direction { Up = 0, Down, Left, Right };

        /**
         * Constructor
         * @param dir is set in construction and does not change in gameplay
         * that means there will be no two walls extending in the same direction
         * in the same time
         */
        KBounceWall( Direction dir, KBounceRenderer* renderer, KBounceBoard* board );
        ~KBounceWall() override;

        /**
         * Changes object's state when collisions have been detected
         * Called once per frame before advance() and update()
         */
        void collide(const KBounceCollision &collision);
        /**
         * Performs various movement and state calculations non-related
         * to collision responses. Also updates m_boundingRect and 
         * m_nextBoundingRect. This method is called once per frame
         * after collide() and before update()
         */
        void goForward();
        /**
         * Updates object's pixmap and position on screen
         * Called once per frame after update()
         */
        void update();

        /**
         * Starts building wall beginning from tile specified by x and y
         * The direction has been specified in constructor
         */
        void build( int x, int y );
        /**
         * Returns the bounding rect that is expected for wall to
         * have in next frame. Collision in KBounceBoard are based on
         * the result of this method
         */
        QRectF nextBoundingRect() const;

        /**
         * Changes on-screen dimensions of the wall. 
         * Calculations are based on tile size, that is the on-screen
         * size of board's tile
         */
        void resize( const QSize& tileSize );

        /**
         * Set the wall velocity for wall filling speed.
         */
        void setWallVelocity(qreal velocity);

        /**
         * Load all sprites for top, down, left and right walls as well as for
         * the vertical and horizontal semi transparent bars drawn before a wall is
         * built.
         */
        void static loadSprites();

    Q_SIGNALS:
        void finished( int left, int top, int right, int bottom );
        void died();

    private:
        /**
         * Returns true if rect2 intersects the edge at the end of wall
         * e.g for wall extending in direction Up this will be the upper
         * edge.
         */
        bool safeEdgeHit( const QRectF& rect2 ) const;
        /**
         * Helper function replacing emiting long finished signal
         * It also hides the wall and plays corresponding sound
         * If &param shorten is true the wall will be one unit in
         * direction &param dir shorter than normal 
         */
        void finish( bool shorten = false, Direction dir = Up);

        KBounceBoard *m_board;
        Direction m_dir;

        KgSound m_soundWallstart;
        KgSound m_soundReflect;

        QRectF m_boundingRect;
        QRectF m_nextBoundingRect;
        qreal m_wallVelocity;

        typedef struct {
            QPixmap wallEndLeft;
            QPixmap wallEndUp;
            QPixmap wallEndRight;
            QPixmap wallEndDown;
            QPixmap wallH;
            QPixmap wallV;

        } Sprites;
        static Sprites *s_sprites;

        static QSize s_tileSize;
        static KBounceRenderer *m_renderer;
};

#endif


