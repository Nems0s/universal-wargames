#pragma once
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
    const FactionParams* _faction;
    std::map<Ressource*, int> _inventaire;
    std::list<Unite*> _unites;
    std::list<City*> _cities;
    std::list<Batiment*> _batiments;

    std::vector<std::vector<bool>> _brouillard;

public:

    // Faction 
    void setName(const std::string& n) { _name = n; }
    void setFaction(const FactionParams* f) { _faction = f; } // Mise à jour avec const
    const FactionParams* getFaction() const { return _faction; }

    // Brouillard
    void initBrouillard(int w, int h);
    void decouvrirZone(int cx, int cy, int rayon, int w, int h);
    bool estDecouvert(int x, int y) const;

    // Ressources
    const std::map<Ressource*, int>& getInventaire() const { return _inventaire; }
    void ajouterRessource(Ressource* r, int n);
    bool consommerRessource(Ressource* r, int n);
    void payer(const std::map<Ressource*, int>& cout);

    // Ajout objets
    void ajouterVille(City* c);
    void ajouterBatiment(Batiment* b);
    void ajouterUnite(Unite* u);

    // Action de plateau
    void perdreVille(City* c);
    void perdreBatiment(Batiment* b);
    void perdreUnite(Unite* u);

    // Accesseurs
    std::string getName() const { return _name; }
    int getNbVilles() const { return _cities.size(); }
    const std::list<City*> & getCities() const { return _cities; }
    const std::list<Unite*> & getUnites() const { return _unites; }
    const std::list<Batiment*> & getBatiments() const { return _batiments; }
    const std::vector<std::vector<bool>>& getBrouillard() const { return _brouillard; }
    void setBrouillard(const std::vector<std::vector<bool>>& b) { _brouillard = b; }

};
