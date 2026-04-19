#pragma once
#include "arbitre.hh"
#include "jeu.hh"
#include "config.hh"
#include "batiment.hh"
#include <vector>
#include <memory>
#include <map>
#include <string>

class MoteurDeJeu {
private:
    // Jeu
    std::unique_ptr<board> _plateau;
    std::vector<Joueur> _joueurs;
    std::map<std::string, Ressource*> _ressourcesDispo;

    // Config
    GameConfig _logicConfig;
    WorldFactory _worldFactory;
    BatimentFactory _batimentFactory;
    UniteFactory _uniteFactory;

    // Logique
    Arbitre _arbitre;
    int _tourActuel;
    int _currentPlayerTurn;
    int _mapSeed;

public:
    MoteurDeJeu();

    // Init
    void chargerConfiguration(const std::string & configPath);
    void initGame(int seed, int nbJoueurs, const std::vector<std::string>& factions);

    // mis a jour du jeu
    void passerTour();

    // utilitaires sauvegarde et UI
    void setTourActuel(int t) { _tourActuel = t; }
    void setCurrentPlayerTurn(int c) { _currentPlayerTurn = c; }
    void overrideWorldWeights(const std::map<char, int>& w) { _worldFactory.overrideWeights(w); }
    board* getPlateauMutable() { return _plateau.get(); }

    // Actions possibles
    bool demanderConstruction(int pIdx, int x, int y, const std::string& batNom);
    bool demanderDeplacement(int pIdx, int xSrc, int ySrc, int xDest, int yDest);
    bool demanderAmeliorationVille(int joueurIdx, int x, int y);

    // Requete état du jeu
    bool peutFonderVille(int joueurIdx, int x, int y) const;
    bool peutAmeliorerVille(int joueurIdx, int x, int y) const;
    bool peutAcheterTerritoire(int joueurIdx, int x, int y) const;
    std::vector<std::pair<int, int>> getDeplacementsPossibles(int joueurIdx, int x, int y) const;

    // Getters
    const board* getPlateau() const { return _plateau.get(); }
    const std::vector<Joueur>& getJoueurs() const { return _joueurs; }
    Joueur& getJoueurMutable(int idx) { return _joueurs[idx]; }
    const Arbitre& getArbitre() const { return _arbitre; }
    const BatimentFactory& getBatimentFactory() const { return _batimentFactory; }
    const UniteFactory& getUniteFactory() const { return _uniteFactory; }
    const GameConfig& getLogicConfig() const { return _logicConfig; }
    int getTourActuel() const { return _tourActuel; }
    int getCurrentPlayerTurn() const { return _currentPlayerTurn; }
    int getMapSeed() const { return _mapSeed; }
    
};