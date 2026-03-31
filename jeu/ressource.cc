#include "ressource.hh"

void TxtRessourceReader::load(const std::string& chemin, std::map<std::string, Ressource*>& catalogue) {
    std::ifstream fichier(chemin);
    std::string nom;
    std::string symb;

    // Format : poussiereDEtoile PE
    while (fichier >> nom >> symb) {
        catalogue[nom] = new Ressource(nom, symb);
    }
}