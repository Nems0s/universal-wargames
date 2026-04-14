#pragma once
#include "arbitre.hh"
#include "jeu.hh"
#include <vector>

class MoteurDeJeu {
private:
    board & _plateau;
    Arbitre & _arbitre;
    std::vector<Joueur*> _joueurs;
    int _tourActuel;

public:
    MoteurDeJeu(board & b, Arbitre & a) : _plateau(b), _arbitre(a), _tourActuel(1) {}

    void ajouterJoueur(Joueur* j) { _joueurs.push_back(j); }
    
    void passerTour(const GameConfig & config);
    
    void phaseProduction(Joueur& j);
};