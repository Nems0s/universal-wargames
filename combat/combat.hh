#pragma once
#include "unite.hh"

class Combat
{
public:
    static bool fight(Unite & attaquant,CompAtt* const& TypeAttaque, Unite & defenseur);
    static bool heal(Unite const& healer, Unite const& cible);
};
