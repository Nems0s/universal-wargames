#pragma once

#include "arbitre.hh"
#include "jeu.hh"
#include "config.hh"
#include "batiment.hh"
#include "commandes.hh"

#include <vector>
#include <memory>
#include <map>
#include <string>

class SaveManager;

class MoteurDeJeu {

    friend class SaveManager;

private:
    // Jeu
    std::unique_ptr<board> _plateau;
    std::vector<Joueur> _joueurs;
    std::map<std::string, Ressource*> _ressourcesDispo;

    // Config
    GameConfig _logicConfig;
    WorldFactory _worldFactory;
    BatimentFactory _batimentFactory;
    CityFactory _cityFactory;
    UniteFactory _uniteFactory;

    // Logique
    Arbitre _arbitre;
    int _tourActuel;
    int _currentPlayerTurn;
    int _mapSeed;

    ResultatAction executer(int pIdx, const CmdFonderVille & cmd);
    ResultatAction executer(int pIdx, const CmdAcheterCase & cmd);
    ResultatAction executer(int pIdx, const CmdAmeliorer & cmd);
    ResultatAction executer(int pIdx, const CmdDeplacement & cmd);
    ResultatAction executer(int pIdx, const CmdAttaque & cmd);
    ResultatAction executer(int pIdx, const CmdRotation & cmd);
    ResultatAction executer(int pIdx, const CmdRecrutement & cmd);
    ResultatAction executer(int pIdx, const CmdConstruction & cmd);
    ResultatAction executer(int pIdx, const CmdSoigner& cmd);
    ResultatAction executer(int pIdx, const CmdCamoufler& cmd);
    ResultatAction executer(int pIdx, const CmdCharger& cmd);
    ResultatAction executer(int pIdx, const CmdDecharger& cmd);
    ResultatAction executer(int pIdx, const CmdEnroler& cmd);
    ResultatAction executer(int pIdx, const CmdFinTour & cmd);

public:
    MoteurDeJeu();

    // Init
    void chargerConfiguration(const std::string & configPath);
    void initGame(int seed, const std::vector<std::string>& noms, const std::vector<std::string>& factions);
    void chargerPartieDepuisJson(const nlohmann::json& j, const std::vector<std::string>& factions);

    // mis a jour du jeu
    void actualiserVisibiliteJoueur(int pIdx);
    void passerTour();

    // utilitaires sauvegarde et UI
    void setTourActuel(int t) { _tourActuel = t; }
    void setCurrentPlayerTurn(int c) { _currentPlayerTurn = c; }
    void overrideWorldWeights(const std::map<char, int>& w) { _worldFactory.overrideWeights(w); }

    // Pour modifier le jeu
    ResultatAction soumettreCommande(int pIdx, const CommandeJeu& commande);
    
    // getter pour lecture du jeu (interface)
    bool peutFonderVille(int joueurIdx, int x, int y) const;
    bool peutAmeliorerVille(int joueurIdx, int x, int y) const;
    bool peutAcheterTerritoire(int joueurIdx, int x, int y) const;
    bool estDansTerritoire(int pIdx, int x, int y) const;
    bool estVilleAuJoueur(int pIdx, int x, int y) const;
    bool peutPayer(int pIdx, const std::map<Ressource*, int> & cout) const;
    bool peutRecruterUnite(int pIdx, const std::string& nomUnite) const;
    int getProprietaireUnite(int x, int y) const;
    std::vector<std::pair<int, int>> getDeplacementsPossibles(int joueurIdx, int x, int y) const;
    bool peutPivoter(int pIdx, int x, int y) const;
    bool peutAttaquer(int pIdx, int xSrc, int ySrc, int xDest, int yDest) const;

    // Encapsulation de la Config
    const std::map<std::string, FactionParams>& getFactionsAvailable() const { return _logicConfig.getFactions(); }
    int getCoutRotation() const { return _logicConfig.getCoutRotation(); }
    std::map<Ressource*, int> getCoutFondationVille(int pIdx, const std::string& nomVille) const;
    std::map<Ressource*, int> getCoutAchatTerritoire(int pIdx) const;

    // Getters
    const board* getPlateau() const { return _plateau.get(); }
    const std::vector<Joueur>& getJoueurs() const { return _joueurs; }
    const Arbitre& getArbitre() const { return _arbitre; }
    const BatimentFactory& getBatimentFactory() const { return _batimentFactory; }
    const CityFactory& getCityFactory() const { return _cityFactory; }
    const UniteFactory& getUniteFactory() const { return _uniteFactory; }
    const GameConfig& getLogicConfig() const { return _logicConfig; }
    int getTourActuel() const { return _tourActuel; }
    int getCurrentPlayerTurn() const { return _currentPlayerTurn; }
    int getMapSeed() const { return _mapSeed; }
};