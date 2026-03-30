#include "jeu.hh"
#include <ctime>

int main() {

    std::srand(std::time(nullptr));

    std::map<std::string, Ressource*> ressources;
    TxtRessourceReader resReader;
    resReader.load("configs/config_ressources.txt", ressources);
    if (ressources.empty()) {
        std::cerr << "ERREUR : Aucune ressource chargee." << std::endl;
        return 1;
    }

    BatimentFactory batFactory;
    TxtBatimentReader batReader;
    batFactory.chargerConfiguration("configs/config_batiments.txt", batReader, ressources);

    FileFactory maFactory;
    maFactory.chargerConfig("configs/config_espace.txt", ressources);

    board jeuSpace(10, maFactory);

    std::cout << "Plateau de jeu : " << std::endl;
    jeuSpace.affichage();



    return 0;
}