/*
    This file is part of the KDE project "KBounce"

    SPDX-FileCopyrightText: 2000-2005 Stefan Schimanski <1Stein@gmx.de>
    SPDX-FileCopyrightText: 2007 Tomasz Boczkowski <tboczkowski@onet.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

    KBounceVector( qreal newx = 0, qreal newy = 0 ) : x( newx ), y( newy ) {}

    KBounceVector& operator+=(KBounceVector rv )
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

