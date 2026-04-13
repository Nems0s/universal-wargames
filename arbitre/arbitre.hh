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




    // --- ZONE PLATEAU & CONSTRUCTION (Toi) ---








    // --- ZONE UNITÉS & COMBAT ---
    bool peutRecruterUnite();
    bool validerAchatUnite();
    bool peutAttaquer();
    bool verifierPorteeAttaque();
    bool peutSoigner();
    bool peutActiverCamouflage();
    bool peutTransporter();
    bool peutRejoindreCommandant();
    bool validerLienHierarchique();
    bool peutEncoreAgir();
    bool finaliserAction();


};