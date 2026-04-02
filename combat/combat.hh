#pragma once
#include "unite.hh"

inline int MAX_MORAL = 20;
inline int MIN_MORAL = -20;

class Combat
{
public:
    static bool fight(Unite & attaquant,CompAtt* const& TypeAttaque, Unite & defenseur);
    static bool heal(Unite const& healer, Unite const& cible);
};

void BuffCommandant(Unite & u, bool & aSoin);
void SoinDuCommandant(Unite & u);

void AugmentationMoral(Unite & u, int x_point);
void DiminussionMoral(Unite & u, int x_point);
void EffetMoral(Unite & u); // Renvoie un multiplicateur lié à la moral
