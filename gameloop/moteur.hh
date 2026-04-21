#pragma once
#include "jeu.hh"
#include "joueur.hh"
#include "arbitre.hh"
#include "unite.hh"

enum class TypeAction 
{
    FIN_TOUR,
    
    CONSTRUIRE_VILLE,
    CONSTRUIRE_BATIMENT,
    AMELIORER_VILLE,
    
    RECRUTER_UNITE,
    
    DEPLACER,
    ATTAQUER,
    SOIGNER,
    CAMMOUFLER,
    
    DEBUT_DEFENSSE,
    ARRET_DEFENSSE,
    
    CHARGEMENT,
    DECHARGEMENT,
    
    ENROLEMENT, // Mise sous un commandant
    DESENROLEMENT // Enleve de sous un commandant
};

struct Action 
{
    TypeAction type;
    int x1, y1;
    int x2, y2;
    std::string data;
};

enum class ResultatAction 
{
    SUCCES,
    FIN_TOUR,
    ECHEC_FONDS_INSUFFISANTS,
    ECHEC_COORD_INVALIDE,
    ECHEC_ARBITRE_REFUS
};

class GameManager{
private:
    board & _plateau;
    std::vector<std::unique_ptr<Joueur>> _joueurs;
    Arbitre & _arbitre;
    GameConfig & _config;
    UniteFactory & _factory;
    int _indexJoueurActuel;

public:
    GameManager(board & b, Arbitre & a, GameConfig & c, UniteFactory & f);

    const std::vector<std::unique_ptr<Joueur>>& getJoueurs() const { return _joueurs; };
    int getIndexJoueurActuel(){return _indexJoueurActuel;};

    void ajouterJoueur(std::unique_ptr<Joueur> j);
    void lancerPartie();

    ResultatAction actionFinTour();
    ResultatAction actionConstruireVille(Joueur& j, const Action& a);
    ResultatAction actionConstruireBatiment(Joueur& j, const Action& a);
    ResultatAction actionAmeliorerVille(Joueur& j, const Action& a);
    ResultatAction actionRecruterUnite(Joueur& j, const Action& a);
    ResultatAction actionDeplacer(Joueur& j, const Action& a);
    ResultatAction actionAttaquer(Joueur& j, const Action& a);
    ResultatAction actionSoigner(Joueur& j, const Action& a);
    ResultatAction actionCamoufler(Joueur& j, const Action& a);
    ResultatAction actionDebutDefense(Joueur& j, const Action& a);
    ResultatAction actionArretDefense(Joueur& j, const Action& a);
    ResultatAction actionChargement(Joueur& j, const Action& a);
    ResultatAction actionDechargement(Joueur& j, const Action& a);
    ResultatAction actionEnrolement(Joueur& j, const Action& a);
    ResultatAction actionDesenrolement(Joueur& j, const Action& a);

    ResultatAction traiterAction(const Action& action);
    
    void passerAuJoueurSuivant();
    void supprimerCadavre(Unite& u);
};
