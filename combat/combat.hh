#pragma once
#include "unite.hh"

// Echelle de moral
const float MAX_MORAL = 20.0;
const float MIN_MORAL = -20.0;

// Pourcentage de modification durant les combats
const float PETIT_CHANGE = 5;
const float GROS_CHANGE  = 10;

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
