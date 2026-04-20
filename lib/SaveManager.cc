#include "SaveManager.hh"
#include "../UI/InterfaceManager.hh"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool SaveManager::saveGame(const std::string& filename, InterfaceManager* ui) {
    json j;

    MoteurDeJeu & moteur = ui->_moteur;

    // 1. État global de la partie
    j["game_state"]["turn"] = moteur.getTourActuel();
    j["game_state"]["current_player"] = moteur.getCurrentPlayerTurn();
    j["game_state"]["num_players"] = ui->_numPlayers;
    j["game_state"]["seed"] = moteur.getMapSeed();

    // 2. Paramètres de la carte (Poids et Factions)
    j["factions"] = ui->_playerFactions;
    for (const auto& [symb, weight] : ui->_customWeights) {
        std::string s(1, symb);
        j["custom_weights"][s] = weight;
    }

    // 3. Les Joueurs (Brouillard, Villes, Unités)
    const auto& joueurs = moteur.getJoueurs();
    for (size_t i = 0; i < joueurs.size(); ++i) {
        json playerJson;
        playerJson["name"] = joueurs[i].getName();
        playerJson["decouvert"] = joueurs[i].getDecouvert(); 
        playerJson["visible"] = joueurs[i].getVisible();
        
        // --- SAUVEGARDE DES VILLES ---
        json villesJson = json::array();
        for (City* c : joueurs[i].getCities()) {
            json cj;
            cj["x"] = c->getX();
            cj["y"] = c->getY();
            cj["level"] = c->getLevel();
            cj["pv"] = c->getPv();
            cj["capitale"] = c->estCapitale();
            
            // Sauvegarde des bâtiments dans la ville
            json batJson = json::array();
            for (const auto& b : c->getBatiments()) { 
                batJson.push_back(b->getName());
            }
            cj["batiments"] = batJson;
            
            villesJson.push_back(cj);
        }
        playerJson["villes"] = villesJson;

        // --- SAUVEGARDE DES UNITÉS ---
        json unitesJson = json::array();
        for (Unite* u : joueurs[i].getUnites()) {
            json uj;
            uj["x"] = u->location().first;
            uj["y"] = u->location().second;
            uj["name"] = u->name();
            uj["hp"] = u->health_point();
            uj["pa"] = u->point_action();
            uj["dir"] = static_cast<int>(u->regarde());
            unitesJson.push_back(uj);
        }
        playerJson["unites"] = unitesJson;
        
        j["players"].push_back(playerJson);
    }

    // 4. Écriture dans le fichier
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4);
        std::cout << "Partie sauvegardee : " << filename << std::endl;
        return true;
    }
    return false;
}


bool SaveManager::loadGame(const std::string& filename, InterfaceManager* ui) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur: Fichier de sauvegarde introuvable." << std::endl;
        return false;
    }

    json j;
    file >> j;
    MoteurDeJeu& moteur = ui->_moteur;

    // 1. Restaurer l'état global
    ui->_numPlayers = j["game_state"]["num_players"];
    ui->_mapSeed = j["game_state"]["seed"];
    ui->_playerFactions = j["factions"].get<std::vector<std::string>>();
    ui->_customWeights.clear();
    for (auto& el : j["custom_weights"].items()) {
        ui->_customWeights[el.key()[0]] = el.value();
    }

    // 2. Restaurer les paramètres du plateau
    moteur.overrideWorldWeights(ui->_customWeights);
    std::vector<std::string> loadedNames;
    for (size_t i = 0; i < j["players"].size(); ++i) {
        loadedNames.push_back(j["players"][i]["name"]);
    }
    moteur.initGame(ui->_mapSeed, loadedNames, ui->_playerFactions);

    // 3. Restaurer le temps de la sauvegarde
    moteur.setTourActuel(j["game_state"]["turn"]);
    moteur.setCurrentPlayerTurn(j["game_state"]["current_player"]);

    // 4. Restaurer les Joueurs, Villes et Unités
    for (size_t i = 0; i < j["players"].size(); ++i) {
        Joueur& joueurActuel = moteur.getJoueurMutable(i);
        
        // A. Brouillard (Restauration des deux états)
        joueurActuel.setDecouvert(j["players"][i]["decouvert"].get<std::vector<std::vector<bool>>>());
        joueurActuel.setVisible(j["players"][i]["visible"].get<std::vector<std::vector<bool>>>());
        
        // B. Reconstruire les Villes
        for (const auto& cj : j["players"][i]["villes"]) {
            int x = cj["x"];
            int y = cj["y"];
            
            // On récupère la tuile pour forcer la construction
            hexa* cell = const_cast<hexa*>(moteur.getPlateau()->getCell(x, y));
            TuileConfigurable* tc = dynamic_cast<TuileConfigurable*>(cell);
            
            if (tc) {
                // On construit gratuitement
                tc->constrVille(x, y, moteur.getLogicConfig(), 5, cj["capitale"]);
                City* city = tc->getCity();
                
                // On restaure les statistiques exactes
                city->setLevel(cj["level"]);
                city->setPv(cj["pv"]);
                
                // On recrée les bâtiments internes
                for (const auto& batName : cj["batiments"]) {
                    auto b = moteur.getBatimentFactory().create(batName.get<std::string>());
                    if (b) {
                        city->creeBatiment(std::move(b));
                    }
                }
                
                // On lie la ville au joueur et au territoire
                joueurActuel.ajouterVille(city);
                tc->setProprietaire(&joueurActuel);
            }
        }

        // C. Reconstruire les Unités
        if (j["players"][i].contains("unites")) {
            for (const auto& uj : j["players"][i]["unites"]) {
                int x = uj["x"];
                int y = uj["y"];
                std::string name = uj["name"];
                
                // 1. On recrée la bonne unité dynamiquement via la Factory
                auto u = moteur.getUniteFactory().create(name);
                
                if (u) {
                    // 2. On restaure ses statistiques exactes
                    u->setHealth_point(uj["hp"]);
                    u->setPoint_action(uj["pa"]); 
                    u->setLocation({x, y});
                    if (uj.contains("dir")) u->setRegarde(static_cast<direction>(uj["dir"]));
                    
                    // 3. On extrait le pointeur brut pour l'inventaire du joueur
                    Unite* ptrUnite = u.get();
                    joueurActuel.ajouterUnite(ptrUnite);
                    
                    // 4. Le plateau prend possession de l'unité aux bonnes coordonnées
                    moteur.getPlateauMutable()->placerUnite(x, y, std::move(u));
                }
            }
        }
    }

    std::cout << "Partie chargee avec succes !" << std::endl;
    return true;
}