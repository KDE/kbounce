/*
    This file is part of the KDE project "KBounce"

    SPDX-FileCopyrightText: 2007 Tomasz Boczkowski <tboczkowski@onet.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

