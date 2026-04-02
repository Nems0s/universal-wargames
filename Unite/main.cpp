#include <iostream>
#include <memory>
#include "unite.hh"
#include "comportement.hh"
#include "rank.hh"
#include "../combat/combat.hh"
#include "orientation.hh"

int main() {
    std::cout << "========== INITIALISATION DU TEST DE COMBAT ==========" << std::endl;

    // 1. Création des Rangs et des Bonus du Commandant
    auto rankCmd = std::make_shared<Rank_Commandant>();
    rankCmd->ajout_bonus(std::make_shared<BonusDegat>(10)); // +10 de dégâts via le chef
    rankCmd->ajout_bonus(std::make_shared<BonusVie>(15));   // Capacité de soin

    auto rankRegulier = std::make_shared<Rank_Regulier>();
    auto rankEnnemi = std::make_shared<Rank_Regulier>();

    // 2. Création des Unités
    // Note : Le constructeur prend désormais la liste de comportements
    auto general = std::make_shared<Unite>("General", 200, 10, Poids::Lourd, direction::est, Case{5,5}, rankCmd, std::list<std::shared_ptr<IComportement>>());

    auto soldat = std::make_shared<Unite>("Fantassin", 100, 20, Poids::Moyen, direction::est, Case{0,0}, rankRegulier, std::list<std::shared_ptr<IComportement>>());
    rankRegulier->setCommandant(general); // Liaison au commandant

    auto tank = std::make_shared<Unite>("Tank Ennemi", 150, 15, Poids::Lourd, direction::ouest, Case{1,0}, rankEnnemi, std::list<std::shared_ptr<IComportement>>());
    tank->ajouterComportement(std::make_shared<CompDefArmure>(10)); // Armure de 10

    // 3. Ajout des capacités d'attaque
    auto poison = std::make_shared<CompAttIndirect>(5, 1, 3); // 5 dmg, portée 1, 3 tours
    soldat->ajouterComportement(poison);

    std::cout << "\n--- Etat Initial ---" << std::endl;
    soldat->affiche(); //
    tank->affiche();

    // --- PHASE 1 : LE COMBAT ---
    std::cout << "\n========== EXECUTION DU COMBAT (Infection) ==========" << std::endl;

    // On utilise l'attaque indirecte (5 dmg de base)
    if (Combat::fight(*soldat, poison.get(), *tank)) { //
        std::cout << "[SYSTEME] Combat termine." << std::endl;
    }

    std::cout << "\n--- Analyse de l'impact ---" << std::endl;
    // Calcul attendu : max(Composant(5), Buff(10)) - Armure(10) = 0 dégâts directs ?
    // Si temporary_damage est 30 (20 base + 10 buff), alors max(5, 30) - 10 = 20 dmg.
    std::cout << "HP Tank : " << tank->health_point() << " / 150" << std::endl;
    std::cout << "Moral Soldat : " << soldat->moral_point() << std::endl;
    std::cout << "Moral Tank : " << tank->moral_point() << std::endl;

    // --- PHASE 2 : TOURS D'INFECTION ---
    std::cout << "\n========== TOURS D'INFECTION (Update) ==========" << std::endl;
    for (int i = 1; i <= 3; ++i) {
        std::cout << "\n>>> TOUR " << i << " <<<" << std::endl;
        soldat->update(); // Applique les dégâts d'infection
        std::cout << "HP Tank : " << tank->health_point() << std::endl;
    }

    // --- PHASE 3 : TEST DE LA FUITE (MORAL CRITIQUE) ---
    std::cout << "\n========== TEST DE LA FUITE ========== " << std::endl;
    std::cout << "Effondrement du moral du Fantassin..." << std::endl;

    // On force le moral au seuil de fuite
    DiminussionMoral(*soldat, 50);

    // Appel de l'EffetMoral pour déclencher la fuite
    EffetMoral(*soldat);

    if (soldat->health_point() <= 0) {
        std::cout << "[INFO] " << soldat->name() << " a disparu (HP a 0)." << std::endl;
    } else {
        std::cout << "Dmg Temporaires : " << soldat->temporary_damage() << std::endl;
    }

    std::cout << "\n========== FIN DES TESTS ==========" << std::endl;
    return 0;
}
