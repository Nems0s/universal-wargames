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

std::list<Coord> Case_visible(Coord const& c, direction dir)
{
    std::list<Coord> liste_adjascence;

    bool estPair = (c.second % 2 == 0);

    Coord c_est   = {c.first + 1, c.second};
    Coord c_ouest = {c.first - 1, c.second};

    Coord c_nord_est, c_nord_ouest, c_sud_est, c_sud_ouest;

    if (estPair)
    {
        c_nord_est   = {c.first, c.second + 1};
        c_nord_ouest = {c.first - 1, c.second + 1};
        c_sud_est   = {c.first, c.second - 1};
        c_sud_ouest = {c.first - 1, c.second - 1};
    }
    else
    {
        c_nord_est   = {c.first + 1, c.second + 1};
        c_nord_ouest = {c.first, c.second + 1};
        c_sud_est   = {c.first + 1, c.second - 1};
        c_sud_ouest = {c.first, c.second - 1};
    }

    switch (dir)
    {
    case direction::nord_est:
        liste_adjascence.push_back(c_nord_est);
        liste_adjascence.push_back(c_nord_ouest);
        liste_adjascence.push_back(c_est);
        break;

    case direction::est:
        liste_adjascence.push_back(c_est);
        liste_adjascence.push_back(c_nord_est);
        liste_adjascence.push_back(c_sud_est);
        break;

    case direction::sud_est:
        liste_adjascence.push_back(c_sud_est);
        liste_adjascence.push_back(c_est);
        liste_adjascence.push_back(c_sud_ouest);
        break;

    case direction::sud_ouest:
        liste_adjascence.push_back(c_sud_ouest);
        liste_adjascence.push_back(c_sud_est);
        liste_adjascence.push_back(c_ouest);
        break;

    case direction::ouest:
        liste_adjascence.push_back(c_ouest);
        liste_adjascence.push_back(c_sud_ouest);
        liste_adjascence.push_back(c_nord_ouest);
        break;

    case direction::nord_ouest:
        liste_adjascence.push_back(c_nord_ouest);
        liste_adjascence.push_back(c_ouest);
        liste_adjascence.push_back(c_nord_est);
        break;
    }

    return liste_adjascence;
}

bool avantage_attaque(Coord const& attaquant, Coord const& defensseur, direction dir_defense)
{
    auto liste = Case_visible(defensseur, dir_defense);
    for(auto c : liste)
    {
        if((attaquant.first == c.first)&&(attaquant.second == c.second))
        {
            return false;
        }
    }
    return true;
}


std::list<Coord> Voisins(Coord const& c)
{
    std::list<Coord> liste_voisins;

    bool estPair = (c.second % 2 == 0);

    Coord c_est   = {c.first + 1, c.second};
    Coord c_ouest = {c.first - 1, c.second};

    Coord c_nord_est, c_nord_ouest, c_sud_est, c_sud_ouest;

    if (estPair)
    {
        c_nord_est   = {c.first, c.second + 1};
        c_nord_ouest = {c.first - 1, c.second + 1};
        c_sud_est   = {c.first, c.second - 1};
        c_sud_ouest = {c.first - 1, c.second - 1};
    }
    else
    {
        c_nord_est   = {c.first + 1, c.second + 1};
        c_nord_ouest = {c.first, c.second + 1};
        c_sud_est   = {c.first + 1, c.second - 1};
        c_sud_ouest = {c.first, c.second - 1};
    }

    liste_voisins.push_back(c_est);
    liste_voisins.push_back(c_nord_est);
    liste_voisins.push_back(c_sud_est);
    liste_voisins.push_back(c_ouest);
    liste_voisins.push_back(c_nord_ouest);
    liste_voisins.push_back(c_sud_ouest);

    return liste_voisins;
}

std::set<Coord> case_adjascentes(Coord const& c, int rayon)
{
    if(rayon == 0)
    {
        std::set<Coord> liste_vide;
        return liste_vide;
    }

    std::set<Coord> liste_ajacence;
    for(auto const& elt : Voisins(c))
    {
        liste_ajacence.insert(elt);
    }

    if(rayon == 1)
    {
        return liste_ajacence;
    }
    else
    {
        for(int i = 1; i<rayon; ++i)
        {
            std::set<Coord> new_voisins;
            for (auto& case_visite : liste_ajacence)
            {
                for(auto const& elt : Voisins(case_visite))
                {
                    new_voisins.insert(elt);
                }
            }
            liste_ajacence.insert(new_voisins.begin(), new_voisins.end());
        }
    }
    return liste_ajacence;
}





















