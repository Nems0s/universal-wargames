#include <algorithm>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>


// Jeu / Plateau
#include "jeu.hh"

// Unite / Combat
#include "combat.hh"
#include "comportement.hh"
#include "orientation.hh"
#include "rank.hh"
#include "unite.hh"


// ============================================================
//               Recherche du dossier configs
// ============================================================
std::string trouverConfigs() {
  if (std::filesystem::exists("configs"))
    return "configs";
  if (std::filesystem::exists("../configs"))
    return "../configs";
  if (std::filesystem::exists("../../configs"))
    return "../../configs";
  throw std::runtime_error(
      "Dossier 'configs' introuvable ! Verifiez le repertoire de travail.");
}

// ============================================================
//                    TEST 1 : PLATEAU DE JEU
// ============================================================
void testPlateau() {
  std::cout << "\n========== TEST 1 : PLATEAU DE JEU ==========" << std::endl;
  std::string cfgDir = trouverConfigs();
  std::map<std::string, Ressource *> ressources;
  WorldFactory world;

  JsonRessourceReader resReader;
  try {
    resReader.load(cfgDir + "/config_ressources.json", ressources);
  } catch (const std::exception &e) {
    throw std::runtime_error("Erreur ressources : " + std::string(e.what()));
  }

  BatimentFactory batFactory;
  JsonBatimentReader batReader;
  try {
    batFactory.chargerConfiguration(cfgDir + "/config_batiments.json", batReader,
                                    ressources);
  } catch (const std::exception &e) {
    throw std::runtime_error("Erreur batiments : " + std::string(e.what()));
  }

  JsonWorldReader worldReader;
  try {
    worldReader.chargerConfig(cfgDir + "/config_espace.json", ressources, world);
    world.initialiserBords();
  } catch (const std::exception &e) {
    throw std::runtime_error("Erreur monde : " + std::string(e.what()));
  }

  board jeuSpace(10, world);
  std::cout << "Plateau de jeu genere avec succes." << std::endl;
  jeuSpace.affichage();

  for (auto const& [nom, res] : ressources) {
    delete res;
  }
  ressources.clear();

  std::cout << "========== FIN TEST PLATEAU ==========" << std::endl;
}

// ============================================================
//                    TEST 2 : COMBAT & INFECTION
// ============================================================
void testCombat() {
  std::cout << "\n========== TEST 2 : MECANIQUES DE COMBAT AVANCEES =========="
            << std::endl;

  // 1. Mise en place de la hiérarchie et des bonus
  auto rankCmd = std::make_shared<Rank_Commandant>();
  rankCmd->ajout_bonus(
      std::make_shared<BonusDegat>(15)); // Un chef qui motive l'attaque
  rankCmd->ajout_bonus(
      std::make_shared<BonusDefense>(10)); // Un chef qui renforce la survie

  auto rankRegulier = std::make_shared<Rank_Regulier>();
  auto rankEnnemi = std::make_shared<Rank_Regulier>();

  // 2. Création des Unités
  // Unité alliée liée à un commandant pour recevoir les buffs de proximité
  auto general = std::make_shared<Unite>(
      "Commandant Allie", 150, Poids::Moyen, direction::est, Coord{5, 5},
      rankCmd, std::list<std::shared_ptr<IComportement>>());
  auto soldat = std::make_shared<Unite>(
      "Infanterie d'Elite", 100, Poids::Moyen, direction::est, Coord{1, 1},
      rankRegulier, std::list<std::shared_ptr<IComportement>>());
  rankRegulier->setCommandant(general);

  // Unité ennemie avec une forte armure
  auto tank = std::make_shared<Unite>(
      "Tank de Garde", 200, Poids::Lourd, direction::ouest, Coord{2, 1},
      rankEnnemi, std::list<std::shared_ptr<IComportement>>());
  tank->ajouterComportement(std::make_shared<CompDefArmure>(15));

  // 3. Attribution des équipements d'attaque
  auto lame = std::make_shared<CompAttMelee>(40);
  auto fusil = std::make_shared<CompAttDistance>(
      30, 4, 5, 1); // 30 dmg, portée 4, 5 munitions
  auto poison =
      std::make_shared<CompAttIndirect>(5, 1, 3); // Infection corrosive

  soldat->ajouterComportement(lame);
  soldat->ajouterComportement(fusil);
  soldat->ajouterComportement(poison);

  std::cout << "\n--- Phase 1 : Buffs et Hierarchie ---" << std::endl;
  std::cout << "Le " << soldat->name() << " est sous l'influence du "
            << general->name() << "." << std::endl;

  std::cout
      << "\n--- Phase 2 : Attaque avec Avantage d'Orientation (Backstab) ---"
      << std::endl;
  // On oriente le tank à l'Est pour que le soldat (en {1,1}) l'attaque par
  // l'arrière
  tank->setRegarde(direction::est);
  std::cout
      << "[TACTIQUE] Le soldat contourne l'ennemi pour frapper par derriere !"
      << std::endl;

  int hpAvant = tank->health_point();
  if (Combat::fight(*soldat, lame.get(), *tank)) {
    int degatsReels = hpAvant - tank->health_point();
    std::cout << "[COMBAT] Impact reussi ! Degats : " << degatsReels
              << " (Reduits par l'armure de 15)" << std::endl;
    std::cout << "HP Cible : " << hpAvant << " -> " << tank->health_point()
              << " (Bonus orientation x1.5 applique)" << std::endl;
  }

  std::cout << "\n--- Phase 3 : Etat Mental et Stress de Combat ---"
            << std::endl;
  // L'attaque a baissé le moral de la cible
  std::cout << "Moral actuel du " << tank->name() << " : "
            << tank->moral_point() << std::endl;
  EffetMoral(*tank, 0); // Analyse l'état mental (Peur, Panique...)

  std::cout << "\n--- Phase 4 : Attaque a Distance et Munitions ---"
            << std::endl;
  std::cout << "Munitions avant tir : " << fusil->munitions() << std::endl;
  if (Combat::fight(*soldat, fusil.get(), *tank)) {
    std::cout << "Tir effectue. Munitions restantes : " << fusil->munitions()
              << std::endl;
  }

  std::cout << "\n--- Phase 5 : Infection et Guerre d'Usure ---" << std::endl;
  if (Combat::fight(*soldat, poison.get(), *tank)) {
    std::cout << "[INFO] Le tank est infecté. Début de la dégradation."
              << std::endl;
  }
  for (int i = 1; i <= 2; ++i) {
    soldat->update();
    std::cout << "Tour " << i
              << " d'infection - HP Tank : " << tank->health_point()
              << std::endl;
  }

  std::cout << "\n--- Phase 6 : Effondrement du Moral (Fuite) ---" << std::endl;
  // Simulation d'une chute de moral poussant à la fuite
  DiminussionMoral(*tank, 25);
  EffetMoral(*tank, 0);
  if (tank->health_point() <= 0) {
    std::cout << "[ALERTE] Le moral a craqua ! Le Tank a fuy le champ de "
                 "bataille (HP à 0)."
              << std::endl;
  }

  std::cout << "========== FIN TEST COMBAT AMELIORE ==========" << std::endl;
}

// ============================================================
//                    TEST 3 : CAMOUFLAGE
// ============================================================
void testCamouflage() {
  std::cout << "\n========== TEST 3 : CAMOUFLAGE (Furtivite) =========="
            << std::endl;

  auto sniper =
      std::make_shared<Unite>("Sniper", 80, Poids::Leger, direction::est,
                              Coord{0, 0}, std::make_shared<Rank_Regulier>(),
                              std::list<std::shared_ptr<IComportement>>());
  auto tank =
      std::make_shared<Unite>("Gros Tank", 200, Poids::Lourd, direction::ouest,
                              Coord{1, 0}, std::make_shared<Rank_Regulier>(),
                              std::list<std::shared_ptr<IComportement>>());

  auto furtif =
      std::make_shared<CompFurtif>(2, 2); // 2 tours d'effet, 2 tours cooldown
  auto attSniper = std::make_shared<CompAttMelee>(30);
  sniper->ajouterComportement(furtif);
  sniper->ajouterComportement(attSniper);

  std::cout << "Activation du Camouflage..." << std::endl;
  furtif->ActiveCammouflage();

  // Test Embuscade (Bonus x1.5)
  int hpAvant = tank->health_point();
  if (Combat::fight(*sniper, attSniper.get(), *tank)) {
    std::cout << "Attaque surprise ! HP Tank : " << hpAvant << " -> "
              << tank->health_point() << std::endl;
    if (!furtif->camoufler())
      std::cout << "Camouflage brise par l'attaque." << std::endl;
  }

  std::cout << "========== FIN TEST CAMOUFLAGE ==========" << std::endl;
}

// ============================================================
//                    TEST 4 : SYSTEMES DE SOIN
// ============================================================
void testSoin() {
  std::cout << "\n========== TEST 4 : SYSTEMES DE SOIN ==========" << std::endl;

  // 1. Creation des Unites
  auto medic =
      std::make_shared<Unite>("Medecin", 100, Poids::Moyen, direction::est,
                              Coord{0, 0}, std::make_shared<Rank_Regulier>(),
                              std::list<std::shared_ptr<IComportement>>());
  auto blesse = std::make_shared<Unite>(
      "Soldat Blesse", 100, Poids::Moyen, direction::est, Coord{1, 0},
      std::make_shared<Rank_Regulier>(),
      std::list<std::shared_ptr<IComportement>>());

  // On blesse l'unite manuellement
  blesse->setHealth_point(30);
  blesse->setMoral_point(0);
  std::cout << "Etat initial : HP " << blesse->name() << " = "
            << blesse->health_point() << " / Moral = " << blesse->moral_point()
            << std::endl;

  // 2. Test Soin Direct
  auto soinDirect =
      std::make_shared<CompSoinDirect>(20, 2, 1); // 20 HP, portee 2
  medic->ajouterComportement(soinDirect);

  std::cout << "\n[ACTION] Tentative de soin direct..." << std::endl;
  if (Combat::heal(*medic, soinDirect.get(), *blesse)) {
    std::cout << "[SUCCES] HP apres soin direct : " << blesse->health_point()
              << std::endl;
    std::cout << "[SUCCES] Moral apres soin : " << blesse->moral_point()
              << " (Augmente par le soin)" << std::endl;
  }

  // 3. Test Soin Indirect (Regeneration sur le temps)
  auto regen =
      std::make_shared<CompSoinIndirect>(10, 2, 3); // 10 HP par tour, 3 tours
  medic->ajouterComportement(regen);

  std::cout << "\n[ACTION] Application d'un kit de regeneration..."
            << std::endl;
  if (Combat::heal(*medic, regen.get(), *blesse)) {
    std::cout << "[KIT] Regeneration activee pour 3 tours." << std::endl;
  }

  for (int i = 1; i <= 3; ++i) {
    medic->update();
    std::cout << "Tour " << i << " de regen - HP " << blesse->name() << " : "
              << blesse->health_point() << std::endl;
  }

  std::cout << "========== FIN TEST SOIN ==========" << std::endl;
}

// ============================================================
//                           MENU
// ============================================================
int main() {
  std::srand(std::time(nullptr));
  int choix = -1;

  while (choix != 0) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   SPACE WARGAMES - MENU COMPLET" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  1. Test Plateau de Jeu (Config)" << std::endl;
    std::cout << "  2. Test Combat & Infection" << std::endl;
    std::cout << "  3. Test Camouflage (Furtivite)" << std::endl;
    std::cout << "  4. Test Soins (Direct & Indirect)" << std::endl;
    std::cout << "  5. Lancer TOUS les tests" << std::endl;
    std::cout << "  0. Quitter" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Choix : ";
    std::cin >> choix;

    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(10000, '\n');
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
        testCamouflage();
        break;
      case 4:
        testSoin();
        break;
      case 5:
        testPlateau();
        testCombat();
        testCamouflage();
        testSoin();
        break;
      case 0:
        std::cout << "Fermeture du programme." << std::endl;
        break;
      default:
        std::cout << "Choix invalide." << std::endl;
        break;
      }
    } catch (const std::exception &e) {
      std::cerr << "ERREUR CRITIQUE : " << e.what() << std::endl;
    }
  }
  return 0;
}
