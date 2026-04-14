#include "ressource.hh"

//===================================================================
//                          Ressource
//===================================================================
Ressource::Ressource(std::string n, std::string s) :
    _name(n),
    _symbole(s)
{}

std::string Ressource::getName() const
{
    return _name;
}
std::string Ressource::getSymbole() const
{
    return _symbole;
}

void TxtRessourceReader::load(const std::string& chemin, std::map<std::string, Ressource*>& catalogue) {
    std::ifstream fichier(chemin);
    std::string nom;
    std::string symb;

    // Format : poussiereDEtoile PE
    while (fichier >> nom >> symb) {
        catalogue[nom] = new Ressource(nom, symb);
    }
}

void JsonRessourceReader::load(const std::string& chemin, std::map<std::string, Ressource*>& catalogue) {
    std::ifstream fichier(chemin);
    if (!fichier.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier ressource : " + chemin);
    }

    json data;
    fichier >> data;

    for (auto& item : data["ressources"]) {
        std::string nom = item["nom"];
        std::string symb = item["symbole"];

        catalogue[nom] = new Ressource(nom, symb);
    }
}

