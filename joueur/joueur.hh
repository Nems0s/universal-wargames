#include <iostream>
#include <map>
#include <list>
#include "unite.hh"

class Ressource;

class Joueur {
private:
    std::string _name;
    std::map<Ressource*, int> _inventaire;
    std::list<Unite*> _unites;

public:
    void ajouterRessource(Ressource* r, int n);

    bool consommerRessource(Ressource* r, int n);

    bool peutPayer(const std::map<Ressource*, int>& cout) const;

    void payer(const std::map<Ressource*, int>& cout);
};
