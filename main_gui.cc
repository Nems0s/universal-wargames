#include <iostream>
#include "moteur.hh"
#include "UI/InterfaceManager.hh"

int main() {
    try {

        // Le moteur du jeu
        MoteurDeJeu moteur;

        // Config
        GameConfigFiles mesConfigs;
        std::string jeu = "space";

        mesConfigs.rulesPath = "configs/" + jeu + "/config_rules.json";
        mesConfigs.ressourcesPath = "configs/" + jeu + "/config_ressources.json";
        mesConfigs.batimentsPath = "configs/" + jeu + "/config_batiments.json";
        mesConfigs.villesPath = "configs/" + jeu + "/config_villes.json";
        mesConfigs.unitesPath = "configs/" + jeu + "/config_unites.json";
        mesConfigs.tuilesPath = "configs/" + jeu + "/config_tuiles.json";
        mesConfigs.winsPath = "configs/" + jeu + "/config_wins.json";

        moteur.chargerConfiguration(mesConfigs);

        // La fenetre
        std::cout << "[MAIN] Creation de la fenetre SFML..." << std::endl;
        sf::RenderWindow window(sf::VideoMode(1280, 720), "Space Wargames");
        
        std::cout << "[MAIN] Lancement de l'interface..." << std::endl;
        InterfaceManager interface(window, moteur);
        
        std::cout << "[MAIN] Entree dans la boucle principale..." << std::endl;
        interface.run(); 

        ImGui::SFML::Shutdown();
        std::cout << "[MAIN] Fermeture propre." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n=========================================\n";
        std::cerr << "[CRASH GLOBAL DU JEU] : " << e.what() << "\n";
        std::cerr << "=========================================\n";
    } catch (...) {
        std::cerr << "\n[CRASH GLOBAL] : Erreur inconnue (pas une std::exception) !\n";
    }
    
    return 0;
}