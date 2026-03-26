#pragma once
#include <string>
#include <list>


enum class direction{nord_ouest, nord_est, ouest, est, sud_ouest, sud_est};

struct Case{
    int x;
    int y;
};

std::string directionToString(direction dir);
std::list<Case> Case_visible(Case const& c, direction dir);
bool avantage_attaque(Case const& attaquant, Case const& defensseur, direction dir_defense);





















