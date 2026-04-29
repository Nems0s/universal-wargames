#pragma once
#include <iostream>
#include <map>
#include <list>

#include "joueur.hh"
#include "jeu.hh"
#include "unite.hh"
#include "comportement.hh"

class Arbitre {
private:

public:
    // --- MÉTHODES GÉNÉRALES ---
    bool peutPayer(const std::map<const Ressource*, int>& cout, const Joueur& j) const;

    bool coordValid(int x, int y, const board & game) const;

    bool checkWin(const Joueur& j, const WinConditions & win) const;
    bool verifierVictoire(const Joueur & j, const GameConfig & config) const;



    // --- ZONE PLATEAU & CONSTRUCTION (Toi) ---

    bool tenterConstruction(int x, int y, std::unique_ptr<Batiment> b, Joueur & j, board & game);

    bool buildCity(const Joueur & j, const board & game, int x, int y) const;
    bool buildBuildingInCity(const Joueur & j, const City & city, const Batiment & b) const;
    bool buildSpecialBuilding(const Joueur & j, const board & game, const Batiment & b, int x, int y) const;
    bool moveUnite(const Joueur & j, const board & game, const Unite & u, int xDest, int yDest) const;                                  //Modifier par Simon
    bool validPayRessource(Joueur & j, const Batiment & b);

    bool peutAmeliorerVille(const Joueur& j, const City& ville, const GameConfig & config) const;
    bool peutAmeliorerBatiment(const Joueur& j, const Batiment& b) const;

    bool estDansTerritoire(const Joueur& j, int x, int y, const board & game, const GameConfig & config) const;
    bool peutAcheterCase(const Joueur & j, int x, int y, const board & game, const GameConfig & config) const;
    std::map<const Ressource*, int> getCostAchatCase(const Joueur & j, const GameConfig & config) const;
    std::map<const Ressource*, int> getCostNouvelleVille(const Joueur & j, const std::map<const Ressource*, int>& coutBase) const;
    bool estCaseHabitable(const TuileConfigurable& t, const Joueur & j) const;

    bool peutDetruireBatiment(const Joueur & j, const Batiment & b) const;

    // --- ZONE UNITÉS & COMBAT ---
    bool appartientJoueur(const Joueur& j, const Unite& unite)const;
    bool peutRecruterUnite(const Joueur& j, const std::map<const Ressource*, int>& cout, const Unite& invocation)const;
    bool peutAttaquer(const Joueur& j, const Unite& attaque, const Unite& cible, CompAtt* const& TypeAttaque)const;                     //Modifier par Simon
    bool peutSoigner(const Joueur& j, const Unite& healer, const Unite& cible, CompSoin* const& TypeSoin)const;                         //Modifier par Simon
    bool peutActiverCamouflage(const Joueur& j, const Unite& unite)const;                                                               //Modifier par Simon
    bool peutTransporter(const Joueur& j, const Unite& unite)const;                                                                     //Modifier par Simon
    bool peutRejoindreCommandant(const Joueur& j, const Unite& commandant, const Unite& unite)const;                                    //Modifier par Simon
    bool peutDechargerTransport(const Joueur& j, const Unite& transporteur,const Unite& transporter, int xDest, int yDest) const;

    std::vector<std::pair<int, int>> getCasesDeplacementPossibles(const board& game, const Unite& u) const;
    std::vector<std::pair<int, int>> getCasesAttaquePossibles(const Joueur& j, const board& game, const Unite& u) const;

};