#include <iostream>
#include <map>
#include <list>

#include "joueur.hh"
#include "jeu.hh"

class Arbitre {
private:

public:
    // --- MÉTHODES GÉNÉRALES ---
    bool peutPayer(const std::map<Ressource*, int>& cout, const Joueur& j) const;

    bool coordValid(int x, int y, const board & game) const;



    // --- ZONE PLATEAU & CONSTRUCTION (Toi) ---

    bool tenterConstruction(int x, int y, std::unique_ptr<Batiment> b, Joueur & j, board & game);

    bool buildCity(const Joueur & j, const board & game, int x, int y) const;
    bool buildBuildingInCity(const Joueur & j, const City & city, const Batiment & b) const;
    bool buildSpecialBuilding(const Joueur & j, const board & game, const Batiment & b, int x, int y) const;
    bool moveUnite(const Joueur & j, const board & game, const Unite & u, int xDest, int yDest) const;
    bool validPayRessource(Joueur & j, const Batiment & b);

    bool peutAmeliorerVille(const Joueur& j, const City& ville, const GameConfig & config) const;
    bool peutAmeliorerBatiment(const Joueur& j, const Batiment& b) const;

    bool estDansTerritoire(const Joueur& j, int x, int y, const board & game, const GameConfig & config) const;
    bool peutAcheterCase(const Joueur & j, int x, int y, const board & game, const GameConfig & config) const;

    bool estCaseHabitable(const TuileConfigurable& t, const Joueur & j) const;

    bool peutDetruireBatiment(const Joueur & j, const Batiment & b) const;

    // --- ZONE UNITÉS & COMBAT (Ton collègue) ---



    


};