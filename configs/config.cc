#include "config.hh"
#include <iostream>

void GameConfig::loadRules(const std::string& chemin) {
    std::ifstream fichier(chemin);
    if (!fichier.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir " << chemin << std::endl;
        return;
    }

    json data;
    fichier >> data;

    // régles taille plateau
    if (data.contains("taille_plateau")) {
        auto& v = data["taille_plateau"];
        _plateauX = v.value("x", 10);
        _plateauY = v.value("y", 10);
    }

    // règles construction des villes
    if (data.contains("regles_villes")) {
        auto& v = data["regles_villes"];
        _coutBaseVille = v.value("cout_base", 100);
        _multiplicateurVille = v.value("multiplicateur", 1.5f);
        _distanceMinVilles = v.value("distance_min_entre_villes", 3);
        _pvMaxVille = v.value("pv_max_base", 200);
        _degatsVille = v.value("degats_base", 20);
    }

    // factions
    if (data.contains("factions")) {
        for (auto& f : data["factions"]) {
            FactionParams fp;
            fp.nom = f["nom"];

            if (f.contains("specifications")) {
                for (auto & [statName, limite] : f["specifications"].items()) {
                    fp.params[statName] = limite;
                }
            }

            if (f.contains("unites")) {
                for (auto& unitName : f["unites"]) {
                    fp.unites_disponibles.push_back(unitName);
                }
            }

            _factions[fp.nom] = fp;
        }
    }
}

void GameConfig::loadWins(const std::string & chemin) {
    std::ifstream f(chemin);
    json data = json::parse(f);

    for (auto& setJson : data["victory_set"]) {
        VictorySet vSet;
        vSet.name = setJson["name"];
        vSet.mode = (setJson["mode"] == "all") ? WinMode::ALL : WinMode::ANY;

        for (auto& item : setJson["conditions"]) {
            WinConditions cond;
            std::string typeStr = item["type"];

            if (typeStr == "ressource_thresold") cond.type = WinType::RESOURCE;
            else if (typeStr == "city_count") cond.type = WinType::CITY_COUNT;
            else if (typeStr == "unit_count") cond.type = WinType::UNIT_COUNT;
            else if (typeStr == "require_capital") cond.type = WinType::CAPITAL_REQ;

            cond.resourceName = item.value("target", "");
            cond.targetAmount = item.value("amount", 0);
            cond.required = item.value("value", true);

            vSet.conditions.push_back(cond);
        }
        _victorySets.push_back(vSet);
    }
}

const FactionParams* GameConfig::getFaction(const std::string& nom) const {
    auto it = _factions.find(nom);
    if (it != _factions.end()) {
        return &(it->second);
    }
    return nullptr;
}