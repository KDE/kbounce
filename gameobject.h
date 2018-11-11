/*
 * Copyright (C) 2000-2005 Stefan Schimanski <1Stein@gmx.de>
 * Copyright (C) 2007 Tomasz Boczkowski <tboczkowski@onet.pl>
 *
 * This file is part of the KDE project "KBounce"
 *
 * KBounce is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * KBounce is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with KBounce; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QList>
#include <QRectF>

enum KBounceObjectType
{
    TILE = 1,
    BALL = 2,
    WALL = 4,
    ALL = 0xFF
};

/*
 * Simple 2D vector ( as in math not container )
 */
struct KBounceVector
{
    qreal x, y;

    KBounceVector( qreal newx = 0, qreal newy = 0 ) : x( newx ), y( newy ) {};

    KBounceVector& operator+=( const KBounceVector& rv )
    {
        x += rv.x; y += rv.y; 
        return *this;
    }

    /*
     * Simple function to calculate a vector perpendicular to 
     * sufrace of rect2 in the spot where rect1 intersects it.
     * Note it is a very simple function as the vectors it generates
     * can have different lengths. 
     */
    static KBounceVector normal( const QRectF& rect1, const QRectF& rect2 );
};

/*
 * This struct contains information about collision of one pair of objects
 * Collision testing functions in this game choose one objects referred as
 * "being hitted" and check if it intersects with another referred as
 * "hitters"
 */
struct KBounceHit
{
    /*
     * Type of hitter
     */
    KBounceObjectType type;
    /*
     * Bounding rect of hitter
     */
    QRectF boundingRect;
    /*
     * Velocity of  hiter
     */
    KBounceVector velocity;
    /*
     * Vector perpendicular to object's being hitted surface in
     * the area of intersection with hitter
     */
    KBounceVector normal;
};

typedef QList<KBounceHit> KBounceCollision;

#define GAME_DELAY 16

#endif //GAMEOBJECT_H

