#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include "imgui.h"
#include "imgui-SFML.h"
#include "NetworkManager.hh"
#include "moteur.hh"

using json = nlohmann::json;

enum class GameState { MENU, PLAY_MENU, MULTI_MENU, HOST_LOBBY, JOIN_LOBBY, FACTION_SELECT, MAP_CONFIG, OPTIONS, IN_GAME };
enum class OptionsTab { GAME_SETTINGS, GRAPHICS, ADVANCED };

class InterfaceManager {
    friend class SaveManager;
private:
    sf::RenderWindow & _window;
    MoteurDeJeu & _moteur;

    GameState _currentState;
    OptionsTab _currentOptionsTab = OptionsTab::GAME_SETTINGS;

    // configuration JSON
    json _configJson;
    json _espaceJson;
    std::string _configPath = "configs/config_rules.json";

    // graphismes
    std::map<char, sf::Texture> _textures;
    float _tileSize = 64.0f;
    sf::View _gameView;
    float _currentZoom = 1.0f;
    bool _isPanning = false;
    sf::Vector2i _lastMousePos;

    // options visuelles
    bool _vsync = false;
    bool _fullscreen = false;
    int _qualityIndex = 1;

    // parametre avant partie
    std::map<char, int> _customWeights;
    int _mapSeed = 42;
    int _numPlayers = 2;
    std::vector<std::string> _playerFactions;

    // réseau
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

    // chat
    std::vector<std::string> _chatMessages;
    char _chatInputBuffer[256] = "";

    // systeme selection et ciblage
    int _selectedCellX = -1;
    int _selectedCellY = -1;
    bool _hasSelection = false;
    
    bool _showPopup = false;
    std::string _popupMsg = "";

    bool _isTargetingMove = false;
    bool _isTargetingAttack = false;
    int _unitSourceX = -1;
    int _unitSourceY = -1;
    std::vector<std::pair<int, int>> _casesPossibles;

public:
    InterfaceManager(sf::RenderWindow& window, MoteurDeJeu & moteur);

    void run();
    
private:

    void loadUIConfig();
    void saveConfig();
    void initGame();
    void loadTextures();
    void applyCustomTheme();
    bool DrawArrowSelector(const char* id, int* current_index, const std::vector<std::string>& items);

    // Menus
    void renderMenu();
    void renderPlayMenu();
    void renderFactionSelect();
    void renderOptions();
    void renderMapConfig();
    void renderGame();

    // Multijoueur
    void renderMultiMenu();
    void renderHostLobby();
    void renderJoinLobby();
    void renderChatWindow();
    void updateNetworkLoop();
    void sendChatMessage(const std::string& msg);

};
