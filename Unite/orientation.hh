#pragma once
#include <string>
#include <list>
#include <set>
#include <utility>
#include <algorithm>
#include <vector>


enum class direction{nord_ouest, nord_est, ouest, est, sud_ouest, sud_est};

using Coord = std::pair<int, int>;

std::string directionToString(direction dir);
std::list<Coord> Case_visible(Coord const& c, direction dir, int fov);
std::list<Coord> ConeVision(Coord const& origine, direction dir, int range, int fov) ;
bool avantage_attaque(Coord const& attaquant, Coord const& defensseur, direction dir_defense, int fov);
std::list<Coord> Voisins(Coord const& c);
std::set<Coord> case_adjascentes(Coord const& c, int rayon=1);
