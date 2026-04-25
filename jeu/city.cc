#include "city.hh"
#include "joueur.hh"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;


// init

City::City(int x, int y, const std::string& nom, bool capitale, int maxLvl, float pvB, float dmgB, int visB, int rayB, const std::map<const Ressource*, int>& coutB, const std::map<const Ressource*, int>& prodB, const std::string& tex)
    : _x(x), _y(y), _nom(nom), _estCapitale(capitale), _level(1), _maxLevel(maxLvl),
      _pvMaxBase(pvB), _pvCurrent(pvB), _pvMax(pvB),
      _damageBase(dmgB), _damage(dmgB),
      _visionRangeBase(visB), _visionRange(visB),
      _rayonBase(rayB), _coutBase(coutB), _productionBase(prodB), _texturePath(tex),
      _nbBatiments(static_cast<size_t>(maxLvl))
{}

std::unique_ptr<City> City::clone(int x, int y) const {
    return std::make_unique<City>(x, y, _nom, _estCapitale, _maxLevel, _pvMaxBase, _damageBase, _visionRangeBase, _rayonBase, _coutBase, _productionBase, _texturePath);
}


// fonctions

bool City::peutAjouterBatiment() const { return _batiments.size() < _nbBatiments; }
void City::creeBatiment(std::unique_ptr<Batiment> b) { if (peutAjouterBatiment()) _batiments.push_back(std::move(b)); }
void City::takeDamage(int d) { _pvCurrent -= d; if (_pvCurrent < 0) _pvCurrent = 0; }

void City::upgrade() {
    if (_level < _maxLevel) {
        _level++;
        _pvMax = _pvMaxBase * _level;
        _pvCurrent = _pvMax;
        _damage = _damageBase + (_level * 2);
        _nbBatiments = static_cast<size_t>(_maxLevel);
    }
}

void City::product(Joueur & j) {
    for (auto& b : _batiments) {
        b->action(j); 
    }
    for (auto const& [res, qte] : _productionBase) {
        j.ajouterRessource(res, qte);
    }
}


// Json

void JsonCityReader::load(const std::string & chemin, std::map<std::string, std::shared_ptr<City>> & catalogue, const std::map<std::string, const Ressource*> & ressourcesDispo) {
    std::ifstream fichier(chemin);
    if (!fichier.is_open()) {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier JSON des villes : " << chemin << std::endl;
        return;
    }

    json data;
    fichier >> data;

    for (auto& item : data["villes"]) {
        std::string nom = item.value("nom", "VilleInconnue");
        bool capitale = item.value("est_capitale", false);
        int maxLvl = item.value("max_level", 5);
        float pvB = item.value("pv_max_base", 100.0f);
        float dmgB = item.value("degats_base", 10.0f);
        int visB = item.value("portee_vue", 2);
        int rayB = item.value("rayon_base", 1);
        std::string tex = item.value("texture", "");

        std::map<const Ressource*, int> coutMap;
        if (item.contains("cout_base")) {
            for (auto& it : item["cout_base"].items()) {
                if (ressourcesDispo.count(it.key())) {
                    coutMap[ressourcesDispo.at(it.key())] = it.value();
                }
            }
        }

        std::map<const Ressource*, int> prodMap;
        if (item.contains("production_base")) {
            for (auto& it : item["production_base"].items()) {
                if (ressourcesDispo.count(it.key())) {
                    prodMap[ressourcesDispo.at(it.key())] = it.value();
                }
            }
        }

        catalogue[nom] = std::make_shared<City>(
            0, 0, nom, capitale, maxLvl, pvB, dmgB, visB, rayB, coutMap, prodMap, tex
        );
    }
}

void CityFactory::chargerConfiguration(const std::string& chemin, JsonCityReader& lecteur, const std::map<std::string, const Ressource*>& ressourcesDispo) {
    lecteur.load(chemin, _catalogue, ressourcesDispo);
}

std::unique_ptr<City> CityFactory::create(const std::string& nom, int x, int y) const {
    auto it = _catalogue.find(nom);
    if (it != _catalogue.end()) return it->second->clone(x, y);
    return nullptr;
}