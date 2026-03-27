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


void FileFactory::chargerConfig(std::string cheminFichier) {

    std::ifstream fichier(cheminFichier);
        if (!fichier.is_open()) {
            std::cerr << "Erreur : Impossible d'ouvrir " << cheminFichier << std::endl;
            return;
        }

        std::string nom;
        char symb;
        int cout, p, min;
        bool m, n;

        // Format attendu : Plaine T 1 1 0 70
        while (fichier >> nom >> symb >> cout >> m >> n >> p >> min) {
            TuileData nouvelleTuile = {nom, symb, cout, m, n, p, min};

            _catalogue[symb] = nouvelleTuile;
            
            std::cout << "Chargé : " << nom << " (" << symb << ")" << std::endl;
        }
}


std::unique_ptr<hexa> FileFactory::createTile(int, int) {
    if (_catalogue.empty()) return nullptr;

    int poidsTotal = 0;
    for (auto const& [symb, data] : _catalogue) {
        poidsTotal += data.poids;
    }

    int tirage = rand() % poidsTotal;

    int seuil = 0;
    for (auto const& [symb, data] : _catalogue) {
        seuil += data.poids;
        if (tirage < seuil) {
            return std::make_unique<TuileConfigurable>(data);
        }
    }

    return std::make_unique<TuileConfigurable>(_catalogue.begin()->second);
}
