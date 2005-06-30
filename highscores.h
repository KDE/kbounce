#ifndef HIGHSCORES_H
#define HIGHSCORES_H

#include <kexthighscore.h>

using namespace KExtHighscore;

class ExtManager : public Manager
{
 public:
    ExtManager();

 private:
    bool isStrictlyLess(const Score &s1, const Score &s2) const;
};

#endif
