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
//                   TEST 2 : COMBAT & INFECTION
// ============================================================
UniteFactory preparerFactory() 
{
  UniteFactory uFactory;
  JsonUniteReader uReader;
  uFactory.chargerConfiguration("configs/config_unite.json", uReader);
  return uFactory;
}

void testCombat() 
{
  std::cout << "\n--- TEST 2 : COMBAT ESPACE ---" << std::endl;
  auto uFactory = preparerFactory();

  auto soldat = uFactory.create("Infanterie d'Elite");
  auto tank = uFactory.create("Tank de Garde");

  if (soldat && tank) {
    // PLACEMENT : Le soldat en (1,1) et le tank en (2,1) -> Distance = 1
    soldat->setLocation({1, 1});
    tank->setLocation({2, 1});
    tank->setRegarde(direction::est); // Dos au soldat

    std::cout << "[INFO] " << soldat->name() << " est en " << soldat->location().first << "," << soldat->location().second << std::endl;

    auto offensive = soldat->Offensive();
    if (!offensive.empty()) 
    {
      int hpAvant = tank->health_point();
      if (Combat::fight(*soldat, offensive.front(), *tank)) 
      {
        std::cout << "[SUCCES] Attaque reussie. HP Tank: " << hpAvant << " -> " << tank->health_point() << std::endl;
      } 
      else 
      {
        std::cout << "[ECHEC] Le moteur de combat a refuse l'attaque." << std::endl;
      }
    }
  }
}

// ============================================================
//                   TEST 3 : EMBUSCADE
// ============================================================
void testCamouflage() 
{
  std::cout << "\n--- TEST 3 : SNIPER EN EMBUSCADE ---" << std::endl;
  auto uFactory = preparerFactory();

  auto sniper = uFactory.create("Sniper");
  auto tank = uFactory.create("Gros Tank");

  if (sniper && tank) 
  {
    // Le Sniper est loin (portée de 6 dans le JSON)
    sniper->setLocation({0, 0});
    tank->setLocation({4, 0}); // Distance de 4, parfait pour un sniper

    if (auto* furtif = sniper->Cammouflage()) 
    {
      furtif->ActiveCammouflage();
      auto offensive = sniper->Offensive();
      if (!offensive.empty()) 
      {
        int hpAvant = tank->health_point();
        Combat::fight(*sniper, offensive.front(), *tank);
        std::cout << "[EMBUSCADE] HP Gros Tank apres tir furtif : " << hpAvant << " -> " << tank->health_point() << std::endl;
      }
    }
  }
}

// ============================================================
//                   TEST 4 : INFIRMERIE
// ============================================================
void testSoin() 
{
  std::cout << "\n--- TEST 4 : INFIRMERIE ---" << std::endl;
  auto uFactory = preparerFactory();

  auto medic = uFactory.create("Medecin");
  auto blesse = uFactory.create("Soldat Blesse");

  if (medic && blesse) 
  {
    medic->setLocation({5, 5});
    blesse->setLocation({5, 6}); // Juste à côté
    blesse->setHealth_point(10); // Presque mort

    auto soins = medic->Soin();
    if (!soins.empty()) 
    {
      int hpAvant = blesse->health_point();
      Combat::heal(*medic, soins.front(), *blesse);
      std::cout << "[MEDIC] " << blesse->name() <<". Soin ! HP: " << hpAvant << " -> " <<blesse->health_point() << std::endl;
        
    }
  }
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
