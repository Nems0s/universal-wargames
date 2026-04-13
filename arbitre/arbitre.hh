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
    bool peutRecruterUnite()const;
    bool validerAchatUnite()const;
    bool peutAttaquer()const;
    bool verifierPorteeAttaque()const;
    bool peutSoigner()const;
    bool peutActiverCamouflage()const;
    bool peutTransporter()const;
    bool peutRejoindreCommandant()const;
    bool validerLienHierarchique()const;
    bool peutEncoreAgir()const;
    bool finaliserAction()const;


};