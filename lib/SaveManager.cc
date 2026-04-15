#include "SaveManager.hh"
#include "../UI/InterfaceManager.hh" // Pour accéder aux variables
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool SaveManager::saveGame(const std::string& filename, InterfaceManager* ui) {
    json j;

    // 1. État global de la partie
    j["game_state"]["turn"] = ui->_currentTurnNumber;
    j["game_state"]["current_player"] = ui->_currentPlayerTurn;
    j["game_state"]["num_players"] = ui->_numPlayers;
    j["game_state"]["seed"] = ui->_mapSeed;

    // 2. Paramètres de la carte (Poids et Factions)
    j["factions"] = ui->_playerFactions;
    for (const auto& [symb, weight] : ui->_customWeights) {
        std::string s(1, symb);
        j["custom_weights"][s] = weight;
    }

    // 3. Les Joueurs (Brouillard de guerre)
    for (size_t i = 0; i < ui->_joueurs.size(); ++i) {
        json playerJson;
        playerJson["name"] = ui->_joueurs[i].getName();
        playerJson["brouillard"] = ui->_joueurs[i].getBrouillard(); // Conversion auto en JSON !
        
        // TODO plus tard : Sauvegarder ici les coordonnées des Villes et Unités du joueur
        
        j["players"].push_back(playerJson);
    }

    // 4. Écriture dans le fichier
    std::ofstream file(filename);
    if (file.is_open()) {
        file << j.dump(4); // Indentation de 4 espaces pour être lisible
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

    // 1. Restaurer l'état global
    ui->_currentTurnNumber = j["game_state"]["turn"];
    ui->_currentPlayerTurn = j["game_state"]["current_player"];
    ui->_numPlayers = j["game_state"]["num_players"];
    ui->_mapSeed = j["game_state"]["seed"];

    // 2. Restaurer les paramètres de carte
    ui->_playerFactions = j["factions"].get<std::vector<std::string>>();
    ui->_customWeights.clear();
    for (auto& el : j["custom_weights"].items()) {
        ui->_customWeights[el.key()[0]] = el.value();
    }

    // 3. Régénérer le plateau avec la Seed sauvegardée
    ui->initGameFromSave(); 

    // 4. Restaurer les Joueurs
    for (size_t i = 0; i < j["players"].size(); ++i) {
        // Appliquer le brouillard sauvegardé
        ui->_joueurs[i].setBrouillard(j["players"][i]["brouillard"].get<std::vector<std::vector<bool>>>());
        
        // TODO plus tard : Re-créer les Villes et Unités ici
    }

    std::cout << "Partie chargee avec succes !" << std::endl;
    return true;
}