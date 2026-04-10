#include <iostream>
#include <map>
#include <list>
#include "unite.hh"
#include "city.hh"
#include "batiment.hh"
#include "config.hh"

class Ressource;

class Joueur {
private:
    std::string _name;
    FactionParams* _faction;
    std::map<Ressource*, int> _inventaire;
    std::list<Unite*> _unites;
    std::list<City*> _cities;
    std::list<Batiment*> _batiments;

public:

    void setFaction(FactionParams* f) { _faction = f; }
    const FactionParams* getFaction() const { return _faction; }

    int getNbVilles() const { return _cities.size(); }

    const std::map<Ressource*, int>& getInventaire() const { return _inventaire; }

    void ajouterRessource(Ressource* r, int n);

    bool consommerRessource(Ressource* r, int n);

    void payer(const std::map<Ressource*, int>& cout);

    const std::list<City*> & getCities() const { return _cities; }
    const std::list<Unite*> & getUnites() const { return _unites; }
    const std::list<Batiment*> & getBatiments() const { return _batiments; }
};
