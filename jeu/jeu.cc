#include "jeu.hh"

void board::affichage() const {
    for (int i = 0; i < _size; ++i) {
        if (i%2 == 0) {
            std::cout << " ";
        }
        for (int j = 0; j < _size; ++j) {
            std::cout << _matrix[i][j]->getSymbole() << " ";
        }
        std::cout << std::endl;
    }
}
