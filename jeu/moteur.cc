#include "moteur.hh"

void MoteurDeJeu::phaseProduction(Joueur& j) {
    for (City* ville : j.getCities()) {
        ville->product(j);
    }
}

void MoteurDeJeu::passerTour(const GameConfig & config) {
    for (Joueur* j : _joueurs) {
        phaseProduction(*j);

        if (_arbitre.verifierVictoire(*j, config)) {
            std::cout << "Le joueur " << j->getName() << " a gagné." << std::endl;
        }
        
    }
    _tourActuel++;
    std::cout << "Tour : " << _tourActuel << std::endl;
}