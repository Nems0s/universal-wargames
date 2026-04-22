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

    std::vector<std::vector<bool>> _decouvert;
    std::vector<std::vector<bool>> _visible;
    int _nbCasesAchetees = 0;

public:

    // Faction 
    void setName(const std::string& n) { _name = n; }
    void setFaction(const FactionParams* f) { _faction = f; }
    const FactionParams* getFaction() const { return _faction; }

    // Brouillard
    void initBrouillard(int w, int h) {
        _decouvert.assign(w, std::vector<bool>(h, false));
        _visible.assign(w, std::vector<bool>(h, false));
    }
    bool estDecouvert(int x, int y) const { return _decouvert[x][y]; }
    bool estVisible(int x, int y) const   { return _visible[x][y]; }

    void resetVision();
    void decouvrirZoneVision(int x, int y, int rayon, int fov, int w, int h, direction dir, bool circulaire);

    const std::vector<std::vector<bool>>& getDecouvert() const { return _decouvert; }
    const std::vector<std::vector<bool>>& getVisible() const { return _visible; }

    void setDecouvert(const std::vector<std::vector<bool>>& d) { _decouvert = d; }
    void setVisible(const std::vector<std::vector<bool>>& v) { _visible = v; }

    // Ressources
    const std::map<Ressource*, int>& getInventaire() const { return _inventaire; }
    void ajouterRessource(Ressource* r, int n);
    void payer(const std::map<Ressource*, int>& cout);

    // Ajout objets
    void ajouterVille(City* c);
    void ajouterBatiment(Batiment* b);
    void ajouterUnite(Unite* u);

    // Action de plateau
    void perdreVille(City* c);
    void perdreBatiment(Batiment* b);
    void perdreUnite(Unite* u);
    void incNbCasesAchetees() { _nbCasesAchetees++; }

    // Accesseurs
    std::string getName() const { return _name; }
    int getNbVilles() const { return _cities.size(); }
    const std::list<City*> & getCities() const { return _cities; }
    const std::list<Unite*> & getUnites() const { return _unites; }
    const std::list<Batiment*> & getBatiments() const { return _batiments; }
    int getNbCasesAchetees() const { return _nbCasesAchetees; }

};
