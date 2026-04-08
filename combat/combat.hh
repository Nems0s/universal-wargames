#pragma once
#include "unite.hh"

inline int MAX_MORAL = 20;
inline int MIN_MORAL = -20;

class Combat
{
public:
    static bool fight(Unite & attaquant,CompAtt* const& TypeAttaque, Unite & defenseur);
    static bool heal(Unite& healer, CompSoin* const& TypeSoin, Unite& cible);
};

void BuffCommandant(Unite & u, bool & aSoin, int degatsArme=0);
void SoinDuCommandant(Unite & u);

void AugmentationMoral(Unite & u, int x_point);
void DiminussionMoral(Unite & u, int x_point);
void EffetMoral(Unite & u, int degatsArme=0); // Renvoie un multiplicateur lié à la moral
