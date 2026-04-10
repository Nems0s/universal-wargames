#include "config.hh"
#include <iostream>

void GameConfig::load(const std::string& chemin) {
    std::ifstream fichier(chemin);
    if (!fichier.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir " << chemin << std::endl;
        return;
    }

    json data;
    fichier >> data;

    // règles construction des villes
    if (data.contains("regles_villes")) {
        auto& v = data["regles_villes"];
        _coutBaseVille = v.value("cout_base_or", 100);
        _multiplicateurVille = v.value("multiplicateur_croissance", 1.5f);
        _distanceMinVilles = v.value("distance_min_entre_villes", 3);
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
}

const FactionParams* GameConfig::getFaction(const std::string& nom) const {
    auto it = _factions.find(nom);
    if (it != _factions.end()) {
        return &(it->second);
    }
    return nullptr;
}