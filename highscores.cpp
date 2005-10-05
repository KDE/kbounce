/*
 * Copyright (C) 2000-2005 Stefan Schimanski <schimmi@kde.org>
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

#include "highscores.h"
#include <klocale.h>

using namespace KExtHighscore;

ExtManager::ExtManager()
{
    Item *item = new Item((uint)0, i18n("Level"), Qt::AlignRight);
    addScoreItem("level", item);
}

bool ExtManager::isStrictlyLess(const Score &s1, const Score &s2) const
{
    if ( s1.score()==s2.score() )
        return s1.data("level").toUInt()>s2.data("level").toUInt();
    return Manager::isStrictlyLess(s1, s2);
}
