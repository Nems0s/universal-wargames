#include "jeu.hh"
#include <ctime>
#include <stdexcept>

int main() {

    std::srand(std::time(nullptr));

    std::map<std::string, Ressource*> ressources;
    WorldFactory world;

    try {
        TxtRessourceReader resReader;
        try {
            resReader.load("configs/config_ressources.txt", ressources);
        } catch(const std::out_of_range& e) {
            throw std::runtime_error("Erreur dans TxtRessourceReader : " + std::string(e.what()));
        }
        if (ressources.empty()) {
            std::cerr << "ERREUR : Aucune ressource chargee." << std::endl;
            return 1;
        }

        BatimentFactory batFactory;
        TxtBatimentReader batReader;
        try {
            batFactory.chargerConfiguration("configs/config_batiments.txt", batReader, ressources);
        } catch(const std::out_of_range& e) {
            throw std::runtime_error("Erreur dans BatimentFactory : " + std::string(e.what()));
        }

        TxtWorldReader worldReader;
        try {
            worldReader.chargerConfig("configs/config_espace.txt", ressources, world);

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
    
    } catch (const std::exception& e) {
        std::cerr << "Erreur (exception) : " << e.what() << std::endl;
        return 1; 
    }

    return 0;
}