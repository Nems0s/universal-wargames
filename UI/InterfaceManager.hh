#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <set>

#include "imgui.h"
#include "imgui-SFML.h"
#include "NetworkManager.hh"
#include "moteur.hh"
#include "SaveManager.hh"

using json = nlohmann::json;

enum class GameState { MENU, PLAY_MENU, MULTI_MENU, LOAD_MENU, HOST_LOBBY, JOIN_LOBBY, FACTION_SELECT, MAP_CONFIG, OPTIONS, IN_GAME };
enum class OptionsTab { GAME_SETTINGS, GRAPHICS, ADVANCED };

class InterfaceManager {
    friend class SaveManager;
private:
    sf::RenderWindow & _window;
    MoteurDeJeu & _moteur;

    GameState _currentState;
    GameState _previousState = GameState::MENU;
    OptionsTab _currentOptionsTab = OptionsTab::GAME_SETTINGS;
    bool _gameConfigLocked = false;

    // configuration JSON
    GameConfigFiles _paths;
    
    json _rulesJson;
    json _tuilesJson;
    json _villesJson;

    // graphismes
    std::map<char, sf::Texture> _textures; // Terrain
    std::map<std::string, sf::Texture> _unitTextures; // Unités
    std::map<std::string, sf::Texture> _buildingTextures; // Batiments
    std::map<std::string, sf::Texture> _cityTextures; // Villes
    std::map<std::string, sf::Texture> _resourceIcons; // Ressources
    std::map<std::string, sf::Texture> _factionTextures; // Factions

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
    int _selectedVictoryIndex = 0;
    std::vector<std::string> _playerFactions;

    // réseau
    NetworkManager _network;
    char _ipBuffer[64] = "127.0.0.1";
    int _maxPlayersBuffer = 4;
    bool _hasSentName = false;
    int _portBuffer = 5000;
    char _playerNameBuffer[64] = "NomGenerique1";
    int _localPlayerIndex = 0;
    void sendLobbySync();

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
    bool _isTargetingHeal = false;
    bool _isTargetingLoad = false;  
    bool _isTargetingEnrol = false;
    int _actionSubIndex = 0;
    int _unitSourceX = -1;
    int _unitSourceY = -1;
    std::vector<std::pair<int, int>> _casesPossibles;
    std::vector<std::pair<int, int>> _casesAttaquePossibles;

    bool _wasMyTurn = true;
    float _turnNotificationTimer = 0.0f;

    // Drag and Drop
    bool _isDragging = false;
    int _dragSourceX = -1;
    int _dragSourceY = -1;

    // Rotation et Apercu
    bool _hasPreviewRotation = false;
    direction _previewDirection = direction::est;
    std::string _activePresetName = "";

    // Système de log de combat
    struct CombatLogEntry {
        std::string message;
        sf::Color color;
        float timer;
    };
    std::vector<CombatLogEntry> _combatLogs;
    void addCombatLog(const std::string& msg, sf::Color col = sf::Color::White);

    // Gestion écran de fin
    bool _gameOver = false;
    bool _gameContinuesAfterWin = false;
    std::string _winnerName = "";
    bool _showSummaryPopup = false;

    // Gestion des saves
    std::vector<std::string> _saveFiles;
    char _newSaveName[64] = "MaPartie";
    char _renameBuffer[64] = "";
    std::string _selectedSave = "";
    bool _isHostingLoadedSave = false;
    bool _loadAsMultiplayer = false;
    std::vector<int> _playerToSlotMapping;

    // Cache de performance : évite de recalculer le territoire et les cases
    // achetables à chaque frame (ces calculs étaient O(N²×P) par frame avant).
    // Le cache est invalidé manuellement après chaque action qui modifie l'état du jeu.
    struct TerritoireCache {
        // vecParJoueur : territoire sous forme de vecteur (pour itérer dans le rendu)
        std::vector<std::vector<std::pair<int,int>>> vecParJoueur;
        // setParJoueur : même données en set (pour les lookups O(1) des bordures)
        std::vector<std::set<std::pair<int,int>>> setParJoueur;
        // dirty : si true, le cache doit être recalculé avant utilisation
        bool dirty = true;
    };
    TerritoireCache _territoireCache;
    std::vector<SegmentFrontiere> _frontieresCache;
    std::set<std::pair<int,int>> _casesAchetablesCache;
    bool _casesAchetablesDirty = true;
    
    void invaliderCaches();
    void recalculerCachesSiNecessaire(int viewIndex);


public:
    InterfaceManager(sf::RenderWindow& window, MoteurDeJeu & moteur);

    void run();
    
private:

    void loadUIConfig();
    void saveConfig();
    void initGame();
    void loadTextures(sf::Sprite& bg);
    void applyCustomTheme();
    bool DrawArrowSelector(const char* id, int* current_index, const std::vector<std::string>& items);

    // Menus
    void renderMenu();
    void renderPlayMenu();
    void renderFactionSelect();
    void renderOptions();
    void renderMapConfig();
    void renderGame();
    void renderUnitActions(Unite* u);
    float getRotationAngle(direction dir);

    // Multijoueur
    void renderMultiMenu();
    void renderHostLobby();
    void renderJoinLobby();
    void renderChatWindow();
    void updateNetworkLoop();
    void sendChatMessage(const std::string& msg);

    // Saves
    void refreshSaveList();
    void renderLoadMenu();

    sf::Color getPlayerColor(int playerIndex) const;
    void centrerSurCapitale(int playerIndex);

};
