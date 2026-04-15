#include "UI/InterfaceManager.hh"
#include <iostream>

int main() {
    try {
        std::cout << "[MAIN] Creation de la fenetre SFML..." << std::endl;
        sf::RenderWindow window(sf::VideoMode(1280, 720), "Space Wargames");
        //window.setVerticalSyncEnabled(false);
        //window.setFramerateLimit(60);
        
        std::cout << "[MAIN] Lancement de l'interface..." << std::endl;
        InterfaceManager ui(window);
        
        std::cout << "[MAIN] Entree dans la boucle principale..." << std::endl;
        ui.run(); 

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