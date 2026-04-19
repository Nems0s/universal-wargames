#include "InterfaceManager.hh"
#include <iostream>
#include <ctime>

InterfaceManager::InterfaceManager(sf::RenderWindow& window, MoteurDeJeu & moteur) 
    : _window(window), _moteur(moteur), _currentState(GameState::MENU) {

    std::cout << "INIT Lancement de l'InterfaceManager..." << std::endl;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    _gameView.setSize(_window.getSize().x, _window.getSize().y);
    _gameView.setCenter(0, 0);

    if (!ImGui::SFML::Init(_window)) {
        std::cerr << "ERREUR Initialisation ImGui-SFML a echoue !" << std::endl;
    }
    
    std::cout << "INIT Chargement de l'UI..." << std::endl;
    loadUIConfig();
    
    std::cout << "INIT Chargement des textures..." << std::endl;
    loadTextures();
    
    std::cout << "INIT Application du theme graphique..." << std::endl;
    applyCustomTheme();

    /*Pour modifier font
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("assets/Cinzel-Regular.ttf", 24.0f);
    ImGui::SFML::UpdateFontTexture();
    */
    
    std::cout << "INIT TERMINÉ" << std::endl;
}

void InterfaceManager::loadUIConfig() {
    std::ifstream fRules(_configPath);
    if (fRules.is_open()) { fRules >> _configJson; fRules.close(); }

    std::ifstream fEspace("configs/config_espace.json");
    if (fEspace.is_open()) { fEspace >> _espaceJson; fEspace.close(); }
}

void InterfaceManager::saveConfig() {
    std::ofstream file(_configPath);
    if (file.is_open()) {
        file << _configJson.dump(4);
        file.close();
        _moteur.chargerConfiguration(_configPath);
    }
}

void InterfaceManager::initGame() {
    try {
        _moteur.overrideWorldWeights(_customWeights);
        _moteur.initGame(_mapSeed, _numPlayers, _playerFactions);

        // Centrage de la vue sur la capitale
        if (_localPlayerIndex < (int)_moteur.getJoueurs().size() && !_moteur.getJoueurs()[_localPlayerIndex].getCities().empty()) {
            City* cap = _moteur.getJoueurs()[_localPlayerIndex].getCities().front();
            float R = _tileSize / 2.0f;
            float W = std::sqrt(3.0f) * R;
            _gameView.setCenter(W * cap->getY() + W * 0.5f * (std::abs(cap->getX()) % 2), 1.5f * R * cap->getX());
        }

        std::cout << "Secteur genere avec succes par le moteur !" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERREUR MOTEUR : " << e.what() << std::endl;
        _currentState = GameState::MENU;
    }
}

void InterfaceManager::run() {
    sf::Clock deltaClock;
    while (_window.isOpen()) {
        sf::Event event;
        while (_window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(_window, event);
            if (event.type == sf::Event::Closed) _window.close();

            if (event.type == sf::Event::Resized) {
                if (event.size.width > 0 && event.size.height > 0) {
                    _gameView.setSize((float)event.size.width, (float)event.size.height);
                }
            }
            
            // CLIC GAUCHE
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

                bool isMultiplayer = (_network.getState() == NetworkState::CONNECTED || _network.getState() == NetworkState::HOSTING);
                int currentTurn = _moteur.getCurrentPlayerTurn();

                // En local = toujours notre tour, en Multi = vérifie l'index
                bool isMyTurn = !isMultiplayer || (currentTurn == _localPlayerIndex);

                if (_currentState == GameState::IN_GAME && !ImGui::GetIO().WantCaptureMouse) {
                    if (_moteur.getPlateau() && currentTurn < (int)_moteur.getJoueurs().size() && isMyTurn) {
                        sf::Vector2i pixelPos = sf::Mouse::getPosition(_window);
                        sf::Vector2f worldPos = _window.mapPixelToCoords(pixelPos, _gameView);

                        float R = _tileSize / 2.0f; 
                        float W = std::sqrt(3.0f) * R; 

                        int estI = std::round(worldPos.y / (1.5f * R)); 
                        int estJ = std::round((worldPos.x / W) - 0.5f * (std::abs(estI) % 2)); 

                        int bestI = -1, bestJ = -1;
                        float minDist = R * 1.5f; 

                        for (int di = -1; di <= 1; ++di) {
                            for (int dj = -1; dj <= 1; ++dj) {
                                int ci = estI + di;
                                int cj = estJ + dj;
                                if (ci >= 0 && ci < _moteur.getPlateau()->getRows() && cj >= 0 && cj < _moteur.getPlateau()->getCols()) {
                                    float hx = W * cj + W * 0.5f * (std::abs(ci) % 2);
                                    float hy = 1.5f * R * ci;
                                    float d = std::sqrt(std::pow(worldPos.x - hx, 2) + std::pow(worldPos.y - hy, 2));
                                    if (d < minDist) { minDist = d; bestI = ci; bestJ = cj; }
                                }
                            }
                        }

                        if (bestI != -1 && minDist <= R) {
                            // SI ON EST EN MODE DEPLACEMENT
                            if (_isTargetingMove) {
                                // On vérifie que la case cliquée fait bien partie des cases valides
                                bool caseValide = false;
                                for (const auto& p : _casesPossibles) {
                                    if (p.first == bestI && p.second == bestJ) {
                                        caseValide = true; break;
                                    }
                                }
                                
                                if (caseValide) {
                                    // demande au moteur
                                    if (_moteur.demanderDeplacement(currentTurn, _unitSourceX, _unitSourceY, bestI, bestJ)) {
                                        // Transmission réseau (gérer par le moteur ensuite)
                                        if (isMultiplayer && !_network.isHost()) {
                                            sf::Packet p;
                                            p << static_cast<sf::Int32>(PacketType::ACTION_MOVE) << _unitSourceX << _unitSourceY << bestI << bestJ;
                                            _network.sendData(p);
                                        }
                                    }
                                }
                                
                                // Fin du ciblage
                                _isTargetingMove = false; 
                                _hasSelection = false;    
                                _casesPossibles.clear(); 
                            } else {
                                // Selection classique
                                if (_moteur.getJoueurs()[currentTurn].estDecouvert(bestI, bestJ)) {
                                    _selectedCellX = bestI; 
                                    _selectedCellY = bestJ; 
                                    _hasSelection = true;
                                }
                            }
                        } else { 
                            _hasSelection = false; 
                            _isTargetingMove = false;
                            _casesPossibles.clear();
                        }
                    }
                }
            }

            // Zoom et Déplacement (Clic droit)
            if (event.type == sf::Event::MouseWheelScrolled && !ImGui::GetIO().WantCaptureMouse) {
                float factor = (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
                _gameView.zoom(factor);
                _currentZoom *= factor;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right && !ImGui::GetIO().WantCaptureMouse) {
                _isPanning = true; _lastMousePos = sf::Mouse::getPosition(_window);
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) {
                _isPanning = false;
            }
            if (event.type == sf::Event::MouseMoved && _isPanning) {
                sf::Vector2i newPos = sf::Mouse::getPosition(_window);
                _gameView.move(_window.mapPixelToCoords(_lastMousePos, _gameView) - _window.mapPixelToCoords(newPos, _gameView));
                _lastMousePos = newPos;
            }
        }

        // Limites de la caméra
        if (_currentState == GameState::IN_GAME && _moteur.getPlateau()) {
            float R = _tileSize / 2.0f;
            float W = std::sqrt(3.0f) * R;
            float boardPxWidth = _moteur.getPlateau()->getCols() * W;
            float boardPxHeight = _moteur.getPlateau()->getRows() * 1.5f * R + R;
            float winW = (float)_window.getSize().x;
            float winH = (float)_window.getSize().y;

            float maxZoomX = boardPxWidth / winW;
            float maxZoomY = boardPxHeight / winH;
            float maxZoomOut = std::max(1.0f, std::max(maxZoomX, maxZoomY) * 1.05f); 
            float maxZoomIn = 0.3f; 
            
            float currentZoomRatio = _gameView.getSize().x / winW;
            if (currentZoomRatio > maxZoomOut) _gameView.setSize(winW * maxZoomOut, winH * maxZoomOut);
            else if (currentZoomRatio < maxZoomIn) _gameView.setSize(winW * maxZoomIn, winH * maxZoomIn);

            sf::Vector2f center = _gameView.getCenter();
            sf::Vector2f size = _gameView.getSize();
            float minX = size.x / 2.0f - W;
            float maxX = boardPxWidth - size.x / 2.0f + W;
            float minY = size.y / 2.0f - R;
            float maxY = boardPxHeight - size.y / 2.0f + R;

            if (boardPxWidth < size.x) center.x = boardPxWidth / 2.0f;
            else center.x = std::max(minX, std::min(center.x, maxX));

            if (boardPxHeight < size.y) center.y = boardPxHeight / 2.0f;
            else center.y = std::max(minY, std::min(center.y, maxY));

            _gameView.setCenter(center);
        }

        updateNetworkLoop();
        ImGui::SFML::Update(_window, deltaClock.restart());

        _window.clear(sf::Color(10, 10, 20));

        if (_currentState != GameState::IN_GAME) {
            // On calcule la taille une seule fois ici
            float w = (float)_window.getSize().x;
            float h = (float)_window.getSize().y;
            _window.setView(sf::View(sf::FloatRect(0, 0, w, h)));
            sf::VertexArray background(sf::Quads, 4);
            background[0].position = {0, 0}; background[0].color = {15, 15, 25};
            background[1].position = {w, 0}; background[1].color = {15, 15, 25};
            background[2].position = {w, h}; background[2].color = {5, 5, 10};
            background[3].position = {0, h}; background[3].color = {5, 5, 10};
            _window.draw(background);
        }

        // Switch de rendu des menus
        switch (_currentState) {
            case GameState::MENU:           renderMenu();          break;
            case GameState::PLAY_MENU:      renderPlayMenu();      break;
            case GameState::MULTI_MENU:     renderMultiMenu();     break;
            case GameState::HOST_LOBBY:     renderHostLobby();     break;
            case GameState::JOIN_LOBBY:     renderJoinLobby();     break;
            case GameState::FACTION_SELECT: renderFactionSelect(); break;
            case GameState::MAP_CONFIG:     renderMapConfig();     break;
            case GameState::OPTIONS:        renderOptions();       break;
            case GameState::IN_GAME:        renderGame();          break;
        }

        ImGui::SFML::Render(_window);
        _window.display();
    }
}


// ------------------------------------------------------------------------ //
// ---------------------- FONCTIONS DE RENDU (MENUS) ---------------------- //
// ------------------------------------------------------------------------ //

void InterfaceManager::renderMenu() {
    // Fenêtre invisible qui prend tout l'écran
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, _window.getSize().y));
    ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    // Titre
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Plus tard, tu pourras mettre une grande police ici
    float textWidth = ImGui::CalcTextSize("SPACE WARGAMES").x;
    ImGui::SetCursorPosX((_window.getSize().x - textWidth) * 0.5f);
    ImGui::SetCursorPosY(_window.getSize().y * 0.2f); // 20% du haut
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "SPACE WARGAMES");
    ImGui::PopFont();

    // Boutons
    ImVec2 buttonSize(300, 50);
    float btnX = (_window.getSize().x - buttonSize.x) * 0.5f;
    float startY = _window.getSize().y * 0.4f;

    ImGui::SetCursorPos(ImVec2(btnX, startY));
    if (ImGui::Button("JOUER", buttonSize)) _currentState = GameState::PLAY_MENU;
    ImGui::SetCursorPos(ImVec2(btnX, startY + 70));
    if (ImGui::Button("OPTIONS", buttonSize)) _currentState = GameState::OPTIONS;
    ImGui::SetCursorPos(ImVec2(btnX, startY + 140));
    if (ImGui::Button("QUITTER", buttonSize)) _window.close();

    ImGui::End();
}

void InterfaceManager::renderPlayMenu() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, _window.getSize().y));
    ImGui::Begin("Play Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    ImVec2 buttonSize(300, 50);
    float btnX = (_window.getSize().x - buttonSize.x) * 0.5f;

    ImGui::SetCursorPos(ImVec2(btnX, _window.getSize().y * 0.4f));
    if (ImGui::Button("NOUVELLE PARTIE (Local)", buttonSize)) {
        _network.disconnect();
        _currentState = GameState::FACTION_SELECT;
    }

    ImGui::SetCursorPos(ImVec2(btnX, _window.getSize().y * 0.4f + 70));
    if (ImGui::Button("MULTIJOUEUR (En Ligne)", buttonSize)) _currentState = GameState::MULTI_MENU;

    ImGui::SetCursorPos(ImVec2(btnX, _window.getSize().y * 0.4f + 140));
    if (ImGui::Button("RETOUR", buttonSize)) _currentState = GameState::MENU;

    ImGui::End();
}

void InterfaceManager::renderOptions() {
    ImVec2 menuSize(900, 700);
    ImGui::SetNextWindowPos(ImVec2((_window.getSize().x - menuSize.x) * 0.5f, (_window.getSize().y - menuSize.y) * 0.5f));
    ImGui::SetNextWindowSize(menuSize);
    ImGui::Begin("Advanced Options", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    ImGui::SetCursorPosX((menuSize.x - ImGui::CalcTextSize("ADVANCED OPTIONS").x) * 0.5f);
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "ADVANCED OPTIONS");
    ImGui::Dummy(ImVec2(0, 15));

    ImGui::SetCursorPosX((menuSize.x - 420) * 0.5f);
    if (ImGui::Button("GAME SETTINGS", ImVec2(200, 35))) _currentOptionsTab = OptionsTab::GAME_SETTINGS;
    ImGui::SameLine(0, 20);
    if (ImGui::Button("GRAPHICS", ImVec2(200, 35))) _currentOptionsTab = OptionsTab::GRAPHICS;
    
    ImGui::Dummy(ImVec2(0, 20));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 20));

    if (ImGui::BeginTable("OptionsTable", 2, ImGuiTableFlags_None)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 350.0f);
        ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);

        if (_currentOptionsTab == OptionsTab::GAME_SETTINGS) {
            
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "GENERAL");
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 10));

            static int diffIndex = 1;
            std::vector<std::string> difficulties = {"Settler", "Viceroy", "Emperor", "Deity"};
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::Text("Difficulty:");
            ImGui::TableSetColumnIndex(1); DrawArrowSelector("##diff", &diffIndex, difficulties);

            static int mapSizeIndex = 1;
            std::vector<std::string> mapSizes = {"Tiny (50x50)", "Standard (100x100)", "Huge (200x200)"};
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 5));
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::Text("Map Size:");
            ImGui::TableSetColumnIndex(1); 
            if (DrawArrowSelector("##mapsize", &mapSizeIndex, mapSizes)) {
                if (mapSizeIndex == 0) { _configJson["taille_plateau"]["x"] = 50; _configJson["taille_plateau"]["y"] = 50; }
                if (mapSizeIndex == 1) { _configJson["taille_plateau"]["x"] = 100; _configJson["taille_plateau"]["y"] = 100; }
                if (mapSizeIndex == 2) { _configJson["taille_plateau"]["x"] = 200; _configJson["taille_plateau"]["y"] = 200; }
            }

            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 20));

            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "CITY RULES");
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 10));

            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::Text("Base City Cost:");
            ImGui::TableSetColumnIndex(1); 
            int cout = _configJson["regles_villes"]["cout_base"];
            ImGui::SetNextItemWidth(240.0f);
            if (ImGui::InputInt("##cout", &cout, 10, 200)) _configJson["regles_villes"]["cout_base"] = cout;

        } 
        else if (_currentOptionsTab == OptionsTab::GRAPHICS) {
            
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "DISPLAY");
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 10));

            std::vector<std::string> qualities = {"Low", "Medium", "High", "Ultra"};
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::Text("Graphics Quality:");
            ImGui::TableSetColumnIndex(1); DrawArrowSelector("##qual", &_qualityIndex, qualities);

            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 5));
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::Text("Vertical Sync (V-Sync):");
            ImGui::TableSetColumnIndex(1); 
            if (ImGui::Checkbox("##vsync", &_vsync)) {
                _window.setVerticalSyncEnabled(_vsync);
            }

            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 5));
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::Text("Fullscreen:");
            ImGui::TableSetColumnIndex(1);
            if (ImGui::Checkbox("##fullscreen", &_fullscreen)) {
                if (_fullscreen) _window.create(sf::VideoMode::getDesktopMode(), "Space Wargames", sf::Style::Fullscreen);
                else _window.create(sf::VideoMode(1280, 720), "Space Wargames", sf::Style::Default);
                _window.setVerticalSyncEnabled(_vsync);
                _gameView.setSize(_window.getSize().x, _window.getSize().y);
            }
        }
        ImGui::EndTable();
    }

    ImGui::SetCursorPosY(menuSize.y - 60.0f);
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10));

    if (ImGui::Button("SAVE CONFIG", ImVec2(150, 40))) saveConfig();
    
    float rightButtonsX = menuSize.x - (150 * 2 + 20); 
    ImGui::SameLine(rightButtonsX);
    if (ImGui::Button("BACK", ImVec2(150, 40))) _currentState = GameState::MENU;
    ImGui::SameLine();
    if (ImGui::Button("LAUNCH GAME", ImVec2(150, 40))) {
        saveConfig(); 
        _currentState = GameState::FACTION_SELECT; 
    }

    ImGui::End();
}

void InterfaceManager::renderFactionSelect() {
    ImVec2 menuSize(800, 500);
    ImGui::SetNextWindowPos(ImVec2((_window.getSize().x - menuSize.x) * 0.5f, (_window.getSize().y - menuSize.y) * 0.5f));
    ImGui::SetNextWindowSize(menuSize);
    ImGui::Begin("Configuration des Joueurs", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "CONFIGURATION DES JOUEURS");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10));

    bool isMultiplayer = (_network.getState() == NetworkState::CONNECTED || _network.getState() == NetworkState::HOSTING);

    if (!isMultiplayer) {
        if (ImGui::InputInt("Nombre de Joueurs", &_numPlayers)) {
            if (_numPlayers < 2) _numPlayers = 2;
            if (_numPlayers > 4) _numPlayers = 4;
        }
    } else {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Mode Multijoueur actif : %d Commandants", _numPlayers);
    }
    
    _playerFactions.resize(_numPlayers, "");
    ImGui::Dummy(ImVec2(0, 20));

    if (ImGui::BeginTable("PlayerTable", 2, ImGuiTableFlags_BordersInnerH)) {
        for (int i = 0; i < _numPlayers; ++i) {
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0);
            
            std::string pName = (isMultiplayer && i < (int)_connectedPlayers.size()) ? _connectedPlayers[i].name : "Joueur " + std::to_string(i + 1);
            ImGui::Text("%s", pName.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            
            std::string comboLabel = _playerFactions[i].empty() ? "Choisir une faction..." : _playerFactions[i];
            if (ImGui::BeginCombo("##factionCombo", comboLabel.c_str())) {
                for (const auto& [nom, params] : _moteur.getLogicConfig().getFactions()) {
                    bool isSelected = (_playerFactions[i] == nom);
                    if (ImGui::Selectable(nom.c_str(), isSelected)) _playerFactions[i] = nom;
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    bool allReady = true;
    for (const auto& f : _playerFactions) {
        if (f.empty()) allReady = false;
    }

    ImGui::SetCursorPosY(menuSize.y - 60);
    ImGui::Separator();
    
    if (ImGui::Button("RETOUR", ImVec2(150, 40))) {
        if (isMultiplayer) _currentState = GameState::HOST_LOBBY;
        else _currentState = GameState::PLAY_MENU;
    }
    
    ImGui::SameLine(menuSize.x - 165);
    if (!allReady) ImGui::BeginDisabled();
    if (ImGui::Button("SUIVANT", ImVec2(150, 40))) _currentState = GameState::MAP_CONFIG;
    if (!allReady) ImGui::EndDisabled();

    ImGui::End();
}





// ---------------------------------------------------------------//
// ---------------- A MODIFIER POUR ETRE GENERAL ---------------- //
// ---------------------------------------------------------------//
void InterfaceManager::renderMapConfig() {
    ImVec2 menuSize(1000, 700);
    ImGui::SetNextWindowPos(ImVec2((_window.getSize().x - menuSize.x) * 0.5f, (_window.getSize().y - menuSize.y) * 0.5f));
    ImGui::SetNextWindowSize(menuSize);
    
    ImGui::Begin("Map Configuration", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    // Titre centré
    ImGui::SetCursorPosX((menuSize.x - ImGui::CalcTextSize("SECTOR CONFIGURATION").x) * 0.5f);
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "SECTOR CONFIGURATION");
    ImGui::Separator();

    if (ImGui::BeginTable("MapSplit", 2)) {
        ImGui::TableSetupColumn("General", ImGuiTableColumnFlags_WidthFixed, 450.0f);
        ImGui::TableSetupColumn("Weights", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow(0);

        // Colonne gauche (paramètres généraux)
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "GENERAL SETTINGS");
        
        static int sizeIdx = 1;
        std::vector<std::string> sizes = {"Tiny (50x50)", "Standard (100x100)", "Huge (200x200)"};
        ImGui::Text("Map Size:"); ImGui::SameLine(150);
        if (DrawArrowSelector("##msize", &sizeIdx, sizes)) {
            int s = (sizeIdx == 0) ? 50 : (sizeIdx == 1) ? 100 : 200;
            _configJson["taille_plateau"]["x"] = s;
            _configJson["taille_plateau"]["y"] = s;
        }

        ImGui::Text("Random Seed:"); ImGui::SameLine(150);
        ImGui::InputInt("##seed", &_mapSeed);

        ImGui::Dummy(ImVec2(0, 20));
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "WORLD PRESETS");
        if (ImGui::Button("GALAXY (Balanced)", ImVec2(400, 30))) {
            _customWeights['.'] = 90; _customWeights['P'] = 5; _customWeights['E'] = 3; _customWeights['X'] = 2;
        }
        if (ImGui::Button("NEBULA (Dense)", ImVec2(400, 30))) {
            _customWeights['.'] = 60; _customWeights['P'] = 20; _customWeights['E'] = 15; _customWeights['X'] = 5;
        }

        // Colonne droite (poids)
        ImGui::TableSetColumnIndex(1);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "TILE DISTRIBUTION (%%)"); // FIX WARNING (%)
        ImGui::Dummy(ImVec2(0, 10));

        if (_espaceJson.contains("tiles")) {
            for (auto& t : _espaceJson["tiles"]) {
                std::string nom = t["nom"];
                char symb = std::string(t["symbole"])[0];
                if (symb == '#') continue; 
                if (_customWeights.find(symb) == _customWeights.end()) _customWeights[symb] = t["gen"]["poids"];

                ImGui::Text("%s:", nom.c_str());
                ImGui::SliderInt((std::string("##w_") + symb).c_str(), &_customWeights[symb], 0, 100);
            }
        }

        ImGui::EndTable();
    }

    // Pied de page
    ImGui::SetCursorPosY(menuSize.y - 60);
    ImGui::Separator();
    if (ImGui::Button("BACK", ImVec2(150, 40))) _currentState = GameState::FACTION_SELECT;
    ImGui::SameLine(menuSize.x - 165);
    
    if (ImGui::Button("LAUNCH SECTOR", ImVec2(150, 40))) {
        // Lancement classique
        saveConfig();
        initGame();
        
        if (_network.getState() == NetworkState::CONNECTED && _network.isHost()) {
            sf::Packet startPacket;
            startPacket << static_cast<sf::Int32>(PacketType::GAME_START) << _mapSeed << _numPlayers; 
            startPacket << static_cast<sf::Int32>(_configJson["taille_plateau"]["x"]) << static_cast<sf::Int32>(_configJson["taille_plateau"]["y"]);
            startPacket << static_cast<sf::Int32>(_customWeights.size());
            for (auto const& [symb, weight] : _customWeights) {
                startPacket << static_cast<sf::Int32>(symb) << static_cast<sf::Int32>(weight);
            }
            for (int i = 0; i < _numPlayers; ++i) {
                std::string pName = (i < (int)_connectedPlayers.size()) ? _connectedPlayers[i].name : "IA " + std::to_string(i);
                std::string pFact = (i < (int)_playerFactions.size()) ? _playerFactions[i] : "";
                startPacket << pName << pFact;
            }
            _network.sendData(startPacket);
        }
    _currentState = GameState::IN_GAME;
    }
    ImGui::End();
}



// ---------------------------------------------------------------//
// ---------------- Réseau et multijoueur ----------------------- //
// ---------------------------------------------------------------//

void InterfaceManager::renderMultiMenu() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, _window.getSize().y));
    ImGui::Begin("Multiplayer Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    ImVec2 buttonSize(300, 50);
    float btnX = (_window.getSize().x - buttonSize.x) * 0.5f;

    ImGui::SetCursorPosX((_window.getSize().x - ImGui::CalcTextSize("MULTIJOUEUR").x) * 0.5f);
    ImGui::SetCursorPosY(_window.getSize().y * 0.15f);
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "MULTIJOUEUR");

    // 1. Nom commun à tous
    ImGui::SetCursorPosX(btnX);
    ImGui::Text("Votre Nom :");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputText("##pname", _playerNameBuffer, IM_ARRAYSIZE(_playerNameBuffer));

    ImGui::Dummy(ImVec2(0, 30));

    // 2. Section HÔTE
    ImGui::SetCursorPosX(btnX);
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "CREER UN SERVEUR");
    ImGui::SetCursorPosX(btnX);
    
    ImGui::BeginGroup();
    ImGui::Text("Port d'ecoute :"); ImGui::SameLine(120);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputInt("##portHost", &_portBuffer, 0, 0);
    
    ImGui::Text("Joueurs Max :"); ImGui::SameLine(120);
    ImGui::SetNextItemWidth(100.0f);
    ImGui::InputInt("##maxP", &_maxPlayersBuffer, 1, 1);
    if (_maxPlayersBuffer < 2) _maxPlayersBuffer = 2;
    if (_maxPlayersBuffer > 4) _maxPlayersBuffer = 4;
    ImGui::EndGroup();

    ImGui::SetCursorPosX(btnX);
    if (ImGui::Button("HEBERGER UNE PARTIE", buttonSize)) {
        _network.startHosting(static_cast<unsigned short>(_portBuffer));
        _currentState = GameState::HOST_LOBBY;
    }

    ImGui::Dummy(ImVec2(0, 30));

    // 3. Section REJOINDRE
    ImGui::SetCursorPosX(btnX);
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.8f, 1.0f), "REJOINDRE UN SERVEUR");
    ImGui::SetCursorPosX(btnX);
    if (ImGui::Button("REJOINDRE UNE PARTIE", buttonSize)) {
        _currentState = GameState::JOIN_LOBBY;
    }

    ImGui::Dummy(ImVec2(0, 30));
    ImGui::SetCursorPosX(btnX);
    if (ImGui::Button("RETOUR", buttonSize)) _currentState = GameState::PLAY_MENU;

    ImGui::End();
}

void InterfaceManager::renderHostLobby() {
    _network.update(); 
    if (_connectedPlayers.empty()) {
        _connectedPlayers.push_back({std::string(_playerNameBuffer), ""});
        _localPlayerIndex = 0; 
    }

    ImGui::SetNextWindowPos(ImVec2((_window.getSize().x - 600) * 0.5f, (_window.getSize().y - 400) * 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("Salon Hote", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "SALON D'ATTENTE (HOTE)");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Text("Joueurs connectes (%d/%d) :", (int)_connectedPlayers.size(), _maxPlayersBuffer);
    for (size_t i = 0; i < _connectedPlayers.size(); ++i) {
        ImGui::BulletText("Joueur %d : %s", (int)i + 1, _connectedPlayers[i].name.c_str());
    }

    ImGui::Dummy(ImVec2(0, 30));

    if (_connectedPlayers.size() < 2) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "En attente d'adversaires sur le port %d...", _portBuffer);
    } else {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Pret a lancer !");
        
        ImGui::SetCursorPos(ImVec2(200, 250));
        if (ImGui::Button("CONFIGURER LA CARTE", ImVec2(200, 50))) {
            _numPlayers = _connectedPlayers.size(); 
            _currentState = GameState::FACTION_SELECT; 
        }
    }

    ImGui::SetCursorPos(ImVec2(20, 340));
    if (ImGui::Button("ANNULER", ImVec2(150, 40))) {
        _network.disconnect();
        _connectedPlayers.clear();
        _currentState = GameState::MULTI_MENU;
    }
    ImGui::End();
}

void InterfaceManager::renderJoinLobby() {
    _network.update(); 

    ImGui::SetNextWindowPos(ImVec2((_window.getSize().x - 600) * 0.5f, (_window.getSize().y - 400) * 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("Rejoindre", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "REJOINDRE UNE PARTIE");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 20));

    // Le client entre le port ET l'IP
    ImGui::Text("Adresse IP :");
    ImGui::InputText("##ip", _ipBuffer, IM_ARRAYSIZE(_ipBuffer));
    ImGui::Dummy(ImVec2(0, 10));
    ImGui::Text("Port de l'hote :");
    ImGui::InputInt("##portClient", &_portBuffer, 0, 0);
    ImGui::Dummy(ImVec2(0, 50));

    if (_network.getState() == NetworkState::CONNECTING) {
        ImGui::Text("Approche de la flotte en cours...");
    } else if (_network.getState() == NetworkState::CONNECTED) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Liaison etablie ! En attente du signal de l'hote...");
        if (!_hasSentName) {
            sf::Packet p;
            p << static_cast<sf::Int32>(PacketType::PLAYER_INFO) << std::string(_playerNameBuffer);
            _network.sendData(p);
            _hasSentName = true;
        }
    }

    ImGui::SetCursorPosY(340);
    ImGui::Separator();
    
    if (ImGui::Button("RETOUR", ImVec2(150, 40))) {
        _network.disconnect();
        _hasSentName = false;
        _connectedPlayers.clear();
        _currentState = GameState::MULTI_MENU;
    }
    
    ImGui::SameLine(430);
    if (_network.getState() != NetworkState::CONNECTED && _network.getState() != NetworkState::CONNECTING) {
        if (ImGui::Button("CONNECTER", ImVec2(150, 40))) {
            _network.connectToHost(_ipBuffer, _portBuffer);
        }
    }

    ImGui::End();
}

void InterfaceManager::updateNetworkLoop() {
    if (_network.getState() == NetworkState::DISCONNECTED || _network.getState() == NetworkState::CONNECTING) return;

    sf::Packet packet;
    while (_network.receiveData(packet)) {
        sf::Int32 typeInt;
        if (packet >> typeInt) {
            PacketType type = static_cast<PacketType>(typeInt);

            switch (type) {
                // Un joueur s'est connecté et envoie son nom
                case PacketType::PLAYER_INFO: {
                    std::string clientName;
                    if (packet >> clientName) {
                        _connectedPlayers.push_back({clientName, ""});
                    }
                    break;
                }
                
                // L'Hôte lance la partie
                case PacketType::GAME_START: {
                    if (packet >> _mapSeed >> _numPlayers) {
                        
                        sf::Int32 sizeX, sizeY, weightsCount;
                        packet >> sizeX >> sizeY >> weightsCount;
                        
                        _configJson["taille_plateau"]["x"] = sizeX;
                        _configJson["taille_plateau"]["y"] = sizeY;
                        saveConfig();

                        _customWeights.clear();
                        for (int i = 0; i < weightsCount; ++i) {
                            sf::Int32 symb, w;
                            packet >> symb >> w;
                            _customWeights[static_cast<char>(symb)] = w;
                        }

                        _connectedPlayers.clear();
                        _playerFactions.clear();
                        
                        // On lit les infos de tous les joueurs
                        for (int i = 0; i < _numPlayers; ++i) {
                            std::string pName, pFact;
                            packet >> pName >> pFact;
                            _connectedPlayers.push_back({pName, pFact});
                            _playerFactions.push_back(pFact);
                            
                            // Le client identifie quel est son numéro de joueur !
                            if (pName == std::string(_playerNameBuffer)) {
                                _localPlayerIndex = i;
                            }
                        }
                        
                        std::srand(_mapSeed);
                        initGame(); // Construit la map identique à l'Hôte
                        
                        _currentState = GameState::IN_GAME;
                    }
                    break;
                }

                // L'autre joueur a passé son tour
                case PacketType::END_TURN: {
                    _moteur.passerTour();
                    _hasSelection = false;
                    if (_moteur.getCurrentPlayerTurn() == _localPlayerIndex && !_moteur.getJoueurs()[_localPlayerIndex].getCities().empty()) {
                        City* cap = _moteur.getJoueurs()[_localPlayerIndex].getCities().front();
                        float R = _tileSize / 2.0f;
                        float W = std::sqrt(3.0f) * R;
                        _gameView.setCenter(W * cap->getY() + W * 0.5f * (std::abs(cap->getX()) % 2), 1.5f * R * cap->getX());
                    }
                    break;
                }

                // L'autre joueur a bougé une unité
                case PacketType::ACTION_MOVE: {
                    int xSrc, ySrc, xDest, yDest;
                    if (packet >> xSrc >> ySrc >> xDest >> yDest) {
                        Unite* u = _moteur.getPlateau()->getUnite(xSrc, ySrc);
                        if (u) const_cast<board*>(_moteur.getPlateau())->deplacerUnite(*u, xDest, yDest);
                    }
                    break;
                }

                // Envoie d'un message
                case PacketType::CHAT: {
                    std::string messageRecu;
                    if (packet >> messageRecu) _chatMessages.push_back(messageRecu);
                    break;
                }

                // Demander une construction
                case PacketType::ACTION_BUILD: {
                    sf::Int32 senderIdx; int x, y; std::string batNom;
                    if (_network.isHost() && (packet >> senderIdx >> x >> y >> batNom)) {
                        _moteur.demanderConstruction(senderIdx, x, y, batNom);
                    }
                    break;
                }

                case PacketType::SYNC_BUILD: {
                    sf::Int32 targetIdx; int x, y; std::string batNom;
                    if (!_network.isHost() && (packet >> targetIdx >> x >> y >> batNom)) {
                        _moteur.demanderConstruction(targetIdx, x, y, batNom);
                        if (targetIdx == _localPlayerIndex) {
                            _popupMsg = batNom + " construit avec succes !";
                            _showPopup = true;
                        }
                    }
                    break;
                }
            }
        }
    }
}



// ---------------------------------------------------------------//
// ----------------------- Jeu et rendu ------------------------- //
// ---------------------------------------------------------------//

void InterfaceManager::renderGame() {
    _window.setView(_gameView);
    if (!_moteur.getPlateau()) return;

    // --- GESTION DU MODE LOCAL VS MULTI ---
    bool isMultiplayer = (_network.getState() == NetworkState::CONNECTED || _network.getState() == NetworkState::HOSTING);
    int viewIndex = isMultiplayer ? _localPlayerIndex : _moteur.getCurrentPlayerTurn(); 
    bool isMyTurn = !isMultiplayer || (_moteur.getCurrentPlayerTurn() == _localPlayerIndex);

    float R = _tileSize / 2.0f;
    float W = std::sqrt(3.0f) * R;

    // OPTIMISATION : View Culling
    sf::Vector2f center = _gameView.getCenter();
    sf::Vector2f size = _gameView.getSize();
    int startRow = std::max(0, (int)((center.y - size.y/2) / (1.5f * R)) - 1);
    int endRow = std::min(_moteur.getPlateau()->getRows(), (int)((center.y + size.y/2) / (1.5f * R)) + 2);
    int startCol = std::max(0, (int)((center.x - size.x/2) / W) - 1);
    int endCol = std::min(_moteur.getPlateau()->getCols(), (int)((center.x + size.x/2) / W) + 2);

    // Formes pour le brouillard et la sélection
    sf::ConvexShape hexFog(6);
    sf::ConvexShape hexSelect(6);
    for (int i = 0; i < 6; ++i) {
        float angle = (3.14159f / 180.0f) * (60.0f * i - 30.0f);
        sf::Vector2f pt(R * std::cos(angle), R * std::sin(angle));
        hexFog.setPoint(i, pt);
        hexSelect.setPoint(i, pt);
    }
    hexFog.setFillColor(sf::Color(5, 5, 15));
    hexFog.setOutlineColor(sf::Color(255, 255, 255, 40));
    hexFog.setOutlineThickness(1.0f);
    hexSelect.setFillColor(sf::Color(255, 255, 255, 80));
    hexSelect.setOutlineColor(sf::Color::White);
    hexSelect.setOutlineThickness(2.0f);

    // ==========================================================
    // OPTIMISATION : PRECALCUL ET NIVEAU DE DETAIL (LOD)
    // ==========================================================
    sf::Vector2f hexOffsets[7];
    for (int pt = 0; pt <= 6; ++pt) {
        float angle = (3.14159f / 180.0f) * (60.0f * (pt % 6) - 30.0f);
        hexOffsets[pt] = sf::Vector2f(std::cos(angle), std::sin(angle));
    }
    
    // Si la caméra voit une zone 3 fois plus large que l'écran normal, on cache les détails
    bool drawDetails = (_gameView.getSize().x / _window.getSize().x) < 3.0f;

    sf::VertexArray hexTex(sf::TriangleFan, 8);

    // ==========================================================
    // BOUCLE DE RENDU DES TUILES
    // ==========================================================
    for (int i = startRow; i < endRow; ++i) {
        for (int j = startCol; j < endCol; ++j) {
            float posX = W * j + W * 0.5f * (std::abs(i) % 2);
            float posY = 1.5f * R * i;

            bool visible = (viewIndex < (int)_moteur.getJoueurs().size()) ? _moteur.getJoueurs()[viewIndex].estDecouvert(i, j) : true;

            if (!visible) {
                hexFog.setPosition(posX, posY);
                _window.draw(hexFog);
                continue;
            }

            const hexa* tile = _moteur.getPlateau()->getCell(i, j);
            if (tile && _textures.count(tile->getSymbole())) {
                sf::Texture& tex = _textures[tile->getSymbole()];
                sf::VertexArray hexTex(sf::TriangleFan, 8); 
                sf::Vector2f texCenter(tex.getSize().x / 2.0f, tex.getSize().y / 2.0f);
                
                hexTex[0].position = sf::Vector2f(posX, posY);
                hexTex[0].texCoords = texCenter;
                hexTex[0].color = sf::Color::White;

                // pt <= 6 pour créer le 7eme sommet extérieur pour fermer la boucle
                for (int pt = 0; pt <= 6; ++pt) {
                    hexTex[pt+1].position = sf::Vector2f(posX + R * hexOffsets[pt].x, posY + R * hexOffsets[pt].y);
                    hexTex[pt+1].texCoords = sf::Vector2f(
                        texCenter.x + hexOffsets[pt].x * (tex.getSize().x / 2.0f),
                        texCenter.y + hexOffsets[pt].y * (tex.getSize().y / 2.0f)
                    );
                    hexTex[pt+1].color = sf::Color::White;
                }
                _window.draw(hexTex, &tex);
            }

            // DESSIN DES SYMBOLES UNIQUEMENT SI ASSEZ ZOOM (Ville / Batiment)
            if (drawDetails) {
            const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(tile);
                if (tc) {
                    if (tc->getCity()) {
                        // Maison pentagonale pour les villes
                        sf::ConvexShape house(5);
                        house.setPoint(0, sf::Vector2f(0, -15));
                        house.setPoint(1, sf::Vector2f(15, -5));
                        house.setPoint(2, sf::Vector2f(15, 15));
                        house.setPoint(3, sf::Vector2f(-15, 15));
                        house.setPoint(4, sf::Vector2f(-15, -5));
                        
                        house.setPosition(posX, posY);
                        house.setFillColor(tc->getCity()->estCapitale() ? sf::Color(255, 215, 0) : sf::Color(0, 200, 255));
                        house.setOutlineThickness(2.0f);
                        house.setOutlineColor(sf::Color::Black);
                        _window.draw(house);
                    }
                    if (tc->getBatimentSpecial()) {
                        sf::CircleShape gear(8, 6);
                        gear.setOrigin(8, 8);
                        gear.setPosition(posX, posY + 10);
                        gear.setFillColor(sf::Color(150, 150, 150));
                        gear.setOutlineThickness(1.0f);
                        gear.setOutlineColor(sf::Color::Black);
                        _window.draw(gear);
                    }
                }
            }

            if (_hasSelection && _selectedCellX == i && _selectedCellY == j) {
                hexSelect.setPosition(posX, posY);
                _window.draw(hexSelect);
            }

            // dessin du mode ciblage (Zone Jaune via l'Arbitre)
            if (_isTargetingMove) {
                // On vérifie si la case actuelle est dans la liste renvoyée par l'arbitre
                bool isPossible = false;
                for (const auto& casePos : _casesPossibles) {
                    if (casePos.first == i && casePos.second == j) {
                        isPossible = true;
                        break;
                    }
                }
                
                // Si oui, on dessine l'hexagone jaune
                if (isPossible) {
                    sf::ConvexShape hexMove = hexSelect;
                    hexMove.setFillColor(sf::Color(255, 255, 0, 50));
                    hexMove.setOutlineColor(sf::Color::Yellow);
                    hexMove.setPosition(posX, posY);
                    _window.draw(hexMove);
                }
            }
        }
    }

    // ========================================================
    // INTERFACE HUD (Fixe)
    // ========================================================
    _window.setView(sf::View(sf::FloatRect(0, 0, _window.getSize().x, _window.getSize().y)));

    // --------------------------------------------------------
    // TOP BAR & MENU DÉROULANT DES RESSOURCES
    // --------------------------------------------------------
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, 40));
    ImGui::Begin("TopBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    
    // Fond semi-transparent
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(ImVec2(0, 0), ImVec2(_window.getSize().x, 40), IM_COL32(20, 25, 35, 220));

    ImGui::SetCursorPos(ImVec2(10, 10));
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "Tour : %d", _moteur.getTourActuel());
    ImGui::SameLine(100);

    if (_moteur.getCurrentPlayerTurn() < (int)_moteur.getJoueurs().size() && _moteur.getCurrentPlayerTurn() < (int)_playerFactions.size()) {
        ImGui::Text("Joueur actuel : %s (%s)", _moteur.getJoueurs()[_moteur.getCurrentPlayerTurn()].getName().c_str(), _playerFactions[_moteur.getCurrentPlayerTurn()].c_str());
    }

    if (viewIndex < (int)_moteur.getJoueurs().size()) {
        ImGui::SameLine(500); 
        ImGui::SetNextItemWidth(250);
        std::string comboLabel = "Ressources de " + _moteur.getJoueurs()[viewIndex].getName();
        if (ImGui::BeginCombo("##ressources", comboLabel.c_str())) {
            for (auto const& [res, qte] : _moteur.getJoueurs()[viewIndex].getInventaire()) {
                if (res) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s :", res->getName().c_str());
                    ImGui::SameLine(150);
                    ImGui::Text("%d", qte);
                }
            }
            ImGui::EndCombo();
        }
    }
    
    ImGui::SameLine(_window.getSize().x - 150);
    if (ImGui::Button("Menu Principal")) _currentState = GameState::MENU;
    ImGui::End();

    // --------------------------------------------------------
    // BOUTON FIN DE TOUR
    // --------------------------------------------------------
    ImVec2 nextTurnSize(200, 100);
    ImGui::SetNextWindowPos(ImVec2(_window.getSize().x - nextTurnSize.x, _window.getSize().y - nextTurnSize.y));
    ImGui::SetNextWindowSize(nextTurnSize);
    ImGui::Begin("NextTurnBox", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.2f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.7f, 0.3f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    
    if (!isMyTurn) ImGui::BeginDisabled();
    if (ImGui::Button("TOUR SUIVANT\n>>", ImVec2(180, 80))) {
        _hasSelection = false;
        
        _moteur.passerTour();
        
        if (isMultiplayer) {
            sf::Packet turnPacket; turnPacket << static_cast<sf::Int32>(PacketType::END_TURN);
            _network.sendData(turnPacket);
        } else {
            if (_moteur.getCurrentPlayerTurn() < (int)_moteur.getJoueurs().size() && !_moteur.getJoueurs()[_moteur.getCurrentPlayerTurn()].getCities().empty()) {
                City* cap = _moteur.getJoueurs()[_moteur.getCurrentPlayerTurn()].getCities().front();
                _gameView.setCenter(W * cap->getY() + W * 0.5f * (std::abs(cap->getX()) % 2), 1.5f * R * cap->getX());
            }
        }
    }
    if (!isMyTurn) ImGui::EndDisabled();

    if (!isMyTurn) {
        ImVec2 centerUi = ImVec2(_window.getSize().x * 0.5f, _window.getSize().y * 0.2f);
        ImGui::SetNextWindowPos(centerUi, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("WaitBox", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Veuillez patienter...");
        ImGui::Separator();
        ImGui::Text("En attente de : %s", _moteur.getJoueurs()[_moteur.getCurrentPlayerTurn()].getName().c_str());
        ImGui::End();
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    ImGui::End();

    // --------------------------------------------------------
    // MINIMAP (Interactive avec Zoom, Pan et Clic)
    // --------------------------------------------------------
    ImVec2 minimapWinSize(300, 250);
    ImGui::SetNextWindowPos(ImVec2(0, _window.getSize().y - minimapWinSize.y));
    ImGui::SetNextWindowSize(minimapWinSize);
    ImGui::Begin("Minimap", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "MINIMAP (Scroll: Zoom | Clic Droit: Glisser)");
    ImGui::Separator();

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 s = ImGui::GetContentRegionAvail();
    ImDrawList* minimapDrawList = ImGui::GetWindowDrawList();

    // Fond spatial de la minimap
    minimapDrawList->AddRectFilled(p, ImVec2(p.x + s.x, p.y + s.y), IM_COL32(15, 15, 25, 255));

    // Variables statiques pour garder le zoom/pan en mémoire entre deux frames
    static float miniZoom = 1.0f;
    static ImVec2 miniOffset(0, 0);

    // --- interactions souris sur la minimap ---
    if (ImGui::IsWindowHovered()) {
        // Scroll pour Zoom/Dézoom de la minimap
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0.0f) {
            miniZoom += wheel * 0.1f;
            if (miniZoom < 1.0f) miniZoom = 1.0f;
            if (miniZoom > 5.0f) miniZoom = 5.0f;
        }
        
        // Clic Droit enfoncé pour glisser (Pan) dans la minimap
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            miniOffset.x += ImGui::GetIO().MouseDelta.x;
            miniOffset.y += ImGui::GetIO().MouseDelta.y;
        }
    }

    if (_moteur.getPlateau() && viewIndex < (int)_moteur.getJoueurs().size()) {
        int rows = _moteur.getPlateau()->getRows();
        int cols = _moteur.getPlateau()->getCols();
        const Joueur& localJ = _moteur.getJoueurs()[viewIndex];

        // Calcul de la taille d'une case sur la minimap pour que la map rentre dans la fenêtre
        float mapRatio = (float)cols / (float)rows;
        float winRatio = s.x / s.y;
        float baseHexSize = (mapRatio > winRatio) ? (s.x / (cols * 1.5f)) : (s.y / (rows * 1.5f));

        float R_mini = baseHexSize * miniZoom;
        float W_mini = std::sqrt(3.0f) * R_mini;

        // Centrage de la map
        float mapWidthPx = cols * W_mini;
        float mapHeightPx = rows * 1.5f * R_mini;
        float startX = p.x + (s.x - mapWidthPx) * 0.5f + miniOffset.x;
        float startY = p.y + (s.y - mapHeightPx) * 0.5f + miniOffset.y;

        // --- dessin des cases découvertes ---
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // Brouillard de guerre : on ne dessine que ce qu'on connait
                if (!localJ.estDecouvert(i, j)) continue;

                float cx = startX + W_mini * j + W_mini * 0.5f * (std::abs(i) % 2);
                float cy = startY + 1.5f * R_mini * i;

                // Optimisation : Ne dessine pas ce qui déborde de la fenêtre minimap
                if (cx < p.x || cx > p.x + s.x || cy < p.y || cy > p.y + s.y) continue;

                const hexa* tile = _moteur.getPlateau()->getCell(i, j);
                ImU32 color = IM_COL32(50, 50, 50, 255);


                // ---------------------------------------------------------------//
                // ---------------- A MODIFIER POUR ETRE GENERAL ---------------- //
                // ---------------------------------------------------------------//
                if (tile) {
                    char symb = tile->getSymbole();
                    if (symb == '.') color = IM_COL32(20, 20, 35, 255); // Espace (Très sombre)
                    else if (symb == 'P') color = IM_COL32(50, 200, 50, 255); // Planète (Vert)
                    else if (symb == 'E') color = IM_COL32(255, 255, 100, 255); // Étoile (Jaune)
                    else if (symb == 'X') color = IM_COL32(150, 0, 200, 255); // Trou Noir (Violet)
                    else if (symb == '#') color = IM_COL32(100, 20, 20, 255); // Limite (Rouge)

                    // Vérification s'il y a une ville dessus
                    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(tile);
                    if (tc && tc->getCity()) {
                        color = tc->getCity()->estCapitale() ? IM_COL32(255, 215, 0, 255) : IM_COL32(0, 200, 255, 255);
                    }
                }
                
                // Dessin du pixel/carré représentant la case
                minimapDrawList->AddRectFilled(ImVec2(cx - R_mini*0.8f, cy - R_mini*0.8f), ImVec2(cx + R_mini*0.8f, cy + R_mini*0.8f), color);
            }
        }

        // --- Dessin du cadre de la caméra ---
        sf::Vector2f viewCenter = _gameView.getCenter();
        sf::Vector2f viewSize = _gameView.getSize();
        float realR = _tileSize / 2.0f;
        float realW = std::sqrt(3.0f) * realR;

        // Conversion des coordonnées du monde réel vers la minimap
        float scaleX = W_mini / realW;
        float scaleY = (1.5f * R_mini) / (1.5f * realR);

        float camMiniX = startX + viewCenter.x * scaleX;
        float camMiniY = startY + viewCenter.y * scaleY;
        float camMiniW = viewSize.x * scaleX;
        float camMiniH = viewSize.y * scaleY;

        minimapDrawList->AddRect(
            ImVec2(camMiniX - camMiniW*0.5f, camMiniY - camMiniH*0.5f),
            ImVec2(camMiniX + camMiniW*0.5f, camMiniY + camMiniH*0.5f),
            IM_COL32(255, 255, 255, 200), 0.0f, 0, 1.5f
        );

        // --- Clic gauche : déplacer la caméra ---
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ImVec2 mousePos = ImGui::GetMousePos();
            
            // On calcule la distance entre le clic et l'origine de la map dessinée
            float targetMiniX = mousePos.x - startX;
            float targetMiniY = mousePos.y - startY;
            
            // On reconvertit cette distance en coordonnées SFML
            float targetWorldX = targetMiniX / scaleX;
            float targetWorldY = targetMiniY / scaleY;
            
            // On téléporte la caméra SFML à cet endroit !
            _gameView.setCenter(targetWorldX, targetWorldY);
        }
    }

    ImGui::End();

    // --------------------------------------------------------
    // 4. ACTION PANEL CONTEXTUEL
    // --------------------------------------------------------
    float actionPanelWidth = _window.getSize().x - 450;
    ImGui::SetNextWindowPos(ImVec2(250, _window.getSize().y - 150));
    ImGui::SetNextWindowSize(ImVec2(actionPanelWidth, 150));
    ImGui::Begin("ActionPanel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    
    if (_hasSelection && viewIndex < (int)_moteur.getJoueurs().size()) {
        int localJIdx = isMultiplayer ? _localPlayerIndex : _moteur.getCurrentPlayerTurn();
        Joueur& localJ = _moteur.getJoueurMutable(localJIdx); 
        const hexa* h = _moteur.getPlateau()->getCell(_selectedCellX, _selectedCellY);
        const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(h);

        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "Case (%d, %d)", _selectedCellX, _selectedCellY);
        ImGui::Separator();

        if (!isMyTurn) ImGui::BeginDisabled();
        bool actionPossible = false;

        if (tc) {
            // Fonder une ville
            if (_moteur.getArbitre().buildCity(localJ, *_moteur.getPlateau(), _selectedCellX, _selectedCellY)) {
                actionPossible = true;
                if (ImGui::Button("Fonder une Ville", ImVec2(150, 40))) {
                    hexa* cellMutable = const_cast<hexa*>(_moteur.getPlateau()->getCell(_selectedCellX, _selectedCellY));
                    TuileConfigurable* tcMutable = dynamic_cast<TuileConfigurable*>(cellMutable);
                    if (tcMutable) {
                        tcMutable->constrVille(_selectedCellX, _selectedCellY, _moteur.getLogicConfig(), 5, false);
                        localJ.ajouterVille(tcMutable->getCity());
                        localJ.decouvrirZone(_selectedCellX, _selectedCellY, 5, _moteur.getPlateau()->getRows(), _moteur.getPlateau()->getCols());
                        _popupMsg = "Ville fondee avec succes !";
                        _showPopup = true;
                    }
                }
                ImGui::SameLine();
            }

            // B. Actions sur une Ville Existante
            if (tc->getCity()) {
                actionPossible = true;
                if (ImGui::Button("Ameliorer Ville", ImVec2(150, 40))) {
                    if (_moteur.demanderAmeliorationVille(localJIdx, _selectedCellX, _selectedCellY)) {
                        _popupMsg = "Ville amelioree !";
                    } else {
                        _popupMsg = "Amelioration impossible (Niveau max ou ressources insuffisantes).";
                    }
                    _showPopup = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("Construire Batiment", ImVec2(150, 40))) ImGui::OpenPopup("Menu Construction Batiments");
                ImGui::SameLine();
            }

            // C. Acheter territoire
            if (!tc->getCity() && !_moteur.getArbitre().estDansTerritoire(localJ, _selectedCellX, _selectedCellY, *_moteur.getPlateau(), _moteur.getLogicConfig())) {
                if (_moteur.getArbitre().peutAcheterCase(localJ, _selectedCellX, _selectedCellY, *_moteur.getPlateau(), _moteur.getLogicConfig())) {
                    actionPossible = true;
                    if (ImGui::Button("Acheter Territoire", ImVec2(150, 40))) {
                        _popupMsg = "Achat de territoire declenche !";
                        _showPopup = true;
                    }
                }
            }

            // --- UNITÉS ---
            Unite* uniteSurCase = _moteur.getPlateau()->getUnite(_selectedCellX, _selectedCellY);
            if (uniteSurCase && _moteur.getArbitre().appartientJoueur(localJ, *uniteSurCase)) {
                actionPossible = true;
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "UNITE : %s", uniteSurCase->name().c_str());
                
                if (ImGui::Button("Deplacer", ImVec2(150, 40))) {
                    _isTargetingMove = true; _isTargetingAttack = false;
                    _unitSourceX = _selectedCellX; _unitSourceY = _selectedCellY;
                    _casesPossibles = _moteur.getArbitre().getCasesDeplacementPossibles(*_moteur.getPlateau(), *uniteSurCase);
                    _popupMsg = "Ciblez une case jaune pour vous deplacer.";
                    _showPopup = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("Attaquer", ImVec2(150, 40))) {
                    _isTargetingAttack = true; _isTargetingMove = false;
                    _unitSourceX = _selectedCellX; _unitSourceY = _selectedCellY;
                    _popupMsg = "Ciblez un ennemi sur la carte pour attaquer.";
                    _showPopup = true;
                }
            }
        }

        if (!actionPossible) ImGui::TextDisabled("Aucune action possible sur cette case.");
        if (!isMyTurn) ImGui::EndDisabled();

        // ========================================================
        // SOUS-MENU : LISTE DES BÂTIMENTS
        // ========================================================
        if (ImGui::BeginPopup("Menu Construction Batiments")) {
            ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "BATIMENTS DISPONIBLES");
            ImGui::Separator();
            for (const auto& [nom, batimentModele] : _moteur.getBatimentFactory().getCatalogue()) {
                std::string coutText = "";
                for (auto const& [res, qte] : batimentModele->getResourceConstr()) {
                    if (!coutText.empty()) coutText += ", ";
                    coutText += std::to_string(qte) + " " + res->getName();
                }
                if (coutText.empty()) coutText = "Gratuit";
                std::string label = nom + " (Cout: " + coutText + ")";

                if (ImGui::Selectable(label.c_str())) {
                    if (isMultiplayer && !_network.isHost()) {
                        sf::Packet p; p << static_cast<sf::Int32>(PacketType::ACTION_BUILD) << static_cast<sf::Int32>(_localPlayerIndex) << _selectedCellX << _selectedCellY << nom; 
                        _network.sendData(p);
                        _popupMsg = "Requete envoyee, en attente de validation...";
                        _showPopup = true;
                    } else {
                        if (_moteur.demanderConstruction(localJIdx, _selectedCellX, _selectedCellY, nom)) {
                            _popupMsg = nom + " construit avec succes !";
                        } else {
                            _popupMsg = "Construction impossible (Ressources ou terrain).";
                        }
                        _showPopup = true;
                    }
                }
            }
            if (_moteur.getBatimentFactory().getCatalogue().empty()) ImGui::TextDisabled("Aucun batiment dans le catalogue.");
            ImGui::EndPopup();
        }

    } else {
        ImGui::SetCursorPosY(60);
        ImGui::SetCursorPosX((actionPanelWidth - ImGui::CalcTextSize("Selectionnez une case").x) * 0.5f);
        ImGui::TextDisabled("Selectionnez une case");
    }
    ImGui::End();

    // --------------------------------------------------------
    // 5. PANNEAU LATÉRAL INFOS VILLE
    // --------------------------------------------------------
    if (_hasSelection) {
        const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(_moteur.getPlateau()->getCell(_selectedCellX, _selectedCellY));
        if (tc && tc->getCity()) {
            City* city = tc->getCity();
            ImGui::SetNextWindowPos(ImVec2(_window.getSize().x - 260, 50));
            ImGui::SetNextWindowSize(ImVec2(250, 300));
            ImGui::Begin("CityInfo", nullptr, ImGuiWindowFlags_NoTitleBar);
            
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "VILLE NIVEAU %d", city->getLevel());
            ImGui::Separator();
            ImGui::Text("PV: %.0f/%.0f", city->getPv(), city->getPvMax());
            ImGui::Text("Degats: %.0f", city->getDegats());
            ImGui::Dummy(ImVec2(0, 10));

            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "BATIMENTS ACTUELS :");
            if (city->getBatiments().empty()) {
                ImGui::TextDisabled("  Aucun batiment.");
            } else {
                for (auto& b : city->getBatiments()) {
                    ImGui::BulletText("%s", b->getName().c_str());
                }
            }
            ImGui::End();
        }
    }

    // --------------------------------------------------------
    // 6. POP-UP DE RETOUR D'ACTION (Validations, Erreurs...)
    // --------------------------------------------------------
    if (_showPopup) { ImGui::OpenPopup("Resultat Action"); _showPopup = false; }
    if (ImGui::BeginPopupModal("Resultat Action", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", _popupMsg.c_str());
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 100) * 0.5f);
        if (ImGui::Button("FERMER", ImVec2(100, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    renderChatWindow();
}


// ---------------------------------------------------------------//
// ------------- Fonctions de textures et themes ---------------- //
// ---------------------------------------------------------------//

// ---------------------------------------------------------------//
// ---------------- A MODIFIER POUR ETRE GENERAL ---------------- //
// ---------------------------------------------------------------//
void InterfaceManager::loadTextures() {
    // On charge chaque image et on l'associe à son symbole JSON
    if (!_textures['P'].loadFromFile("assets/planet.png")) std::cerr << "Erreur : assets/planet.png introuvable" << std::endl;
    if (!_textures['E'].loadFromFile("assets/star.png")) std::cerr << "Erreur : assets/star.png introuvable" << std::endl;
    if (!_textures['X'].loadFromFile("assets/black_hole.png")) std::cerr << "Erreur : assets/black_hole.png introuvable" << std::endl;
    if (!_textures['.'].loadFromFile("assets/space.png")) std::cerr << "Erreur : assets/space.png introuvable" << std::endl;
    if (!_textures['#'].loadFromFile("assets/border.png")) std::cerr << "Erreur : assets/border.png introuvable" << std::endl;
}

void InterfaceManager::applyCustomTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Bordures et arrondis
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    // Couleurs (Style Nuit / Or)
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]       = ImVec4(0.08f, 0.10f, 0.14f, 0.95f); // Bleu très sombre
    colors[ImGuiCol_Border]         = ImVec4(0.80f, 0.70f, 0.30f, 0.80f); // Doré
    colors[ImGuiCol_FrameBg]        = ImVec4(0.15f, 0.18f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.30f, 0.40f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.35f, 0.40f, 0.50f, 1.00f);
    colors[ImGuiCol_Button]         = ImVec4(0.12f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.80f, 0.70f, 0.30f, 0.30f); // Hover doré transparent
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.80f, 0.70f, 0.30f, 0.60f);
    colors[ImGuiCol_Text]           = ImVec4(0.95f, 0.95f, 0.90f, 1.00f);
    colors[ImGuiCol_Header]         = ImVec4(0.80f, 0.70f, 0.30f, 0.40f); // Sélection
    colors[ImGuiCol_HeaderHovered]  = ImVec4(0.80f, 0.70f, 0.30f, 0.60f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(0.80f, 0.70f, 0.30f, 0.80f);
}

bool InterfaceManager::DrawArrowSelector(const char* id, int* current_index, const std::vector<std::string>& items) {
    bool changed = false;
    ImGui::PushID(id);

    // Bouton gauche
    if (ImGui::Button("<") && *current_index > 0) {
        (*current_index)--;
        changed = true;
    }
    ImGui::SameLine();

    // On mémorise la position X exacte après le bouton "<"
    float startX = ImGui::GetCursorPosX();
    float width = 150.0f; 

    const char* text = items[*current_index].c_str();
    float textWidth = ImGui::CalcTextSize(text).x;

    // Centrage mathématique
    ImGui::SetCursorPosX(startX + (width - textWidth) * 0.5f);
    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", text);

    // On force la position du bouton ">"
    ImGui::SameLine();
    ImGui::SetCursorPosX(startX + width);

    if (ImGui::Button(">") && *current_index < (int)items.size() - 1) {
        (*current_index)++;
        changed = true;
    }
    
    ImGui::PopID();
    return changed;
}

void InterfaceManager::sendChatMessage(const std::string& msg) {
    if (_network.getState() == NetworkState::CONNECTED) {
        sf::Packet chatPacket;
        chatPacket << static_cast<sf::Int32>(PacketType::CHAT) << msg; 
        _network.sendData(chatPacket);
    }
}

void InterfaceManager::renderChatWindow() {
    // On la place au-dessus de la Minimap (la minimap fait 200 de haut par exemple)
    ImGui::SetNextWindowPos(ImVec2(0, _window.getSize().y - 200 - 250), ImGuiCond_FirstUseEver); 
    ImGui::SetNextWindowSize(ImVec2(350, 250), ImGuiCond_FirstUseEver);
    
    // Fenêtre légèrement transparente pour voir l'espace derrière
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.10f, 0.14f, 0.85f));
    ImGui::Begin("Chat de Flotte", nullptr, ImGuiWindowFlags_NoCollapse);

    // --- historique des messages ---
    // Child window pour pouvoir scroller dans l'historique
    ImGui::BeginChild("ChatHistory", ImVec2(0, 170), true);
    for (const auto& msg : _chatMessages) {
        ImGui::TextWrapped("%s", msg.c_str());
    }
    
    // Auto-scroll vers le bas si un nouveau message arrive
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    ImGui::Separator();

    // --- saisie d'un nouveau message ---
    bool envoyer = false;
    ImGui::SetNextItemWidth(260);
    if (ImGui::InputText("##chatInput", _chatInputBuffer, IM_ARRAYSIZE(_chatInputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        envoyer = true;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Envoyer", ImVec2(60, 0))) {
        envoyer = true;
    }

    if (envoyer && strlen(_chatInputBuffer) > 0) {
        std::string nom = _localPlayerIndex < (int)_moteur.getJoueurs().size() ? _moteur.getJoueurs()[_localPlayerIndex].getName() : "Moi";
        std::string msgFormate = nom + " : " + _chatInputBuffer;
        _chatMessages.push_back(msgFormate);
        sendChatMessage(msgFormate);
        _chatInputBuffer[0] = '\0';
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
    ImGui::PopStyleColor();
}
