#include "InterfaceManager.hh"
#include <iostream>
#include <ctime>


InterfaceManager::InterfaceManager(sf::RenderWindow& window) 
    : _window(window), _currentState(GameState::MENU) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    // INITIALISATION DE LA VUE (Caméra)
    _gameView.setSize(_window.getSize().x, _window.getSize().y);
    _gameView.setCenter(0, 0); // On commence au centre théorique

    if (!ImGui::SFML::Init(_window)) {
        std::cerr << "Erreur d'initialisation ImGui-SFML" << std::endl;
    }
    loadConfig();
    loadTextures();
}

void InterfaceManager::loadConfig() {
    std::ifstream file(_configPath);
    if (file.is_open()) {
        file >> _configJson;
        file.close();
        // On synchronise la classe logique
        _logicConfig.loadRules(_configPath);
    } else {
        std::cerr << "Impossible d'ouvrir " << _configPath << ", chargement par defaut." << std::endl;
        _configJson["regles_villes"]["cout_base"] = 100;
        _configJson["regles_villes"]["multiplicateur"] = 1.5f;
    }
}

void InterfaceManager::saveConfig() {
    std::ofstream file(_configPath);
    if (file.is_open()) {
        file << _configJson.dump(4);
        file.close();
        // On recharge la logique pour qu'elle prenne les changements
        _logicConfig.loadRules(_configPath);
    }
}

void InterfaceManager::run() {
    sf::Clock deltaClock;
    while (_window.isOpen()) {
        sf::Event event;
        while (_window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(_window, event);
            if (event.type == sf::Event::Closed) _window.close();

            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) {
                    _gameView.zoom(0.9f); // Zoom avant
                    _currentZoom *= 0.9f;
                } else {
                    _gameView.zoom(1.1f); // Zoom arrière
                    _currentZoom *= 1.1f;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                _isPanning = true;
                _lastMousePos = sf::Mouse::getPosition(_window);
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) {
                _isPanning = false;
            }

            if (event.type == sf::Event::MouseMoved && _isPanning) {
                sf::Vector2i newPos = sf::Mouse::getPosition(_window);
                // On utilise mapPixelToCoords pour que le déplacement soit proportionnel au zoom
                sf::Vector2f oldCoords = _window.mapPixelToCoords(_lastMousePos, _gameView);
                sf::Vector2f newCoords = _window.mapPixelToCoords(newPos, _gameView);
                _gameView.move(oldCoords - newCoords);
                _lastMousePos = newPos;
            }
        }

        ImGui::SFML::Update(_window, deltaClock.restart());
        _window.clear(sf::Color(20, 20, 30));

        switch (_currentState) {
            case GameState::MENU:    renderMenu();    break;
            case GameState::OPTIONS: renderOptions(); break;
            case GameState::SETUP:   renderSetup();   break;
            case GameState::IN_GAME: renderGame();    break;
        }

        ImGui::SFML::Render(_window);
        _window.display();
    }
}


void InterfaceManager::renderOptions() {
    ImGui::Begin("Configurations du Secteur");

    if (ImGui::CollapsingHeader("Dimensions du Plateau", ImGuiTreeNodeFlags_DefaultOpen)) {
        int x = _configJson["taille_plateau"]["x"];
        int y = _configJson["taille_plateau"]["y"];
        
        if (ImGui::SliderInt("Largeur (X)", &x, 5, 50)) _configJson["taille_plateau"]["x"] = x;
        if (ImGui::SliderInt("Hauteur (Y)", &y, 5, 50)) _configJson["taille_plateau"]["y"] = y;
    }

    if (ImGui::CollapsingHeader("Economie des Villes", ImGuiTreeNodeFlags_DefaultOpen)) {
        int cout = _configJson["regles_villes"]["cout_base"];
        if (ImGui::InputInt("Cout de base", &cout)) {
            _configJson["regles_villes"]["cout_base"] = cout;
        }

        float mult = _configJson["regles_villes"]["multiplicateur"];
        if (ImGui::SliderFloat("Croissance", &mult, 1.0f, 3.0f)) {
            _configJson["regles_villes"]["multiplicateur"] = mult;
        }
    }

    if (ImGui::Button("Sauvegarder", ImVec2(120, 35))) {
        saveConfig();
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Retour", ImVec2(120, 35))) {
        _currentState = GameState::MENU;
    }

    ImGui::End();
}

void InterfaceManager::loadTextures() {
    // On charge chaque image et on l'associe à son symbole JSON
    if (!_textures['P'].loadFromFile("assets/planet.png")) {
        std::cerr << "Erreur : assets/planet.png introuvable" << std::endl;
    }
    if (!_textures['E'].loadFromFile("assets/star.png")) {
        std::cerr << "Erreur : assets/star.png introuvable" << std::endl;
    }
    if (!_textures['X'].loadFromFile("assets/black_hole.png")) {
        std::cerr << "Erreur : assets/black_hole.png introuvable" << std::endl;
    }
    if (!_textures['.'].loadFromFile("assets/space.png")) {
        std::cerr << "Erreur : assets/space.png introuvable" << std::endl;
    }
    if (!_textures['#'].loadFromFile("assets/border.png")) {
        std::cerr << "Erreur : assets/border.png introuvable" << std::endl;
    }
}

void InterfaceManager::initGame() {
    JsonWorldReader reader;
    std::map<std::string, Ressource*> resEmpty;
    
    _factory = WorldFactory(); 

    try {
        // 2. IMPORTANT : Définir la tuile de bordure '#'
        _factory.initialiserBords(); 

        // 3. Charger les tuiles depuis le JSON
        reader.chargerConfig("configs/config_espace.json", resEmpty, _factory);
        
        // 4. Vérification de sécurité
        if (_factory.estVide()) {
            throw std::runtime_error("La factory est vide (aucune tuile chargee)");
        }

        // 5. Création du board
        _board = std::make_unique<board>(_factory, _logicConfig);
        std::cout << "Secteur genere avec succes !" << std::endl;

        float centerX = 0; 
        float centerY = (_board->getRows() + _board->getCols()) * (_tileSize / 8.0f);
        _gameView.setCenter(centerX, centerY);

    } catch (const std::exception& e) {
        std::cerr << "ERREUR : " << e.what() << std::endl;
        _currentState = GameState::MENU;
    }
}


void InterfaceManager::renderMenu() {
    ImGui::Begin("Space Wargames - Menu Principal", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::Button("Jouer", ImVec2(200, 40))) _currentState = GameState::SETUP;
    if (ImGui::Button("Options", ImVec2(200, 40))) _currentState = GameState::OPTIONS;
    if (ImGui::Button("Quitter", ImVec2(200, 40))) _window.close();
    ImGui::End();
}

void InterfaceManager::renderSetup() {
    ImGui::Begin("Initialisation du Monde");
    ImGui::Text("Dimensions : %d x %d", _logicConfig.getPlateauX(), _logicConfig.getPlateauY());
    
    if (ImGui::Button("Generer le secteur", ImVec2(200, 40))) {
        initGame(); 
        _currentState = GameState::IN_GAME;
    }
    ImGui::End();
}

void InterfaceManager::renderGame() {
    _window.setView(_gameView);
    if (!_board) return;

    int rows = _board->getRows();
    int cols = _board->getCols();

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            const hexa* tile = _board->getCell(i, j);
            if (!tile) continue;

            // 1. Calcul de la position isométrique pure (relative au point 0,0 du monde)
            float posX = (i - j) * (_tileSize / 2.0f);
            float posY = (i + j) * (_tileSize / 4.0f);

            char s = tile->getSymbole();

            // 2. Si on a une texture pour ce symbole, on utilise un Sprite
            if (_textures.count(s)) {
                sf::Sprite sprite;
                sprite.setTexture(_textures[s]);
                sprite.setPosition(posX, posY);

                // 3. Redimensionnement (optionnel)
                // Si tes images ne font pas la taille de tes tuiles, on les scale
                float scaleX = _tileSize / sprite.getLocalBounds().width;
                float scaleY = (_tileSize / 2.0f) / sprite.getLocalBounds().height;
                sprite.setScale(scaleX, scaleY);

                _window.draw(sprite);
            } 
            else {
                // Fallback : si l'image manque, on dessine un rectangle de secours
                sf::RectangleShape fallback(sf::Vector2f(_tileSize, _tileSize / 2.0f));
                fallback.setPosition(posX, posY);
                fallback.setFillColor(sf::Color(50, 50, 50));
                _window.draw(fallback);
            }
        }
    }
    // 2. HUD de jeu ImGui
    _window.setView(_window.getDefaultView());
    ImGui::Begin("Infos Secteur");
    ImGui::Text("Tour en cours : Joueur 1");
    if (ImGui::Button("Retour au Menu")) _currentState = GameState::MENU;
    ImGui::End();
}








/*
void InterfaceManager::renderGame() {
    if (!_board) return;

    int rows = _board->getRows();
    int cols = _board->getCols();

    // 1. Dessin du plateau (Grille 2D -> Rendu Iso)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            const hexa* tile = _board->getCell(i, j); //

            if (!tile) continue;
            
            // Calcul Isométrique (i=ligne, j=colonne)
            float posX = (i - j) * (_tileSize / 2.0f) + (_window.getSize().x / 2.0f);
            float posY = (i + j) * (_tileSize / 4.0f) + 100.0f;

            sf::RectangleShape shape(sf::Vector2f(_tileSize, _tileSize / 2.0f));
            shape.setPosition(posX, posY);
            shape.setOutlineThickness(1);
            shape.setOutlineColor(sf::Color(255, 255, 255, 50));

            // Couleur selon le symbole
            if (tile->getSymbole() == '#') {
                shape.setFillColor(sf::Color(10, 10, 20));
                shape.setOutlineColor(sf::Color(50, 50, 80));
            }
            else if (tile->getSymbole() == 'P') shape.setFillColor(sf::Color::Green);
            else if (tile->getSymbole() == 'E') shape.setFillColor(sf::Color::Yellow);
            else if (tile->getSymbole() == 'X') shape.setFillColor(sf::Color::Red);
            else if (tile->getSymbole() == '.') shape.setFillColor(sf::Color::Black);
            else shape.setFillColor(sf::Color::Transparent);

            _window.draw(shape);
        }
    }

    // 2. HUD de jeu ImGui
    ImGui::Begin("Infos Secteur");
    ImGui::Text("Tour en cours : Joueur 1");
    if (ImGui::Button("Retour au Menu")) _currentState = GameState::MENU;
    ImGui::End();
}
*/
