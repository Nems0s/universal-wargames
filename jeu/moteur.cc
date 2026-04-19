#include "moteur.hh"
#include <iostream>

MoteurDeJeu::MoteurDeJeu() 
    : _tourActuel(1), _currentPlayerTurn(0), _mapSeed(42) {}

void MoteurDeJeu::chargerConfiguration(const std::string& configPath) {
    try {
        JsonRessourceReader resReader;
        resReader.load("configs/config_ressources.json", _ressourcesDispo);

        JsonBatimentReader batReader;
        _batimentFactory.chargerConfiguration("configs/config_batiments.json", batReader, _ressourcesDispo);

        _logicConfig.loadRules(configPath);
        _logicConfig.loadWins("configs/config_wins.json");

        JsonUniteReader uniteReader;
        _uniteFactory.chargerConfiguration("configs/config_unites.json", uniteReader, _ressourcesDispo);

        JsonWorldReader worldReader;
        _worldFactory.initialiserBords();
        worldReader.chargerConfig("configs/config_espace.json", _ressourcesDispo, _worldFactory);
    } catch (const std::exception& e) {
        std::cerr << "[MOTEUR] Erreur de chargement : " << e.what() << std::endl;
    }
}

void MoteurDeJeu::initGame(int seed, int nbJoueurs, const std::vector<std::string>& factions) {
    _mapSeed = seed;
    std::srand(_mapSeed);

    // Création du monde
    _plateau = std::make_unique<board>(_worldFactory, _logicConfig);
    _joueurs.clear();

    // Création des joueurs
    for (int i = 0; i < nbJoueurs; ++i) {
        Joueur j;
        j.setName("Joueur" + std::to_string(i + 1));
        if (i < (int)factions.size()) j.setFaction(_logicConfig.getFaction(factions[i]));
        j.initBrouillard(_plateau->getRows(), _plateau->getCols());

        // Placement de la capitale
        bool placed = false;
        int attempts = 0;
        while (!placed && attempts < 1000) {
            int rx = std::rand() % _plateau->getRows();
            int ry = std::rand() % _plateau->getCols();

            // vérifier si la ville peut exister
            if (_arbitre.buildCity(j, *_plateau, rx, ry)) {
                hexa* cell = const_cast<hexa*>(_plateau->getCell(rx, ry));
                TuileConfigurable* tc = dynamic_cast<TuileConfigurable*>(cell);

                // lire JSON de la tuile et regarder si properties a spawn_capitale
                if (tc && tc->getStat("spawn_capitale") > 0.0f) {
                    tc->constrVille(rx, ry, _logicConfig, 5, true);
                    j.ajouterVille(tc->getCity());
                    j.decouvrirZone(rx, ry, 5, _plateau->getRows(), _plateau->getCols());
                    placed = true;
                }
            }
            attempts++;
        }
        _joueurs.push_back(j);
    }
    _tourActuel = 1;
    _currentPlayerTurn = 0;
}


bool MoteurDeJeu::peutFonderVille(int joueurIdx, int x, int y) const {
    if (joueurIdx < 0 || joueurIdx >= (int)_joueurs.size()) return false;
    return _arbitre.buildCity(_joueurs[joueurIdx], *_plateau, x, y);
}

bool MoteurDeJeu::peutAmeliorerVille(int joueurIdx, int x, int y) const {
    if (joueurIdx < 0 || joueurIdx >= (int)_joueurs.size()) return false;
    const hexa* cell = _plateau->getCell(x, y);
    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(cell);
    if (tc && tc->getCity()) {
        return _arbitre.peutAmeliorerVille(_joueurs[joueurIdx], *tc->getCity(), _logicConfig);
    }
    return false;
}

bool MoteurDeJeu::peutAcheterTerritoire(int joueurIdx, int x, int y) const {
    if (joueurIdx < 0 || joueurIdx >= (int)_joueurs.size()) return false;
    if (!_arbitre.estDansTerritoire(_joueurs[joueurIdx], x, y, *_plateau, _logicConfig)) {
        return _arbitre.peutAcheterCase(_joueurs[joueurIdx], x, y, *_plateau, _logicConfig);
    }
    return false;
}

std::vector<std::pair<int, int>> MoteurDeJeu::getDeplacementsPossibles(int joueurIdx, int x, int y) const {
    if (joueurIdx < 0 || joueurIdx >= (int)_joueurs.size()) return {};
    Unite* u = _plateau->getUnite(x, y);
    if (u && _arbitre.appartientJoueur(_joueurs[joueurIdx], *u)) {
        return _arbitre.getCasesDeplacementPossibles(*_plateau, *u);
    }
    return {};
}


bool MoteurDeJeu::demanderConstruction(int pIdx, int x, int y, const std::string& batNom) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    
    auto nvBat = _batimentFactory.create(batNom);

    return _arbitre.tenterConstruction(x, y, std::move(nvBat), _joueurs[pIdx], *_plateau);
}

bool MoteurDeJeu::demanderDeplacement(int pIdx, int xSrc, int ySrc, int xDest, int yDest) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    
    Unite* u = _plateau->getUnite(xSrc, ySrc);
    if (u && _arbitre.moveUnite(_joueurs[pIdx], *_plateau, *u, xDest, yDest)) {
        if (_plateau->deplacerUnite(*u, xDest, yDest)) {
            return true;
        }
    }
    return false;
}

bool MoteurDeJeu::demanderAmeliorationVille(int pIdx, int x, int y) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    
    const hexa* cell = _plateau->getCell(x, y);
    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(cell);
    
    if (tc && tc->getCity()) {
        if (_arbitre.peutAmeliorerVille(_joueurs[pIdx], *tc->getCity(), _logicConfig)) {
            tc->getCity()->upgrade();
            return true;
        }
    }
    return false;
}

void MoteurDeJeu::passerTour() {
    // Production des ressources
    Joueur& currentJ = _joueurs[_currentPlayerTurn];
    for (City* v : currentJ.getCities()) {
        v->product(currentJ);
    }

    // Vérification de la victoire
    if (_arbitre.verifierVictoire(currentJ, _logicConfig)) {
        std::cout << "Victoire de " << currentJ.getName() << std::endl;
    }

    // Passage au joueur suivant
    _currentPlayerTurn++;
    if (_currentPlayerTurn >= (int)_joueurs.size()) {
        _currentPlayerTurn = 0;
        _tourActuel++;
    }
}
