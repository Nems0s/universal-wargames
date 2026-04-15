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
    FactionParams* _faction;
    std::map<Ressource*, int> _inventaire;
    std::list<Unite*> _unites;
    std::list<City*> _cities;
    std::list<Batiment*> _batiments;

public:
    Joueur(std::string name) : _name(name), _faction(nullptr) {}

    // Faction 
    void setFaction(FactionParams* f) { _faction = f; }
    const FactionParams* getFaction() const { return _faction; }

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

    //Action sur les Unites
    void Attaquer(Unite& attaque, Unite& cible, CompAtt* const& TypeAttaque);
    void Soigner(Unite& healer, Unite& cible, CompSoin* const& TypeSoin);
    void ActiverCamouflage(Unite& unite);
    void RejoindreCommandant(Unite& commandant, Unite& unite);
    
    void Transporter(Unite& transporteur, Unite& passager);
    void DechargerTransport(Unite& transporteur, Unite& passager, int xDest, int yDest); 

    void ChangerPositionDefensive(Unite& unite);

    // Accesseurs
    std::string getName() const { return _name; }
    int getNbVilles() const { return _cities.size(); }
    const std::list<City*> & getCities() const { return _cities; }
    const std::list<Unite*> & getUnites() const { return _unites; }
    const std::list<Batiment*> & getBatiments() const { return _batiments;}

};
