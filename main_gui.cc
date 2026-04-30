#include <iostream>
#include "moteur.hh"
#include "UI/InterfaceManager.hh"

int main() {
    try {

        // Le moteur du jeu
        MoteurDeJeu moteur;
        moteur.chargerConfiguration("configs/config_rules.json");

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