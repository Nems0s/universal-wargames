#include "UI/InterfaceManager.hh"

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Space Wargames");
    
    // On passe la main à l'InterfaceManager
    InterfaceManager ui(window);
    ui.run(); // C'est ici que tout se passe désormais
    
    return 0;
}