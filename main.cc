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
        limite.nom = "Limite"; limite.symbole = '#'; limite.cout = -1;
        limite.constructible = false; limite.gen = {0, 0};
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

    auto rankCmd = std::make_shared<Rank_Commandant>();
    rankCmd->ajout_bonus(std::make_shared<BonusDegat>(10));
    rankCmd->ajout_bonus(std::make_shared<BonusVie>(15));

    auto rankRegulier = std::make_shared<Rank_Regulier>();
    auto rankEnnemi = std::make_shared<Rank_Regulier>();

    auto general = std::make_shared<Unite>("General", 200, 10, Poids::Lourd, direction::est, Coord{5,5}, rankCmd, std::list<std::shared_ptr<IComportement>>());
    auto soldat = std::make_shared<Unite>("Fantassin", 100, 20, Poids::Moyen, direction::est, Coord{0,0}, rankRegulier, std::list<std::shared_ptr<IComportement>>());
    rankRegulier->setCommandant(general);

    auto tank = std::make_shared<Unite>("Tank Ennemi", 150, 15, Poids::Lourd, direction::ouest, Coord{1,0}, rankEnnemi, std::list<std::shared_ptr<IComportement>>());
    tank->ajouterComportement(std::make_shared<CompDefArmure>(10));

    // Ajout d'un CompFurtif vide aux deux pour éviter les crashs de pointeurs dans Combat::fight
    soldat->ajouterComportement(std::make_shared<CompFurtif>());
    tank->ajouterComportement(std::make_shared<CompFurtif>());

    auto poison = std::make_shared<CompAttIndirect>(5, 1, 3);
    soldat->ajouterComportement(poison);

    std::cout << "\n--- Etat Initial ---" << std::endl;
    soldat->affiche();
    tank->affiche();

    std::cout << "\n========== EXECUTION DU COMBAT (Infection) ==========" << std::endl;
    if (Combat::fight(*soldat, poison.get(), *tank)) {
        std::cout << "[SYSTEME] Combat termine." << std::endl;
    }

    std::cout << "\n--- Analyse de l'impact ---" << std::endl;
    std::cout << "HP Tank : " << tank->health_point() << " / 150" << std::endl;

    std::cout << "\n========== TOURS D'INFECTION (Update) ==========" << std::endl;
    for (int i = 1; i <= 3; ++i) {
        std::cout << "\n>>> TOUR " << i << " <<<" << std::endl;
        soldat->update();
        std::cout << "HP Tank : " << tank->health_point() << std::endl;
    }

    std::cout << "\n========== TEST DE LA FUITE ========== " << std::endl;
    DiminussionMoral(*soldat, 50);
    EffetMoral(*soldat);

    if (soldat->health_point() <= 0) {
        std::cout << "[INFO] " << soldat->name() << " a disparu (HP a 0)." << std::endl;
    }
    std::cout << "\n========== FIN TEST COMBAT ==========" << std::endl;
}

// ============================================================
//                    TEST 3 : CAMOUFLAGE
// ============================================================
void testCamouflage() {
    std::cout << "\n========== TEST CAMOUFLAGE (Furtivite) ==========" << std::endl;

    // 1. Création des Unités
    // On utilise Coord{x, y} ou simplement {x, y} car Coord est un std::pair
    auto sniper = std::make_shared<Unite>("Sniper", 80, 25, Poids::Leger, direction::est, Coord{0,0},
                                          std::make_shared<Rank_Regulier>(), std::list<std::shared_ptr<IComportement>>());

    auto tank = std::make_shared<Unite>("Gros Tank", 200, 10, Poids::Lourd, direction::ouest, Coord{1,0},
                                        std::make_shared<Rank_Regulier>(), std::list<std::shared_ptr<IComportement>>());

    // 2. Configuration des composants
    auto furtif = std::make_shared<CompFurtif>(2, 2);
    auto attSniper = std::make_shared<CompAttMelee>(30);
    auto attTank = std::make_shared<CompAttMelee>(15);

    sniper->ajouterComportement(furtif);
    sniper->ajouterComportement(attSniper);

    // Sécurité : On ajoute un CompFurtif vide au tank pour éviter le crash nullptr
    tank->ajouterComportement(std::make_shared<CompFurtif>());
    tank->ajouterComportement(attTank);

    std::cout << "--- Sniper : Activation du Camouflage ---" << std::endl;
    furtif->ActiveCammouflage();
    sniper->affiche();

    // TEST 1 : Immunité
    std::cout << "\n[TEST 1] Le Tank tente d'attaquer le Sniper camoufle..." << std::endl;
    // fight utilise Coord via location()
    if (!Combat::fight(*tank, attTank.get(), *sniper)) {
        std::cout << "[OK] L'attaque a echoue : la cible est invisible." << std::endl;
    }

    // TEST 2 : Bonus Embuscade
    std::cout << "\n[TEST 2] Le Sniper attaque le Tank depuis son camouflage..." << std::endl;
    int hpAvant = tank->health_point();
    if (Combat::fight(*sniper, attSniper.get(), *tank)) {
        std::cout << "[OK] Attaque reussie avec bonus de furtivite (x1.5) !" << std::endl;
        std::cout << "HP Tank : " << hpAvant << " -> " << tank->health_point() << std::endl;

        if (!furtif->camoufler()) {
            std::cout << "[OK] Le camouflage s'est dissipe apres l'attaque." << std::endl;
        }
    }

    // TEST 3 : Cooldown
    std::cout << "\n[TEST 3] Verification du Cooldown..." << std::endl;
    sniper->update(); // Tour 1
    furtif->ActiveCammouflage();
    if (!furtif->camoufler()) std::cout << "   Cooldown en cours (T-1)..." << std::endl;

    sniper->update(); // Tour 2
    furtif->ActiveCammouflage(); // On utilise bien le nom de variable 'furtif'
    if (furtif->camoufler()) std::cout << "[OK] Camouflage a nouveau disponible !" << std::endl;

    std::cout << "\n========== FIN TEST CAMOUFLAGE ==========" << std::endl;
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
        std::cout << "  3. Test Camouflage (Furtivite)" << std::endl;
        std::cout << "  4. Lancer tous les tests" << std::endl;
        std::cout << "  0. Quitter" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Choix : ";
        std::cin >> choix;

        if (std::cin.fail()) {
            std::cin.clear(); std::cin.ignore(10000, '\n');
            continue;
        }

        try {
            switch (choix) {
            case 1: testPlateau(); break;
            case 2: testCombat(); break;
            case 3: testCamouflage(); break;
            case 4: testPlateau(); testCombat(); testCamouflage(); break;
            case 0: std::cout << "Au revoir !" << std::endl; break;
            default: std::cout << "Choix invalide." << std::endl; break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Erreur : " << e.what() << std::endl;
        }
    }
    return 0;
}
