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
    std::map<const Ressource*, int> _inventaire;
    std::list<Unite*> _unites;
    std::list<City*> _cities;
    std::list<Batiment*> _batiments;

    std::vector<std::vector<bool>> _decouvert;
    std::vector<std::vector<bool>> _visible;
    int _nbCasesAchetees = 0;

public:

    void debutTour();


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
    const std::map<const Ressource*, int>& getInventaire() const { return _inventaire; }
    void ajouterRessource(const Ressource* r, int n);
    void payer(const std::map<const Ressource*, int>& cout);

    // Ajout objets
    void ajouterVille(City* c);
    void ajouterBatiment(Batiment* b);
    void ajouterUnite(Unite* u);

    // Action de plateau
    void perdreVille(City* c);
    void perdreBatiment(Batiment* b);
    void perdreUnite(Unite* u);
    void incNbCasesAchetees() { _nbCasesAchetees++; }

    //Action sur les Unites
    bool Attaquer(Unite& attaque, Unite& cible, CompAtt* const& TypeAttaque);
    void Soigner(Unite& healer, Unite& cible, CompSoin* const& TypeSoin);
    void ActiverCamouflage(Unite& unite);
    void RejoindreCommandant(Unite& commandant, Unite& unite);
    void QuitterCommandant(Unite& commandant, Unite& unite);
    
    void Transporter(Unite& transporteur, Unite& passager);
    void DechargerTransport(Unite& transporteur, Unite& passager, int xDest, int yDest); 

    void ChangerPositionDefensive(Unite& unite);

    // Accesseurs
    std::string getName() const { return _name; }
    int getNbVilles() const { return _cities.size(); }
    const std::list<City*> & getCities() const { return _cities; }
    const std::list<Unite*> & getUnites() const { return _unites; }
    const std::list<Batiment*> & getBatiments() const { return _batiments; }
    int getNbCasesAchetees() const { return _nbCasesAchetees; }

};
