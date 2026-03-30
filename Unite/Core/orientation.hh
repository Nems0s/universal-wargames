#pragma once
#include <string>
#include <list>
#include <set>


enum class direction{nord_ouest, nord_est, ouest, est, sud_ouest, sud_est};

struct Case{
    int x;
    int y;

    // Pour std::list::find ou std::ranges::contains
    bool operator==(const Case& autre) const {
        return (x == autre.x && y == autre.y);
    }

    //Obligatoire pour utiliser set
    bool operator<(const Case& autre) const {
        if (x != autre.x)
        {
            return x < autre.x;
        }
        return y < autre.y;
    }

};

std::string directionToString(direction dir);
std::list<Case> Case_visible(Case const& c, direction dir);
bool avantage_attaque(Case const& attaquant, Case const& defensseur, direction dir_defense);
std::list<Case> Voisins(Case const& c);
std::set<Case> case_adjascentes(Case const& c, int rayon=1);




















