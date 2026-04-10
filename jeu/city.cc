#include "city.hh"

//===================================================================
//                          City
//===================================================================
City::City(int max, bool capitale, int x, int y, int level):
    _level(level),
    _nbBatiments(max),
    _estCapitale(capitale),
    _x(x),
    _y(y)
{}

bool City::peutAjouterBatiment() const {
    return _batiments.size() < _nbBatiments;
}

void City::creeBatiment(std::unique_ptr<Batiment> b) {
    if (_batiments.size() >= _nbBatiments) return;
    if (!b->getRessourcesSolRequired().empty()) return;
    _batiments.push_back(std::move(b));
}

bool City::estCapitale() const {
    return _estCapitale;
}

void City::product(Joueur & j) {
    for (auto & b : _batiments) {
        b->action(j);
    }
}
