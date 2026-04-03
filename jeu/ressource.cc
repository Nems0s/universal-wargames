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
