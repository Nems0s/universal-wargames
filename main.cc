#include <algorithm>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <map> // Ajouté pour la gestion des ressources

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

// Helper pour afficher les stats d'une unité
void afficherEtat(const Unite& u) {
    std::cout << "[" << u.name() << "] HP: " << u.health_point() << "/" << u.health_point_max() 
              << " | Moral: " << u.moral_point() << std::endl;
}

// ============================================================
//                    TEST 0 : PLATEAU DE JEU
// ============================================================
void testPlateau() {
  std::cout << "\n========== TEST 0 : PLATEAU DE JEU ==========" << std::endl;
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
// FONCTIONS DE TESTS UNITES
// ============================================================

void testChargementJson(UniteFactory& factory) {
    std::cout << "\n--- TEST 1 : CHARGEMENT & CATALOGUE ---" << std::endl;
    auto u = factory.create("Infanterie d'Elite");
    if(u) {
        u->affiche();
        std::cout << "[SUCCES] Unite chargee avec " << u->liste_comportements().size() << " comportements." << std::endl;
    } else {
        std::cout << "[ERREUR] Verifiez le nom 'Infanterie d'Elite' dans config_unite.json" << std::endl;
    }
}

void testCombatTactique(UniteFactory& factory) {
    std::cout << "\n--- TEST 2 : COMBAT, ORIENTATION & MORAL ---" << std::endl;
    auto soldat = factory.create("Infanterie d'Elite");
    auto tank = factory.create("Tank de Garde");

    if(!soldat || !tank) return;

    soldat->setLocation({1, 1});
    tank->setLocation({2, 1});
    tank->setRegarde(direction::est); // Le tank tourne le dos au soldat (Backstab possible)

    std::cout << "Avant attaque : " << std::endl;
    afficherEtat(*tank);

    auto attaques = soldat->Offensive();
    if(!attaques.empty()) {
        std::cout << "[ACTION] " << soldat->name() << " attaque " << tank->name() << " par derriere !" << std::endl;
        Combat::fight(*soldat, attaques.front(), *tank);
        afficherEtat(*tank);
        std::cout << "Note : Les degats sont reduits par l'armure de 15 du Tank." << std::endl;
    }
}

void testFurtivite(UniteFactory& factory) {
    std::cout << "\n--- TEST 3 : FURTIVITE & EMBUSCADE ---" << std::endl;
    auto sniper = factory.create("Sniper");
    auto cible = factory.create("Gros Tank");
    
    if(!sniper || !cible) return;

    sniper->setLocation({0,0});
    cible->setLocation({3,0}); // A portee de tir (portee 6 dans le JSON)

    auto furtif = sniper->Cammouflage();
    if(furtif) {
        std::cout << "[ACTION] Activation du camouflage du Sniper." << std::endl;
        furtif->ActiveCammouflage();
        
        auto tirs = sniper->Offensive();
        if(!tirs.empty()){
            std::cout << "[ACTION] Tir d'embuscade (Bonus x1.5) sur le Gros Tank !" << std::endl;
            Combat::fight(*sniper, tirs.front(), *cible);
            afficherEtat(*cible);
            
            if(!furtif->camoufler()) std::cout << "[INFO] Le camouflage a ete brise par l'attaque." << std::endl;
        }
    }
}

void testInfection(UniteFactory& factory) {
    std::cout << "\n--- TEST 4 : INFECTION (DEGATS SUR LE TEMPS) ---" << std::endl;
    // Note: L'Infanterie d'Elite doit avoir un comportement AttaqueIndirect dans le JSON pour ce test
    auto infecteur = factory.create("Infanterie d'Elite");
    auto victime = factory.create("Tank de Garde");
    
    if(!infecteur || !victime) return;

    auto styleInfect = infecteur->Offensive(); 
    CompAtt* poison = nullptr;
    for(auto* a : styleInfect) {
        if(dynamic_cast<CompAttIndirect*>(a)) poison = a;
    }

    if(poison) {
        Combat::fight(*infecteur, poison, *victime);
        std::cout << "Cible infectee. Passage de 3 tours (Appel de update())..." << std::endl;
        for(int i = 1; i <= 3; ++i) {
            infecteur->update(); // Met à jour les infections
            std::cout << "Tour " << i << " : "; afficherEtat(*victime);
        }
    } else {
        std::cout << "[INFO] L'Infanterie d'Elite n'a pas d'AttaqueIndirect dans le JSON actuel." << std::endl;
    }
}

void testHierarchie(UniteFactory& factory) {
    std::cout << "\n--- TEST 5 : COMMANDANT & BUFFS ---" << std::endl;
    auto chef = factory.create("Commandant Allie");
    auto recrue = factory.create("Infanterie d'Elite");

    if(!chef || !recrue) return;

    // Création du lien de hiérarchie
    if(auto r = std::dynamic_pointer_cast<Rank_Regulier>(recrue->rank())) {
        r->setCommandant(chef);
        std::cout << "[INFO] " << recrue->name() << " est lie au " << chef->name() << "." << std::endl;
    }

    std::cout << "[ACTION] Verification des buffs (Inspiration) en combat..." << std::endl;
    auto tank = factory.create("Tank de Garde");
    auto att = recrue->Offensive();
    if(!att.empty()) {
        Combat::fight(*recrue, att.front(), *tank);
        std::cout << "[INFO] Les degats ont ete augmentes par le BonusDegat (15) du Commandant." << std::endl;
    }
}

void testSystemeSoin(UniteFactory& factory) {
    std::cout << "\n--- TEST 6 : SOINS ET COOLDOWN ---" << std::endl;
    auto medic = factory.create("Medecin");
    auto blesse = factory.create("Soldat Blesse");
    
    if(!medic || !blesse) return;

    blesse->setHealth_point(20);
    medic->setLocation({0,0});
    blesse->setLocation({1,0}); // A portee de soin
    std::cout << "Etat initial : "; afficherEtat(*blesse);

    auto soins = medic->Soin();
    if(!soins.empty()) {
        std::cout << "[ACTION] Soin direct sur " << blesse->name() << " !" << std::endl;
        Combat::heal(*medic, soins.front(), *blesse);
        afficherEtat(*blesse);
        
        std::cout << "[ACTION] Tentative de soin immediat (Attente echec Cooldown)..." << std::endl;
        if(!Combat::heal(*medic, soins.front(), *blesse)) {
            std::cout << "[SUCCES] Le cooldown a bien bloque le soin consecutif." << std::endl;
        }
    }
}

// ============================================================
// MAIN INTERACTIF
// ============================================================

int main() {
    std::srand(std::time(nullptr));
    
    UniteFactory factory;
    JsonUniteReader reader;

    std::string cfgDir = trouverConfigs();
    JsonRessourceReader resReader;
    std::map<std::string, Ressource*> ressources;
    try {
    resReader.load(cfgDir + "/config_ressources.json", ressources);
    } catch (const std::exception &e) {
    throw std::runtime_error("Erreur ressources : " + std::string(e.what()));
    }

    try {
        factory.chargerConfiguration(cfgDir + "/config_unite.json", reader, ressources);
    } catch (const std::exception& e) {
        std::cerr << "Erreur critique : " << e.what() << std::endl;
        return 1;
    }

    int choix = -1;
    // Changement de la condition de boucle : 8 est Quitter
    while(choix != 8) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "       SPACE WARGAMES : TEST SUITE" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "0. Test Plateau de Jeu (Config)" << std::endl;
        std::cout << "1. Test Chargement JSON & Catalogue" << std::endl;
        std::cout << "2. Test Combat (Orientation/Armure/Moral)" << std::endl;
        std::cout << "3. Test Furtivite & Embuscade" << std::endl;
        std::cout << "4. Test Infection (Evolution/Tours)" << std::endl;
        std::cout << "5. Test Hierarchie (Commandant/Buffs)" << std::endl;
        std::cout << "6. Test Soins & Cooldowns" << std::endl;
        std::cout << "7. Lancer TOUS les tests" << std::endl;
        std::cout << "8. Quitter" << std::endl;
        std::cout << "Choix : ";
        
        if (!(std::cin >> choix)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }

        switch(choix) {
            case 0: testPlateau(); break;
            case 1: testChargementJson(factory); break;
            case 2: testCombatTactique(factory); break;
            case 3: testFurtivite(factory); break;
            case 4: testInfection(factory); break;
            case 5: testHierarchie(factory); break;
            case 6: testSystemeSoin(factory); break;
            case 7:
                testPlateau();
                testChargementJson(factory);
                testCombatTactique(factory);
                testFurtivite(factory);
                testInfection(factory);
                testHierarchie(factory);
                testSystemeSoin(factory);
                break;
            case 8:
                std::cout << "Fin du programme." << std::endl;
                break;
            default: 
                std::cout << "Choix invalide." << std::endl;
                break;
        }
    }

    return 0;
}