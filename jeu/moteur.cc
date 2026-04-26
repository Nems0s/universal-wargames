#include <moteur.hh>
#include "combat.hh"
#include <iostream>

MoteurDeJeu::MoteurDeJeu() 
    : _tourActuel(1), _currentPlayerTurn(0), _mapSeed(42) {}


// ------------------------------------------- //
// ---------- Init et configuration ---------- //
// ------------------------------------------- //

void MoteurDeJeu::chargerConfiguration(const std::string& configPath) {

    try {
        JsonRessourceReader resReader;
        _ressourceFactory.chargerConfiguration("configs/config_ressources.json", resReader);
        
        _ressourcesDispo.clear();
        for (const auto& [nom, resPtr] : _ressourceFactory.getCatalogue()) {
            _ressourcesDispo[nom] = resPtr.get();
        }

        JsonBatimentReader batReader;
        _batimentFactory.chargerConfiguration("configs/config_batiments.json", batReader, _ressourcesDispo);
        
        JsonCityReader cityReader;
        _cityFactory.chargerConfiguration("configs/config_villes.json", cityReader, _ressourcesDispo);

        _logicConfig.loadRules(configPath);
        _logicConfig.loadWins("configs/config_wins.json");
        
        JsonUniteReader uniteReader;
        _uniteFactory.chargerConfiguration("configs/config_unite.json", uniteReader, _ressourcesDispo);
        
        JsonWorldReader worldReader;
        _worldFactory.initialiserBords();
        worldReader.chargerConfig("configs/config_espace.json", _ressourcesDispo, _worldFactory);
    
    } catch (const std::exception& e) {
        std::cerr << "Erreur de chargement du moteur : " << e.what() << std::endl;
    }
}

void MoteurDeJeu::initGame(int seed, const std::vector<std::string>& noms, const std::vector<std::string>& factions) {
    _mapSeed = seed;
    std::srand(_mapSeed);

    _plateau = std::make_unique<board>(_worldFactory, _logicConfig);
    _joueurs.clear();

    int nbJoueurs = noms.size();

    for (int i = 0; i < nbJoueurs; ++i) {
        Joueur j;
        j.setName(noms[i]);
        if (i < (int)factions.size()) j.setFaction(_logicConfig.getFaction(factions[i]));
        j.initBrouillard(_plateau->getRows(), _plateau->getCols());

        for (const auto& [resName, qt] : _logicConfig.getRessourcesDepart()) {
            if (_ressourcesDispo.count(resName)) j.ajouterRessource(_ressourcesDispo.at(resName), qt);
        }

        _joueurs.push_back(j);
    }

    std::string nomCapitale = "";
    for (const auto& [nom, cityModele] : _cityFactory.getCatalogue()) {
        if (cityModele->estCapitale()) {
            nomCapitale = nom;
            break;
        }
    }

    for (int i = 0; i < nbJoueurs; ++i) {
        Joueur& joueur = _joueurs[i];

        bool placed = false;
        int attempts = 0;
        while (!placed && attempts < 1000) {
            int rx = std::rand() % _plateau->getRows();
            int ry = std::rand() % _plateau->getCols();
            if (_arbitre.buildCity(joueur, *_plateau, rx, ry)) {
                hexa* cell = const_cast<hexa*>(_plateau->getCell(rx, ry));
                TuileConfigurable* tc = dynamic_cast<TuileConfigurable*>(cell);
                if (tc && tc->getStat("spawn_capitale") > 0.0f && !nomCapitale.empty()) {
                    tc->placerVille(_cityFactory.create(nomCapitale, rx, ry));
                    joueur.ajouterVille(tc->getCity());
                    tc->setProprietaire(&joueur);
                    placed = true;
                }
            }
            attempts++;
        }
    }

    for (int i = 0; i < nbJoueurs; ++i) {
        actualiserVisibiliteJoueur(i);
    }

    _tourActuel = 1;
    _currentPlayerTurn = 0;
}

void MoteurDeJeu::passerTour() {
    Joueur& currentJ = _joueurs[_currentPlayerTurn];
    for (City* v : currentJ.getCities()) {
        v->product(currentJ);
        for (const auto& [resPtr, qty] : v->getProduits()) {
            currentJ.ajouterRessource(resPtr, qty);
        }
    }
    if (!currentJ.getCities().empty()) {
        for (const auto& [resName, qty] : _logicConfig.getProductionCapitale()) {
            if (_ressourcesDispo.count(resName)) {
                currentJ.ajouterRessource(_ressourcesDispo.at(resName), qty);
            }
        }
    }

    std::map<const Ressource*, int> factureTotale;
    for (Unite* u : currentJ.getUnites()) {
        if (!u) continue;
        auto coutU = u->getCoutEntretien();
        
        if (coutU.empty()) {
            for (auto const& [nomRes, qte] : _logicConfig.getEntretienCoutDefaut()) {
                const Ressource* r = _ressourceFactory.getRessource(nomRes);
                if (r) factureTotale[r] += qte;
            }
        } else {
            for (auto const& [resPtr, qte] : coutU) {
                factureTotale[resPtr] += qte;
            }
        }
    }

    if (!factureTotale.empty()) {
        if (_arbitre.peutPayer(factureTotale, currentJ)) {
            currentJ.payer(factureTotale);
        } else {
            std::map<const Ressource*, int> fondDeTiroir;
            for (auto const& [resPtr, qteDemandee] : factureTotale) {
                auto invIt = currentJ.getInventaire().find(resPtr);
                if (invIt != currentJ.getInventaire().end()) {
                    fondDeTiroir[resPtr] = invIt->second;
                }
            }
            currentJ.payer(fondDeTiroir); 
            
            for (Unite* u : currentJ.getUnites()) {
                if (u) {
                    int degats = std::max(1, (int)(u->health_point_max() * 0.15f));
                    u->setHealth_point(u->health_point() - degats);
                    if (u->health_point() <= 0) u->setHealth_point(1); 
                }
            }
        }
    }

    verifierVictoireGlobale();

    for (Unite* u : currentJ.getUnites()) {
        if (u) u->setPoint_action(u->point_action_max());
    }

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
    j.resetVision();
    int w = _plateau->getRows();
    int h = _plateau->getCols();

    for (Unite* u : j.getUnites()) {
        if (u) j.decouvrirZoneVision(u->location().first, u->location().second, u->visionRange(), u->fov(), w, h, u->regarde(), false);
    }
    for (City* c : j.getCities()) {
        if (c) j.decouvrirZoneVision(c->getX(), c->getY(), c->getVisionRange(), 360, w, h, direction::est, true);
    }
}

void MoteurDeJeu::chargerPartieDepuisJson(const nlohmann::json& j, const std::vector<std::string>& factions) {
    
    // 1. Initialiser le plateau vide
    std::map<char, int> customWeights;
    if (j.contains("custom_weights")) {
        for (auto& el : j["custom_weights"].items()) {
            customWeights[el.key()[0]] = el.value();
        }
    }
    _worldFactory.overrideWeights(customWeights);

    std::vector<std::string> loadedNames;
    for (size_t i = 0; i < j["players"].size(); ++i) loadedNames.push_back(j["players"][i]["name"]);
    
    initGame(j["game_state"]["seed"], loadedNames, factions);
    
    _tourActuel = j["game_state"]["turn"];
    _currentPlayerTurn = j["game_state"]["current_player"];

    // 2. Restaurer les conditions de victoire
    if (j["game_state"].contains("victory_set")) {
        _logicConfig.setActiveVictorySet(j["game_state"]["victory_set"]);
    }

    // 3. Restaurer les Joueurs, Villes et Unités
    for (size_t i = 0; i < j["players"].size(); ++i) {
        Joueur& joueurActuel = _joueurs[i];
        
        // A. Brouillard (Restauration des deux états)
        joueurActuel.setDecouvert(j["players"][i]["decouvert"].get<std::vector<std::vector<bool>>>());
        joueurActuel.setVisible(j["players"][i]["visible"].get<std::vector<std::vector<bool>>>());
        
        // B. Reconstruire les Villes
        for (const auto& cj : j["players"][i]["villes"]) {
            int x = cj["x"];
            int y = cj["y"];
            bool estCapitale = cj["capitale"];
            std::string typeVille = estCapitale ? "Capitale" : "Colonie";
            
            hexa* cell = const_cast<hexa*>(_plateau->getCell(x, y));
            TuileConfigurable* tc = dynamic_cast<TuileConfigurable*>(cell);
            
            if (tc) {

                auto nvVille = _cityFactory.create(typeVille, x, y);
                
                if (nvVille) {
                    int targetLevel = cj["level"];
                    while (nvVille->getLevel() < targetLevel) { nvVille->upgrade(); }
                    
                    float pvSauvegardes = cj["pv"];
                    float pvMax = nvVille->getPvMax();
                    if (pvSauvegardes < pvMax) { nvVille->takeDamage(pvMax - pvSauvegardes); }
                    
                    for (const auto& batName : cj["batiments"]) {
                        auto b = _batimentFactory.create(batName.get<std::string>());
                        if (b) { nvVille->creeBatiment(std::move(b)); }
                    }
                    
                    tc->placerVille(std::move(nvVille));
                    joueurActuel.ajouterVille(tc->getCity());
                    tc->setProprietaire(&joueurActuel);
                }
            }
        }

        // C. Reconstruire les Unités
        if (j["players"][i].contains("unites")) {
            for (const auto& uj : j["players"][i]["unites"]) {
                int x = uj["x"];
                int y = uj["y"];
                std::string name = uj["name"];
                
                auto u = _uniteFactory.create(name);
                
                if (u) {
                    u->setHealth_point(uj["hp"]);
                    u->setPoint_action(uj["pa"]); 
                    u->setLocation({x, y});
                    if (uj.contains("dir")) u->setRegarde(static_cast<direction>(uj["dir"]));
                    
                    Unite* ptrUnite = u.get();
                    ptrUnite->setProprietaire(&joueurActuel);
                    joueurActuel.ajouterUnite(ptrUnite);
                    
                    _plateau->placerUnite(x, y, std::move(u));
                }
            }
        }

        // D. Reconstruire les inventaires
        if (j["players"][i].contains("inventaire")) {
            for (auto& el : j["players"][i]["inventaire"].items()) {
                const std::string& nomRessource = el.key();
                int quantite = el.value();
                
                const Ressource* resPtr = _ressourceFactory.getRessource(nomRessource);
                
                if (resPtr) {
                    joueurActuel.ajouterRessource(resPtr, quantite);
                }
            }
        }
    }
}

// ------------------------------------------- //
// ---------- Commandes (variant) ------------ //
// ------------------------------------------- //

ResultatAction MoteurDeJeu::soumettreCommande(int pIdx, const CommandeJeu& commande) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return ResultatAction::ECHEC_ARBITRE_REFUS;

    // std::visit : lire variant
    ResultatAction res = std::visit([this, pIdx](const auto& cmdSpecifique) {
        return this->executer(pIdx, cmdSpecifique);
    }, commande);

    if (res == ResultatAction::SUCCES) {
        actualiserVisibiliteJoueur(pIdx);
    }

    verifierVictoireGlobale();

    return res;
}


// -------------------------------------------------- //
// ------- Execution des commandes de variant ------- //
// -------------------------------------------------- //

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdDetruireUnite& cmd) {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return ResultatAction::ECHEC_ARBITRE_REFUS;
    
    Joueur& joueur = _joueurs[pIdx];
    Unite* u = _plateau->getUnite(cmd.x, cmd.y);
    
    // Vérifier que l'unité existe et appartient au joueur
    if (!u || getProprietaireUnite(cmd.x, cmd.y) != pIdx) {
        return ResultatAction::ECHEC_ARBITRE_REFUS;
    }

    // Retirer l'unité du joueur et du plateau
    joueur.perdreUnite(u);
    _plateau->retirerUnite(cmd.x, cmd.y);
    
    return ResultatAction::SUCCES;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdFinTour& cmd) {
    passerTour();
    return ResultatAction::FIN_TOUR;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdFonderVille& cmd) {
    Joueur& j = _joueurs[pIdx];
    hexa* cell = const_cast<hexa*>(_plateau->getCell(cmd.x, cmd.y));
    TuileConfigurable* tc = dynamic_cast<TuileConfigurable*>(cell);
    
    if (!tc || tc->getCity()) return ResultatAction::ECHEC_COORD_INVALIDE;
    if (!_arbitre.buildCity(j, *_plateau, cmd.x, cmd.y)) return ResultatAction::ECHEC_ARBITRE_REFUS;

    auto it = _cityFactory.getCatalogue().find(cmd.nomVille);
    if (it == _cityFactory.getCatalogue().end()) return ResultatAction::ECHEC_ARBITRE_REFUS;
    auto modeleVille = it->second;

    if (modeleVille->estCapitale() && j.getNbVilles() > 0) return ResultatAction::ECHEC_ARBITRE_REFUS;

    auto coutVille = getCoutFondationVille(pIdx, cmd.nomVille);
    if (!_arbitre.peutPayer(coutVille, j)) return ResultatAction::ECHEC_FONDS_INSUFFISANTS;
    
    j.payer(coutVille);

    tc->placerVille(_cityFactory.create(cmd.nomVille, cmd.x, cmd.y));
    j.ajouterVille(tc->getCity());

    tc->setProprietaire(&j);

    return ResultatAction::SUCCES;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdAcheterCase& cmd) {
    Joueur& j = _joueurs[pIdx];
    if (!_arbitre.peutAcheterCase(j, cmd.x, cmd.y, *_plateau, _logicConfig)) return ResultatAction::ECHEC_ARBITRE_REFUS;

    auto cout = _arbitre.getCostAchatCase(j, _logicConfig);
    if (!_arbitre.peutPayer(cout, j)) return ResultatAction::ECHEC_FONDS_INSUFFISANTS;

    j.payer(cout);
    j.incNbCasesAchetees();

    hexa* cell = const_cast<hexa*>(_plateau->getCell(cmd.x, cmd.y));
    TuileConfigurable* tc = dynamic_cast<TuileConfigurable*>(cell);
    if (tc) tc->setProprietaire(&j);
    return ResultatAction::SUCCES;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdConstruction& cmd) {
    auto nvBat = _batimentFactory.create(cmd.nomBatiment);
    if (!nvBat) return ResultatAction::ECHEC_ARBITRE_REFUS;

    if (_arbitre.tenterConstruction(cmd.x, cmd.y, std::move(nvBat), _joueurs[pIdx], *_plateau)) {
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdAmeliorer& cmd) {
    Joueur& j = _joueurs[pIdx];
    const hexa* cell = _plateau->getCell(cmd.x, cmd.y);
    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(cell);
    
    if (!tc || !tc->getCity() || !_arbitre.peutAmeliorerVille(j, *tc->getCity(), _logicConfig)) 
        return ResultatAction::ECHEC_ARBITRE_REFUS;

    std::map<const Ressource*, int> coutAmelioration;
    for (const auto& [nomRes, qte] : _logicConfig.getCoutBaseVille()) {
        for (const auto& [resPtr, invQte] : j.getInventaire()) {
            if (resPtr->getName() == nomRes) coutAmelioration[resPtr] = qte * tc->getCity()->getLevel(); 
        }
    }
    
    if (!_arbitre.peutPayer(coutAmelioration, j)) return ResultatAction::ECHEC_FONDS_INSUFFISANTS;

    j.payer(coutAmelioration);
    tc->getCity()->upgrade();
    return ResultatAction::SUCCES;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdDeplacement& cmd) {
    Joueur& j = _joueurs[pIdx];
    Unite* u = _plateau->getUnite(cmd.xSrc, cmd.ySrc);
    
    if (!u || !_arbitre.appartientJoueur(j, *u)) return ResultatAction::ECHEC_COORD_INVALIDE;
    if (!_arbitre.moveUnite(j, *_plateau, *u, cmd.xDest, cmd.yDest)) return ResultatAction::ECHEC_ARBITRE_REFUS;

    const hexa* tile = _plateau->getCell(cmd.xDest, cmd.yDest);
    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(tile);
    int coutPA = tc ? tc->getCoutDeplacement() : 1;

    if (u->point_action() < coutPA) return ResultatAction::ECHEC_PA_INSUFFISANTS;

    if (_plateau->deplacerUnite(*u, cmd.xDest, cmd.yDest)) {
        u->setLocation({cmd.xDest, cmd.yDest});
        u->setPoint_action(u->point_action() - coutPA);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_COORD_INVALIDE;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdRotation& cmd) {
    Unite* u = _plateau->getUnite(cmd.x, cmd.y);
    if (!u || !_arbitre.appartientJoueur(_joueurs[pIdx], *u)) return ResultatAction::ECHEC_COORD_INVALIDE;

    int coutRot = _logicConfig.getCoutRotation();
    if (u->point_action() < coutRot) return ResultatAction::ECHEC_PA_INSUFFISANTS;

    u->setRegarde(cmd.dir);
    u->setPoint_action(u->point_action() - coutRot);
    return ResultatAction::SUCCES;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdAttaque& cmd) {
    Joueur& j = _joueurs[pIdx];
    Unite* att = _plateau->getUnite(cmd.xSrc, cmd.ySrc);
    Unite* def = _plateau->getUnite(cmd.xDest, cmd.yDest);
    
    if (!att || !def || !_arbitre.appartientJoueur(j, *att)) return ResultatAction::ECHEC_ARBITRE_REFUS;
    if (att->point_action() <= 0) return ResultatAction::ECHEC_PA_INSUFFISANTS;

    for (CompAtt* attComp : att->Offensive()) {
        if (_arbitre.peutAttaquer(j, *att, *def, attComp)) {
            att->setPoint_action(att->point_action() - 1);
            if (Combat::fight(*att, attComp, *def)) {
                if (def->health_point() <= 0) {
                    if (def->getProprietaire()) def->getProprietaire()->perdreUnite(def);
                    _plateau->retirerUnite(cmd.xDest, cmd.yDest);
                }
                return ResultatAction::SUCCES;
            }
        }
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdRecrutement& cmd) {
    Joueur& j = _joueurs[pIdx];

    int capaciteMax = _logicConfig.getCapaciteBase();
    for (City* c : j.getCities()) {
        if (c) capaciteMax += c->getLevel() * _logicConfig.getCapaciteVilleNiveau();
    }
    
    if ((int)j.getUnites().size() >= capaciteMax) {
        return ResultatAction::ECHEC_ARBITRE_REFUS;
    }

    auto unite = _uniteFactory.create(cmd.nomUnite);
    if (!unite) return ResultatAction::ECHEC_ARBITRE_REFUS;
    if (_plateau->getUnite(cmd.x, cmd.y) != nullptr) return ResultatAction::ECHEC_COORD_INVALIDE;

    if (!_arbitre.peutRecruterUnite(j, unite->cout(), *unite)) return ResultatAction::ECHEC_FONDS_INSUFFISANTS;

    j.payer(unite->cout());
    unite->setLocation({cmd.x, cmd.y});

    Unite* raw = unite.get();
    raw->setProprietaire(&j);
    _plateau->placerUnite(cmd.x, cmd.y, std::move(unite));
    j.ajouterUnite(raw);
    return ResultatAction::SUCCES;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdSoigner& cmd) {
    Joueur& j = _joueurs[pIdx];
    Unite* healer = _plateau->getUnite(cmd.xSrc, cmd.ySrc);
    Unite* cible = _plateau->getUnite(cmd.xDest, cmd.yDest);

    if (!healer || !cible || !_arbitre.appartientJoueur(j, *healer)) return ResultatAction::ECHEC_ARBITRE_REFUS;
    if (healer->point_action() <= 0) return ResultatAction::ECHEC_PA_INSUFFISANTS;

    for (CompSoin* soinComp : healer->Soin()) {
        if (_arbitre.peutSoigner(j, *healer, *cible, soinComp)) {
            
            int soin = soinComp->healing_point(); 
            cible->setHealth_point(std::min(cible->health_point_max(), cible->health_point() + soin));
            
            healer->setPoint_action(healer->point_action() - 1);
            return ResultatAction::SUCCES;
        }
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdCamoufler& cmd) {
    Joueur& j = _joueurs[pIdx];
    Unite* u = _plateau->getUnite(cmd.x, cmd.y);

    if (!u || !_arbitre.appartientJoueur(j, *u)) return ResultatAction::ECHEC_ARBITRE_REFUS;
    if (u->point_action() <= 0) return ResultatAction::ECHEC_PA_INSUFFISANTS;

    if (_arbitre.peutActiverCamouflage(j, *u)) {
        CompFurtif* furtif = u->Cammouflage();
        if (furtif) {
            
            furtif->ActiveCammouflage(); 
            
            u->setPoint_action(u->point_action() - 1);
            return ResultatAction::SUCCES;
        }
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdCharger& cmd) {
    Joueur& j = _joueurs[pIdx];
    Unite* passager = _plateau->getUnite(cmd.xPassager, cmd.yPassager);
    Unite* transport = _plateau->getUnite(cmd.xTransport, cmd.yTransport);

    if (!passager || !transport || !_arbitre.appartientJoueur(j, *transport) || !_arbitre.appartientJoueur(j, *passager)) 
        return ResultatAction::ECHEC_ARBITRE_REFUS;
    
    if (transport->point_action() <= 0 || passager->point_action() <= 0) 
        return ResultatAction::ECHEC_PA_INSUFFISANTS;

    if (_arbitre.peutTransporter(j, *transport)) {
        
        std::shared_ptr<Unite> ptrPassager = _plateau->extraireUnite(cmd.xPassager, cmd.yPassager);
        transport->Transport()->MonterUnite(*transport, ptrPassager); 
        
        passager->setPoint_action(passager->point_action() - 1);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdDecharger& cmd) {
    Joueur& j = _joueurs[pIdx];
    Unite* transport = _plateau->getUnite(cmd.xTransport, cmd.yTransport);

    if (!transport || !_arbitre.appartientJoueur(j, *transport)) return ResultatAction::ECHEC_ARBITRE_REFUS;
    if (transport->point_action() <= 0) return ResultatAction::ECHEC_PA_INSUFFISANTS;
    if (cmd.xDest < 0 || cmd.xDest >= _plateau->getRows() || cmd.yDest < 0 || cmd.yDest >= _plateau->getCols()) return ResultatAction::ECHEC_COORD_INVALIDE;

    if (_plateau->getUnite(cmd.xDest, cmd.yDest) == nullptr) {
         
         auto liste = transport->Transport()->liste_unite_transporter();
         if (cmd.indexPassager >= 0 && cmd.indexPassager < liste.size()) {
             auto it = std::next(liste.begin(), cmd.indexPassager);
             std::shared_ptr<Unite> passager = *it;

             if (transport->Transport()->DescenteUniteUnite(*transport, passager)) {
                 passager->setLocation({cmd.xDest, cmd.yDest});
                 _plateau->placerUnite(cmd.xDest, cmd.yDest, passager);
                 
                 transport->setPoint_action(transport->point_action() - 1);
                 return ResultatAction::SUCCES;
             }
         }
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction MoteurDeJeu::executer(int pIdx, const CmdEnroler& cmd) {
    Joueur& j = _joueurs[pIdx];
    Unite* com = _plateau->getUnite(cmd.xCommandant, cmd.yCommandant);
    Unite* recrue = _plateau->getUnite(cmd.xRecrue, cmd.yRecrue);

    if (!com || !recrue || !_arbitre.appartientJoueur(j, *com) || !_arbitre.appartientJoueur(j, *recrue)) 
        return ResultatAction::ECHEC_ARBITRE_REFUS;
    
    if (com->point_action() <= 0) return ResultatAction::ECHEC_PA_INSUFFISANTS;

    if (_arbitre.peutRejoindreCommandant(j, *com, *recrue)) {
        auto r = com->rank();
        auto comRank = std::dynamic_pointer_cast<Rank_Commandant>(r);
        if (comRank) {
            
            comRank->ajout_unite(recrue->shared_from_this());
            com->setPoint_action(com->point_action() - 1);
            return ResultatAction::SUCCES;
        }
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

// ------------------------------------------- //
// --------------- Etat du jeu --------------- //
// ------------------------------------------- //

bool MoteurDeJeu::peutFonderVille(int joueurIdx, int x, int y) const {
    if (joueurIdx < 0 || joueurIdx >= (int)_joueurs.size()) return false;
    return _arbitre.buildCity(_joueurs[joueurIdx], *_plateau, x, y);
}

bool MoteurDeJeu::peutAmeliorerVille(int joueurIdx, int x, int y) const {
    if (joueurIdx < 0 || joueurIdx >= (int)_joueurs.size()) return false;
    const hexa* cell = _plateau->getCell(x, y);
    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(cell);
    if (tc && tc->getCity()) return _arbitre.peutAmeliorerVille(_joueurs[joueurIdx], *tc->getCity(), _logicConfig);
    return false;
}

bool MoteurDeJeu::peutAcheterTerritoire(int joueurIdx, int x, int y) const {
    if (joueurIdx < 0 || joueurIdx >= (int)_joueurs.size()) return false;
    if (!_arbitre.estDansTerritoire(_joueurs[joueurIdx], x, y, *_plateau, _logicConfig)) {
        return _arbitre.peutAcheterCase(_joueurs[joueurIdx], x, y, *_plateau, _logicConfig);
    }
    return false;
}

bool MoteurDeJeu::estDansTerritoire(int pIdx, int x, int y) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    return _arbitre.estDansTerritoire(_joueurs[pIdx], x, y, *_plateau, _logicConfig);
}

bool MoteurDeJeu::estVilleAuJoueur(int pIdx, int x, int y) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    const hexa* cell = _plateau->getCell(x, y);
    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(cell);
    if (tc && tc->getCity() && tc->getProprietaire() == &_joueurs[pIdx]) {
        return true;
    }
    return false;
}

bool MoteurDeJeu::peutPayer(int pIdx, const std::map<const Ressource*, int>& cout) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return false;
    return _arbitre.peutPayer(cout, _joueurs[pIdx]);
}

std::map<const Ressource*, int> MoteurDeJeu::getCoutFondationVille(int pIdx, const std::string& nomVille) const {
    if (pIdx < 0 || pIdx >= (int)_joueurs.size()) return {};
    
    auto it = _cityFactory.getCatalogue().find(nomVille);
    if (it == _cityFactory.getCatalogue().end()) return {};

    std::map<const Ressource*, int> coutBase = it->second->getCoutBase();
    if (coutBase.empty() && !it->second->estCapitale()) {
        for (const auto& [nomRes, qte] : _logicConfig.getCoutBaseVille()) {
            if (_ressourcesDispo.count(nomRes)) {
                coutBase[_ressourcesDispo.at(nomRes)] = qte;
            }
        }
    }

    return _arbitre.getCostNouvelleVille(_joueurs[pIdx], it->second->getCoutBase());
}

std::map<const Ressource*, int> MoteurDeJeu::getCoutAchatTerritoire(int pIdx) const {
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

std::vector<std::pair<int, int>> MoteurDeJeu::getDeplacementsPossibles(int joueurIdx, int x, int y) const {
    if (joueurIdx < 0 || joueurIdx >= (int)_joueurs.size()) return {};
    Unite* u = _plateau->getUnite(x, y);
    if (u && _arbitre.appartientJoueur(_joueurs[joueurIdx], *u)) {
        return _arbitre.getCasesDeplacementPossibles(*_plateau, *u);
    }
    return {};
}

std::vector<std::pair<int, int>> MoteurDeJeu::getAttaquesPossibles(int joueurIdx, int x, int y) const {
    if (joueurIdx < 0 || joueurIdx >= (int)_joueurs.size()) return {};
    Unite* u = _plateau->getUnite(x, y);
    if (u && _arbitre.appartientJoueur(_joueurs[joueurIdx], *u)) {
        return _arbitre.getCasesAttaquePossibles(_joueurs[joueurIdx], *_plateau, *u);
    }
    return {};
}

bool MoteurDeJeu::peutPivoter(int pIdx, int x, int y) const {
    Unite* u = _plateau->getUnite(x, y);
    if (!u || !_arbitre.appartientJoueur(_joueurs[pIdx], *u)) return false;
    return u->point_action() >= _logicConfig.getCoutRotation();
}

bool MoteurDeJeu::peutAttaquer(int pIdx, int xSrc, int ySrc, int xDest, int yDest) const {
    Unite* att = _plateau->getUnite(xSrc, ySrc);
    Unite* def = _plateau->getUnite(xDest, yDest);
    
    if (!att || !def || !_arbitre.appartientJoueur(_joueurs[pIdx], *att)) return false;
    if (att->point_action() <= 0) return false;

    for (CompAtt* attComp : att->Offensive()) {
        if (_arbitre.peutAttaquer(_joueurs[pIdx], *att, *def, attComp)) return true;
    }
    return false;
}

std::vector<std::pair<int, int>> MoteurDeJeu::getTerritoireJoueur(int pIdx) const {
    std::vector<std::pair<int, int>> territoire;
    if (!_plateau || pIdx < 0 || pIdx >= (int)_joueurs.size()) return territoire;

    for (int i = 0; i < _plateau->getRows(); ++i) {
        for (int y = 0; y < _plateau->getCols(); ++y) {
            if (estDansTerritoire(pIdx, i, y)) {
                territoire.push_back({i, y});
            }
        }
    }
    return territoire;
}

void MoteurDeJeu::verifierVictoireGlobale() {
    if (_partieTerminee) return;

    for (const Joueur& j : _joueurs) {
        if (_arbitre.verifierVictoire(j, _logicConfig)) {
            _partieTerminee = true;
            _nomVainqueur = j.getName();
            std::cout << "VICTOIRE DETECTEE : " << _nomVainqueur << " REMPORTE LA PARTIE" << std::endl;
            break;
        }
    }
}