#include <iostream>
#include <map>
#include <list>
#include "unite.hh"
#include "city.hh"
#include "batiment.hh"

class Ressource;

class Joueur {
private:
    std::string _name;
    std::map<Ressource*, int> _inventaire;
    std::list<Unite*> _unites;
    std::list<City*> _cities;
    std::list<Batiment*> _batiments;

public:

    const std::map<Ressource*, int>& getInventaire() const { return _inventaire; }

    void ajouterRessource(Ressource* r, int n);

    bool consommerRessource(Ressource* r, int n);

    void payer(const std::map<Ressource*, int>& cout);
};
