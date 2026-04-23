#include "ressource.hh"
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

//===================================================================
//                          Ressource
//===================================================================
Ressource::Ressource(std::string n, std::string s, std::string i) :
    _name(n),
    _symbole(s),
    _iconPath(i)
{}

/*
void TxtRessourceReader::load(const std::string& chemin, std::map<std::string, Ressource*>& catalogue) {
    std::ifstream fichier(chemin);
    std::string nom;
    std::string symb;

    // Format : poussiereDEtoile PE
    while (fichier >> nom >> symb) {
        catalogue[nom] = new Ressource(nom, symb);
    }
}
*/

void JsonRessourceReader::load(const std::string& chemin, std::map<std::string, std::unique_ptr<Ressource>>& catalogue) {
    std::ifstream fichier(chemin);
    if (!fichier.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier ressource : " + chemin);
    }

    nlohmann::json j;
    fichier >> j;

    for (auto & item : j["ressources"]) {
        std::string nom = item["nom"];
        std::string symb = item["symbole"];
        std::string icone = item.contains("icone") ? item["icone"] : "";

        catalogue[nom] = std::make_unique<Ressource>(nom, symb, icone);
    }
}

const Ressource* RessourceFactory::getRessource(const std::string& id) const {
    auto it = _catalogue.find(id);
    if (it != _catalogue.end()) {
        return it->second.get();
    }
    return nullptr;
}

void RessourceFactory::chargerConfiguration(const std::string& chemin, RessourceConfigReader& lecteur) {
    lecteur.load(chemin, _catalogue);
}
