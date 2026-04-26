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
        if (v.contains("cout_base")) {
            for (auto& [res, qte] : v["cout_base"].items()) {
                _coutBaseVille[res] = qte;
            }
        }
        _multiplicateurVille = v.value("multiplicateur", 1.5f);
        _distanceMinVilles = v.value("distance_min_entre_villes", 3);
        _pvMaxVille = v.value("pv_max_base", 200);
        _degatsVille = v.value("degats_base", 20);
        _textureVille = v.value("texture", "");
        _porteeVueVille = v.value("portee_vue_ville", 4);
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
            _factions[fp.nom] = fp;
        }
    }
    
    // regles unites
    if (data.contains("regles_unites")) {
        auto& v = data["regles_unites"];
        _coutRotation = v.value("cout_rotation", 1);
    }

    if (data.contains("ressources_depart")) {
        for (auto& [resName, qty] : data["ressources_depart"].items()) {
            _ressourcesDepart[resName] = qty;
        }
    }

    if (data.contains("production_capitale")) {
        for (auto& [resName, qty] : data["production_capitale"].items()) {
            _productionCapitale[resName] = qty;
        }
    }

    if (data.contains("regles_entretien")) {
        if (data["regles_entretien"].contains("cout_defaut")) {
            for (auto& [res, qte] : data["regles_entretien"]["cout_defaut"].items()) {
                _entretienCoutDefaut[res] = qte;
            }
        }
        _capaciteBase = data["regles_entretien"].value("capacite_ville", 5);
        _capaciteVilleNiveau = data["regles_entretien"].value("capacite_ville_niveau", 3);
    }
}

void GameConfig::loadWins(const std::string & chemin) {
    std::ifstream f(chemin);
    json data = json::parse(f);
    _victorySets.clear();

    for (auto& setJson : data["victory_set"]) {
        VictorySet vSet;
        vSet.name = setJson["name"];
        vSet.mode = (setJson["mode"] == "all") ? WinMode::ALL : WinMode::ANY;

        for (auto& item : setJson["conditions"]) {
            WinConditions cond;
            std::string typeStr = item["type"];

            if (typeStr == "resource_threshold") cond.type = WinType::RESOURCE;
            else if (typeStr == "city_count") cond.type = WinType::CITY_COUNT;
            else if (typeStr == "unit_count") cond.type = WinType::UNIT_COUNT;
            else if (typeStr == "require_capital") cond.type = WinType::CAPITAL_REQ;
            else if (typeStr == "capital_conquest") cond.type = WinType::CAPITAL_CONQUEST;

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