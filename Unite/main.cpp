#include <iostream>
#include <memory>
#include <cassert>
#include <vector>

#include "unite.hh"
#include "comportement.hh"
#include "rank.hh"
#include "orientation.hh"
#include "../combat/combat.hh"

/**
 * CLAUDE_VAS_Y : TEST SUITE COMPLET
 * Ce main teste la chaîne complète : du déplacement au combat buffé.
 */

int main() {
    std::cout << "--- DEBUT DU TEST DE COMBAT ---" << std::endl;

    // 1. Setup des Rangs et Bonus
    auto rankCmd = std::make_shared<Rank_Commandant>();
    rankCmd->ajout_bonus(std::make_shared<BonusDegat>(10)); // +10 Dmg
    rankCmd->ajout_bonus(std::make_shared<BonusVie>(20));   // Soin après combat

    auto rankSoldat = std::make_shared<Rank_Regulier>();
    auto rankCible = std::make_shared<Rank_Regulier>();

    // 2. Création des Unités
    // Un Commandant pour donner les buffs
    auto chef = std::make_shared<Unite>("General", 200, 10, Poids::Lourd, direction::est, Case{5,5}, rankCmd);

    // L'attaquant (lié au chef pour avoir les buffs)
    auto soldat = std::make_shared<Unite>("Soldat", 100, 20, Poids::Moyen, direction::est, Case{0,0}, rankSoldat);
    rankSoldat->setCommandant(chef);

    // Le défenseur (avec une armure)
    auto cible = std::make_shared<Unite>("Tank", 150, 10, Poids::Lourd, direction::ouest, Case{1,0}, rankCible);
    cible->ajouterComportement(std::make_shared<CompDefArmure>(5));

    // 3. Attribution des attaques
    auto epee = std::make_shared<CompAttMelee>(20);
    soldat->ajouterComportement(epee);

    // ---------------------------------------------------------
    // SCÉNARIO 1 : Attaque de face (Dégâts normaux + Buffs)
    // ---------------------------------------------------------
    std::cout << "\n> SCENARIO 1 : Attaque de face" << std::endl;
    // On récupère le pointeur nu car c'est ce qu'attend Combat::fight
    CompAtt* ptrEpee = epee.get();

    if (Combat::fight(*soldat, ptrEpee, *cible)) {
        std::cout << "Combat fini !" << std::endl;
        std::cout << "HP Tank : " << cible->health_point() << " (Attendu: 150 - (20+10-5) = 125)" << std::endl;
        std::cout << "Moral Soldat : " << soldat->moral_point() << " (Attendu: +1)" << std::endl;
    }

    // ---------------------------------------------------------
    // SCÉNARIO 2 : Attaque de dos (Avantage tactique x1.5)
    // ---------------------------------------------------------
    std::cout << "\n> SCENARIO 2 : Attaque de dos" << std::endl;
    // Le soldat se déplace derrière (Case{-1,0} n'est pas dans le champ de vision ouest)
    soldat->setLocation(Case{2,0});

    // Le soldat attaque dans le dos du Tank (qui regarde Ouest)
    Combat::fight(*soldat, ptrEpee, *cible);
    std::cout << "HP Tank apres coup de dos : " << cible->health_point() << std::endl;

    // ---------------------------------------------------------
    // SCÉNARIO 3 : Test du Moral (Panique)
    // ---------------------------------------------------------
    std::cout << "\n> SCENARIO 3 : Chute du Moral" << std::endl;
    cible->setMoral_point(-12); // Seuil de Panique (-10)

    std::cout << "HP Tank avant combat panique : " << cible->health_point() << std::endl;
    Combat::fight(*soldat, ptrEpee, *cible);
    std::cout << "HP Tank apres panique (perte 20% auto) : " << cible->health_point() << std::endl;

    // ---------------------------------------------------------
    // SCÉNARIO 4 : Defense totale (Dégâts <= 0)
    // ---------------------------------------------------------
    std::cout << "\n> SCENARIO 4 : Defense parfaite" << std::endl;
    cible->ajouterComportement(std::make_shared<CompDefBouclier>(1)); // Annule le prochain coup
    soldat->setMoral_point(-10); // Soldat a peur, ses degats baissent

    Combat::fight(*soldat, ptrEpee, *cible);
    std::cout << "Moral Soldat apres echec : " << soldat->moral_point() << " (Doit baisser)" << std::endl;
    std::cout << "Moral Tank apres defense reussie : " << cible->moral_point() << " (Doit monter)" << std::endl;
    return 0;
}
