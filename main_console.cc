#include <iostream>
#include <memory>
#include <ctime>
#include <filesystem>
#include <map>
#include <vector>

// Inclusions du moteur de jeu
#include "jeu.hh"
#include "joueur.hh"
#include "arbitre.hh"
#include "moteur.hh"
#include "unite.hh"
#include "config.hh"

// Helper pour trouver le dossier des configurations
std::string trouverConfigs() {
    if (std::filesystem::exists("configs")) return "configs";
    if (std::filesystem::exists("../configs")) return "../configs";
    throw std::runtime_error("Dossier 'configs' introuvable !");
}

int main() {
    // 1. Initialisation
    std::srand(std::time(nullptr));
    std::string cfgDir = trouverConfigs();
    
    // 2. Chargement des Ressources Globales
    std::map<std::string, Ressource*> catalogueRessources;
    JsonRessourceReader resReader;
    try {
        resReader.load(cfgDir + "/config_ressources.json", catalogueRessources);
        std::cout << "[SYSTEME] Ressources chargees." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Erreur critique Ressources : " << e.what() << std::endl;
        return 1;
    }

    // 3. Configuration des Règles et du Monde
    GameConfig config;
    config.loadRules(cfgDir + "/config_rules.json");

    WorldFactory world;
    JsonWorldReader worldReader;
    try {
        worldReader.chargerConfig(cfgDir + "/config_espace.json", catalogueRessources, world);
        world.initialiserBords();
    } catch (const std::exception &e) {
        std::cerr << "Erreur critique Monde : " << e.what() << std::endl;
        return 1;
    }

    // 4. Initialisation du Plateau et de l'Arbitre
    board plateau(world, config); 
    Arbitre arbitre;

    // 5. Initialisation de l'Usine d'Unités
    UniteFactory uniteFactory;
    JsonUniteReader uniteReader;
    try {
        uniteFactory.chargerConfiguration(cfgDir + "/config_unite.json", uniteReader, catalogueRessources);
        std::cout << "[SYSTEME] Unites chargees." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Erreur critique Unites : " << e.what() << std::endl;
        return 1;
    }

    // 6. Création du Manager de Jeu
    GameManager moteur(plateau, arbitre, config, uniteFactory);

    // -----------------------------------------------------------------
    // NOUVEAU : Création des Factions (Assurez-vous que FactionParams 
    // dans config.hh possède bien ces champs : nom et unites_disponibles)
    // -----------------------------------------------------------------
    FactionParams factionAlliance;
    factionAlliance.nom = "Alliance Terrestre";
    factionAlliance.unites_disponibles = {"Commandant Allie", "Infanterie d'Elite", "Tank de Garde", "Medecin"};

    FactionParams factionMercenaire;
    factionMercenaire.nom = "Syndicat Mercenaire";
    factionMercenaire.unites_disponibles = {"Commandant Allie", "Sniper", "Gros Tank", "Soldat Blesse"};

    // 7. Création et Configuration des Joueurs
    auto j1 = std::make_unique<Joueur>("Commandant Alpha");
    auto j2 = std::make_unique<Joueur>("Commandant Beta");

    // Attribution des factions
    j1->setFaction(&factionAlliance);
    j2->setFaction(&factionMercenaire);

    // Donner des ressources initiales (500 de chaque ressource du jeu)
    for (auto const& [nom, resPtr] : catalogueRessources) {
        j1->ajouterRessource(resPtr, 500);
        j2->ajouterRessource(resPtr, 500);
    }

    // 8. Placement des unités de départ
    
    // Joueur 1 : Position (1,1)
    std::shared_ptr<Unite> u1 = uniteFactory.create("Commandant Allie");
    if (u1) {
        u1->setLocation({1, 1});
        j1->ajouterUnite(u1.get());
        plateau.placerUnite(1, 1, u1); 
    }

    // Joueur 2 : Position (8,8)
    std::shared_ptr<Unite> u2 = uniteFactory.create("Commandant Allie");
    if (u2) {
        u2->setLocation({8, 8});
        j2->ajouterUnite(u2.get());
        plateau.placerUnite(8, 8, u2); 
    }

    // Ajouter les joueurs au moteur
    moteur.ajouterJoueur(std::move(j1));
    moteur.ajouterJoueur(std::move(j2));

    // 9. Lancement de la boucle de jeu
    std::cout << "\n========================================" << std::endl;
    std::cout << "       LANCEMENT DE SPACE WARGAMES" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    try {
        moteur.lancerPartie();
    } catch (const std::exception &e) {
        std::cerr << "Erreur en cours de partie : " << e.what() << std::endl;
    }

    // 10. Nettoyage de la mémoire
    for (auto const& [nom, res] : catalogueRessources) {
        delete res;
    }
    catalogueRessources.clear();

    return 0;
}