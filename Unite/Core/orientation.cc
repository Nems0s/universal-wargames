#include "orientation.hh"
std::string directionToString(direction dir) {
    switch (dir)
    {
    case direction::nord_ouest: return "Nord-Ouest";
    case direction::nord_est:   return "Nord-Est";
    case direction::ouest:      return "Ouest";
    case direction::est:        return "Est";
    case direction::sud_ouest:  return "Sud-Ouest";
    case direction::sud_est:    return "Sud-Est";
    default:                    return "Direction inconnue";
    }
}

std::list<Case> Case_visible(Case const& c, direction dir)
{

    std::list<Case> liste_adjascence;
    Case c_droite = {c.x+1, c.y};
    Case c_gauche = {c.x-1, c.y};
    Case c_bas = {c.x, c.y-1};
    Case c_haut = {c.x, c.y+1};
    Case c_haut_gauche = {c.x-1, c.y+1};
    Case c_bas_gauche = {c.x-1, c.y+1};

    switch (dir)
    {
    case direction::nord_est:
        liste_adjascence.push_back(c_haut);
        liste_adjascence.push_back(c_haut_gauche);
        liste_adjascence.push_back(c_droite);
        return liste_adjascence;

    case direction::est:
        liste_adjascence.push_back(c_haut);
        liste_adjascence.push_back(c_bas);
        liste_adjascence.push_back(c_droite);
        return liste_adjascence;

    case direction::sud_est:
        liste_adjascence.push_back(c_bas);
        liste_adjascence.push_back(c_bas_gauche);
        liste_adjascence.push_back(c_droite);
        return liste_adjascence;

    case direction::sud_ouest:
        liste_adjascence.push_back(c_bas);
        liste_adjascence.push_back(c_bas_gauche);
        liste_adjascence.push_back(c_gauche);
        return liste_adjascence;

    case direction::ouest:
        liste_adjascence.push_back(c_bas_gauche);
        liste_adjascence.push_back(c_haut_gauche);
        liste_adjascence.push_back(c_gauche);
        return liste_adjascence;

    case direction::nord_ouest:
        liste_adjascence.push_back(c_haut);
        liste_adjascence.push_back(c_haut_gauche);
        liste_adjascence.push_back(c_gauche);
        return liste_adjascence;

    default: return liste_adjascence;
    }
}

bool avantage_attaque(Case const& attaquant, Case const& defensseur, direction dir_defense)
{
    auto liste = Case_visible(defensseur, dir_defense);
    for(auto c : liste)
    {
        if((attaquant.x == c.x)&&(attaquant.y == c.y))
        {
            return false;
        }
    }
    return true;
}





















