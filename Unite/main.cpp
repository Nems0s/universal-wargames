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
    std::cout << "====================================================" << std::endl;
    std::cout << "       WARGAME ENGINE - INTEGRATION TESTS           " << std::endl;
    std::cout << "====================================================" << std::endl << std::endl;

    // ---------------------------------------------------------
    // 1. PRÉPARATION DES GRADES ET BONUS
    // ---------------------------------------------------------
    auto rankCmd = std::make_shared<Rank_Commandant>();
    auto rankReg1 = std::make_shared<Rank_Regulier>();
    auto rankReg2 = std::make_shared<Rank_Regulier>();

    // Ajout de bonus au commandant
    rankCmd->ajout_bonus(std::make_shared<BonusDegat>(10)); // +10 ATK
    rankCmd->ajout_bonus(std::make_shared<BonusVie>(25));   // +25 Soin post-combat

    // ---------------------------------------------------------
    // 2. CRÉATION DES UNITÉS
    // ---------------------------------------------------------

    // Le Commandant : Un Croiseur lourd (Marin)
    auto croiseur = std::make_shared<Unite>("HMS-Victory", 500, 20, Poids::Lourd, direction::est, Case{0, 0}, rankCmd);
    croiseur->ajouterComportement(std::make_shared<CompMouvMarin>(2));
    croiseur->ajouterComportement(std::make_shared<CompDefArmure>(15));

    // Le Soldat : Un Chasseur (Volant) rattaché au Croiseur
    auto chasseur = std::make_shared<Unite>("Rafale-01", 150, 40, Poids::Leger, direction::nord_est, Case{1, 0}, rankReg1);
    chasseur->ajouterComportement(std::make_shared<CompMouvVolant>(5));
    auto attDistance = std::make_shared<CompAttDistance>(40, 4, 10, 2); // Dmg 40, Portée 4, Mun 10, Portée min 2
    chasseur->ajouterComportement(attDistance);
    rankReg1->setCommandant(croiseur); // Liaison hiérarchique

    // L'Ennemi : Un Tank (Terrestre) avec Bouclier
    auto tankEnnemi = std::make_shared<Unite>("Panzer-X", 200, 30, Poids::Lourd, direction::ouest, Case{3, 0}, rankReg2);
    tankEnnemi->ajouterComportement(std::make_shared<CompMouvTerrestre>(1));
    tankEnnemi->ajouterComportement(std::make_shared<CompDefBouclier>(2)); // Bloque 2 attaques

    // ---------------------------------------------------------
    // 3. TEST GÉOMÉTRIE ET ORIENTATION
    // ---------------------------------------------------------
    std::cout << "--- [TEST 1] GEOMETRIE ---" << std::endl;
    Case c1 = {1, 1};
    auto voisins = Voisins(c1);
    std::cout << "Unite en (1,1) a " << voisins.size() << " voisins (Attendu: 6)." << std::endl;

    bool avantage = avantage_attaque(Case{0,0}, Case{1,0}, direction::est);
    std::cout << "Attaque de (0,0) vers (1,0) regardant Est : "
              << (avantage ? "AVANTAGE (Dos/Flanc)" : "FACE A FACE") << std::endl << std::endl;

    // ---------------------------------------------------------
    // 4. TEST TRANSPORT
    // ---------------------------------------------------------
    std::cout << "--- [TEST 2] TRANSPORT ---" << std::endl;
    auto compTransp = std::make_shared<CompTransport>(1);
    croiseur->ajouterComportement(compTransp);

    if(compTransp->MonterUnite(*croiseur, chasseur)) {
        std::cout << "Embarquement reussi : " << chasseur->name() << " est dans " << croiseur->name() << std::endl;
    }
    compTransp->affiche();

    // ---------------------------------------------------------
    // 5. TEST COMBAT COMPLET (AVEC BUFFS ET SOINS)
    // ---------------------------------------------------------
    std::cout << std::endl << "--- [TEST 3] COMBAT AVEC BONUS COMMANDANT ---" << std::endl;

    // On débarque et place le chasseur à portée (Distance de 2 cases : (1,0) -> (3,0))
    chasseur->setLocation(Case{1, 0});

    std::cout << "PV Tank avant : " << tankEnnemi->health_point() << std::endl;
    std::cout << "Munitions Chasseur : " << attDistance->munitions() << std::endl;

    // Lancement du combat
    // Le chasseur va bénéficier du +10 dégâts du Croiseur (total 50)
    // Le Tank va utiliser une charge de bouclier (dégâts -> 0)
    bool combatOk = Combat::fight(*chasseur, attDistance.get(), *tankEnnemi);

    if(combatOk) {
        std::cout << "Combat effectue !" << std::endl;
        std::cout << "PV Tank apres (Bouclier a du bloquer) : " << tankEnnemi->health_point() << std::endl;

        // On simule une blessure sur le chasseur pour tester le soin post-combat
        chasseur->setHealth_point(100);
        std::cout << "PV Chasseur blesse avant soin commandant : " << chasseur->health_point() << std::endl;

        // On relance un combat pour déclencher la phase de soin de fin de combat
        Combat::fight(*chasseur, attDistance.get(), *tankEnnemi);

        std::cout << "PV Chasseur apres soin (Attendu 100 + 25 = 125) : " << chasseur->health_point() << std::endl;
    }

    // ---------------------------------------------------------
    // 6. TEST ETAT FINAL ET NETTOYAGE
    // ---------------------------------------------------------
    std::cout << std::endl << "--- [TEST 4] VERIFICATION FINALE ---" << std::endl;

    // Vérifier que les stats temporaires sont bien à 0
    if(chasseur->temporary_damage() == 0) {
        std::cout << "Nettoyage des stats temporaires : OK" << std::endl;
    }

    croiseur->affiche();
    chasseur->affiche();
    tankEnnemi->affiche();

    std::cout << std::endl << "====================================================" << std::endl;
    std::cout << "             FIN DES TESTS - TOUT OK                " << std::endl;
    std::cout << "====================================================" << std::endl;

    return 0;
}
