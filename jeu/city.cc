#include "city.hh"

//===================================================================
//                          City
//===================================================================
City::City(int x, int y, const GameConfig & config, int max, bool capitale,int level):
    _x(x),
    _y(y),
    _config(config),
    _maxLevel(max),
    _estCapitale(capitale),
    _level(level)
{
    _pvMax = config.getPvMaxVille() * level;
    _pvCurrent = _pvMax;
    _damage = config.getDegatsVille() + (level * 2);
}

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

void City::takeDamage(int d) {
    _pvCurrent -= d;
    if (_pvCurrent < 0) _pvCurrent = 0;
}


void City::upgrade()
{
    //Après faut revoir tout la logique d'évolution sur la production, etc
    if(_level < _maxLevel) ++ _level;
}

void City::affiche()
{
    if(_estCapitale)
    {
        std::cout << "=== [Capital, level "<< _level <<"/"<< _maxLevel <<"] ===" << std::endl;
    }
    else std::cout << "=== [Ville, level "<< _level <<"/"<< _maxLevel <<"] ===" << std::endl;

    std::cout << "Position: (" << _x << "," << _y << ")" << std::endl;
    std::cout << "Infos: HP=" << _pvCurrent <<"/"<< _pvMax << ", Dmg=" << _damage << ", Bâtiments="<< _nbBatiments << std::endl;
}
