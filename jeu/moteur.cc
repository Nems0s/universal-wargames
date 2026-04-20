#include "moteur.hh"
#include "combat.hh"
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
        _uniteFactory.chargerConfiguration("configs/config_unite.json", uniteReader, _ressourcesDispo);

        JsonWorldReader worldReader;
        _worldFactory.initialiserBords();
        worldReader.chargerConfig("configs/config_espace.json", _ressourcesDispo, _worldFactory);
    } catch (const std::exception& e) {
        std::cerr << "[MOTEUR] Erreur de chargement : " << e.what() << std::endl;
    }
}

void MoteurDeJeu::initGame(int seed, const std::vector<std::string>& noms, const std::vector<std::string>& factions) {
    _mapSeed = seed;
    std::srand(_mapSeed);

    _plateau = std::make_unique<board>(_worldFactory, _logicConfig);
    _joueurs.clear();

    int nbJoueurs = noms.size();

    // Création et placement
    for (int i = 0; i < nbJoueurs; ++i) {
        Joueur j;
        j.setName(noms[i]);
        if (i < (int)factions.size()) j.setFaction(_logicConfig.getFaction(factions[i]));
        j.initBrouillard(_plateau->getRows(), _plateau->getCols());

        for (const auto& [resName, qt] : _logicConfig.getRessourcesDepart()) {
            if (_ressourcesDispo.count(resName)) j.ajouterRessource(_ressourcesDispo.at(resName), qt);
        }

        bool placed = false;
        int attempts = 0;
        while (!placed && attempts < 1000) {
            int rx = std::rand() % _plateau->getRows();
            int ry = std::rand() % _plateau->getCols();
            if (_arbitre.buildCity(j, *_plateau, rx, ry)) {
                hexa* cell = const_cast<hexa*>(_plateau->getCell(rx, ry));
                TuileConfigurable* tc = dynamic_cast<TuileConfigurable*>(cell);
                if (tc && tc->getStat("spawn_capitale") > 0.0f) {
                    tc->constrVille(rx, ry, _logicConfig, 5, true);
                    j.ajouterVille(tc->getCity());
                    placed = true;
                }
            }
            attempts++;
        }
        
        _joueurs.push_back(j); 
    }

    // calcule de la vision
    for (int i = 0; i < nbJoueurs; ++i) {
        actualiserVisibiliteJoueur(i);
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
        // Calcul du coût de PA par rapport à la tuile cible
        const hexa* tile = _plateau->getCell(xDest, yDest);
        const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(tile);
        int coutPA = tc ? tc->getCoutDeplacement() : 1;

        if (u->point_action() >= coutPA) {
            if (_plateau->deplacerUnite(*u, xDest, yDest)) {
                u->setLocation({xDest, yDest});
                u->setPoint_action(u->point_action() - coutPA);
                actualiserVisibiliteJoueur(pIdx);
                return true;
            }
        }
    }
    return false;
}

bool MoteurDeJeu::demanderRotation(int pIdx, int x, int y, direction d) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    Unite* u = _plateau->getUnite(x, y);
    if (u && _arbitre.appartientJoueur(_joueurs[pIdx], *u)) {
        int coutRot = _logicConfig.getCoutRotation();
        if (u->point_action() >= coutRot) {
            u->setRegarde(d);
            u->setPoint_action(u->point_action() - coutRot);
            actualiserVisibiliteJoueur(pIdx);
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
        Joueur& j = _joueurs[pIdx];
        if (_arbitre.peutAmeliorerVille(j, *tc->getCity(), _logicConfig)) {
            
            std::map<Ressource*, int> coutAmelioration;
            for (const auto& [nomRes, qte] : _logicConfig.getCoutBaseVille()) {
                for (const auto& [resPtr, invQte] : j.getInventaire()) {
                    if (resPtr->getName() == nomRes) {
                        coutAmelioration[resPtr] = qte * tc->getCity()->getLevel(); 
                    }
                }
            }
            j.payer(coutAmelioration);
            
            tc->getCity()->upgrade();
            return true;
        }
    }
    return false;
}

bool MoteurDeJeu::demanderAchatTerritoire(int pIdx, int x, int y) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    Joueur& j = _joueurs[pIdx];

    if (_arbitre.peutAcheterCase(j, x, y, *_plateau, _logicConfig)) {
        std::map<Ressource*, int> cout = _arbitre.getCostAchatCase(j, _logicConfig);
        j.payer(cout);
        
        j.incNbCasesAchetees();

        hexa* cell = const_cast<hexa*>(_plateau->getCell(x, y));
        TuileConfigurable* tc = dynamic_cast<TuileConfigurable*>(cell);
        if (tc) {
            tc->setProprietaire(&j);
            actualiserVisibiliteJoueur(pIdx);
            return true;
        }
    }
    return false;
}

bool MoteurDeJeu::demanderAttaque(int pIdx, int xSrc, int ySrc, int xDest, int yDest) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    Unite* att = _plateau->getUnite(xSrc, ySrc);
    Unite* def = _plateau->getUnite(xDest, yDest);
    
    if (att && def && _arbitre.appartientJoueur(_joueurs[pIdx], *att)) {
        if (att->point_action() <= 0) return false; 

        for (CompAtt* attComp : att->Offensive()) {
            if (_arbitre.peutAttaquer(_joueurs[pIdx], *att, *def, attComp)) {
                
                att->setPoint_action(att->point_action() - 1);

                if (Combat::fight(*att, attComp, *def)) {
                    if (def->health_point() <= 0) {
                        for (auto& joueur : _joueurs) joueur.perdreUnite(def);
                        _plateau->retirerUnite(xDest, yDest);
                    }
                    actualiserVisibiliteJoueur(pIdx);
                    return true;
                }
            }
        }
    }
    return false;
}

bool MoteurDeJeu::demanderFondationVille(int pIdx, int x, int y) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    Joueur& j = _joueurs[pIdx];

    hexa* cell = const_cast<hexa*>(_plateau->getCell(x, y));
    TuileConfigurable* tc = dynamic_cast<TuileConfigurable*>(cell);
    
    if (!tc || tc->getCity() != nullptr) return false;

    if (!_arbitre.buildCity(j, *_plateau, x, y)) return false;

    auto coutVille = _arbitre.getCostNouvelleVille(j, _logicConfig);
    if (!_arbitre.peutPayer(coutVille, j)) return false;
    
    j.payer(coutVille);

    // Construction
    int rayonBase = _logicConfig.getRayonBaseVille();
    tc->constrVille(x, y, _logicConfig, rayonBase, false);
    City* nouvelleVille = tc->getCity();
    j.ajouterVille(nouvelleVille);

    actualiserVisibiliteJoueur(pIdx);

    return true;
}

bool MoteurDeJeu::demanderRecrutementUnite(int pIdx, int x, int y, const std::string& nomUnite) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    Joueur& j = _joueurs[pIdx];

    auto unite = _uniteFactory.create(nomUnite);
    if (!unite) return false;

    if (!_arbitre.peutRecruterUnite(j, unite->cout(), *unite)) return false;
    if (_plateau->getUnite(x, y) != nullptr) return false;

    j.payer(unite->cout());
    unite->setLocation({x, y});

    Unite* raw = unite.get();
    _plateau->placerUnite(x, y, std::move(unite));
    j.ajouterUnite(raw);
    
    actualiserVisibiliteJoueur(pIdx);

    return true;
}

void MoteurDeJeu::passerTour() {
    // Production des ressources
    Joueur& currentJ = _joueurs[_currentPlayerTurn];
    for (City* v : currentJ.getCities()) {
        v->product(currentJ);
    }

    // Production de base par la capitale (via configuration)
    if (!currentJ.getCities().empty()) {
        for (const auto& [resName, qty] : _logicConfig.getProductionCapitale()) {
            if (_ressourcesDispo.count(resName)) {
                currentJ.ajouterRessource(_ressourcesDispo.at(resName), qty);
            }
        }
    }

    // Vérification de la victoire
    if (_arbitre.verifierVictoire(currentJ, _logicConfig)) {
        std::cout << "Victoire de " << currentJ.getName() << std::endl;
    }

    // Réinitialiser les PA de toutes les unités du joueur courant
    for (Unite* u : currentJ.getUnites()) {
        if (u) u->setPoint_action(u->point_action_max());
    }

    // Passage au joueur suivant
    _currentPlayerTurn++;
    if (_currentPlayerTurn >= (int)_joueurs.size()) {
        _currentPlayerTurn = 0;
        _tourActuel++;
    }
    actualiserVisibiliteJoueur(_currentPlayerTurn);
}

void MoteurDeJeu::actualiserVisibiliteJoueur(int pIdx) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return;
    Joueur& j = _joueurs[pIdx];
    
    // 1. On remet tout le "Visible" à false (le "Découvert" reste tel quel)
    j.resetVision();

    int w = _plateau->getRows();
    int h = _plateau->getCols();

    // 2. Vision des unités (cones)
    for (Unite* u : j.getUnites()) {
        if (u) {
            j.decouvrirZoneVision(u->location().first, u->location().second, u->visionRange(), u->fov(), w, h, u->regarde(), false);
        }
    }

    // 3. Vision des villes (cercle)
    for (City* c : j.getCities()) {
        if (c) {
            j.decouvrirZoneVision(c->getX(), c->getY(), c->getVisionRange(), 360, w, h, direction::est, true);
        }
    }
}

bool MoteurDeJeu::estDansTerritoire(int pIdx, int x, int y) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    return _arbitre.estDansTerritoire(_joueurs[pIdx], x, y, *_plateau, _logicConfig);
}

bool MoteurDeJeu::estVilleAuJoueur(int pIdx, int x, int y) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    const hexa* cell = _plateau->getCell(x, y);
    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(cell);
    if (!tc || !tc->getCity()) return false;
    
    for (City* v : _joueurs[pIdx].getCities()) {
        if (v == tc->getCity()) return true;
    }
    return false;
}

bool MoteurDeJeu::peutPayer(int pIdx, const std::map<Ressource*, int>& cout) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    return _arbitre.peutPayer(cout, _joueurs[pIdx]);
}

std::map<Ressource*, int> MoteurDeJeu::getCoutFondationVille(int pIdx) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return {};
    return _arbitre.getCostNouvelleVille(_joueurs[pIdx], _logicConfig);
}

std::map<Ressource*, int> MoteurDeJeu::getCoutAchatTerritoire(int pIdx) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return {};
    return _arbitre.getCostAchatCase(_joueurs[pIdx], _logicConfig);
}

bool MoteurDeJeu::peutRecruterUnite(int pIdx, const std::string& nomUnite) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    auto unite = _uniteFactory.create(nomUnite);
    if (!unite) return false;
    return _arbitre.peutRecruterUnite(_joueurs[pIdx], unite->cout(), *unite);
}

int MoteurDeJeu::getProprietaireUnite(int x, int y) const {
    Unite* u = _plateau->getUnite(x, y);
    if (!u) return -1;
    for (size_t i = 0; i < _joueurs.size(); ++i) {
        if (_arbitre.appartientJoueur(_joueurs[i], *u)) return i;
    }
    return -1;
}