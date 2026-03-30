#include "jeu.hh"


bool TuileConfigurable::estFranchissable(const Unite& u) const {
    if (_d.marche && u.peutMarcher()) return true;
    if (_d.nage && u.peutNager()) return true;
    return false;
}

bool TuileConfigurable::peutConstrVille() const {
    return _d.constructible && !_city;
}

bool TuileConfigurable::peutConstrBatiment(const Batiment & b) const {
    return (b.getRessourceRequired() == nullptr);
}

bool TuileConfigurable::peutConstrBatimentSpecial(const Batiment & b) const {
    return (b.getRessourceRequired() != nullptr && b.getRessourceRequired() == _d.ressourceSpeciale);
}

void TuileConfigurable::constrVille(int max, bool capitale) {
    if (peutConstrVille()) {
        _city = std::make_unique<City>(max, capitale);
    }
}

void TuileConfigurable::constrBatimentSpeciale(std::unique_ptr<Batiment> b) {
    if (peutConstrBatimentSpecial(*b)) {
        _batimentSpecial = std::move(b);
    }
}


// BOARD //
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

void board::placerUnite(int x, int y, std::unique_ptr<Unite> u) {
    if (x >= 0 && x < _size && y >= 0 && y < _size) {
        _unites[{x, y}] = std::move(u);
    }
}

Unite * board::getUnite(int x, int y) const {
    auto it = _unites.find({x, y});
    if (it != _unites.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool board::deplacerUnite(int xSrc, int ySrc, int xDest, int yDest) {
    auto it = _unites.find({xSrc, ySrc});
    if (it == _unites.end()) return false;

    if (xDest < 0 || xDest >= _size || yDest < 0 || yDest >= _size) return false;
    if (_unites.count({xDest, yDest})) return false;

    if (!_matrix[xDest][yDest]->estFranchissable(*(it->second))) {
        return false;
    }

    _unites[{xDest, yDest}] = std::move(it->second);
    _unites.erase(it);
    
    return true;
}


void board::tenterConstruction(int x, int y, std::unique_ptr<Batiment> b, Joueur & j) {
    TuileConfigurable* tuile = dynamic_cast<TuileConfigurable*>(_matrix[x][y].get());
    if (!tuile) return;

    Ressource* resRequise = b->getRessourceRequired();

    if (_matrix[x][y]->getSymbole() != '#') {
        if (resRequise != nullptr) {
            if (tuile->getRessource() == resRequise) {
                if (j.peutPayer(b->getResourceConstr())) {
                    j.payer(b->getResourceConstr());
                    tuile->constrBatimentSpeciale(std::move(b));
                }
            }
        } else {
            if (tuile->getCity() && tuile->getCity()->peutAjouterBatiment()) {
                if (j.peutPayer(b->getResourceConstr())) {
                    j.payer(b->getResourceConstr());
                    tuile->getCity()->creeBatiment(std::move(b));
                }
            }
        }
    }
}


// FILEFACTORY //

void FileFactory::chargerConfig(std::string cheminFichier, const std::map<std::string, Ressource*> & ressourcesDispo) {

    std::ifstream fichier(cheminFichier);
        if (!fichier.is_open()) {
            std::cerr << "Erreur : Impossible d'ouvrir " << cheminFichier << std::endl;
            return;
        }

        std::string nom, nomRes;
        char symb;
        int cout, p, min;
        bool m, n, constr;

        // Format attendu : Etoile E 1 1 0 5 1 1 poussiereDEtoile
        while (fichier >> nom >> symb >> cout >> m >> n >> p >> min >> constr >> nomRes) {
            Ressource* r = nullptr;

            if (nomRes != "None" && ressourcesDispo.count(nomRes)) {
                r = ressourcesDispo.at(nomRes);
            }

            TuileData nouvelleTuile = {nom, symb, cout, m, n, p, min, constr, r};

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

    // tuile au hasard si poidstotal à 0
    if (poidsTotal == 0) {
        auto it = _catalogue.begin();
        std::advance(it, rand() % _catalogue.size());
        return std::make_unique<TuileConfigurable>(it->second);
    } else {
        // par rapport aux poids
        int tirage = rand() % poidsTotal;
        int seuil = 0;
        for (auto const& [symb, data] : _catalogue) {
            seuil += data.poids;
            if (tirage < seuil) {
                return std::make_unique<TuileConfigurable>(data);
            }
        }
    }

    return std::make_unique<TuileConfigurable>(_catalogue.begin()->second);
}

void FileFactory::postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>>& matrix, int size) {
    std::map<char, int> compteurs;
    
    for (auto & ligne : matrix) {
        for (auto & tuile : ligne) {
            compteurs[tuile->getSymbole()]++;
        }
    }

    for (auto const & [symb, data] : _catalogue) {
        while (compteurs[symb] < data.nbMin) {
            int x = rand() % (size - 2) + 1;
            int y = rand() % (size - 2) + 1;

            if (matrix[x][y]->getSymbole() != '#' && matrix[x][y]->getSymbole() != symb) {
                matrix[x][y] = std::make_unique<TuileConfigurable>(data);
                compteurs[symb]++;
            }
        }
    }
}
