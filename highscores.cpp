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
