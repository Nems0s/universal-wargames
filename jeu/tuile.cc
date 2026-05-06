#include "tuile.hh"
#include "joueur.hh"

#include <iostream>
#include <fstream>
#include <cmath>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

//===================================================================
//                     Tuile Configurable
//===================================================================
TuileConfigurable::TuileConfigurable(const TuileData* data) : _d(data) {}

std::string TuileConfigurable::getType() const {
    return _d->nom;
}

char TuileConfigurable::getSymbole() const {
    return _d->symbole;
}

int TuileConfigurable::getCoutDeplacement() const {
    return _d->cout;
}

std::vector<const Ressource*> TuileConfigurable::getRessource() const {
    return _d->ressourceSpeciale;
}


bool TuileConfigurable::estFranchissable(const Unite& u) const {
    bool peutnager = false;
    bool peutvoler = false;
    bool peutmarcher = false;

    for (auto* mov : u.Mobilite())
    {
        switch(mov->Nature())
        {
        case NatureMouv::TERRE:
            peutmarcher = true;
            break;
        case NatureMouv::MER:
            peutnager = true;
            break;
        case NatureMouv::AIR:
            peutvoler = true;
            break;
        }
    }

    if (_d->mouv.marche && peutmarcher) return true;
    if (_d->mouv.nage && peutnager) return true;
    if (_d->mouv.aerien && peutvoler) return true;
    return false;
}

bool TuileConfigurable::peutConstrVille() const {
    return _d->constructible && !_city;
}

bool TuileConfigurable::peutConstrBatiment(const Batiment & b) const {
    return b.getRessourcesSolRequired().empty();
}

bool TuileConfigurable::peutConstrBatimentSpecial(const Batiment & b) const {
    const BatimentRessource* br = dynamic_cast<const BatimentRessource*>(&b);
    if (!br) return false;

    const auto& requis = br->getRessourcesSolRequired();
    if (requis.empty()) return true;

    for (const Ressource* req : requis) {
        bool trouve = false;
        for (const Ressource* rTuile : _d->ressourceSpeciale) {
            if (rTuile == req) {
                trouve = true;
                break;
            }
        }
        if (!trouve) return false;
    }

    return true;
}

void TuileConfigurable::placerVille(std::unique_ptr<City> c) {
    if (peutConstrVille()) {
        _city = std::move(c);
    }
}

void TuileConfigurable::constrBatimentSpeciale(std::unique_ptr<Batiment> b) {
    if (peutConstrBatimentSpecial(*b)) {
        _batimentSpecial = std::move(b);
    }
}

City * TuileConfigurable::getCity() const {
    return _city.get();
}

float TuileConfigurable::getStat(const std::string & key) const {
    auto itLocal = _localStats.find(key);
    if (itLocal != _localStats.end()) return itLocal->second;

    auto itBase = _d->properties.find(key);
    if (itBase != _d->properties.end()) return itBase->second;

    return 0;
}

void TuileConfigurable::setStat(const std::string & key, float val) {
    _localStats[key] = val;
}


//===================================================================
//                          Factory/Config
//===================================================================
void WorldFactory::ajouterAuCatalogue(char symbole, const TuileData& data) {
    _catalogue[symbole] = data;
}

std::unique_ptr<hexa> WorldFactory::createTile(char symbole) {
    auto it = _catalogue.find(symbole);
    if (it != _catalogue.end()) {
        return std::make_unique<TuileConfigurable>(&(it->second));
    } else {
        return nullptr;
    }
}

void WorldFactory::initialiserBords() {
    TuileData limite;
    limite.nom = "Limite";
    limite.symbole = '#';
    limite.cout = -1;
    limite.constructible = false;
    
    limite.mouv = {false, false, false};
    limite.gen = {0, 0};
    limite.env = {0.0f, 0.0f, 0.0f};
    
    this->ajouterAuCatalogue('#', limite);
}

std::unique_ptr<hexa> WorldFactory::createRandomTile() {
    if (_catalogue.empty()) return nullptr;

    int poidsTotal = 0;
    for (auto const& [symb, data] : _catalogue) {
        poidsTotal += data.gen.poids;
    }

    // tuile au hasard si poidstotal à 0
    if (poidsTotal == 0) {
        auto it = _catalogue.begin();
        std::advance(it, rand() % _catalogue.size());
        return std::make_unique<TuileConfigurable>(&(it->second));
    } else {
        // par rapport aux poids
        int tirage = rand() % poidsTotal;
        int seuil = 0;
        for (auto & [symb, data] : _catalogue) {
            seuil += data.gen.poids;
            if (tirage < seuil) {
                return std::make_unique<TuileConfigurable>(&data);
            }
        }
    }
    return std::make_unique<TuileConfigurable>(&(_catalogue.begin()->second));
}

void WorldFactory::postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>>& matrix, int width, int height) {
    std::map<char, int> compteurs;

    for (auto & ligne : matrix) {
        for (auto & tuile : ligne) {
            if (tuile) {
                compteurs[tuile->getSymbole()]++;
            }
        }
    }

    for (auto const & [symb, data] : _catalogue) {
        while (compteurs[symb] < data.gen.nbMin) {
            int x = rand() % (width - 2) + 1;
            int y = rand() % (height - 2) + 1;

            if (matrix[x][y]->getSymbole() != '#' && matrix[x][y]->getSymbole() != symb) {
                matrix[x][y] = std::make_unique<TuileConfigurable>(&(_catalogue.at(symb)));
                compteurs[symb]++;
            }
        }
    }
}

bool WorldFactory::estVide() const {
    return _catalogue.empty();
}

void WorldFactory::overrideWeights(const std::map<char, int>& overrides) {
    for (auto const& [symb, weight] : overrides) {
        if (_catalogue.count(symb)) {
            _catalogue[symb].gen.poids = weight;
        }
    }
}

void JsonWorldReader::chargerConfig(std::string chemin, const std::map<std::string, const Ressource*>& resDispo, WorldFactory& factory) {
    std::ifstream fichier(chemin);
    if (!fichier.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier JSON : " + chemin);
    }

    json data;
    fichier >> data;

    // Presets Perlin
    if (data.contains("presets_perlin")) {
        for (auto& [nom, p] : data["presets_perlin"].items()) {
            PerlinParams pParams;
            pParams.scale = p.value("perlin_scale", 0.1f);
            pParams.octaves = p.value("octaves", 3);
            pParams.lacunarity = p.value("lacunarity", 2.0f);
            pParams.persistence = p.value("persistence", 0.5f);
            pParams.redistribution = p.value("redistribution", 1.0f);
            pParams.inversion = p.value("inversion", false);

            if (p.contains("seuils")) {
                for (auto& [symbStr, seuilVal] : p["seuils"].items()) {
                    if (!symbStr.empty()) pParams.seuils[seuilVal.get<float>()] = symbStr[0];
                }
            }
            factory.ajouterPresetPerlin(nom, pParams);
        }
    }

    // Presets random
    if (data.contains("presets_random")) {
        for (auto& [presetName, weightsObj] : data["presets_random"].items()) {
            std::map<char, int> w;
            for (auto& [symbStr, weightVal] : weightsObj.items()) {
                if (!symbStr.empty() && weightVal.is_number_integer()) {
                    w[symbStr[0]] = weightVal.get<int>();
                }
            }
            factory.ajouterPresetRandom(presetName, w); 
        }
    }

    // Lecture tuiles
    for (auto& t : data["tiles"]) {
        TuileData d;
        d.nom = t["nom"];
        // Le symbole est un string en JSON, on prend le premier caractère
        std::string s = t["symbole"];
        d.symbole = s[0]; 
        d.cout = t["cout"];
        d.constructible = t["constructible"];

        // Bloc MOUV
        d.mouv.marche = t["mouv"]["marche"];
        d.mouv.nage = t["mouv"]["nage"];
        d.mouv.aerien = t["mouv"]["aerien"];

        // Bloc GEN
        d.gen.poids = t["gen"]["poids"];
        d.gen.nbMin = t["gen"]["nbMin"];

        // Bloc ENV
        d.env.temperature = t["env"]["temperature"];
        d.env.radiation = t["env"]["radiation"];
        d.env.gravite = t["env"]["gravite"];

        // Bloc RES (Ressources multiples)
        for (const std::string& resNom : t.value("ressources", json::array())) {
            if (auto it = resDispo.find(resNom); it != resDispo.end()) {
                d.ressourceSpeciale.push_back(it->second);
            }
        }

        // Bloc DATA (Properties)
        auto props = t.value("properties", json::object());
        for (const auto& [key, val] : props.items()) {
            d.properties[key] = val.get<float>();
        }

        factory.ajouterAuCatalogue(d.symbole, d);
    }

    if (data.contains("default_setup")) {
        auto& setup = data["default_setup"];
        std::string mode = setup.value("mode", "perlin");
        std::string presetNom = "";

        if (setup.contains("preset")) {
            presetNom = setup["preset"];
        }

        if (mode == "perlin" && !factory.getPresetsPerlinDispos().empty()) {
            if (presetNom.empty() || factory.getPresetsPerlinDispos().find(presetNom) == factory.getPresetsPerlinDispos().end()) {
                presetNom = factory.getPresetsPerlinDispos().begin()->first;
            }
        } else if (mode == "random" && !factory.getPresetsRandomDispos().empty()) {
            if (presetNom.empty() || factory.getPresetsRandomDispos().find(presetNom) == factory.getPresetsRandomDispos().end()) {
                presetNom = factory.getPresetsRandomDispos().begin()->first;
            }
        }

        factory.appliquerPreset(mode, presetNom);
    }
}

bool WorldFactory::appliquerPreset(const std::string& mode, const std::string& nomPreset) {
    _generationMode = mode;
    _activePresetName = nomPreset;

    if (mode == "perlin") {
        if (_presetsPerlin.count(nomPreset)) {
            _activePerlinParams = _presetsPerlin[nomPreset];
            return true;
        }
    } else if (mode == "random") {
        if (_presetsRandom.count(nomPreset)) {
            _customWeights = _presetsRandom[nomPreset];
            overrideWeights(_customWeights);
            return true;
        }
    }
    return false;
}