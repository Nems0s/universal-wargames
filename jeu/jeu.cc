#include "jeu.hh"


bool TuileConfigurable::estFranchissable(const Unite& u) const {
    if (_d->mouv.marche && u.peutMarcher()) return true;
    if (_d->mouv.nage && u.peutNager()) return true;
    if (_d->mouv.aerien && u.peutVoler()) return true;
    return false;
}

bool TuileConfigurable::peutConstrVille() const {
    return _d->constructible && !_city;
}

bool TuileConfigurable::peutConstrBatiment(const Batiment & b) const {
    return (b.getRessourceRequired() == nullptr);
}

bool TuileConfigurable::peutConstrBatimentSpecial(const Batiment & b) const {
    Ressource* required = b.getRessourceRequired();
    if (required == nullptr) return false;

    for (Ressource* r : _d->ressourceSpeciale) {
        if (r == required) return true;
    }

    return false;
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

    if (_matrix[x][y]->getSymbole() != '#') {
        if (b->getRessourceRequired() != nullptr) {
            if (tuile->peutConstrBatimentSpecial(*b)) {
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

void TxtWorldReader::chargerConfig(std::string chemin, const std::map<std::string, Ressource*>& resDispo, WorldFactory& factory) {
    std::ifstream fichier(chemin);
    std::string mot, ligne;

    while (fichier >> mot) {
        if (mot == "TILE") {
            TuileData d;
            // nom tuile, symbole, cout unite, constructible dessus ou non
            fichier >> d.nom >> d.symbole >> d.cout >> d.constructible;

            while (fichier >> mot && mot != "END") {
                if (mot == "GEN") {
                    // Pourcentage quantité, nombres minimum sur le terrain
                    fichier >> d.gen.poids >> d.gen.nbMin;
                } 
                else if (mot == "MOUV") {
                    // marche, nage, aerien
                    fichier >> d.mouv.marche >> d.mouv.nage >> d.mouv.aerien;
                }
                else if (mot == "RES") {
                    // lis les ressources
                    std::getline(fichier, ligne);
                    std::stringstream ss(ligne);
                    std::string nomRes;

                    while (ss >> nomRes) {
                        if (nomRes == "None" || nomRes.empty()) continue;

                        if (resDispo.count(nomRes)) {
                            d.ressourceSpeciale.push_back(resDispo.at(nomRes));
                        }
                    }
                }
                else if (mot == "ENV") {
                    // température, radiation, gravite
                    fichier >> d.env.temperature >> d.env.radiation >> d.env.gravite;
                }
                else if (mot == "DATA") {
                    // lecture clé-valeur des data
                    std::getline(fichier, ligne);
                    std::stringstream ss(ligne);
                    std::string cle;
                    float val;
                    while (ss >> cle) {
                        if (cle == "None") break;
                        if (ss >> val) {
                            d.properties[cle] = val; 
                        }
                    }
                }
            }
            factory.ajouterAuCatalogue(d.symbole, d);
            std::cout << "Chargé : " << d.nom << " (" << d.symbole << ")" << std::endl;
        }
    }

    if (!fichier.eof() && fichier.fail()) {
        throw std::runtime_error("Erreur dans le fichier : " + chemin);
    }
}

std::unique_ptr<hexa> WorldFactory::createTile(char symbole) {
    auto it = _catalogue.find(symbole);
    if (it != _catalogue.end()) {
        return std::make_unique<TuileConfigurable>(&(it->second));
    } else {
        return nullptr;
    }
}

std::unique_ptr<hexa> WorldFactory::createRandomTile() {
    if (_catalogue.empty()) return nullptr;

    int poidsTotal = 0;
    for (auto const& [symb, data] : _catalogue) {
        poidsTotal += data.gen.poids;
    }

    // tuile au hasard si poidstotal à 0
    if (poidsTotal == 0) {
        auto it = _catalogue.begin();
        std::advance(it, rand() % _catalogue.size());
        return std::make_unique<TuileConfigurable>(&(it->second));
    } else {
        // par rapport aux poids
        int tirage = rand() % poidsTotal;
        int seuil = 0;
        for (auto & [symb, data] : _catalogue) {
            seuil += data.gen.poids;
            if (tirage < seuil) {
                return std::make_unique<TuileConfigurable>(&data);
            }
        }
    }
    return std::make_unique<TuileConfigurable>(&(_catalogue.begin()->second));
}

void WorldFactory::postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>>& matrix, int size) {
    std::map<char, int> compteurs;
    
    for (auto & ligne : matrix) {
        for (auto & tuile : ligne) {
            compteurs[tuile->getSymbole()]++;
        }
    }

    for (auto const & [symb, data] : _catalogue) {
        while (compteurs[symb] < data.gen.nbMin) {
            int x = rand() % (size - 2) + 1;
            int y = rand() % (size - 2) + 1;

            if (matrix[x][y]->getSymbole() != '#' && matrix[x][y]->getSymbole() != symb) {
                matrix[x][y] = std::make_unique<TuileConfigurable>(&(_catalogue.at(symb)));
                compteurs[symb]++;
            }
        }
    }
}
