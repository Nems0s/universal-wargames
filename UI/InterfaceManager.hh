#pragma once
#ifndef INTERFACE_MANAGER_HH
#define INTERFACE_MANAGER_HH

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include "imgui.h"
#include "imgui-SFML.h"
#include "jeu.hh"
#include "config.hh"
#include "SaveManager.hh"
#include "NetworkManager.hh"

using json = nlohmann::json;

enum class GameState { MENU, PLAY_MENU, MULTI_MENU, HOST_LOBBY, JOIN_LOBBY, FACTION_SELECT, MAP_CONFIG, OPTIONS, IN_GAME };
enum class OptionsTab { GAME_SETTINGS, GRAPHICS, ADVANCED };

class InterfaceManager {
    friend class SaveManager;
private:
    sf::RenderWindow& _window;
    GameState _currentState;
    OptionsTab _currentOptionsTab = OptionsTab::GAME_SETTINGS;
    std::vector<Joueur> _joueurs;

    // Gestion de la configuration JSON
    json _configJson; // On n'utilise plus qu'un seul objet JSON
    json _espaceJson;
    std::string _configPath = "configs/config_rules.json";
    GameConfig _logicConfig; // Ta classe de logique
    
    // Logique du jeu
    std::unique_ptr<board> _board;
    WorldFactory _factory;
    
    // Paramètres d'initialisation
    int _gridSize = 10;
    
    // Graphismes
    std::map<char, sf::Texture> _textures;
    float _tileSize = 64.0f;

    // factions
    std::string _selectedFaction = "";

    // Options
    bool _vsync = false;
    bool _fullscreen = false;
    int _qualityIndex = 1;
    bool DrawArrowSelector(const char* id, int* current_index, const std::vector<std::string>& items);

    // Opions de carte
    std::map<char, int> _customWeights;
    int _mapSeed = 42;

    // joueurs
    int _numPlayers = 2;
    std::vector<std::string> _playerFactions;

    // --- État de la Partie ---
    int _currentPlayerTurn = 0; // Index du joueur dont c'est le tour
    int _currentTurnNumber = 1; // Numéro du tour global
    
    // --- Sélection sur la carte ---
    int _selectedCellX = -1;
    int _selectedCellY = -1;
    bool _hasSelection = false;

public:
    InterfaceManager(sf::RenderWindow& window);
    
    void loadConfig();
    void saveConfig();
    void run(); // Boucle principale
    
private:
    sf::View _gameView;
    float _currentZoom = 1.0f;
    bool _isPanning = false;
    sf::Vector2i _lastMousePos;

    void renderMenu();
    void renderPlayMenu();
    void renderFactionSelect();
    void renderOptions();
    void renderMapConfig();
    void renderSetup();
    void renderGame();
    
    void initGame();
    void loadTextures();
    void applyCustomTheme();

    void initGameFromSave();

    // Multijoueur
    NetworkManager _network;
    char _ipBuffer[64] = "127.0.0.1";
    int _maxPlayersBuffer = 4;
    bool _hasSentName = false;
    int _portBuffer = 5000;

    char _playerNameBuffer[64] = "NomGenerique1";
    int _localPlayerIndex = 0;
    
    struct NetPlayer { 
        std::string name; 
        std::string faction; 
    };
    std::vector<NetPlayer> _connectedPlayers;
    
    void updateNetworkLoop();

    void renderMultiMenu();
    void renderHostLobby();
    void renderJoinLobby();

    // Système de Chat
    std::vector<std::string> _chatMessages; // Historique des messages
    char _chatInputBuffer[256] = "";        // Texte en cours de saisie
    
    void sendChatMessage(const std::string& msg); // Fonction d'envoi
    void renderChatWindow();                      // Fenêtre UI du chat
};

#endif