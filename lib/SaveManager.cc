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
    if (!file.is_open()) return false;

    json j;
    file >> j;
    
    ui->_numPlayers = j["game_state"]["num_players"];
    ui->_mapSeed = j["game_state"]["seed"];
    ui->_playerFactions = j["factions"].get<std::vector<std::string>>();
    ui->_customWeights.clear();
    for (auto& el : j["custom_weights"].items()) {
        ui->_customWeights[el.key()[0]] = el.value();
    }

    ui->_moteur.chargerPartieDepuisJson(j, ui->_playerFactions);

    return true;
}
