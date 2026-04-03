#include <iostream>
#include <ctime>
#include <stdexcept>
#include <memory>
#include <filesystem>

// Jeu / Plateau
#include "jeu.hh"

// Unite / Combat
#include "unite.hh"
#include "comportement.hh"
#include "rank.hh"
#include "combat.hh"
#include "orientation.hh"

// ============================================================
//               Recherche du dossier configs
// ============================================================
// Qt Creator lance depuis build/, donc on cherche configs/
// d'abord dans le répertoire courant, puis en remontant.
std::string trouverConfigs() {
    if (std::filesystem::exists("configs")) return "configs";
    if (std::filesystem::exists("../configs")) return "../configs";
    if (std::filesystem::exists("../../configs")) return "../../configs";
    throw std::runtime_error("Dossier 'configs' introuvable ! Verifiez le repertoire de travail.");
}

// ============================================================
//                    TEST 1 : PLATEAU DE JEU
// ============================================================
void testPlateau() {
    std::cout << "\n========== TEST PLATEAU DE JEU ==========" << std::endl;

    std::string cfgDir = trouverConfigs();

    std::map<std::string, Ressource*> ressources;
    WorldFactory world;

    TxtRessourceReader resReader;
    try {
        resReader.load(cfgDir + "/config_ressources.txt", ressources);
    } catch(const std::out_of_range& e) {
        throw std::runtime_error("Erreur dans TxtRessourceReader : " + std::string(e.what()));
    }
    if (ressources.empty()) {
        std::cerr << "ERREUR : Aucune ressource chargee." << std::endl;
        return;
    }

    BatimentFactory batFactory;
    TxtBatimentReader batReader;
    try {
        batFactory.chargerConfiguration(cfgDir + "/config_batiments.txt", batReader, ressources);
    } catch(const std::out_of_range& e) {
        throw std::runtime_error("Erreur dans BatimentFactory : " + std::string(e.what()));
    }

    TxtWorldReader worldReader;
    try {
        worldReader.chargerConfig(cfgDir + "/config_espace.txt", ressources, world);

        TuileData limite;
        limite.nom = "Limite";
        limite.symbole = '#';
        limite.cout = -1;
        limite.constructible = false;
        limite.gen = {0, 0};
        limite.mouv = {false, false, false};

        world.ajouterAuCatalogue('#', limite);

    } catch(const std::out_of_range& e) {
        throw std::runtime_error("Erreur dans FileFactory : " + std::string(e.what()));
    }

    board jeuSpace(10, world);

    std::cout << "Plateau de jeu : " << std::endl;
    jeuSpace.affichage();

    std::cout << "\n========== FIN TEST PLATEAU ==========" << std::endl;
}

// ============================================================
//                    TEST 2 : COMBAT
// ============================================================
void testCombat() {
    std::cout << "\n========== TEST COMBAT (Unites, Rangs, Moral) ==========" << std::endl;

    // 1. Création des Rangs et des Bonus du Commandant
    auto rankCmd = std::make_shared<Rank_Commandant>();
    rankCmd->ajout_bonus(std::make_shared<BonusDegat>(10)); // +10 de dégâts via le chef
    rankCmd->ajout_bonus(std::make_shared<BonusVie>(15));   // Capacité de soin

    auto rankRegulier = std::make_shared<Rank_Regulier>();
    auto rankEnnemi = std::make_shared<Rank_Regulier>();

    // 2. Création des Unités
    auto general = std::make_shared<Unite>("General", 200, 10, Poids::Lourd, direction::est, Coord{5,5}, rankCmd, std::list<std::shared_ptr<IComportement>>());

    auto soldat = std::make_shared<Unite>("Fantassin", 100, 20, Poids::Moyen, direction::est, Coord{0,0}, rankRegulier, std::list<std::shared_ptr<IComportement>>());
    rankRegulier->setCommandant(general); // Liaison au commandant

    auto tank = std::make_shared<Unite>("Tank Ennemi", 150, 15, Poids::Lourd, direction::ouest, Coord{1,0}, rankEnnemi, std::list<std::shared_ptr<IComportement>>());
    tank->ajouterComportement(std::make_shared<CompDefArmure>(10)); // Armure de 10

    // 3. Ajout des capacités d'attaque
    auto poison = std::make_shared<CompAttIndirect>(5, 1, 3); // 5 dmg, portée 1, 3 tours
    soldat->ajouterComportement(poison);

    std::cout << "\n--- Etat Initial ---" << std::endl;
    soldat->affiche();
    tank->affiche();

    // --- PHASE 1 : LE COMBAT ---
    std::cout << "\n========== EXECUTION DU COMBAT (Infection) ==========" << std::endl;

    if (Combat::fight(*soldat, poison.get(), *tank)) {
        std::cout << "[SYSTEME] Combat termine." << std::endl;
    }

    std::cout << "\n--- Analyse de l'impact ---" << std::endl;
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

    DiminussionMoral(*soldat, 50);
    EffetMoral(*soldat);

    if (soldat->health_point() <= 0) {
        std::cout << "[INFO] " << soldat->name() << " a disparu (HP a 0)." << std::endl;
    } else {
        std::cout << "Dmg Temporaires : " << soldat->temporary_damage() << std::endl;
    }

    std::cout << "\n========== FIN TEST COMBAT ==========" << std::endl;
}

// ============================================================
//                           MENU
// ============================================================
int main() {
    std::srand(std::time(nullptr));

    int choix = -1;

    while (choix != 0) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "   SPACE WARGAMES - MENU TEST" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "  1. Test Plateau de Jeu" << std::endl;
        std::cout << "  2. Test Combat (Unites, Rangs, Moral)" << std::endl;
        std::cout << "  3. Lancer tous les tests" << std::endl;
        std::cout << "  0. Quitter" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Choix : ";
        std::cin >> choix;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Entree invalide." << std::endl;
            continue;
        }

        try {
            switch (choix) {
            case 1:
                testPlateau();
                break;
            case 2:
                testCombat();
                break;
            case 3:
                testPlateau();
                testCombat();
                break;
            case 0:
                std::cout << "Au revoir !" << std::endl;
                break;
            default:
                std::cout << "Choix invalide." << std::endl;
                break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Erreur : " << e.what() << std::endl;
        }
    }

    return 0;
}
