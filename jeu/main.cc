#include "jeu.hh"
#include <ctime>

int main() {

    std::srand(std::time(nullptr));

    FileFactory maFactory;
    
    maFactory.chargerConfig("config_espace.txt");

    board jeuSpace(10, maFactory);

    std::cout << "Plateau de jeu : " << std::endl;
    jeuSpace.affichage();

    int x = 4;
    int y = 7;

    std::cout << std::endl << "Le symbole de la case est : " << jeuSpace.getCell(x,y)->getSymbole() << std::endl;

    terre t1;

    if (jeuSpace.getCell(x, y)->estFranchissable(t1)) {
        std::cout << "Est franchissable." << std::endl;
    } else {
        std::cout << "N'est pas franchissable." << std::endl;
    }

}