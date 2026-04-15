#include "InterfaceManager.hh"
#include <iostream>
#include <ctime>

InterfaceManager::InterfaceManager(sf::RenderWindow& window) 
    : _window(window), _currentState(GameState::MENU) {
    std::cout << "[INIT] Lancement de l'InterfaceManager..." << std::endl;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    _gameView.setSize(_window.getSize().x, _window.getSize().y);
    _gameView.setCenter(0, 0);

    if (!ImGui::SFML::Init(_window)) {
        std::cerr << "[ERREUR FATALE] Initialisation ImGui-SFML a echoue !" << std::endl;
    }
    
    loadConfig();
    
    std::cout << "[INIT] 3. Chargement des textures..." << std::endl;
    loadTextures();
    
    std::cout << "[INIT] 4. Application du theme graphique..." << std::endl;
    applyCustomTheme();

    /*Pour modifier font
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("assets/Cinzel-Regular.ttf", 24.0f);
    ImGui::SFML::UpdateFontTexture();
    */
    
    std::cout << "[INIT] TERMINÉ. L'application est prete." << std::endl;
}

void InterfaceManager::loadConfig() {
    std::cout << "[INIT] 1. Chargement de config_rules.json..." << std::endl;
    try {
        std::ifstream file(_configPath);
        if (file.is_open()) {
            file >> _configJson;
            file.close();
            _logicConfig.loadRules(_configPath);
            std::cout << "[INIT] -> config_rules.json OK !" << std::endl;
        } else {
            std::cerr << "[ERREUR] Impossible d'ouvrir " << _configPath << std::endl;
        }
    } catch (const json::exception& e) {
        std::cerr << "[CRASH JSON] Erreur de syntaxe dans config_rules.json : " << e.what() << std::endl;
    }

    std::cout << "[INIT] 2. Chargement de config_espace.json..." << std::endl;
    try {
        std::ifstream fEspace("configs/config_espace.json");
        if (fEspace.is_open()) {
            fEspace >> _espaceJson;
            fEspace.close();
            std::cout << "[INIT] -> config_espace.json OK !" << std::endl;
        } else {
            std::cerr << "[ERREUR] Attention : config_espace.json introuvable." << std::endl;
        }
    } catch (const json::exception& e) {
        std::cerr << "[CRASH JSON] Erreur de syntaxe dans config_espace.json : " << e.what() << std::endl;
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

            // --- FIX RESIZE : On met juste à jour la caméra, sans bloquer ImGui ---
            if (event.type == sf::Event::Resized) {
                if (event.size.width > 0 && event.size.height > 0) {
                    _gameView.setSize((float)event.size.width, (float)event.size.height);
                }
            }
            
            // --- CLIC GAUCHE : Mathématiques Hexagonales Parfaites ---
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (_currentState == GameState::IN_GAME && !ImGui::GetIO().WantCaptureMouse) {
                    if (_board && _localPlayerIndex < _joueurs.size() && _currentPlayerTurn == _localPlayerIndex) {
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
                                if (ci >= 0 && ci < _board->getRows() && cj >= 0 && cj < _board->getCols()) {
                                    // Math: j + 0.5 * (i % 2)
                                    float hx = W * cj + W * 0.5f * (std::abs(ci) % 2);
                                    float hy = 1.5f * R * ci;
                                    float d = std::sqrt(std::pow(worldPos.x - hx, 2) + std::pow(worldPos.y - hy, 2));
                                    if (d < minDist) { minDist = d; bestI = ci; bestJ = cj; }
                                }
                            }
                        }

                        if (bestI != -1 && minDist <= R) {
                            if (_joueurs[_localPlayerIndex].estDecouvert(bestI, bestJ)) {
                                _selectedCellX = bestI; _selectedCellY = bestJ; _hasSelection = true;
                            }
                        } else { _hasSelection = false; }
                    }
                }
            }

            // Zoom et Déplacement (Clic droit)
            if (event.type == sf::Event::MouseWheelScrolled) {
                float factor = (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
                _gameView.zoom(factor);
                _currentZoom *= factor;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                _isPanning = true; _lastMousePos = sf::Mouse::getPosition(_window);
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) _isPanning = false;
            if (event.type == sf::Event::MouseMoved && _isPanning) {
                sf::Vector2i newPos = sf::Mouse::getPosition(_window);
                _gameView.move(_window.mapPixelToCoords(_lastMousePos, _gameView) - _window.mapPixelToCoords(newPos, _gameView));
                _lastMousePos = newPos;
            }
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
            
            ImGui::TableNextRow(0); // <--- FIX
            ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "GENERAL");
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 10)); // <--- FIX

            static int diffIndex = 1;
            std::vector<std::string> difficulties = {"Settler", "Viceroy", "Emperor", "Deity"};
            ImGui::TableNextRow(0); // <--- FIX
            ImGui::TableSetColumnIndex(0); ImGui::Text("Difficulty:");
            ImGui::TableSetColumnIndex(1); DrawArrowSelector("##diff", &diffIndex, difficulties);

            static int mapSizeIndex = 1;
            std::vector<std::string> mapSizes = {"Tiny (50x50)", "Standard (100x100)", "Huge (200x200)"};
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 5)); // <--- FIX
            ImGui::TableNextRow(0); // <--- FIX
            ImGui::TableSetColumnIndex(0); ImGui::Text("Map Size:");
            ImGui::TableSetColumnIndex(1); 
            if (DrawArrowSelector("##mapsize", &mapSizeIndex, mapSizes)) {
                if (mapSizeIndex == 0) { _configJson["taille_plateau"]["x"] = 50; _configJson["taille_plateau"]["y"] = 50; }
                if (mapSizeIndex == 1) { _configJson["taille_plateau"]["x"] = 100; _configJson["taille_plateau"]["y"] = 100; }
                if (mapSizeIndex == 2) { _configJson["taille_plateau"]["x"] = 200; _configJson["taille_plateau"]["y"] = 200; }
            }

            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 20)); // <--- FIX

            ImGui::TableNextRow(0); // <--- FIX
            ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "CITY RULES");
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 10)); // <--- FIX

            ImGui::TableNextRow(0); // <--- FIX
            ImGui::TableSetColumnIndex(0); ImGui::Text("Base City Cost:");
            ImGui::TableSetColumnIndex(1); 
            int cout = _configJson["regles_villes"]["cout_base"];
            ImGui::SetNextItemWidth(240.0f);
            if (ImGui::InputInt("##cout", &cout, 10, 200)) _configJson["regles_villes"]["cout_base"] = cout;

        } 
        else if (_currentOptionsTab == OptionsTab::GRAPHICS) {
            
            ImGui::TableNextRow(0); // <--- FIX
            ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "DISPLAY");
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 10)); // <--- FIX

            std::vector<std::string> qualities = {"Low", "Medium", "High", "Ultra"};
            ImGui::TableNextRow(0); // <--- FIX
            ImGui::TableSetColumnIndex(0); ImGui::Text("Graphics Quality:");
            ImGui::TableSetColumnIndex(1); DrawArrowSelector("##qual", &_qualityIndex, qualities);

            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 5)); // <--- FIX

            ImGui::TableNextRow(0); // <--- FIX
            ImGui::TableSetColumnIndex(0); ImGui::Text("Vertical Sync (V-Sync):");
            ImGui::TableSetColumnIndex(1); 
            if (ImGui::Checkbox("##vsync", &_vsync)) {
                _window.setVerticalSyncEnabled(_vsync);
            }

            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0, 5)); // <--- FIX

            ImGui::TableNextRow(0); // <--- FIX
            ImGui::TableSetColumnIndex(0); ImGui::Text("Fullscreen:");
            ImGui::TableSetColumnIndex(1);
            if (ImGui::Checkbox("##fullscreen", &_fullscreen)) {
                if (_fullscreen) {
                    _window.create(sf::VideoMode::getDesktopMode(), "Space Wargames", sf::Style::Fullscreen);
                } else {
                    _window.create(sf::VideoMode(1280, 720), "Space Wargames", sf::Style::Default);
                }
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

        // 5. Applique les poids mis à jour
        _factory.overrideWeights(_customWeights);

        // 6. Création du board
        _board = std::make_unique<board>(_factory, _logicConfig);

        // 7. Création des joueurs
        _joueurs.clear();
        for (int i = 0; i < _numPlayers; ++i) {
            Joueur j;
            
            if (i < _connectedPlayers.size()) {
                j.setName(_connectedPlayers[i].name);
            } else {
                j.setName("Joueur " + std::to_string(i + 1));
            }
            
            const FactionParams* fp = _logicConfig.getFaction(_playerFactions[i]);
            if (fp) j.setFaction(fp);
            
            j.initBrouillard(_board->getRows(), _board->getCols());
            
            // Placement de la Capitale
            bool placed = false;
            int attempts = 0;
            while (!placed && attempts < 2000) { 
                int rx = rand() % _board->getRows();
                int ry = rand() % _board->getCols();
                const hexa* tile = _board->getCell(rx, ry);
                
                if (tile && tile->getSymbole() == 'P') { 
                    TuileConfigurable* tc = const_cast<TuileConfigurable*>(dynamic_cast<const TuileConfigurable*>(tile));
                    if (tc && !tc->getCity()) { 
                        tc->constrVille(rx, ry, _logicConfig, 5, true); 
                        j.ajouterVille(tc->getCity());
                        j.decouvrirZone(rx, ry, 5, _board->getRows(), _board->getCols()); 
                        placed = true;
                    }
                }
                attempts++;
            }
            _joueurs.push_back(j);
        }

        // --- FIX : CENTRAGE DE LA VUE SUR LA CAPITALE ---
        if (_localPlayerIndex < _joueurs.size() && !_joueurs[_localPlayerIndex].getCities().empty()) {
            City* cap = _joueurs[_localPlayerIndex].getCities().front();
            int ci = cap->getX(); 
            int cj = cap->getY(); 
            float R = _tileSize / 2.0f;
            float W = std::sqrt(3.0f) * R;
            
            // Calcul mis à jour avec le (+)
            _gameView.setCenter(W * cj + W * 0.5f * (std::abs(ci) % 2), 1.5f * R * ci);
        }

        std::cout << "Secteur genere avec succes !" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "ERREUR : " << e.what() << std::endl;
        _currentState = GameState::MENU;
    }
}

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

    // On affiche par rapport à _maxPlayersBuffer choisi juste avant
    ImGui::Text("Joueurs connectes (%d/%d) :", (int)_connectedPlayers.size(), _maxPlayersBuffer);
    
    for (size_t i = 0; i < _connectedPlayers.size(); ++i) {
        ImGui::BulletText("Joueur %d : %s", (int)i + 1, _connectedPlayers[i].name.c_str());
    }

    ImGui::Dummy(ImVec2(0, 30));

    if (_connectedPlayers.size() < 2) {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "En attente d'adversaires sur le port %d...", _portBuffer);
    } 
    else {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Pret a lancer !");
        
        ImGui::SetCursorPos(ImVec2(200, 250));
        if (ImGui::Button("CONFIGURER LA CARTE", ImVec2(200, 50))) {
            // CRUCIAL : On force le jeu à démarrer avec le nombre RÉEL de joueurs connectés !
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

    // Le client entre le port ET l'IP ici
    ImGui::Text("Adresse IP :");
    ImGui::InputText("##ip", _ipBuffer, IM_ARRAYSIZE(_ipBuffer));
    ImGui::Dummy(ImVec2(0, 10));
    ImGui::Text("Port de l'hote :");
    ImGui::InputInt("##portClient", &_portBuffer, 0, 0);

    ImGui::Dummy(ImVec2(0, 50));

    if (_network.getState() == NetworkState::CONNECTING) {
        ImGui::Text("Approche de la flotte en cours...");
    } 
    else if (_network.getState() == NetworkState::CONNECTED) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Liaison etablie ! En attente du signal de l'hote...");
        
        if (!_hasSentName) {
            sf::Packet infoPacket;
            infoPacket << static_cast<sf::Int32>(PacketType::PLAYER_INFO) << std::string(_playerNameBuffer);
            _network.sendData(infoPacket);
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
                // 1. Un joueur s'est connecté et envoie son nom
                case PacketType::PLAYER_INFO: {
                    std::string clientName;
                    if (packet >> clientName) {
                        _connectedPlayers.push_back({clientName, ""});
                    }
                    break;
                }
                
                // 2. L'Hôte lance la partie
                case PacketType::GAME_START: {
                    if (packet >> _mapSeed >> _numPlayers) {
                        _connectedPlayers.clear();
                        _playerFactions.clear();
                        
                        // On lit les infos de tous les joueurs
                        for (int i = 0; i < _numPlayers; ++i) {
                            std::string pName, pFact;
                            packet >> pName >> pFact;
                            _connectedPlayers.push_back({pName, pFact});
                            _playerFactions.push_back(pFact);
                            
                            // CRUCIAL : Le client identifie quel est son numéro de joueur !
                            if (pName == std::string(_playerNameBuffer)) {
                                _localPlayerIndex = i;
                            }
                        }
                        
                        std::srand(_mapSeed);
                        initGame(); // Construit la map et place les joueurs
                        
                        _currentState = GameState::IN_GAME;
                    }
                    break;
                }

                // 3. L'autre joueur a passé son tour
                case PacketType::END_TURN: {
                    _currentPlayerTurn++;
                    if (_currentPlayerTurn >= _numPlayers) {
                        _currentPlayerTurn = 0;
                        _currentTurnNumber++;
                    }
                    _hasSelection = false;
                    
                    // Centrer la caméra sur ma capitale quand c'est mon tour
                    if (_currentPlayerTurn == _localPlayerIndex && !_joueurs[_localPlayerIndex].getCities().empty()) {
                        City* cap = _joueurs[_localPlayerIndex].getCities().front();
                        int ci = cap->getX(); 
                        int cj = cap->getY(); 
                        float R = _tileSize / 2.0f;
                        float W = std::sqrt(3.0f) * R;
                        _gameView.setCenter(W * cj + W * 0.5f * (std::abs(ci) % 2), 1.5f * R * ci);
                    }
                    break;
                }

                // 4. L'autre joueur a bougé une unité
                case PacketType::ACTION_MOVE: {
                    int xSrc, ySrc, xDest, yDest;
                    if (packet >> xSrc >> ySrc >> xDest >> yDest) {
                        Unite* u = _board->getUnite(xSrc, ySrc);
                        if (u) _board->deplacerUnite(*u, xDest, yDest);
                    }
                    break;
                }

                case PacketType::CHAT: {
                    std::string messageRecu;
                    if (packet >> messageRecu) {
                        // On ajoute le message reçu à l'historique
                        _chatMessages.push_back(messageRecu);
                    }
                    break;
                }

                // TODO plus tard : Ajouter ACTION_BUILD, ATTACK, etc.
            }
        }
    }
}

void InterfaceManager::sendChatMessage(const std::string& msg) {
    if (_network.getState() == NetworkState::CONNECTED) {
        sf::Packet chatPacket;
        chatPacket << static_cast<sf::Int32>(PacketType::CHAT);
        chatPacket << msg; // On met le texte dans le paquet
        
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

    // --- 1. HISTORIQUE DES MESSAGES ---
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

    // --- 2. SAISIE D'UN NOUVEAU MESSAGE ---
    bool envoyer = false;
    
    ImGui::SetNextItemWidth(260);
    // ImGuiInputTextFlags_EnterReturnsTrue permet de valider avec la touche "Entrée"
    if (ImGui::InputText("##chatInput", _chatInputBuffer, IM_ARRAYSIZE(_chatInputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        envoyer = true;
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Envoyer", ImVec2(60, 0))) {
        envoyer = true;
    }

    // Si on a validé et que le texte n'est pas vide
    if (envoyer && strlen(_chatInputBuffer) > 0) {
        // On formate le message : "Nom : Texte"
        std::string nom = _localPlayerIndex < _joueurs.size() ? _joueurs[_localPlayerIndex].getName() : "Moi";
        std::string msgFormate = nom + " : " + _chatInputBuffer;
        
        // 1. On l'affiche sur notre propre écran
        _chatMessages.push_back(msgFormate);
        
        // 2. On l'envoie à l'autre joueur
        sendChatMessage(msgFormate);
        
        // 3. On vide la zone de texte
        _chatInputBuffer[0] = '\0';
        
        // 4. On garde le focus sur la zone de texte pour enchaîner les messages
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();
    ImGui::PopStyleColor();
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
            ImGui::TableNextRow(0); // <--- FIX : On force le paramètre à 0
            ImGui::TableSetColumnIndex(0);
            
            // FIX WARNING : Ajout du (int)
            std::string pName = (isMultiplayer && i < (int)_connectedPlayers.size()) ? _connectedPlayers[i].name : "Joueur " + std::to_string(i + 1);
            ImGui::Text("%s", pName.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            
            std::string comboLabel = _playerFactions[i].empty() ? "Choisir une faction..." : _playerFactions[i];
            if (ImGui::BeginCombo("##factionCombo", comboLabel.c_str())) {
                for (const auto& [nom, params] : _logicConfig.getFactions()) {
                    bool isSelected = (_playerFactions[i] == nom);
                    if (ImGui::Selectable(nom.c_str(), isSelected)) {
                        _playerFactions[i] = nom;
                    }
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
        ImGui::TableNextRow(0); // <--- FIX

        // --- COLONNE GAUCHE : PARAMÈTRES GÉNÉRAUX ---
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

        // PRESETS 
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "WORLD PRESETS");
        if (ImGui::Button("GALAXY (Balanced)", ImVec2(400, 30))) {
            _customWeights['.'] = 90; _customWeights['P'] = 5; _customWeights['E'] = 3; _customWeights['X'] = 2;
        }
        if (ImGui::Button("NEBULA (Dense)", ImVec2(400, 30))) {
            _customWeights['.'] = 60; _customWeights['P'] = 20; _customWeights['E'] = 15; _customWeights['X'] = 5;
        }

        // --- COLONNE DROITE : DISTRIBUTION (POIDS) ---
        ImGui::TableSetColumnIndex(1);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "TILE DISTRIBUTION (%%)"); // FIX WARNING (%)
        ImGui::Dummy(ImVec2(0, 10));

        if (_espaceJson.contains("tiles")) {
            for (auto& t : _espaceJson["tiles"]) {
                std::string nom = t["nom"];
                char symb = std::string(t["symbole"])[0];
                
                // Initialiser si vide
                if (_customWeights.find(symb) == _customWeights.end()) 
                    _customWeights[symb] = t["gen"]["poids"];

                ImGui::Text("%s:", nom.c_str());
                ImGui::SliderInt((std::string("##w_") + symb).c_str(), &_customWeights[symb], 0, 100);
            }
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Erreur: Impossible de lire config_espace.json");
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
        std::srand(_mapSeed);
        initGame();
        
        // Si on est connecté et qu'on est l'hôte, on envoie toutes les infos
        if (_network.getState() == NetworkState::CONNECTED && _network.isHost()) {
            sf::Packet startPacket;
            startPacket << static_cast<sf::Int32>(PacketType::GAME_START);
            startPacket << _mapSeed << _numPlayers; 
            
            for (int i = 0; i < _numPlayers; ++i) {
                // FIX WARNING : Ajout du (int)
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

    float R = _tileSize / 2.0f;
    float W = std::sqrt(3.0f) * R;

    // --- OPTIMISATION : View Culling ---
    sf::Vector2f center = _gameView.getCenter();
    sf::Vector2f size = _gameView.getSize();

    // On calcule exactement quelles lignes/colonnes sont à l'écran
    int startRow = std::max(0, (int)((center.y - size.y/2) / (1.5f * R)) - 1);
    int endRow = std::min(_board->getRows(), (int)((center.y + size.y/2) / (1.5f * R)) + 2);
    int startCol = std::max(0, (int)((center.x - size.x/2) / W) - 1);
    int endCol = std::min(_board->getCols(), (int)((center.x + size.x/2) / W) + 2);

    // ==========================================================
    // CREATION DES FORMES (HORS DE LA BOUCLE !)
    // C'est le secret pour retrouver 100% de fluidité
    // ==========================================================
    sf::ConvexShape hexFog(6);
    sf::ConvexShape hexTex(6);
    sf::ConvexShape hexSelect(6);

    for (int i = 0; i < 6; ++i) {
        float angle = (3.14159f / 180.0f) * (60.0f * i - 30.0f);
        sf::Vector2f pt(R * std::cos(angle), R * std::sin(angle));
        hexFog.setPoint(i, pt);
        hexTex.setPoint(i, pt);
        hexSelect.setPoint(i, pt);
    }

    // 1. Apparence fixe du Brouillard
    hexFog.setFillColor(sf::Color(5, 5, 15));
    hexFog.setOutlineColor(sf::Color(255, 255, 255, 40));
    hexFog.setOutlineThickness(1.0f);

    // 2. Apparence fixe des Tuiles (La ligne blanche est gérée ici !)
    hexTex.setFillColor(sf::Color::White); // Blanc pour ne pas assombrir l'image
    hexTex.setOutlineColor(sf::Color(255, 255, 255, 150)); // Contour blanc bien visible
    hexTex.setOutlineThickness(1.0f);

    // 3. Apparence fixe de la Sélection
    hexSelect.setFillColor(sf::Color(255, 255, 255, 80));
    hexSelect.setOutlineColor(sf::Color::White);
    hexSelect.setOutlineThickness(2.0f);

    // ==========================================================
    // BOUCLE DE RENDU (Instantanée)
    // ==========================================================
    for (int i = startRow; i < endRow; ++i) {
        for (int j = startCol; j < endCol; ++j) {
            
            float posX = W * j + W * 0.5f * (std::abs(i) % 2);
            float posY = 1.5f * R * i;

            bool visible = (_localPlayerIndex < _joueurs.size()) ? _joueurs[_localPlayerIndex].estDecouvert(i, j) : true;

            // DESSIN : BROUILLARD
            if (!visible) {
                hexFog.setPosition(posX, posY);
                _window.draw(hexFog);
                continue;
            }

            // DESSIN : TUILE DÉCOUVERTE
            const hexa* tile = _board->getCell(i, j);
            if (tile && _textures.count(tile->getSymbole())) {
                // On se contente de lier la texture et bouger la forme. SFML découpera l'image automatiquement !
                hexTex.setTexture(&_textures[tile->getSymbole()]);
                hexTex.setPosition(posX, posY);
                _window.draw(hexTex);
            }

            // DESSIN : SÉLECTION
            if (_hasSelection && _selectedCellX == i && _selectedCellY == j) {
                hexSelect.setPosition(posX, posY);
                _window.draw(hexSelect);
            }
            
            // INDICATION DE CAPITALE (Si tu avais gardé la couronne)
            const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(tile);
            if (tc && tc->getCity() && tc->getCity()->estCapitale()) {
                sf::CircleShape crown(8, 5); 
                crown.setFillColor(sf::Color(255, 215, 0));
                crown.setOutlineColor(sf::Color::Black);
                crown.setOutlineThickness(1);
                crown.setPosition(posX - 8, posY - 15);
                _window.draw(crown);
            }
        }
    }

    _window.setView(sf::View(sf::FloatRect(0, 0, _window.getSize().x, _window.getSize().y)));

    // --------------------------------------------------------
    // 1. BARRE SUPERIEURE (Top Bar - Ressources et Tour)
    // --------------------------------------------------------
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, 40));
    ImGui::Begin("TopBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);
    
    // Fond semi-transparent fait maison
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(ImVec2(0, 0), ImVec2(_window.getSize().x, 40), IM_COL32(20, 25, 35, 220));

    ImGui::SetCursorPos(ImVec2(10, 10));
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "Tour : %d", _currentTurnNumber);
    ImGui::SameLine(100);

    // Affichage des NOMS réels :
    if (_currentPlayerTurn < _joueurs.size() && _currentPlayerTurn < _playerFactions.size()) {
        ImGui::Text("Joueur actuel : %s (%s)", _joueurs[_currentPlayerTurn].getName().c_str(), _playerFactions[_currentPlayerTurn].c_str());
    }
    
    ImGui::SameLine(_window.getSize().x - 150);
    if (ImGui::Button("Menu Principal")) _currentState = GameState::MENU;
    ImGui::End();

    ImGui::SameLine(_window.getSize().x - 300);
    if (ImGui::Button("Sauvegarder")) {
        SaveManager::saveGame("save.json", this);
    }

    // --------------------------------------------------------
    // 2. MINIMAP (En bas à gauche)
    // --------------------------------------------------------
    ImVec2 minimapSize(250, 200);
    ImGui::SetNextWindowPos(ImVec2(0, _window.getSize().y - minimapSize.y));
    ImGui::SetNextWindowSize(minimapSize);
    ImGui::Begin("Minimap", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "MINIMAP");
    ImGui::Separator();
    ImGui::TextDisabled("(Rendu de la minimap ici)");
    // Plus tard : tu pourras dessiner un RenderTexture SFML de ta carte dézoomée ici
    ImGui::End();


    // --------------------------------------------------------
    // 3. BOUTON TOUR SUIVANT (En bas à droite)
    // --------------------------------------------------------
    ImVec2 nextTurnSize(200, 100);
    ImGui::SetNextWindowPos(ImVec2(_window.getSize().x - nextTurnSize.x, _window.getSize().y - nextTurnSize.y));
    ImGui::SetNextWindowSize(nextTurnSize);
    ImGui::Begin("NextTurnBox", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    
    // Style gros bouton doré
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.2f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.7f, 0.3f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    
    // On désactive le bouton si ce n'est pas notre tour
    bool isMyTurn = (_currentPlayerTurn == _localPlayerIndex);
    if (!isMyTurn) ImGui::BeginDisabled();
    
    if (ImGui::Button("TOUR SUIVANT\n>>", ImVec2(180, 80))) {
        // Logique locale
        _currentPlayerTurn++;
        if (_currentPlayerTurn >= _numPlayers) {
            _currentPlayerTurn = 0;
            _currentTurnNumber++;
        }
        _hasSelection = false;
        
        // --- ENVOI RÉSEAU ---
        sf::Packet turnPacket;
        turnPacket << static_cast<sf::Int32>(PacketType::END_TURN);
        _network.sendData(turnPacket);
    }
    
    if (!isMyTurn) ImGui::EndDisabled();

    // Si ce n'est pas mon tour, on affiche une alerte au centre !
    if (!isMyTurn) {
        ImVec2 center = ImVec2(_window.getSize().x * 0.5f, _window.getSize().y * 0.2f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("WaitBox", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
        
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Veuillez patienter...");
        ImGui::Separator();
        ImGui::Text("En attente de : %s", _joueurs[_currentPlayerTurn].getName().c_str());
        
        ImGui::End();
    }
    
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    ImGui::End();


    // --------------------------------------------------------
    // 4. PANNEAU D'ACTIONS (En bas au centre)
    // --------------------------------------------------------
    float actionPanelWidth = _window.getSize().x - minimapSize.x - nextTurnSize.x;
    ImGui::SetNextWindowPos(ImVec2(minimapSize.x, _window.getSize().y - 150));
    ImGui::SetNextWindowSize(ImVec2(actionPanelWidth, 150));
    ImGui::Begin("ActionPanel", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    if (_hasSelection) {
        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "ACTIONS : Case (%d, %d)", _selectedCellX, _selectedCellY);
        ImGui::Separator();

        // --- ON EMPÊCHE D'AGIR PENDANT LE TOUR ENNEMI ---
        bool isMyTurn = (_currentPlayerTurn == _localPlayerIndex);
        if (!isMyTurn) ImGui::BeginDisabled();
        
        // Sous-menus avec des Onglets
        if (ImGui::BeginTabBar("ActionTabs")) {
            
            if (ImGui::BeginTabItem("CONSTRUCTIONS")) {
                ImGui::Dummy(ImVec2(0, 10));
                if (ImGui::Button("Fonder une Ville", ImVec2(150, 40))) { /* appel arbitre */ }
                ImGui::SameLine();
                if (ImGui::Button("Construire Mine", ImVec2(150, 40))) { /* appel arbitre */ }
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("UNITÉS")) {
                ImGui::Dummy(ImVec2(0, 10));
                if (ImGui::Button("Déplacer", ImVec2(150, 40))) { /* Mode déplacement */ }
                ImGui::SameLine();
                if (ImGui::Button("Attaquer", ImVec2(150, 40))) { /* Mode attaque */ }
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }

        if (!isMyTurn) ImGui::EndDisabled();

    } else {
        // Rien n'est sélectionné
        ImGui::SetCursorPosY(60);
        ImGui::SetCursorPosX((actionPanelWidth - ImGui::CalcTextSize("Sélectionnez une case ou une unité").x) * 0.5f);
        ImGui::TextDisabled("Sélectionnez une case ou une unité");
    }

    renderChatWindow();
    
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
    
    // Bouton Gauche
    if (ImGui::Button("<") && *current_index > 0) {
        (*current_index)--;
        changed = true;
    }
    
    ImGui::SameLine();
    
    // On mémorise la position X exacte après le bouton "<"
    float startX = ImGui::GetCursorPosX();
    float width = 150.0f; // Largeur de la zone de texte
    
    const char* text = items[*current_index].c_str();
    float textWidth = ImGui::CalcTextSize(text).x;
    
    // Centrage mathématique parfait
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

void InterfaceManager::initGameFromSave() {
    JsonWorldReader reader;
    std::map<std::string, Ressource*> resEmpty;
    _factory = WorldFactory(); 
    _factory.initialiserBords(); 
    reader.chargerConfig("configs/config_espace.json", resEmpty, _factory);
    _factory.overrideWeights(_customWeights);

    // CRUCIAL : On force le hasard avec la graine sauvegardée
    std::srand(_mapSeed); 
    _board = std::make_unique<board>(_factory, _logicConfig);

    // On prépare les joueurs sans placer de villes (elles seront chargées par le JSON plus tard)
    _joueurs.clear();
    _joueurs.resize(_numPlayers);
}