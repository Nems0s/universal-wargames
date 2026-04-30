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

std::list<Coord> Case_visible(Coord const& c, direction dir, int fov)
{
    std::list<Coord> resultat;
    if (fov <= 0) return resultat;
    if (fov >= 6) return Voisins(c);

    bool estPair = (c.second % 2 == 0); //Parité de la ligne, pour connaitre le décalage des cases

    std::vector<Coord> tousVoisins(6);
    if(estPair) 
    {
        tousVoisins[0] = {c.first + 1, c.second};
        tousVoisins[1] = {c.first, c.second - 1};
        tousVoisins[2] = {c.first - 1, c.second - 1};
        tousVoisins[3] = {c.first - 1, c.second};
        tousVoisins[4] = {c.first - 1, c.second + 1};
        tousVoisins[5] = {c.first, c.second + 1};
    } 
    else 
    {
        tousVoisins[0] = {c.first + 1, c.second};
        tousVoisins[1] = {c.first + 1, c.second - 1};
        tousVoisins[2] = {c.first, c.second - 1};
        tousVoisins[3] = {c.first - 1, c.second};
        tousVoisins[4] = {c.first, c.second + 1};
        tousVoisins[5] = {c.first + 1, c.second + 1};
    }

    int dirIdx;
    switch(dir) 
    {
        case direction::est: 
            dirIdx = 0; 
            break;
        case direction::sud_est: 
            dirIdx = 1; 
            break;
        case direction::sud_ouest: 
            dirIdx = 2; 
            break;
        case direction::ouest: 
            dirIdx = 3; 
            break;
        case direction::nord_ouest: 
            dirIdx = 4; 
            break;
        case direction::nord_est: 
            dirIdx = 5; 
            break;
    }

    int ecart_a_gauche = -(fov - 1) / 2; // Centre le cone
    //fov = 3; ecart_a_gauche = -(3 - 1) / 2 = -1; Donc: dirIdx-1, dirIdx et dirIdx+1
    for (int i = 0; i < fov; ++i) 
    {
        int index = (dirIdx + ecart_a_gauche + i + 6) % 6;
        resultat.push_back(tousVoisins[index]);
    }

    return resultat;
}

std::list<Coord> ConeVision(Coord const& origine, direction dir, int range, int fov) 
{
    std::list<Coord> vue;
    if (range <= 0) return vue;
    
    std::list<Coord> current_liste_case = Case_visible(origine, dir, fov);
    vue.insert(vue.end(), current_liste_case.begin(), current_liste_case.end());

    for(int r = 1; r < range; ++r) 
    {
        std::list<Coord> next_liste_case;
        for(const auto& c : current_liste_case) 
        {
            std::list<Coord> expansion = Case_visible(c, dir, fov);
            for(const auto& e : expansion) 
            {
                if (e != origine && std::find(vue.begin(), vue.end(), e) == vue.end() && std::find(next_liste_case.begin(), next_liste_case.end(), e) == next_liste_case.end()) 
                {
                    next_liste_case.push_back(e);
                }
            }
        }
        vue.insert(vue.end(), next_liste_case.begin(), next_liste_case.end());
        current_liste_case = next_liste_case;
    }
    return vue;
}

bool avantage_attaque(Coord const& attaquant, Coord const& defensseur, direction dir_defense, int fov)
{
    auto liste = Case_visible(defensseur, dir_defense, fov);
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
    if(rayon == 0) return {};

    std::set<Coord> liste_ajacence;

    for(auto const& elt : Voisins(c))
    {
        liste_ajacence.insert(elt);
    }

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
    
    liste_ajacence.erase(c);
    return liste_ajacence;
}