/*
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
 * You should have recived a copy of the GNU Library General Public
 * License along with KBounce; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.  
 */

#include "gameobject.h"

KBounceVector KBounceVector::normal( const QRectF& rect1, const QRectF& rect2 )
{
    KBounceVector normal( 0, 0 );

    if ( rect1.bottom() > rect2.top() && rect1.bottom() < rect2.bottom() )
	normal += KBounceVector( 0, -1 );
    if ( rect1.top() < rect2.bottom() && rect1.top() > rect2.top() )
	normal += KBounceVector( 0, 1 );
    if ( rect1.right() < rect2.right() && rect1.right() > rect2.left() )
	normal += KBounceVector( 1, 0 );
    if ( rect1.left() > rect2.left() && rect1.left() < rect2.right() )
	normal += KBounceVector( -1, 0 );

    return normal;
}

