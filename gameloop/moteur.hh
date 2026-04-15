#pragma once
#include "jeu.hh"
#include "joueur.hh"
#include "arbitre.hh"
#include "unite.hh"

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

    void ajouterJoueur(std::unique_ptr<Joueur> j);

    void lancerPartie();
    void executerTour(Joueur & j, const Arbitre & a);
 
};