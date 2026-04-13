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

using json = nlohmann::json;

enum class GameState { MENU, OPTIONS, SETUP, IN_GAME };

class InterfaceManager {
private:
    sf::RenderWindow& _window;
    GameState _currentState;

    // Gestion de la configuration JSON
    json _configJson; // On n'utilise plus qu'un seul objet JSON
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
    void renderOptions();
    void renderSetup();
    void renderGame();
    
    void initGame();
    void loadTextures();
};

#endif