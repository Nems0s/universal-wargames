#include "InterfaceManager.hh"
#include <iostream>
#include <ctime>
#include <set>
#include <fstream>
#include <filesystem>
#include <fstream>
#include <sstream>

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
    std::ifstream fRules(_rulesPath);
    if (fRules.is_open()) { fRules >> _rulesJson; fRules.close(); }

    std::ifstream fEspace(_espacePath);
    if (fEspace.is_open()) { fEspace >> _espaceJson; fEspace.close(); }
    
    std::ifstream fVilles(_villesPath);
    if (fVilles.is_open()) { fVilles >> _villesJson; fVilles.close(); }

    std::ifstream fSettings("saves/settings.json");
    if (fSettings.is_open()) {
        nlohmann::json sJson;
        fSettings >> sJson;
        fSettings.close();

        if (sJson.contains("player_name")) {
            strncpy(_playerNameBuffer, sJson["player_name"].get<std::string>().c_str(), sizeof(_playerNameBuffer) - 1);
            _playerNameBuffer[sizeof(_playerNameBuffer) - 1] = '\0';
        }
        if (sJson.contains("last_ip")) {
            strncpy(_ipBuffer, sJson["last_ip"].get<std::string>().c_str(), sizeof(_ipBuffer) - 1);
            _ipBuffer[sizeof(_ipBuffer) - 1] = '\0';
        }
        if (sJson.contains("port")) _portBuffer = sJson["port"];
        if (sJson.contains("vsync")) _vsync = sJson["vsync"];
        if (sJson.contains("fullscreen")) _fullscreen = sJson["fullscreen"];
        
        _window.setVerticalSyncEnabled(_vsync);
    }
}

void InterfaceManager::saveConfig() {
    std::ofstream fileRules(_rulesPath);
    if (fileRules.is_open()) {
        fileRules << _rulesJson.dump(4);
        fileRules.close();
    }

    std::ofstream fileVilles(_villesPath);
    if (fileVilles.is_open()) {
        fileVilles << _villesJson.dump(4);
        fileVilles.close();
    }

    nlohmann::json sJson;
    sJson["player_name"] = std::string(_playerNameBuffer);
    sJson["last_ip"] = std::string(_ipBuffer);
    sJson["port"] = _portBuffer;
    sJson["vsync"] = _vsync;
    sJson["fullscreen"] = _fullscreen;

    std::ofstream fSettings("saves/settings.json");
    if (fSettings.is_open()) {
        fSettings << sJson.dump(4);
        fSettings.close();
    }
    
    if (!_gameConfigLocked) {
        _moteur.chargerConfiguration(_rulesPath);
    }
}

void InterfaceManager::initGame() {
    _gameConfigLocked = true;
    try {
        _moteur.overrideWorldWeights(_customWeights);

        std::vector<std::string> noms;
        for(int i = 0; i < _numPlayers; ++i) {
            noms.push_back(_connectedPlayers[i].name);
        }

        _moteur.initGame(_mapSeed, noms, _playerFactions);

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
            
            // CLIC GAUCHE ET DRAG AND DROP
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                bool isMultiplayer = (_network.getState() == NetworkState::CONNECTED || _network.getState() == NetworkState::HOSTING);
                bool isWaitingForPlayer = (isMultiplayer && _network.isHost() && (int)_connectedPlayers.size() < _numPlayers);
                int currentTurn = _moteur.getCurrentPlayerTurn();
                bool isMyTurn = !isMultiplayer || (currentTurn == _localPlayerIndex);
                bool canInteract = (!_gameOver ||_gameContinuesAfterWin);

                if (_currentState == GameState::IN_GAME && canInteract && !ImGui::GetIO().WantCaptureMouse && !isWaitingForPlayer) {
                    sf::Vector2i pixelPos = sf::Mouse::getPosition(_window);
                    sf::Vector2f worldPos = _window.mapPixelToCoords(pixelPos, _gameView);
                    float R = _tileSize / 2.0f; float W = std::sqrt(3.0f) * R;
                    int estI = std::round(worldPos.y / (1.5f * R));
                    int estJ = std::round((worldPos.x / W) - 0.5f * (std::abs(estI) % 2));
                    int bestI = -1, bestJ = -1; float minDist = R;

                    for (int di = -1; di <= 1; ++di) {
                        for (int dj = -1; dj <= 1; ++dj) {
                            int ci = estI + di; int cj = estJ + dj;
                            if (ci >= 0 && ci < _moteur.getPlateau()->getRows() && cj >= 0 && cj < _moteur.getPlateau()->getCols()) {
                                float hx = W * cj + W * 0.5f * (std::abs(ci) % 2); float hy = 1.5f * R * ci;
                                float d = std::sqrt(std::pow(worldPos.x - hx, 2) + std::pow(worldPos.y - hy, 2));
                                if (d < minDist) { minDist = d; bestI = ci; bestJ = cj; }
                            }
                        }
                    }

                    if (bestI != -1 && isMyTurn) {
                        if (_isTargetingMove || _isTargetingAttack) {
                            if (_isTargetingMove) {
                                CmdDeplacement cmd = { _unitSourceX, _unitSourceY, bestI, bestJ };
                                if (_moteur.soumettreCommande(currentTurn, cmd) == ResultatAction::SUCCES) {
                                    if (isMultiplayer) {
                                        sf::Packet pk;
                                        pk << static_cast<sf::Int32>(PacketType::ACTION_MOVE) << static_cast<sf::Int32>(currentTurn) << _unitSourceX << _unitSourceY << bestI << bestJ;
                                        _network.sendData(pk);
                                    }
                                }
                            } else if (_isTargetingAttack) {
                                Unite* uAtt = _moteur.getPlateau()->getUnite(_unitSourceX, _unitSourceY);
                                Unite* uDef = _moteur.getPlateau()->getUnite(bestI, bestJ);
                                std::string nomAtt = uAtt ? uAtt->name() : "Unite";
                                std::string nomDef = uDef ? uDef->name() : "Cible";
                                int pvDefAvant = uDef ? uDef->health_point() : 0;
                                int pvAttAvant = uAtt ? uAtt->health_point() : 0;

                                CmdAttaque cmd = { _unitSourceX, _unitSourceY, bestI, bestJ };
                                if (_moteur.soumettreCommande(currentTurn, cmd) == ResultatAction::SUCCES) {
                                    
                                    Unite* uDefApres = _moteur.getPlateau()->getUnite(bestI, bestJ);
                                    Unite* uAttApres = _moteur.getPlateau()->getUnite(_unitSourceX, _unitSourceY);
                                    int pvDefPerdus = pvDefAvant - (uDefApres ? uDefApres->health_point() : 0);
                                    int pvAttPerdus = pvAttAvant - (uAttApres ? uAttApres->health_point() : 0);
                                    
                                    std::string rapport = nomAtt + " inflige -" + std::to_string(pvDefPerdus) + " a " + nomDef;
                                    if (pvAttPerdus > 0) rapport += " (Riposte: -" + std::to_string(pvAttPerdus) + ")";
                                    if (!uDefApres) rapport += " [" + nomDef + " DETRUIT !]";
                                    
                                    addCombatLog(rapport, sf::Color(255, 100, 100));

                                    if (isMultiplayer) {
                                        sf::Packet pk; 
                                        pk << static_cast<sf::Int32>(PacketType::ACTION_ATTACK) << _unitSourceX << _unitSourceY << bestI << bestJ;
                                        _network.sendData(pk);
                                    }
                                } else {
                                    _popupMsg = "Cible hors de portee ou invalide !";
                                    _showPopup = true;
                                }
                            }
                            
                            _isTargetingMove = false;
                            _isTargetingAttack = false;
                            _casesPossibles.clear();
                            _casesAttaquePossibles.clear();
                            _selectedCellX = bestI; _selectedCellY = bestJ;
                            _hasSelection = true;
                            continue;
                        }

                        if (_moteur.getProprietaireUnite(bestI, bestJ) == currentTurn) {
                            _isDragging = true;
                            _dragSourceX = bestI;
                            _dragSourceY = bestJ;
                            _unitSourceX = bestI;
                            _unitSourceY = bestJ;
                            _casesPossibles = _moteur.getDeplacementsPossibles(currentTurn, bestI, bestJ);
                            _casesAttaquePossibles = _moteur.getAttaquesPossibles(currentTurn, bestI, bestJ);
                            _hasSelection = true;
                            _selectedCellX = bestI;
                            _selectedCellY = bestJ;
                            _hasPreviewRotation = false;
                        } else {
                            _selectedCellX = bestI;
                            _selectedCellY = bestJ;
                            _hasSelection = true;
                            _hasPreviewRotation = false;
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                if (_isDragging) {
                    bool isMultiplayer = (_network.getState() == NetworkState::CONNECTED || _network.getState() == NetworkState::HOSTING);
                    int currentTurn = _moteur.getCurrentPlayerTurn();
                    sf::Vector2i pixelPos = sf::Mouse::getPosition(_window);
                    sf::Vector2f worldPos = _window.mapPixelToCoords(pixelPos, _gameView);
                    float R = _tileSize / 2.0f; float W = std::sqrt(3.0f) * R;
                    int estI = std::round(worldPos.y / (1.5f * R));
                    int estJ = std::round((worldPos.x / W) - 0.5f * (std::abs(estI) % 2));
                    int targetI = -1, targetJ = -1; float minDist = R;

                    for (int di = -1; di <= 1; ++di) {
                        for (int dj = -1; dj <= 1; ++dj) {
                            int ci = estI + di; int cj = estJ + dj;
                            if (ci >= 0 && ci < _moteur.getPlateau()->getRows() && cj >= 0 && cj < _moteur.getPlateau()->getCols()) {
                                float hx = W * cj + W * 0.5f * (std::abs(ci) % 2); float hy = 1.5f * R * ci;
                                float d = std::sqrt(std::pow(worldPos.x - hx, 2) + std::pow(worldPos.y - hy, 2));
                                if (d < minDist) { minDist = d; targetI = ci; targetJ = cj; }
                            }
                        }
                    }

                    if (targetI != -1 && (targetI != _dragSourceX || targetJ != _dragSourceY)) {
                        int propDest = _moteur.getProprietaireUnite(targetI, targetJ);

                        if (propDest != -1 && propDest != currentTurn) {
                            // Attaque

                            bool cibleValide = false;
                            for (const auto& pos : _casesAttaquePossibles) {
                                if (pos.first == targetI && pos.second == targetJ) {
                                    cibleValide = true;
                                    break;
                                }
                            }

                            if (cibleValide) {
                                // Stockez les PV avant l'attaque (pour le Combat Log)
                                Unite* uAtt = _moteur.getPlateau()->getUnite(_dragSourceX, _dragSourceY);
                                Unite* uDef = _moteur.getPlateau()->getUnite(targetI, targetJ);
                                std::string nomAtt = uAtt ? uAtt->name() : "Unite";
                                std::string nomDef = uDef ? uDef->name() : "Cible";
                                int pvDefAvant = uDef ? uDef->health_point() : 0;
                                int pvAttAvant = uAtt ? uAtt->health_point() : 0;

                                CmdAttaque cmd = { _dragSourceX, _dragSourceY, targetI, targetJ };
                                if (_moteur.soumettreCommande(currentTurn, cmd) == ResultatAction::SUCCES) {
                                    
                                    // Combat Log
                                    Unite* uDefApres = _moteur.getPlateau()->getUnite(targetI, targetJ);
                                    Unite* uAttApres = _moteur.getPlateau()->getUnite(_dragSourceX, _dragSourceY);
                                    int pvDefPerdus = pvDefAvant - (uDefApres ? uDefApres->health_point() : 0);
                                    int pvAttPerdus = pvAttAvant - (uAttApres ? uAttApres->health_point() : 0);
                                    std::string rapport = nomAtt + " inflige -" + std::to_string(pvDefPerdus) + " a " + nomDef;
                                    if (pvAttPerdus > 0) rapport += " (Riposte: -" + std::to_string(pvAttPerdus) + ")";
                                    if (!uDefApres) rapport += " [" + nomDef + " DETRUIT !]";
                                    addCombatLog(rapport, sf::Color(255, 100, 100));

                                    // Synchro réseau
                                    if (isMultiplayer) {
                                        sf::Packet pk;
                                        pk << static_cast<sf::Int32>(PacketType::ACTION_ATTACK) << _dragSourceX << _dragSourceY << targetI << targetJ;
                                        _network.sendData(pk);
                                    }
                                }
                            } else {
                                _popupMsg = "Cible hors de portee ou non valide !";
                                _showPopup = true;
                            }
                        } else {
                            // Deplacement
                            bool caseValide = false;
                            for (const auto& pos : _casesPossibles) {
                                if (pos.first == targetI && pos.second == targetJ) {
                                    caseValide = true;
                                    break;
                                }
                            }

                            if (caseValide) {
                                CmdDeplacement cmd = { _dragSourceX, _dragSourceY, targetI, targetJ };
                                if (_moteur.soumettreCommande(currentTurn, cmd) == ResultatAction::SUCCES) {
                                    if (isMultiplayer) {
                                        sf::Packet pk;
                                        pk << static_cast<sf::Int32>(PacketType::ACTION_MOVE) << static_cast<sf::Int32>(currentTurn) << _dragSourceX << _dragSourceY << targetI << targetJ;
                                        _network.sendData(pk);
                                    }
                                    _selectedCellX = targetI; _selectedCellY = targetJ;
                                    _unitSourceX = targetI; _unitSourceY = targetJ;
                                    _hasPreviewRotation = false;
                                }
                            } else {
                                // Feedback si on lâche l'unité trop loin
                                _popupMsg = "Deplacement impossible : hors de portee !";
                                _showPopup = true;
                            }
                        }
                    }
                    _isDragging = false;
                    _casesPossibles.clear();
                    _casesAttaquePossibles.clear();
                }
            }


            // Zoom et Déplacement (Clic droit)
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right && !ImGui::GetIO().WantCaptureMouse) {
                if (_isTargetingMove || _isTargetingAttack || _hasPreviewRotation || _isDragging) {
                    _isTargetingMove = false;
                    _isTargetingAttack = false;
                    _isDragging = false;
                    _hasPreviewRotation = false;
                    _casesPossibles.clear();
                    _casesAttaquePossibles.clear();
                    
                    addCombatLog("Action annulee.", sf::Color(150, 150, 150));
                } 
                else {
                    _isPanning = true; 
                    _lastMousePos = sf::Mouse::getPosition(_window);
                }
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right) {
                _isPanning = false;
            }
            if (event.type == sf::Event::MouseMoved && _isPanning) {
                sf::Vector2i newPos = sf::Mouse::getPosition(_window);
                _gameView.move(_window.mapPixelToCoords(_lastMousePos, _gameView) - _window.mapPixelToCoords(newPos, _gameView));
                _lastMousePos = newPos;
            }
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (!ImGui::GetIO().WantCaptureMouse) {
                    float factor = (event.mouseWheelScroll.delta > 0) ? 0.9f : 1.1f;
                    sf::Vector2i pixelPos(event.mouseWheelScroll.x, event.mouseWheelScroll.y);
                    sf::Vector2f worldBefore = _window.mapPixelToCoords(pixelPos, _gameView);
                    _gameView.zoom(factor);
                    _currentZoom *= factor; 
                    sf::Vector2f worldAfter = _window.mapPixelToCoords(pixelPos, _gameView);
                    _gameView.move(worldBefore - worldAfter);
                }
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
            case GameState::LOAD_MENU:      renderLoadMenu();      break;
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
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, _window.getSize().y));
    ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    // Titre
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); 
    float textWidth = ImGui::CalcTextSize("SPACE WARGAMES").x;
    ImGui::SetCursorPosX((_window.getSize().x - textWidth) * 0.5f);
    ImGui::SetCursorPosY(_window.getSize().y * 0.2f);
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "SPACE WARGAMES");
    ImGui::PopFont();

    // Boutons
    ImVec2 buttonSize(300, 50);
    float btnX = (_window.getSize().x - buttonSize.x) * 0.5f;
    float startY = _window.getSize().y * 0.4f;

    if (_previousState == GameState::IN_GAME) {
        ImGui::SetCursorPos(ImVec2(btnX, startY));
        if (ImGui::Button("REPRENDRE LE JEU", buttonSize)) {
            _currentState = GameState::IN_GAME;
        }
        startY += buttonSize.y + 20.0f;
    }

    ImGui::SetCursorPos(ImVec2(btnX, startY));
    if (ImGui::Button("JOUER", buttonSize)) _currentState = GameState::PLAY_MENU; 
    startY += buttonSize.y + 20;

    ImGui::SetCursorPos(ImVec2(btnX, startY));
    if (ImGui::Button("OPTIONS", buttonSize)) _currentState = GameState::OPTIONS;
    startY += buttonSize.y + 20;

    ImGui::SetCursorPos(ImVec2(btnX, startY));
    if (ImGui::Button("QUITTER", buttonSize)) _window.close();

    if (_showPopup) {
        ImGui::OpenPopup("Information");
        _showPopup = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Information", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImVec4 color = (_popupMsg.find("perdue") != std::string::npos || _popupMsg.find("Erreur") != std::string::npos) 
                       ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) 
                       : ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
                       
        ImGui::TextColored(color, "%s", _popupMsg.c_str());
        ImGui::Dummy(ImVec2(0, 15));
        
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 100) * 0.5f);
        if (ImGui::Button("OK", ImVec2(100, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void InterfaceManager::renderPlayMenu() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, _window.getSize().y));
    ImGui::Begin("Play Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    ImVec2 buttonSize(300, 50);
    float btnX = (_window.getSize().x - buttonSize.x) * 0.5f;
    float startY = _window.getSize().y * 0.35f;

    ImGui::SetCursorPos(ImVec2(btnX, startY));
    if (ImGui::Button("CHARGER UNE PARTIE", buttonSize)) {
        refreshSaveList();
        _currentState = GameState::LOAD_MENU;
    }

    ImGui::SetCursorPos(ImVec2(btnX, startY + 70));
    if (ImGui::Button("NOUVELLE PARTIE (Local)", buttonSize)) {
        _network.disconnect();
        _localPlayerIndex = 0;
        _currentState = GameState::FACTION_SELECT;
    }

    ImGui::SetCursorPos(ImVec2(btnX, startY + 140));
    if (ImGui::Button("MULTIJOUEUR (En Ligne)", buttonSize)) _currentState = GameState::MULTI_MENU;

    ImGui::SetCursorPos(ImVec2(btnX, startY + 210));
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
            
            if (_gameConfigLocked) ImGui::BeginDisabled();

            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "GENERAL");
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0.0f, 10.0f));

            static int diffIndex = 1;
            std::vector<std::string> difficulties = {"Settler", "Viceroy", "Emperor", "Deity"};
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::Text("Difficulty:");
            ImGui::TableSetColumnIndex(1); DrawArrowSelector("##diff", &diffIndex, difficulties);

            static int mapSizeIndex = 1;
            std::vector<std::string> mapSizes = {"Tiny (50x50)", "Standard (100x100)", "Huge (200x200)"};
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::Text("Map Size:");
            ImGui::TableSetColumnIndex(1); 
            if (DrawArrowSelector("##mapsize", &mapSizeIndex, mapSizes)) {
                if (mapSizeIndex == 0) { _rulesJson["taille_plateau"]["x"] = 50; _rulesJson["taille_plateau"]["y"] = 50; }
                if (mapSizeIndex == 1) { _rulesJson["taille_plateau"]["x"] = 100; _rulesJson["taille_plateau"]["y"] = 100; }
                if (mapSizeIndex == 2) { _rulesJson["taille_plateau"]["x"] = 200; _rulesJson["taille_plateau"]["y"] = 200; }
            }

            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0.0f, 20.0f));

            ImGui::TableNextRow(0);
            ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "CITY RULES");
            ImGui::TableNextRow(0); ImGui::TableSetColumnIndex(0); ImGui::Dummy(ImVec2(0.0f, 10.0f));

            if (_villesJson.contains("villes")) {
                for (auto& ville : _villesJson["villes"]) {
                    std::string nomVille = ville["nom"];
                    
                    if (ville.contains("cout_base") && !ville["cout_base"].empty()) {
                        ImGui::TableNextRow(0);
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("Cout %s:", nomVille.c_str());
                        ImGui::TableSetColumnIndex(1);

                        for (auto& it : ville["cout_base"].items()) {
                            int qte = it.value();
                            std::string label = "##" + nomVille + it.key();
                            ImGui::SetNextItemWidth(80.0f);
                            
                            if (ImGui::InputInt(label.c_str(), &qte, 1, 10)) {
                                ville["cout_base"][it.key()] = qte;
                            }
                            ImGui::SameLine();
                            ImGui::Text("%s", it.key().c_str());
                            ImGui::SameLine(0.0f, 10.0f);
                        }
                    }
                }
            }

            if (_gameConfigLocked) {
                ImGui::EndDisabled();
                ImGui::TableNextRow(0);
                ImGui::TableSetColumnIndex(0); 
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "\nGame settings locked during active gameplay.");
            }

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

    if (ImGui::Button("SAVE CONFIG", ImVec2(150, 40))) {
        saveConfig();
        _popupMsg = "Configuration sauvegardee avec succes !";
        _showPopup = true;
    }
    
    float rightButtonsX = menuSize.x - (150 * 2 + 20); 
    ImGui::SameLine(rightButtonsX);
    if (ImGui::Button("BACK", ImVec2(150, 40))) _currentState = _previousState;
    ImGui::SameLine(0.0f, -1.0f);
    
    if (_gameConfigLocked) ImGui::BeginDisabled();
    if (ImGui::Button("LAUNCH GAME", ImVec2(150, 40))) {
        saveConfig(); 
        _currentState = GameState::FACTION_SELECT; 
    }
    if (_gameConfigLocked) ImGui::EndDisabled();

    if (_showPopup) { 
        ImGui::OpenPopup("Confirmation"); 
        _showPopup = false; 
    }
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Confirmation", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", _popupMsg.c_str());
        ImGui::Dummy(ImVec2(0, 15));
        
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 100) * 0.5f);
        if (ImGui::Button("OK", ImVec2(100, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

void InterfaceManager::renderFactionSelect() {
    ImVec2 menuSize(1000, 500); 
    ImGui::SetNextWindowPos(ImVec2((_window.getSize().x - menuSize.x) * 0.5f, (_window.getSize().y - menuSize.y) * 0.5f));
    ImGui::SetNextWindowSize(menuSize);
    ImGui::Begin("Configuration des Joueurs", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "CONFIGURATION DES JOUEURS");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Columns(2, "FactionLayout", false);
    ImGui::SetColumnWidth(0, 600);

    bool isClient = (_network.getState() == NetworkState::CONNECTED && !_network.isHost());
    
    bool isMultiplayer = (_network.getState() == NetworkState::CONNECTED || _network.getState() == NetworkState::HOSTING);

    if (isClient) ImGui::BeginDisabled();

    if (!isMultiplayer) {
        if (ImGui::InputInt("Nombre de Joueurs", &_numPlayers)) {
            if (_numPlayers < 2) _numPlayers = 2;
            if (_numPlayers > 4) _numPlayers = 4;
        }
        while ((int)_connectedPlayers.size() < _numPlayers) _connectedPlayers.push_back({"Joueur " + std::to_string(_connectedPlayers.size() + 1), ""});
        while ((int)_connectedPlayers.size() > _numPlayers) _connectedPlayers.pop_back();
    } else {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Mode Multijoueur actif : %d Commandants", _numPlayers);
    }
    
    _playerFactions.resize(_numPlayers, "");
    ImGui::Dummy(ImVec2(0, 20));

    if (ImGui::BeginTable("PlayerTable", 2, ImGuiTableFlags_BordersInnerH)) {
        for (int i = 0; i < _numPlayers; ++i) {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::PushID(i);

            if (isMultiplayer) {
                ImGui::Text("%s", _connectedPlayers[i].name.c_str());
            } else {
                char buf[64];
                strncpy(buf, _connectedPlayers[i].name.c_str(), sizeof(buf));
                if (ImGui::InputText("##nom", buf, sizeof(buf))) _connectedPlayers[i].name = buf;
            }
            
            std::string comboLabel = _playerFactions[i].empty() ? "Choisir une faction..." : _playerFactions[i];
            if (ImGui::BeginCombo("##factionCombo", comboLabel.c_str())) {
                for (const auto& [nom, params] : _moteur.getFactionsAvailable()) {
                    bool isSelected = (_playerFactions[i] == nom);
                    if (ImGui::Selectable(nom.c_str(), isSelected)) {
                        _playerFactions[i] = nom;
                        sendLobbySync();
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    
    if (isClient) ImGui::EndDisabled();

    // ----------------------------------------------------
    // PANNEAU DE DROITE : Détails de la Faction
    // ----------------------------------------------------
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "INFORMATIONS DE FACTION");
    ImGui::Separator();
    
    std::string myFaction = "";
    int myIndex = -1;
    
    for (size_t i = 0; i < _connectedPlayers.size(); ++i) {
        if (_connectedPlayers[i].name == _playerNameBuffer) {
            myIndex = i;
            break;
        }
    }
    
    if (!isMultiplayer && myIndex == -1) myIndex = 0;

    if (myIndex >= 0 && myIndex < (int)_playerFactions.size()) {
        myFaction = _playerFactions[myIndex];
    }
    
    if (myFaction.empty()) {
        ImGui::TextDisabled("Veuillez choisir une faction\npour voir ses specifications.");
    } else {
        auto factions = _moteur.getFactionsAvailable();
        if (factions.count(myFaction)) {
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "%s", myFaction.c_str());
            ImGui::Dummy(ImVec2(0, 10));
            
            if (_factionTextures.count(myFaction)) {
                sf::Texture& tex = _factionTextures[myFaction];
                
                float maxImgHeight = 180.0f;
                float ratio = (float)tex.getSize().x / (float)tex.getSize().y;
                float imgWidth = maxImgHeight * ratio;
                
                if (imgWidth > 350.0f) {
                    imgWidth = 350.0f;
                    maxImgHeight = imgWidth / ratio;
                }
                
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (350.0f - imgWidth) * 0.5f);
                ImGui::Image(tex, sf::Vector2f(imgWidth, maxImgHeight));
                ImGui::Dummy(ImVec2(0, 10));
            }
            
            const auto& factionParams = factions[myFaction];
            if (factionParams.params.empty()) {
                ImGui::TextDisabled("Aucune specification particuliere.");
            } else {
                for (auto const& [statName, valeur] : factionParams.params) {
                    ImGui::BulletText("%s : %.2f", statName.c_str(), valeur);
                }
            }
        }
    }

    ImGui::Columns(1);
    
    bool allReady = true;
    for (const auto& f : _playerFactions) {
        if (f.empty()) allReady = false;
    }

    ImGui::SetCursorPosY(menuSize.y - 60);
    ImGui::Separator();
    
    if (ImGui::Button("RETOUR", ImVec2(150, 40))) {
        if (isMultiplayer) {
            if (isClient) {
                _network.disconnect();
                _connectedPlayers.clear();
                _hasSentName = false;
                _currentState = GameState::MULTI_MENU;
            } else {
                _currentState = GameState::HOST_LOBBY;
                sendLobbySync();
            }
        } else {
            _currentState = GameState::PLAY_MENU;
        }
    }
    
    ImGui::SameLine(menuSize.x - 165);
    
    if (isClient || !allReady) ImGui::BeginDisabled();
    if (ImGui::Button("SUIVANT", ImVec2(150, 40))) {
        _currentState = GameState::MAP_CONFIG;
        sendLobbySync();
    }
    if (isClient || !allReady) ImGui::EndDisabled();

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

    bool isClient = (_network.getState() == NetworkState::CONNECTED && !_network.isHost());
    if (isClient) ImGui::BeginDisabled();

    if (ImGui::BeginTable("MapSplit", 2)) {
        ImGui::TableSetupColumn("General", ImGuiTableColumnFlags_WidthFixed, 450.0f);
        ImGui::TableSetupColumn("Weights", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow(0);

        // Colonne gauche (paramètres généraux)
        ImGui::TableSetColumnIndex(0);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "GENERAL SETTINGS");

        // Taille de la carte
        std::vector<std::string> sizesNames;
        std::vector<std::pair<int, int>> sizesValues;

        if (_rulesJson.contains("tailles_disponibles")) {
            for (auto& taille : _rulesJson["tailles_disponibles"]) {
                std::string label = taille["nom"].get<std::string>() + " (" + 
                                    std::to_string(taille["x"].get<int>()) + "x" + 
                                    std::to_string(taille["y"].get<int>()) + ")";
                sizesNames.push_back(label);
                sizesValues.push_back({taille["x"].get<int>(), taille["y"].get<int>()});
            }
        } else {
            sizesNames = {"Standard (100x100)"};
            sizesValues = {{100, 100}};
        }

        int currentX = _rulesJson["taille_plateau"]["x"].get<int>();
        int sizeIdx = 0;
        for (size_t i = 0; i < sizesValues.size(); ++i) {
            if (sizesValues[i].first == currentX) {
                sizeIdx = i;
                break;
            }
        }

        ImGui::Text("Map Size:"); ImGui::SameLine(150);
        if (DrawArrowSelector("##msize", &sizeIdx, sizesNames)) {
            _rulesJson["taille_plateau"]["x"] = sizesValues[sizeIdx].first;
            _rulesJson["taille_plateau"]["y"] = sizesValues[sizeIdx].second;
            sendLobbySync();
        }

        ImGui::Dummy(ImVec2(0.0f, 5.0f));
        ImGui::Text("Condition de Victoire:"); ImGui::SameLine(150);
        
        std::vector<std::string> vicNames;
        for (const auto& vs : _moteur.getLogicConfig().getVictorySets()) {
            vicNames.push_back(vs.name);
        }
        
        if (DrawArrowSelector("##vicSelect", &_selectedVictoryIndex, vicNames)) {
            sendLobbySync(); 
        }

        ImGui::Text("Random Seed:"); ImGui::SameLine(150);
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::InputInt("##seed", &_mapSeed)) {
            sendLobbySync();
        }
        ImGui::SameLine();
        if (ImGui::Button("Aleatoire")) {
            _mapSeed = std::rand() % 1000000;
            sendLobbySync();
        }

        ImGui::Dummy(ImVec2(0, 20));
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "WORLD PRESETS");

        if (_espaceJson.contains("presets_world")) {
            for (auto& [presetName, weightsObj] : _espaceJson["presets_world"].items()) {
                if (ImGui::Button(presetName.c_str(), ImVec2(400, 30))) {
                    for (auto& [symbStr, weightVal] : weightsObj.items()) {
                        if (!symbStr.empty()) {
                            _customWeights[symbStr[0]] = weightVal.get<int>();
                        }
                    }
                    sendLobbySync();
                }
            }
        } else {
            ImGui::TextDisabled("Aucun preset 'presets_world' trouve dans la configuration.");
        }

        // Colonne droite (poids)
        ImGui::TableSetColumnIndex(1);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "TILE DISTRIBUTION (%%)");
        ImGui::Dummy(ImVec2(0, 10));

        std::string activePreset = "Custom";
        if (_espaceJson.contains("presets_world")) {
            for (auto& [presetName, weightsObj] : _espaceJson["presets_world"].items()) {
                bool matches = true;
                for (auto& [symbStr, weightVal] : weightsObj.items()) {
                    if (!symbStr.empty()) {
                        char s = symbStr[0];
                        if (_customWeights.find(s) == _customWeights.end() || _customWeights[s] != weightVal.get<int>()) {
                            matches = false;
                            break;
                        }
                    }
                }
                if (matches) {
                    activePreset = presetName;
                    break;
                }
            }
        }
        
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Preset selectionne : %s", activePreset.c_str());
        ImGui::Dummy(ImVec2(0, 10));

        if (_espaceJson.contains("tiles")) {
            for (auto& t : _espaceJson["tiles"]) {
                std::string nom = t["nom"];
                char symb = std::string(t["symbole"])[0];
                if (symb == '#') continue; 
                if (_customWeights.find(symb) == _customWeights.end()) _customWeights[symb] = t["gen"]["poids"];

                ImGui::Text("%s:", nom.c_str());
                if (ImGui::SliderInt((std::string("##w_") + symb).c_str(), &_customWeights[symb], 0, 100)) {
                    sendLobbySync();
                }
            }
        }

        ImGui::EndTable();
    }

    // Pied de page
    ImGui::SetCursorPosY(menuSize.y - 60);
    ImGui::Separator();
    if (ImGui::Button("BACK", ImVec2(150, 40))) {
        _currentState = GameState::FACTION_SELECT;
        sendLobbySync();
    }

    ImGui::SameLine(menuSize.x - 165);
    
    if (ImGui::Button("LAUNCH SECTOR", ImVec2(150, 40))) {
        // Lancement classique
        saveConfig();
        _moteur.setActiveVictorySet(_selectedVictoryIndex);
        initGame();
        
        if (_network.getState() == NetworkState::CONNECTED && _network.isHost()) {
            sf::Packet startPacket;
            startPacket << static_cast<sf::Int32>(PacketType::GAME_START) << static_cast<sf::Int32>(_mapSeed) << static_cast<sf::Int32>(_numPlayers) << static_cast<sf::Int32>(_selectedVictoryIndex); 
            startPacket << static_cast<sf::Int32>(_rulesJson["taille_plateau"]["x"]) << static_cast<sf::Int32>(_rulesJson["taille_plateau"]["y"]);
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

    if (isClient) ImGui::EndDisabled();

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

    // 1. Section HÔTE
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

    // 2. Section REJOINDRE
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

    ImGui::SetNextWindowPos(ImVec2((_window.getSize().x - 800) * 0.5f, (_window.getSize().y - 500) * 0.5f));
    ImGui::SetNextWindowSize(ImVec2(800, 500));
    ImGui::Begin("Salon Hote", nullptr, ImGuiWindowFlags_NoDecoration);

    // EN-TÊTE COMMUN
    if (_isHostingLoadedSave) {
        ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.8f, 1.0f), "SALON DE REPRISE (HOTE) - Sauvegarde : %s", _selectedSave.c_str());
    } else {
        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "SALON D'ATTENTE (HOTE)");
    }
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Text("Votre Nom de Commandant :");
    ImGui::SetNextItemWidth(250.0f);
    if (ImGui::InputText("##hostName", _playerNameBuffer, IM_ARRAYSIZE(_playerNameBuffer))) {
        if (!_connectedPlayers.empty()) {
            _connectedPlayers[0].name = _playerNameBuffer;
        }
    }
    ImGui::Dummy(ImVec2(0, 20));

    // Affichage selon le mode
    if (_isHostingLoadedSave) {
        
        // Mode reprise avec slots
        if (ImGui::BeginTable("SlotsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Empire (Sauvegarde)", ImGuiTableColumnFlags_WidthFixed, 250.0f);
            ImGui::TableSetupColumn("Joueur Connecte", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableHeadersRow();

            const auto& savePlayers = _moteur.getJoueurs();

            for (int i = 0; i < (int)savePlayers.size(); ++i) {
                ImGui::TableNextRow();
                
                // Colonne 1 : Infos de la sauvegarde
                ImGui::TableSetColumnIndex(0);
                std::string factionNom = savePlayers[i].getFaction() ? savePlayers[i].getFaction()->nom : "Inconnue";
                ImGui::Text("%s (%s)", savePlayers[i].getName().c_str(), factionNom.c_str());

                // Colonne 2 : Qui est dedans
                ImGui::TableSetColumnIndex(1);
                int occupantIdx = -1;
                for(int p = 0; p < (int)_playerToSlotMapping.size(); ++p) {
                    if(_playerToSlotMapping[p] == i) { occupantIdx = p; break; }
                }

                if (occupantIdx != -1 && occupantIdx < (int)_connectedPlayers.size()) {
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Occupe par : %s", _connectedPlayers[occupantIdx].name.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "[LIBRE]");
                }

                // Colonne 3 : Bouton pour prendre le slot
                ImGui::TableSetColumnIndex(2);
                if (occupantIdx == -1) {
                    if (ImGui::Button((std::string("Prendre##") + std::to_string(i)).c_str())) {
                        _playerToSlotMapping[0] = i; 
                        sendLobbySync(); 
                    }
                } else if (occupantIdx == 0 && i != 0) { 
                    if (ImGui::Button((std::string("Quitter##") + std::to_string(i)).c_str())) {
                        _playerToSlotMapping[0] = -1;
                        sendLobbySync();
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::Dummy(ImVec2(0, 20));

        // Vérifier que chaque joueur a pris un slot
        bool ready = true;
        for (size_t p = 0; p < _connectedPlayers.size(); ++p) {
            if (p < _playerToSlotMapping.size() && _playerToSlotMapping[p] == -1) ready = false;
        }

        if (!ready) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "En attente que chaque joueur choisisse un emplacement...");
        } else {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Pret a reprendre !");
            
            ImGui::SetCursorPos(ImVec2(300, 400));
            if (ImGui::Button("LANCER LA SAUVEGARDE", ImVec2(200, 50))) {
                _numPlayers = _connectedPlayers.size(); 
                
                sf::Packet loadPacket;
                loadPacket << static_cast<sf::Int32>(PacketType::LOAD_SAVE_STATE); 
                
                std::ifstream file("saves/" + _selectedSave);
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    loadPacket << buffer.str();
                    _network.sendData(loadPacket);
                }

                _currentState = GameState::IN_GAME; 
            }
        }

    } else {

        // Mode nouvelle partie avec liste classique
        ImGui::Text("Joueurs connectes (%d/%d) :", (int)_connectedPlayers.size(), _maxPlayersBuffer);
        for (size_t i = 0; i < _connectedPlayers.size(); ++i) {
            ImGui::BulletText("Joueur %d : %s", (int)i + 1, _connectedPlayers[i].name.c_str());
        }

        ImGui::Dummy(ImVec2(0, 30));

        if (_connectedPlayers.size() < 2) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "En attente d'adversaires sur le port 5000...");
        } else {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Pret a lancer !");
            
            ImGui::SetCursorPos(ImVec2(300, 400));
            if (ImGui::Button("CONFIGURER LA PARTIE", ImVec2(200, 50))) {
                _numPlayers = _connectedPlayers.size(); 
                _currentState = GameState::FACTION_SELECT; 
            }
        }
    }

    // PIED DE PAGE COMMUN
    ImGui::SetCursorPos(ImVec2(20, 440));
    if (ImGui::Button("ANNULER", ImVec2(150, 40))) {
        _network.disconnect();
        _connectedPlayers.clear();
        _isHostingLoadedSave = false;
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

    ImGui::Text("Votre Nom de Commandant :");
    ImGui::SetNextItemWidth(250.0f);
    
    bool isConnected = (_network.getState() == NetworkState::CONNECTED || _network.getState() == NetworkState::CONNECTING);
    if (isConnected) ImGui::BeginDisabled();
    
    ImGui::InputText("##clientName", _playerNameBuffer, IM_ARRAYSIZE(_playerNameBuffer));
    
    if (isConnected) ImGui::EndDisabled();
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
    }

    if (_network.getState() == NetworkState::CONNECTED) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Liaison etablie ! En attente du signal de l'hote...");
        
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Text("Equipage actuel (%d/2) :", (int)_connectedPlayers.size());
        
        for (size_t i = 0; i < _connectedPlayers.size(); ++i) {
            ImGui::BulletText("Commandant %d : %s", (int)i+1, _connectedPlayers[i].name.c_str());
        }

        if (!_hasSentName) {
            sf::Packet p;
            p << static_cast<sf::Int32>(PacketType::PLAYER_INFO) << std::string(_playerNameBuffer);
            _network.sendData(p);
            _hasSentName = true;
            if (_connectedPlayers.empty()) _connectedPlayers.push_back({std::string(_playerNameBuffer), ""}); 
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
                        if (_network.isHost()) {
                            if ((int)_connectedPlayers.size() < _maxPlayersBuffer) {
                                _connectedPlayers.push_back({clientName, ""});
                                sendLobbySync(); 
                            }
                        }
                    }
                    break;
                }

                // Info dans le lobby de connexion
                case PacketType::LOBBY_STATE: {
                    if (!_network.isHost()) {
                        sf::Int32 stateInt, count;
                        packet >> stateInt >> count;
                        GameState hostState = static_cast<GameState>(stateInt);
                        
                        // A. Le client suit l'hôte dans les menus
                        if (hostState == GameState::HOST_LOBBY) _currentState = GameState::JOIN_LOBBY;
                        else if (hostState == GameState::FACTION_SELECT) _currentState = GameState::FACTION_SELECT;
                        else if (hostState == GameState::MAP_CONFIG) _currentState = GameState::MAP_CONFIG;

                        // B. Synchroniser les joueurs
                        _connectedPlayers.clear();
                        _playerFactions.clear();
                        for (int i = 0; i < count; ++i) {
                            std::string n, f;
                            packet >> n >> f;
                            _connectedPlayers.push_back({n, f});
                            _playerFactions.push_back(f);
                        }
                        
                        // C. Synchroniser la config
                        sf::Int32 s_seed, s_numP, s_vicIdx;
                        if (packet >> s_seed >> s_numP >> s_vicIdx) {
                            _mapSeed = s_seed;
                            _numPlayers = s_numP;
                            _selectedVictoryIndex = s_vicIdx;
                        }

                        sf::Int32 sizeX, sizeY;
                        packet >> sizeX >> sizeY;
                        _rulesJson["taille_plateau"]["x"] = sizeX;
                        _rulesJson["taille_plateau"]["y"] = sizeY;

                        sf::Int32 wCount; 
                        packet >> wCount;
                        _customWeights.clear();
                        for(int i = 0; i < wCount; ++i) {
                            sf::Int32 symb, w; 
                            packet >> symb >> w;
                            _customWeights[static_cast<char>(symb)] = w;
                        }

                        // D. Synchroniser les slots
                        sf::Int32 isLoadedSave;
                        if (packet >> isLoadedSave) {
                            _isHostingLoadedSave = (isLoadedSave == 1);
                            
                            if (_isHostingLoadedSave) {
                                sf::Int32 mappingSize;
                                if (packet >> mappingSize) {
                                    _playerToSlotMapping.clear();
                                    for (int i = 0; i < mappingSize; ++i) {
                                        sf::Int32 slot;
                                        packet >> slot;
                                        _playerToSlotMapping.push_back(static_cast<int>(slot));
                                    }
                                }
                            }
                        }
                    }
                    break;
                }
                
                // L'Hôte lance la partie
                case PacketType::GAME_START: {
                    sf::Int32 vicIdx;
                    if (packet >> _mapSeed >> _numPlayers >> vicIdx) {
                        _moteur.setActiveVictorySet(vicIdx);

                        sf::Int32 sizeX, sizeY, weightsCount;
                        packet >> sizeX >> sizeY >> weightsCount;
                        
                        _rulesJson["taille_plateau"]["x"] = sizeX;
                        _rulesJson["taille_plateau"]["y"] = sizeY;
                        saveConfig();

                        _customWeights.clear();
                        for (int i = 0; i < weightsCount; ++i) {
                            sf::Int32 symb, w;
                            packet >> symb >> w;
                            _customWeights[static_cast<char>(symb)] = w;
                        }

                        _connectedPlayers.clear();
                        _playerFactions.clear();
                        
                        std::vector<std::string> noms;
                        
                        // On lit les infos de tous les joueurs
                        for (int i = 0; i < _numPlayers; ++i) {
                            std::string pName, pFact;
                            packet >> pName >> pFact;
                            
                            _connectedPlayers.push_back({pName, pFact});
                            _playerFactions.push_back(pFact);
                            noms.push_back(pName);
                            
                            // Le client identifie quel est son numéro de joueur
                            if (pName == std::string(_playerNameBuffer)) {
                                _localPlayerIndex = i;
                            }
                        }
                        
                        std::srand(_mapSeed);
                        
                        // Initialisation avec la nouvelle architecture
                        _moteur.overrideWorldWeights(_customWeights);
                        _moteur.initGame(_mapSeed, noms, _playerFactions);
                        
                        if (_localPlayerIndex < (int)_moteur.getJoueurs().size() && !_moteur.getJoueurs()[_localPlayerIndex].getCities().empty()) {
                            City* cap = _moteur.getJoueurs()[_localPlayerIndex].getCities().front();
                            float R = _tileSize / 2.0f;
                            float W = std::sqrt(3.0f) * R;
                            _gameView.setCenter(W * cap->getY() + W * 0.5f * (std::abs(cap->getX()) % 2), 1.5f * R * cap->getX());
                        }
                        
                        _currentState = GameState::IN_GAME;
                    }
                    break;
                }

                // Le joueur à chargé une save
                case PacketType::LOAD_SAVE_STATE: {
                    std::string jsonContent;
                    if (packet >> jsonContent) {
                        std::cout << "[RESEAU] Sauvegarde recue de l'hote. Taille : " << jsonContent.size() << " octets." << std::endl;

                        std::filesystem::create_directories("saves");
                        std::string tempSavePath = "saves/client_sync_save.json";

                        std::ofstream outFile(tempSavePath);
                        if (outFile.is_open()) {
                            outFile << jsonContent;
                            outFile.close();

                            if (SaveManager::loadGame(tempSavePath, _moteur)) {
                                std::cout << "[CLIENT] Partie synchronisee avec succes !" << std::endl;
                                
                                _currentState = GameState::IN_GAME;
                            } else {
                                std::cerr << "[CLIENT] Erreur: Impossible de lire la sauvegarde reseau." << std::endl;
                                _popupMsg = "Erreur de synchronisation avec l'hote !";
                                _showPopup = true;
                            }
                        } else {
                            std::cerr << "[CLIENT] Erreur: Impossible d'ecrire le fichier temporaire." << std::endl;
                        }
                    }
                    break;
                }

                // L'autre joueur a passé son tour
                case PacketType::END_TURN: {
                    _moteur.passerTour();
                    _hasSelection = false;
                    // La caméra reste où le joueur l'a laissée (pas de recentrage)
                    break;
                }

                // L'autre joueur a bougé une unité
                case PacketType::ACTION_MOVE: {
                    sf::Int32 pIdx;
                    int xSrc, ySrc, xDest, yDest;
                    if (packet >> pIdx >> xSrc >> ySrc >> xDest >> yDest) {
                        CmdDeplacement cmd = { xSrc, ySrc, xDest, yDest };
                        _moteur.soumettreCommande(pIdx, cmd);
                    }
                    break;
                }

                // L'autre joueur a recruté une unité
                case PacketType::ACTION_RECRUIT: {
                    sf::Int32 pIdx; int x, y; std::string nomUnite;
                    if (packet >> pIdx >> x >> y >> nomUnite) {
                        CmdRecrutement cmd = { x, y, nomUnite };
                        _moteur.soumettreCommande(pIdx, cmd);
                    }
                    break;
                }

                // L'autre joueur a attaqué
                case PacketType::ACTION_ATTACK: {
                    int xSrc, ySrc, xDest, yDest;
                    if (packet >> xSrc >> ySrc >> xDest >> yDest) {
                        int turn = _moteur.getCurrentPlayerTurn();
                        
                        // Etat avant l'attaque par le réseau
                        Unite* uAtt = _moteur.getPlateau()->getUnite(xSrc, ySrc);
                        Unite* uDef = _moteur.getPlateau()->getUnite(xDest, yDest);
                        std::string nomAtt = uAtt ? uAtt->name() : "Ennemi";
                        std::string nomDef = uDef ? uDef->name() : "Allie";
                        int pvDefAvant = uDef ? uDef->health_point() : 0;
                        int pvAttAvant = uAtt ? uAtt->health_point() : 0;

                        CmdAttaque cmd = { xSrc, ySrc, xDest, yDest };
                        if (_moteur.soumettreCommande(turn, cmd) == ResultatAction::SUCCES) {
                            
                            // Etat après l'attaque
                            Unite* uDefApres = _moteur.getPlateau()->getUnite(xDest, yDest);
                            Unite* uAttApres = _moteur.getPlateau()->getUnite(xSrc, ySrc);
                            
                            int pvDefPerdus = pvDefAvant - (uDefApres ? uDefApres->health_point() : 0);
                            int pvAttPerdus = pvAttAvant - (uAttApres ? uAttApres->health_point() : 0);

                            // Combat Log pour le joueur qui subit l'attaque
                            std::string rapport = nomAtt + " inflige -" + std::to_string(pvDefPerdus) + " a " + nomDef;
                            if (pvAttPerdus > 0) rapport += " (Riposte: -" + std::to_string(pvAttPerdus) + ")";
                            if (!uDefApres) rapport += " [" + nomDef + " DETRUIT !]";
                            
                            addCombatLog(rapport, sf::Color(255, 150, 50));
                        }
                    }
                    break;
                }

                // L'autre joueur a détruit une de ses unités
                case PacketType::ACTION_DESTROY_UNIT: {
                    sf::Int32 pIdx; 
                    int x, y;
                    if (packet >> pIdx >> x >> y) {
                        CmdDetruireUnite cmd = { x, y };
                        if (_moteur.soumettreCommande(pIdx, cmd) == ResultatAction::SUCCES) {
                            addCombatLog("Une unite ennemie s'est sabordee.", sf::Color(150, 150, 150));
                        }
                    }
                    break;
                }

                // L'autre joueur a pivoté
                case PacketType::ACTION_ROTATE: {
                    sf::Int32 pIdx; int x, y; sf::Int32 dirInt;
                    if (packet >> pIdx >> x >> y >> dirInt) {
                        CmdRotation cmd = { x, y, static_cast<direction>(dirInt) };
                        _moteur.soumettreCommande(pIdx, cmd);
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
                        CmdConstruction cmd = { x, y, batNom };
                        if (_moteur.soumettreCommande(senderIdx, cmd) == ResultatAction::SUCCES) {
                            sf::Packet p; p << static_cast<sf::Int32>(PacketType::SYNC_BUILD) << senderIdx << x << y << batNom;
                            _network.sendData(p);
                        }
                    }
                    break;
                }

                case PacketType::ACTION_BUILD_CITY: {
                    sf::Int32 pIdx; int x, y; std::string nomVille;
                    if (packet >> pIdx >> x >> y >> nomVille) {
                        CmdFonderVille cmd = { x, y, nomVille };
                        _moteur.soumettreCommande(pIdx, cmd);
                    }
                    break;
                }

                case PacketType::ACTION_UPGRADE_CITY: {
                    sf::Int32 pIdx; 
                    int x, y;
                    
                    if (packet >> pIdx >> x >> y) {
                        CmdAmeliorer cmd = { x, y };
                        _moteur.soumettreCommande(pIdx, cmd);
                    }
                    break;
                }

                case PacketType::ACTION_BUY_TILE: {
                    sf::Int32 pIdx;
                    int x, y;
                    
                    if (packet >> pIdx >> x >> y) {
                        CmdAcheterCase cmd = { x, y };
                        _moteur.soumettreCommande(pIdx, cmd);
                    }
                    break;
                }

                case PacketType::SYNC_BUILD: {
                    sf::Int32 targetIdx; int x, y; std::string batNom;
                    if (!_network.isHost() && (packet >> targetIdx >> x >> y >> batNom)) {
                        CmdConstruction cmd = { x, y, batNom };
                        _moteur.soumettreCommande(targetIdx, cmd);
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

    if (_network.getState() == NetworkState::DISCONNECTED) {
        std::cerr << "Deconnexion detectee dans la boucle reseau !" << std::endl;
        _connectedPlayers.clear();
        _hasSentName = false;
        _popupMsg = "Connexion reseau perdue avec l'hote / adversaire !";
        _showPopup = true;
        
        if (_currentState == GameState::IN_GAME || _currentState == GameState::HOST_LOBBY || 
            _currentState == GameState::JOIN_LOBBY || _currentState == GameState::FACTION_SELECT || 
            _currentState == GameState::MAP_CONFIG) {
            _previousState = GameState::MENU;
            _currentState = GameState::MENU;

            if (!_network.isHost()) { 
                if (std::filesystem::exists("saves/last_save.json")) {
                    std::filesystem::remove("saves/last_save.json");
                }
            }
        }
    } else if (_network.isHost() && _network.getState() == NetworkState::HOSTING && _connectedPlayers.size() > 1) {
        _connectedPlayers.resize(1);
        _popupMsg = "L'adversaire s'est deconnecte.";
        _showPopup = true;
        if (_currentState == GameState::FACTION_SELECT || _currentState == GameState::MAP_CONFIG) {
            _currentState = GameState::HOST_LOBBY;
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

    // -- DETECTION CHANGEMENT DE TOUR ---
    if (isMyTurn && !_wasMyTurn) {
        _turnNotificationTimer = 2.5f;
    }
    _wasMyTurn = isMyTurn;

    float R = _tileSize / 2.0f;
    float W = std::sqrt(3.0f) * R;

    // OPTIMISATION : View Culling
    sf::Vector2f center = _gameView.getCenter();
    sf::Vector2f size = _gameView.getSize();
    int startRow = std::max(0, (int)((center.y - size.y/2) / (1.5f * R)) - 1);
    int endRow = std::min(_moteur.getPlateau()->getRows(), (int)((center.y + size.y/2) / (1.5f * R)) + 2);
    int startCol = std::max(0, (int)((center.x - size.x/2) / W) - 1);
    int endCol = std::min(_moteur.getPlateau()->getCols(), (int)((center.x + size.x/2) / W) + 2);

    // Si la caméra voit une zone > 3x l'écran normal, on passe en mode simplifié
    float zoomRatio = _gameView.getSize().x / _window.getSize().x;
    bool drawDetails = (zoomRatio < 3.0f);
    bool drawUnits   = (zoomRatio < 5.0f);

    // ==========================================================
    // PRÉ-CALCUL DES OFFSETS HEXAGONAUX (TriangleFan → Triangles)
    // 6 triangles par hex = 18 vertices en mode sf::Triangles
    // ==========================================================
    static const float PI = 3.14159265f;
    sf::Vector2f hexOffsets[7];
    for (int pt = 0; pt <= 6; ++pt) {
        float angle = (PI / 180.0f) * (60.0f * (pt % 6) - 30.0f);
        hexOffsets[pt] = sf::Vector2f(std::cos(angle), std::sin(angle));
    }

    // ==========================================================
    // BATCHING : On accumule tous les vertices par texture
    // ==========================================================
    std::map<char, sf::VertexArray> batches;
    for (auto& [sym, tex] : _textures) {
        batches[sym] = sf::VertexArray(sf::Triangles);
    }

    // VertexArray pour le brouillard (triangles simples, pas de texture)
    sf::VertexArray moveBatch(sf::Triangles);
    sf::VertexArray attackBatch(sf::Triangles);
    sf::VertexArray territoryBatch(sf::Quads);
    sf::VertexArray buyBatch(sf::Triangles);
    sf::VertexArray fogBlackBatch(sf::Triangles);  // Inexploré (opaque)
    sf::VertexArray shroudBatch(sf::Triangles);    // Exploré mais hors de vue (semi-transparent)
    sf::VertexArray selectBatch(sf::Triangles);

    // Ensemble des cases possibles en set pour accès O(1)
    std::set<std::pair<int,int>> casesSet(_casesPossibles.begin(), _casesPossibles.end());
    std::set<std::pair<int,int>> casesAttaqueSet(_casesAttaquePossibles.begin(), _casesAttaquePossibles.end());

    const bool joueurValide = viewIndex < (int)_moteur.getJoueurs().size();

    // ==========================================================
    // BOUCLE DE COLLECTE DES VERTICES (PAS DE DRAW)
    // ==========================================================
    for (int i = startRow; i < endRow; ++i) {
        for (int j = startCol; j < endCol; ++j) {
            float posX = W * j + W * 0.5f * (std::abs(i) % 2);
            float posY = 1.5f * R * i;

            bool discovered = joueurValide ? _moteur.getJoueurs()[viewIndex].estDecouvert(i, j) : true;
            bool visible    = joueurValide ? _moteur.getJoueurs()[viewIndex].estVisible(i, j) : true;

            // --- BROUILLARD NOIR (Inexploré) ---
            if (!discovered) {
                sf::Color fogColor(5, 5, 15, 255);
                for (int tri = 0; tri < 6; ++tri) {
                    sf::Vertex v0, v1, v2;
                    v0.position = {posX, posY}; v0.color = fogColor;
                    v1.position = {posX + R * hexOffsets[tri].x, posY + R * hexOffsets[tri].y}; v1.color = fogColor;
                    v2.position = {posX + R * hexOffsets[(tri+1)%6].x, posY + R * hexOffsets[(tri+1)%6].y}; v2.color = fogColor;
                    fogBlackBatch.append(v0); fogBlackBatch.append(v1); fogBlackBatch.append(v2);
                }
                continue;
            }

            // --- BROUILLARD GRIS / SHROUD (Exploré mais hors de vue) ---
            // Ajouté APRÈS les tuiles texturées dans l'ordre de rendu
            if (!visible) {
                sf::Color shroudColor(0, 0, 0, 100);
                for (int tri = 0; tri < 6; ++tri) {
                    sf::Vertex v0, v1, v2;
                    v0.position = {posX, posY}; v0.color = shroudColor;
                    v1.position = {posX + R * hexOffsets[tri].x, posY + R * hexOffsets[tri].y}; v1.color = shroudColor;
                    v2.position = {posX + R * hexOffsets[(tri+1)%6].x, posY + R * hexOffsets[(tri+1)%6].y}; v2.color = shroudColor;
                    shroudBatch.append(v0); shroudBatch.append(v1); shroudBatch.append(v2);
                }
            }

            // --- TUILE TEXTURÉE ---
            const hexa* tile = _moteur.getPlateau()->getCell(i, j);
            if (tile && batches.count(tile->getSymbole())) {
                char sym = tile->getSymbole();
                sf::Texture& tex = _textures[sym];
                sf::Vector2f texCenter(tex.getSize().x / 2.0f, tex.getSize().y / 2.0f);
                float tw = tex.getSize().x / 2.0f;
                float th = tex.getSize().y / 2.0f;

                // 6 triangles pour l'hexagone texturé
                for (int tri = 0; tri < 6; ++tri) {
                    sf::Vertex v0, v1, v2;
                    v0.position  = {posX, posY};
                    v0.texCoords = texCenter;
                    v0.color = sf::Color::White;

                    v1.position  = {posX + R * hexOffsets[tri].x, posY + R * hexOffsets[tri].y};
                    v1.texCoords = {texCenter.x + hexOffsets[tri].x * tw, texCenter.y + hexOffsets[tri].y * th};
                    v1.color = sf::Color::White;

                    v2.position  = {posX + R * hexOffsets[(tri+1)%6].x, posY + R * hexOffsets[(tri+1)%6].y};
                    v2.texCoords = {texCenter.x + hexOffsets[(tri+1)%6].x * tw, texCenter.y + hexOffsets[(tri+1)%6].y * th};
                    v2.color = sf::Color::White;

                    batches[sym].append(v0);
                    batches[sym].append(v1);
                    batches[sym].append(v2);
                }
            }

            // --- SÉLECTION ---
            if (_hasSelection && _selectedCellX == i && _selectedCellY == j) {
                sf::Color selColor(255, 255, 255, 80);
                for (int tri = 0; tri < 6; ++tri) {
                    sf::Vertex v0, v1, v2;
                    v0.position = {posX, posY};         v0.color = selColor;
                    v1.position = {posX + R * hexOffsets[tri].x, posY + R * hexOffsets[tri].y}; v1.color = selColor;
                    v2.position = {posX + R * hexOffsets[(tri+1)%6].x, posY + R * hexOffsets[(tri+1)%6].y}; v2.color = selColor;
                    selectBatch.append(v0); selectBatch.append(v1); selectBatch.append(v2);
                }
            }

            // --- SURBRILLANCES DE DRAG & DROP (Mouvement/Attaque) ---
            if (_isDragging && i == _dragSourceX && j == _dragSourceY) {
                // On peut optionnellement griser la case source
            }

            // Mouvement Possible (Jaune)
            if ((_isDragging || _isTargetingMove) && casesSet.count({i, j})) {
                sf::Color moveCol(255, 255, 0, 70);
                for (int tri = 0; tri < 6; ++tri) {
                    sf::Vertex v0, v1, v2;
                    v0.position = {posX, posY}; v0.color = moveCol;
                    v1.position = {posX + R * hexOffsets[tri].x, posY + R * hexOffsets[tri].y}; v1.color = moveCol;
                    v2.position = {posX + R * hexOffsets[(tri+1)%6].x, posY + R * hexOffsets[(tri+1)%6].y}; v2.color = moveCol;
                    moveBatch.append(v0); moveBatch.append(v1); moveBatch.append(v2);
                }
            }

            // Attaque Possible (Rouge)
            if ((_isDragging || _isTargetingAttack) && casesAttaqueSet.count({i, j})) { 
                sf::Color attCol(255, 0, 0, 80);
                for (int tri = 0; tri < 6; ++tri) {
                    sf::Vertex v0, v1, v2;
                    v0.position = {posX, posY}; v0.color = attCol;
                    v1.position = {posX + R * hexOffsets[tri].x, posY + R * hexOffsets[tri].y}; v1.color = attCol;
                    v2.position = {posX + R * hexOffsets[(tri+1)%6].x, posY + R * hexOffsets[(tri+1)%6].y}; v2.color = attCol;
                    attackBatch.append(v0); attackBatch.append(v1); attackBatch.append(v2);
                }
            }

            // --- TRACÉ DES TERRITOIRES ---
            static const sf::Color playerColors[] = {
                sf::Color(80, 180, 255, 255), sf::Color(255, 80, 80, 255), sf::Color(80, 255, 80, 255), sf::Color(255, 200, 0, 255)
            };

            for (int pIdx = 0; pIdx < (int)_moteur.getJoueurs().size(); ++pIdx) {
                auto territoireVec = _moteur.getTerritoireJoueur(pIdx);
                if (territoireVec.empty()) continue;

                std::set<std::pair<int, int>> territoireMap(territoireVec.begin(), territoireVec.end());
                
                const int neighEven[6][2] = {{-1, 0}, {-1, 1}, {0, 1}, {1, 0}, {0, -1}, {-1, -1}};
                const int neighOdd[6][2]  = {{-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}};

                for (const auto& tuile : territoireVec) {
                    int i = tuile.first;
                    int j = tuile.second;
                    
                    if (i < startRow - 2 || i > endRow + 2 || j < startCol - 2 || j > endCol + 2) continue;

                    if (joueurValide && !_moteur.getJoueurs()[viewIndex].estDecouvert(i, j)) continue;

                    float posX = W * j + W * 0.5f * (std::abs(i) % 2);
                    float posY = 1.5f * R * i;

                    const auto& neigh = (std::abs(i) % 2 == 0) ? neighEven : neighOdd;

                    for (int side = 0; side < 6; ++side) {
                        int ni = i + neigh[side][0];
                        int nj = j + neigh[side][1];

                        if (territoireMap.find({ni, nj}) == territoireMap.end()) {
                            sf::Color borderCol = playerColors[pIdx % 4];

                            sf::Vector2f p1(posX + R * std::cos(PI/180.0f * (60.0f * side - 30.0f)), posY + R * std::sin(PI/180.0f * (60.0f * side - 30.0f)));
                            sf::Vector2f p2(posX + R * std::cos(PI/180.0f * (60.0f * ((side+1)%6) - 30.0f)), posY + R * std::sin(PI/180.0f * (60.0f * ((side+1)%6) - 30.0f)));

                            sf::Vector2f dir = p2 - p1;
                            float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
                            dir.x /= len; dir.y /= len;
                            sf::Vector2f normal(-dir.y, dir.x);
                            
                            float thickness = 3.0f;
                            float extension = thickness / 1.732f;
                            p1 -= dir * extension;
                            p2 += dir * extension;

                            sf::Vertex q1, q2, q3, q4;
                            q1.position = p1 - normal * (thickness / 2.0f); q1.color = borderCol;
                            q2.position = p2 - normal * (thickness / 2.0f); q2.color = borderCol;
                            q3.position = p2 + normal * (thickness / 2.0f); q3.color = borderCol;
                            q4.position = p1 + normal * (thickness / 2.0f); q4.color = borderCol;

                            territoryBatch.append(q1); territoryBatch.append(q2);
                            territoryBatch.append(q3); territoryBatch.append(q4);
                        }
                    }
                }
            }

            // --- CASES ACHETABLES (S'il y a une sélection de territoire en cours ou simplement visible) ---
            if (joueurValide) {
                if (_moteur.peutAcheterTerritoire(viewIndex, i, j)) {
                    sf::Color buyColor(0, 255, 100, 30);
                    for (int tri = 0; tri < 6; ++tri) {
                        sf::Vertex v0, v1, v2;
                        v0.position = {posX, posY}; v0.color = buyColor;
                        v1.position = {posX + R * hexOffsets[tri].x, posY + R * hexOffsets[tri].y}; v1.color = buyColor;
                        v2.position = {posX + R * hexOffsets[(tri+1)%6].x, posY + R * hexOffsets[(tri+1)%6].y}; v2.color = buyColor;
                        buyBatch.append(v0); buyBatch.append(v1); buyBatch.append(v2);
                    }
                }
            }
        }
    }

    // ==========================================================
    // DRAW CALLS BATCHÉS (1 par type de texture + fog + overlays)
    // ==========================================================
    // 1. D'abord le fog noir opaque (fond pour les zones inexplorées)
    if (fogBlackBatch.getVertexCount() > 0) _window.draw(fogBlackBatch);
    // 2. Ensuite les tuiles texturées (par-dessus le fond noir)
    for (auto& [sym, va] : batches) {
        if (va.getVertexCount() > 0)
            _window.draw(va, &_textures[sym]);
    }
    // 3. Puis le shroud semi-transparent (assombrit les tuiles découvertes mais hors de vue)
    if (shroudBatch.getVertexCount() > 0) _window.draw(shroudBatch);
    if (buyBatch.getVertexCount() > 0) _window.draw(buyBatch);
    if (moveBatch.getVertexCount() > 0) _window.draw(moveBatch);
    if (attackBatch.getVertexCount() > 0) _window.draw(attackBatch);
    if (selectBatch.getVertexCount() > 0) _window.draw(selectBatch);
    if (territoryBatch.getVertexCount() > 0) {
        _window.draw(territoryBatch);
    }

    // ==========================================================
    // DÉTAILS : Villes, Bâtiments, Unités (uniquement si zoomé)
    // ==========================================================
    if (drawDetails) {
        for (int i = startRow; i < endRow; ++i) {
            for (int j = startCol; j < endCol; ++j) {
                bool visible = joueurValide ? _moteur.getJoueurs()[viewIndex].estDecouvert(i, j) : true;
                if (!visible) continue;

                float posX = W * j + W * 0.5f * (std::abs(i) % 2);
                float posY = 1.5f * R * i;

                const hexa* tile = _moteur.getPlateau()->getCell(i, j);
                const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(tile);
                if (tc) {
                    if (tc->getCity()) {
                        std::string cNom = tc->getCity()->getNom();
                        if (_cityTextures.count(cNom)) {
                            sf::Sprite citySpr;
                            citySpr.setTexture(_cityTextures[cNom]);
                            citySpr.setOrigin(citySpr.getLocalBounds().width / 2.0f, citySpr.getLocalBounds().height / 2.0f);
                            citySpr.setPosition(posX, posY - 10.0f);
                            citySpr.setColor(tc->getCity()->estCapitale() ? sf::Color(255, 215, 0) : sf::Color(200, 230, 255));
                            _window.draw(citySpr);
                        }
                    }
                    if (tc->getBatimentSpecial()) {
                        std::string bNom = tc->getBatimentSpecial()->getName();
                        if (_buildingTextures.count(bNom)) {
                            sf::Sprite bSpr;
                            bSpr.setTexture(_buildingTextures[bNom]);
                            bSpr.setOrigin(bSpr.getLocalBounds().width / 2.0f, bSpr.getLocalBounds().height / 2.0f);
                            bSpr.setPosition(posX, posY + 10.0f);
                            
                            sf::FloatRect bounds = bSpr.getLocalBounds();
                            float scale = (_tileSize * 0.6f) / std::max(bounds.width, bounds.height);
                            bSpr.setScale(scale, scale);

                            _window.draw(bSpr);
                        }
                    }
                }
            }
        }
    }

    // === DESSIN DES UNITÉS ===
    if (drawUnits) {
        // Cercle pour fond/couleur du joueur
        sf::CircleShape unitDot(R * 0.35f);
        unitDot.setOrigin(R * 0.35f, R * 0.35f);

        for (int i = startRow; i < endRow; ++i) {
            for (int j = startCol; j < endCol; ++j) {
                
                bool visible = joueurValide ? _moteur.getJoueurs()[viewIndex].estVisible(i, j) : true;
                Unite* u = _moteur.getPlateau()->getUnite(i, j);
                if (!u) continue;

                int propIdx = _moteur.getProprietaireUnite(i, j);
                bool isMine = (propIdx == viewIndex);
                
                // On voit toujours ses propres unités, même hors vision
                if (!visible && !isMine) continue;

                float posX = W * j + W * 0.5f * (std::abs(i) % 2);
                float posY = 1.5f * R * i;

                // Couleur selon le propriétaire
                sf::Color unitColor(180, 180, 180);
                if (propIdx != -1) {
                    static const sf::Color playerColors[] = {
                        sf::Color(80, 180, 255), sf::Color(255, 80, 80), sf::Color(80, 255, 80), sf::Color(255, 200, 0)
                    };
                    unitColor = playerColors[propIdx % 4];
                }
                
                // Dessin de fond de faction (halo)
                unitDot.setFillColor(unitColor);
                unitDot.setOutlineThickness(1.0f);
                unitDot.setOutlineColor(sf::Color::Black);
                unitDot.setPosition(posX, posY);
                _window.draw(unitDot);

                // Sprite par dessus
                if (_unitTextures.count(u->name())) {
                    sf::Sprite uSpr;
                    uSpr.setTexture(_unitTextures[u->name()]);
                    uSpr.setOrigin(uSpr.getLocalBounds().width / 2.0f, uSpr.getLocalBounds().height / 2.0f);
                    uSpr.setPosition(posX, posY);
                    
                    // Ajustement de la taille : l'unité doit faire environ 80% de la taille d'une case
                    sf::FloatRect bounds = uSpr.getLocalBounds();
                    float scale = (_tileSize * 0.8f) / std::max(bounds.width, bounds.height);
                    uSpr.setScale(scale, scale);

                    // Orientation
                    if (_hasSelection && _selectedCellX == i && _selectedCellY == j && _hasPreviewRotation) {
                        uSpr.setRotation(getRotationAngle(_previewDirection));
                    } else {
                        uSpr.setRotation(getRotationAngle(u->regarde()));
                    }

                    // --- DESSIN DU CÔNE DE VISION (Faisceau lumineux) ---
                    if (_hasSelection && _selectedCellX == i && _selectedCellY == j) {
                        direction d = (_hasPreviewRotation) ? _previewDirection : u->regarde();
                        float angleDeg = getRotationAngle(d); 
                        float angleRad = angleDeg * PI / 180.0f;
                        
                        float halfFov = (u->fov() / 2.0f) * PI / 180.0f; 
                        float coneLength = u->visionRange() * W * 1.5f;
                        
                        sf::VertexArray cone(sf::TriangleFan, 4);
                        sf::Color coneColor(0, 200, 255, 90);
                        sf::Color coneEnd(0, 200, 255, 0);
                        
                        cone[0].position = sf::Vector2f(posX, posY);
                        cone[0].color = coneColor;
                        
                        cone[1].position = sf::Vector2f(posX + coneLength * std::cos(angleRad - halfFov), posY + coneLength * std::sin(angleRad - halfFov));
                        cone[1].color = coneEnd;
                        
                        cone[2].position = sf::Vector2f(posX + coneLength * std::cos(angleRad), posY + coneLength * std::sin(angleRad));
                        cone[2].color = coneEnd;

                        cone[3].position = sf::Vector2f(posX + coneLength * std::cos(angleRad + halfFov), posY + coneLength * std::sin(angleRad + halfFov));
                        cone[3].color = coneEnd;
                        
                        sf::RenderStates states;
                        states.blendMode = sf::BlendAdd;
                        _window.draw(cone, states);
                    }

                    _window.draw(uSpr);

                    // DESSIN DE LA BARRE DE HP
                    float hpPercent = std::max(0.0f, (float)u->health_point() / (float)u->health_point_max());
                    float barWidth = R * 0.9f;
                    float barHeight = 6.0f;
                    
                    // Fond de la jauge
                    sf::RectangleShape bgBar(sf::Vector2f(barWidth, barHeight));
                    bgBar.setOrigin(barWidth / 2.0f, barHeight / 2.0f);
                    bgBar.setPosition(posX, posY - R * 0.65f); 
                    bgBar.setFillColor(sf::Color(40, 10, 10, 220));
                    bgBar.setOutlineThickness(1.0f);
                    bgBar.setOutlineColor(sf::Color::Black);
                    _window.draw(bgBar);

                    // Jauge de vie restante
                    if (hpPercent > 0.0f) {
                        sf::RectangleShape hpBar(sf::Vector2f(barWidth * hpPercent, barHeight));
                        hpBar.setPosition(posX - barWidth / 2.0f, posY - R * 0.65f - barHeight / 2.0f);
                        
                        sf::Color hpColor = sf::Color(50, 255, 50);
                        if (hpPercent <= 0.5f) hpColor = sf::Color(255, 200, 0);
                        if (hpPercent <= 0.25f) hpColor = sf::Color(255, 50, 50);
                        
                        hpBar.setFillColor(hpColor);
                        _window.draw(hpBar);
                    }
                }
            }
        }
    }

    // === CONTOURS DE SÉLECTION (outline dessiné après les fills) ===
    if (_hasSelection) {
        float posX = W * _selectedCellY + W * 0.5f * (std::abs(_selectedCellX) % 2);
        float posY = 1.5f * R * _selectedCellX;
        sf::ConvexShape hexSel(6);
        for (int pt = 0; pt < 6; ++pt) {
            hexSel.setPoint(pt, {R * hexOffsets[pt].x, R * hexOffsets[pt].y});
        }
        hexSel.setPosition(posX, posY);
        hexSel.setFillColor(sf::Color::Transparent);
        hexSel.setOutlineColor(sf::Color::White);
        hexSel.setOutlineThickness(2.0f);
        _window.draw(hexSel);
    }

    // === DESSIN DE L'UNITÉ DRAGUÉE ===
    if (_isDragging) {
        Unite* u = _moteur.getPlateau()->getUnite(_dragSourceX, _dragSourceY);
        if (u && _unitTextures.count(u->name())) {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(_window);
            sf::Vector2f worldPos = _window.mapPixelToCoords(pixelPos, _gameView);

            sf::Sprite dragSpr;
            dragSpr.setTexture(_unitTextures[u->name()]);
            dragSpr.setOrigin(dragSpr.getLocalBounds().width / 2.0f, dragSpr.getLocalBounds().height / 2.0f);
            dragSpr.setPosition(worldPos);
            
            sf::FloatRect bounds = dragSpr.getLocalBounds();
            float scale = (_tileSize * 0.8f) / std::max(bounds.width, bounds.height);
            dragSpr.setScale(scale, scale);
            dragSpr.setColor(sf::Color(255, 255, 255, 180));
            
            _window.draw(dragSpr);
        }
    }

    // ========================================================
    // INTERFACE HUD (Fixe)
    // ========================================================
    _window.setView(sf::View(sf::FloatRect(0, 0, _window.getSize().x, _window.getSize().y)));

    // --------------------------------------------------------
    // TOP BAR (Tour, Joueur, Boutons)
    // --------------------------------------------------------
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, 40));
    ImGui::Begin("TopBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings);
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(ImVec2(0, 0), ImVec2(_window.getSize().x, 40), IM_COL32(20, 25, 35, 220));

    ImGui::SetCursorPos(ImVec2(10, 10));
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "Tour : %d", _moteur.getTourActuel());
    ImGui::SameLine(100);

    if (_moteur.getCurrentPlayerTurn() < (int)_moteur.getJoueurs().size() && _moteur.getCurrentPlayerTurn() < (int)_playerFactions.size()) {
        ImGui::Text("Joueur actuel : %s (%s)", _moteur.getJoueurs()[_moteur.getCurrentPlayerTurn()].getName().c_str(), _playerFactions[_moteur.getCurrentPlayerTurn()].c_str());
    }

    // GOD MODE
    ImGui::SameLine(400);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    if (ImGui::Button("God Mode (Test Perlin)")) {
        _moteur.revealMap(_moteur.getCurrentPlayerTurn());
    }
    ImGui::PopStyleColor(2);

    ImGui::SameLine(_window.getSize().x - 560);
    static bool showPlayerMenu = false;
    if (ImGui::Button("Joueurs")) showPlayerMenu = !showPlayerMenu;
    
    ImGui::SameLine(_window.getSize().x - 480);
    static bool showEmpireMenu = false;
    if (ImGui::Button("Empire")) showEmpireMenu = !showEmpireMenu;

    ImGui::SameLine(_window.getSize().x - 400);
    static bool showInventoryPanel = false;
    if (ImGui::Button("Inventaire")) showInventoryPanel = !showInventoryPanel;

    ImGui::SameLine(_window.getSize().x - 290);
    static bool showProdPanel = false;
    if (ImGui::Button("Production")) showProdPanel = !showProdPanel;
    
    bool isClient = (_network.getState() == NetworkState::CONNECTED && !_network.isHost());
        
    if (!isClient) {
        ImGui::SameLine(static_cast<float>(_window.getSize().x) - 170.0f);
        if (ImGui::Button("Sauvegarder")) {
            ImGui::OpenPopup("SaveGamePopup"); 
        }
    }

    float menuPosOffset = isClient ? 80.0f : 80.0f; 
    ImGui::SameLine(static_cast<float>(_window.getSize().x) - menuPosOffset);
    if (ImGui::Button("Menu")) {
        _previousState = GameState::IN_GAME;
        _currentState = GameState::MENU;
    }
    ImGui::End();

    // --------------------------------------------------------
    // MENU DÉROULANT DES RESSOURCES
    // --------------------------------------------------------
    ImGui::SetNextWindowPos(ImVec2(0, 40)); 
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, 35));
    ImGuiWindowFlags resFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | 
                                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs;

    ImGui::Begin("ResourceBar", nullptr, resFlags);

    if (viewIndex < (int)_moteur.getJoueurs().size()) {
        const Joueur& joueurCourant = _moteur.getJoueurs()[viewIndex];

        int maxRessourcesAffichees = 6;
        int nbAffiche = 0;

        for (const auto& item : joueurCourant.getInventaire()) {
            if (nbAffiche >= maxRessourcesAffichees) {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "... (+%d)", (int)joueurCourant.getInventaire().size() - maxRessourcesAffichees);
                break;
            }

            const Ressource* resPtr = item.first;
            int quantite = item.second;
            std::string resName = resPtr->getName();
            
            if (_resourceIcons.count(resName) > 0) {
                ImGui::Image(_resourceIcons[resName], sf::Vector2f(20.f, 20.f));
                ImGui::SameLine(0.0f, -1.0f);
                ImGui::Text("%d", quantite);
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s:", resName.c_str());
                ImGui::SameLine(0.0f, 5.0f);
                ImGui::Text("%d", quantite);
            }
            
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", resName.c_str());
                ImGui::EndTooltip();
            }
            ImGui::SameLine(0, 25.0f);
            nbAffiche++;
        }
    }
    ImGui::End();

    // --------------------------------------------------------
    // Panneau Liste des Joueurs (T5)
    // --------------------------------------------------------
    if (showPlayerMenu) {
        ImGui::SetNextWindowPos(ImVec2(200, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Liste des Commandants", &showPlayerMenu);

        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "RAPPORTS D'INTELLIGENCE");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5));

        if (ImGui::BeginTable("PlayersTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("Commandant", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Faction", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Villes", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Unites", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableHeadersRow();

            const auto& joueurs = _moteur.getJoueurs();
            for (size_t i = 0; i < joueurs.size(); ++i) {
                const Joueur& j = joueurs[i];
                ImGui::TableNextRow();
                
                ImGui::TableSetColumnIndex(0); 
                if (i == _localPlayerIndex) {
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s (Vous)", j.getName().c_str());
                } else {
                    ImGui::Text("%s", j.getName().c_str());
                }

                // Colonne 1 : Faction
                ImGui::TableSetColumnIndex(1); 
                std::string factionName = "Inconnue";
                if (i < _playerFactions.size() && !_playerFactions[i].empty()) {
                    factionName = _playerFactions[i];
                }
                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "%s", factionName.c_str());

                // Colonne 2 : Nombre de Villes
                ImGui::TableSetColumnIndex(2); 
                ImGui::Text("%d", j.getNbVilles());

                // Colonne 3 : Puissance Militaire (Nombre d'unités)
                ImGui::TableSetColumnIndex(3); 
                ImGui::Text("%d", (int)j.getUnites().size());
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    // --------------------------------------------------------
    // Panneau de Production
    // --------------------------------------------------------
    if (showProdPanel && viewIndex < (int)_moteur.getJoueurs().size()) {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Production de l'Empire", &showProdPanel);
        
        std::map<const Ressource*, int> totalProd;
        
        // 1. Collecte de la production des villes
        for (City* c : _moteur.getJoueurs()[viewIndex].getCities()) {
            for (auto& b : c->getBatiments()) {
                for (auto const& [res, qte] : b->getProduits()) {
                    totalProd[res] += qte;
                }
            }
        }

        // 2. Affichage des dépenses (Entretien de l'armée)
        std::map<const Ressource*, int> factureProdTotale;
        for (Unite* u : _moteur.getJoueurs()[viewIndex].getUnites()) {
            if (u) {
                auto coutU = u->getCoutEntretien();
                if (coutU.empty()) {
                    for (auto const& [nomRes, qte] : _moteur.getLogicConfig().getEntretienCoutDefaut()) {
                        const Ressource* r = _moteur.getRessourceFactory().getRessource(nomRes);
                        if (r) factureProdTotale[r] += qte;
                    }
                } else {
                    for (auto const& [resPtr, qte] : coutU) {
                        factureProdTotale[resPtr] += qte;
                    }
                }
            }
        }
        
        if (!factureProdTotale.empty()) {
            ImGui::Dummy(ImVec2(0, 10));
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "DEPENSES (ENTRETIEN)");
            ImGui::Separator();
            
            if (ImGui::BeginTable("DepensesTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV)) {
                ImGui::TableSetupColumn("Ressource", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Cout / Tour", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                ImGui::TableHeadersRow();

                for (auto const& [resPtr, qte] : factureProdTotale) {
                    std::string resName = resPtr->getName();
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    if (_resourceIcons.count(resName) > 0) {
                        ImGui::Image(_resourceIcons[resName], sf::Vector2f(16.f, 16.f));
                        ImGui::SameLine(0.0f, -1.0f);
                    }
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", resName.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "-%d", qte);
                }
                ImGui::EndTable();
            }
        }
                
        // 3. Avertissement visuel de famine
        bool warningFamine = false;
        for (auto const& [resPtr, qte] : factureProdTotale) {
            auto invIt = _moteur.getJoueurs()[viewIndex].getInventaire().find(resPtr);
            int stock = (invIt != _moteur.getJoueurs()[viewIndex].getInventaire().end()) ? invIt->second : 0;
            if (stock + totalProd[resPtr] < qte) warningFamine = true;
        }
        
        if (warningFamine) {
            ImGui::Dummy(ImVec2(0, 10));
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "ATTENTION : PENURIE IMMINENTE !");
            ImGui::TextDisabled("Vos troupes vont subir des degats d'usure.");
        }
        
        // 4. Collecte de la production des bâtiments spéciaux
        for (Batiment* b : _moteur.getJoueurs()[viewIndex].getBatiments()) {
            for (auto const& [res, qte] : b->getProduits()) {
                totalProd[res] += qte;
            }
        }

        // 5. Collecte de la production de la capitale
        const Joueur& localJ = _moteur.getJoueurs()[viewIndex];
        if (!localJ.getCities().empty()) {
            for (const auto& [resName, qty] : _moteur.getLogicConfig().getProductionCapitale()) {
                const Ressource* resPtr = _moteur.getRessourceFactory().getRessource(resName);
                if (resPtr) {
                    totalProd[resPtr] += qty;
                }
            }
        }

        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "REVENUS GLOBAUX");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5));

        if (totalProd.empty()) {
            ImGui::TextDisabled("Votre empire ne produit aucune ressource pour le moment.");
        } else {
            if (ImGui::BeginTable("ProdTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV)) {
                ImGui::TableSetupColumn("Ressource", ImGuiTableColumnFlags_WidthStretch); 
                ImGui::TableSetupColumn("Production / Tour", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                ImGui::TableHeadersRow();
                
                for (auto const& [res, qte] : totalProd) {
                    std::string resName = res->getName();
                    
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); 
                    
                    // icone de la ressource
                    if (_resourceIcons.count(resName) > 0) {
                        ImGui::Image(_resourceIcons[resName], sf::Vector2f(16.f, 16.f));
                        ImGui::SameLine(0.0f, -1.0f);
                    }
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", resName.c_str());
                    
                    ImGui::TableSetColumnIndex(1); 
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "+%d", qte);
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    // --------------------------------------------------------
    // Panneau INVENTAIRE
    // --------------------------------------------------------
    if (showInventoryPanel && viewIndex < (int)_moteur.getJoueurs().size()) {
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(450, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Inventaire Global", &showInventoryPanel);
        
        std::map<const Ressource*, int> totalProd;
        for (City* c : _moteur.getJoueurs()[viewIndex].getCities()) {
            for (auto& b : c->getBatiments()) {
                for (auto const& [res, qte] : b->getProduits()) totalProd[res] += qte;
            }
        }
        for (Batiment* b : _moteur.getJoueurs()[viewIndex].getBatiments()) {
            for (auto const& [res, qte] : b->getProduits()) totalProd[res] += qte;
        }

        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "RESSOURCES POSSEDEES ET REVENUS");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5));

        if (ImGui::BeginTable("InvTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("Ressource", ImGuiTableColumnFlags_WidthStretch); 
            ImGui::TableSetupColumn("Stock", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Production", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();
            
            for (auto const& [resPtr, quantite] : _moteur.getJoueurs()[viewIndex].getInventaire()) {
                std::string resName = resPtr->getName();
                int prod = totalProd[resPtr];
                
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); 
                if (_resourceIcons.count(resName) > 0) {
                    ImGui::Image(_resourceIcons[resName], sf::Vector2f(16.f, 16.f));
                    ImGui::SameLine(0.0f, -1.0f);
                }
                ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", resName.c_str());
                
                ImGui::TableSetColumnIndex(1); 
                ImGui::Text("%d", quantite);

                ImGui::TableSetColumnIndex(2);
                if (prod > 0) ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "+%d / tour", prod);
                else ImGui::TextDisabled("-");
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    // --------------------------------------------------------
    // Panneau Menu Empire
    // --------------------------------------------------------
    if (showEmpireMenu && viewIndex < (int)_moteur.getJoueurs().size()) {
        ImGui::SetNextWindowPos(ImVec2(150, 150), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
        ImGui::Begin("Gestion de l'Empire", &showEmpireMenu);

        if (ImGui::BeginTabBar("EmpireTabs")) {
            const Joueur& localJ = _moteur.getJoueurs()[viewIndex];

            // --- ONGLET 1 : VILLES ---
            if (ImGui::BeginTabItem("Villes & Colonies")) {
                ImGui::Dummy(ImVec2(0, 5));
                if (localJ.getCities().empty()) {
                    ImGui::TextDisabled("Vous ne possedez aucune ville.");
                } else {
                    if (ImGui::BeginTable("CitiesTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                        ImGui::TableSetupColumn("Nom", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Niveau", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                        ImGui::TableSetupColumn("PV", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                        ImGui::TableHeadersRow();
                        
                        for (City* c : localJ.getCities()) {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0); 
                            ImGui::TextColored(c->estCapitale() ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f), 
                                               "%s %s", c->estCapitale() ? "[CAP]" : "", c->getNom().c_str());
                            
                            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", c->getLevel());
                            ImGui::TableSetColumnIndex(2); ImGui::Text("%.0f/%.0f", c->getPv(), c->getPvMax());
                            
                            ImGui::TableSetColumnIndex(3);
                            ImGui::PushID(c);
                            if (ImGui::Button("Focus")) {
                                float R = _tileSize / 2.0f;
                                float W = std::sqrt(3.0f) * R;
                                _gameView.setCenter(W * c->getY() + W * 0.5f * (std::abs(c->getX()) % 2), 1.5f * R * c->getX());
                                _selectedCellX = c->getX();
                                _selectedCellY = c->getY();
                                _hasSelection = true;
                            }
                            ImGui::PopID();
                        }
                        ImGui::EndTable();
                    }
                }
                ImGui::EndTabItem();
            }

            // --- ONGLET 2 : UNITÉS ---
            if (ImGui::BeginTabItem("Forces Armees")) {
                ImGui::Dummy(ImVec2(0, 5));
                if (ImGui::BeginTable("ArmyTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("PV", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                    ImGui::TableSetupColumn("PA", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableHeadersRow();
                    
                    int unitCount = 0;
                    int rows = _moteur.getPlateau()->getRows();
                    int cols = _moteur.getPlateau()->getCols();
                    
                    for (int i = 0; i < rows; ++i) {
                        for (int j = 0; j < cols; ++j) {
                            if (_moteur.getProprietaireUnite(i, j) == viewIndex) {
                                Unite* u = _moteur.getPlateau()->getUnite(i, j);
                                if(u) {
                                    unitCount++;
                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", u->name().c_str());
                                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d/%d", u->health_point(), u->health_point_max());
                                    ImGui::TableSetColumnIndex(2); ImGui::Text("%d/%d", u->point_action(), u->point_action_max());
                                    
                                    ImGui::TableSetColumnIndex(3);
                                    std::string btnId = "Focus##U_" + std::to_string(i) + "_" + std::to_string(j);
                                    if (ImGui::Button(btnId.c_str())) {
                                        float R = _tileSize / 2.0f;
                                        float W = std::sqrt(3.0f) * R;
                                        _gameView.setCenter(W * j + W * 0.5f * (std::abs(i) % 2), 1.5f * R * i);
                                        _selectedCellX = i;
                                        _selectedCellY = j;
                                        _hasSelection = true;
                                    }
                                }
                            }
                        }
                    }
                    if (unitCount == 0) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0); ImGui::TextDisabled("Aucune unite deployee.");
                    }
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }

            // --- ONGLET 3 : BÂTIMENTS SPÉCIAUX ---
            if (ImGui::BeginTabItem("Batiments Speciaux")) {
                ImGui::Dummy(ImVec2(0, 5));
                if (localJ.getBatiments().empty()) {
                    ImGui::TextDisabled("Vous ne possedez aucun batiment special.");
                } else {
                    if (ImGui::BeginTable("SpecialBuildingsTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                        ImGui::TableSetupColumn("Nom", ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("Niveau", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                        ImGui::TableHeadersRow();
                        
                        for (Batiment* b : localJ.getBatiments()) {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", b->getName().c_str());
                            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", b->getLevel());
                        }
                        ImGui::EndTable();
                    }
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    // --------------------------------------------------------
    // BOUTON FIN DE TOUR ET PAUSE RESEAU
    // --------------------------------------------------------
    bool isWaitingForPlayer = (isMultiplayer && _network.isHost() && (int)_connectedPlayers.size() < _numPlayers);

    ImVec2 nextTurnSize(200, 100);
    ImGui::SetNextWindowPos(ImVec2(_window.getSize().x - nextTurnSize.x, _window.getSize().y - nextTurnSize.y));
    ImGui::SetNextWindowSize(nextTurnSize);
    ImGui::Begin("NextTurnBox", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.2f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.7f, 0.3f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
    
    if (!isMyTurn || isWaitingForPlayer) ImGui::BeginDisabled();
    if (ImGui::Button("TOUR SUIVANT\n>>", ImVec2(180, 80))) {
        _hasSelection = false;
        
        _moteur.passerTour();
        
        if (isMultiplayer) {
            sf::Packet turnPacket; turnPacket << static_cast<sf::Int32>(PacketType::END_TURN);
            _network.sendData(turnPacket);
        }
        // La caméra reste où le joueur l'a laissée (pas de recentrage automatique)
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
            
                Unite* u = _moteur.getPlateau()->getUnite(i, j);
                if (u && localJ.estVisible(i, j)) {
                    int propIdx = _moteur.getProprietaireUnite(i, j);
                    ImU32 uColor = IM_COL32(200, 200, 200, 255);
                    
                    if (propIdx == viewIndex) uColor = IM_COL32(50, 255, 50, 255); // Allié (Vert)
                    else if (propIdx != -1)   uColor = IM_COL32(255, 50, 50, 255); // Ennemi (Rouge)

                    minimapDrawList->AddCircleFilled(ImVec2(cx, cy), R_mini * 0.5f, uColor);
                    minimapDrawList->AddCircle(ImVec2(cx, cy), R_mini * 0.5f, IM_COL32(0, 0, 0, 255), 0, 1.0f);
                }
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
    ImGui::SetNextWindowPos(ImVec2(320, _window.getSize().y - 250), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(actionPanelWidth, 250), ImGuiCond_FirstUseEver);
    ImGui::Begin("ActionPanel", nullptr, ImGuiWindowFlags_NoTitleBar);
    
    if (_hasSelection && viewIndex < (int)_moteur.getJoueurs().size()) {
        int localJIdx = isMultiplayer ? _localPlayerIndex : _moteur.getCurrentPlayerTurn();
        
        const Joueur& localJ = _moteur.getJoueurs()[localJIdx]; 
        
        const hexa* h = _moteur.getPlateau()->getCell(_selectedCellX, _selectedCellY);
        const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(h);

        bool cellDiscovered = localJ.estDecouvert(_selectedCellX, _selectedCellY);
        bool cellVisible = localJ.estVisible(_selectedCellX, _selectedCellY);

        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "Case (%d, %d)", _selectedCellX, _selectedCellY);
        ImGui::Separator();

        if (!isMyTurn) ImGui::BeginDisabled();
        bool actionPossible = false;

        if (!cellDiscovered) {
            ImGui::TextDisabled("Zone inexploree.");
        } else if (tc) {
            bool estDansTerritoire = _moteur.estDansTerritoire(localJIdx, _selectedCellX, _selectedCellY);

            // A. Fonder une ville (Totalement dynamique)
            if (!tc->getCity() && _moteur.peutFonderVille(localJIdx, _selectedCellX, _selectedCellY)) {
                actionPossible = true;
                bool isCapitalTurn = (localJ.getNbVilles() == 0); 
                
                for (const auto& [nom, modele] : _moteur.getCityFactory().getCatalogue()) {
                    if (modele->estCapitale() != isCapitalTurn) continue; 

                    std::map<const Ressource*, int> coutVille = _moteur.getCoutFondationVille(localJIdx, nom);
                    bool peutPayer = _moteur.peutPayer(localJIdx, coutVille);
                    
                    std::string textCout = "Cout (" + nom + ") : ";
                    if (coutVille.empty()) textCout += "Gratuit";
                    else { for (auto const& [res, qte] : coutVille) textCout += std::to_string(qte) + " " + res->getName() + " "; }
                    
                    ImGui::TextColored(peutPayer ? ImVec4(0.8f, 0.8f, 0.8f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", textCout.c_str());

                    if (!modele->getProduits().empty()) {
                        std::string textProd = "Produit : ";
                        for (auto const& [res, qte] : modele->getProduits()) textProd += "+" + std::to_string(qte) + " " + res->getName() + " ";
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", textProd.c_str());
                    }

                    if (!peutPayer) ImGui::BeginDisabled();
                    if (ImGui::Button(("Fonder " + nom).c_str(), ImVec2(150, 40))) {
                        CmdFonderVille cmd = { _selectedCellX, _selectedCellY, nom };
                        if (_moteur.soumettreCommande(localJIdx, cmd) == ResultatAction::SUCCES) {
                            _popupMsg = nom + " fondee avec succes !";
                            if (isMultiplayer) {
                                sf::Packet p;
                                p << static_cast<sf::Int32>(PacketType::ACTION_BUILD_CITY) << static_cast<sf::Int32>(localJIdx) << _selectedCellX << _selectedCellY << nom;
                                _network.sendData(p);
                            }
                        } else {
                            _popupMsg = "Erreur: Impossible de fonder la ville.";
                        }
                        _showPopup = true;
                    }
                    if (!peutPayer) ImGui::EndDisabled();
                }
                ImGui::Separator();
            }

            // B. Si c'est une Ville
            if (tc->getCity()) {
                if (_moteur.estVilleAuJoueur(localJIdx, _selectedCellX, _selectedCellY)) {
                    actionPossible = true;

                    bool isMaxLevel = (tc->getCity()->getLevel() >= tc->getCity()->getMaxLevel());

                    if (isMaxLevel) {
                        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "VILLE AU NIVEAU MAXIMUM");
                    } else {
                        bool canUpgrade = _moteur.peutAmeliorerVille(localJIdx, _selectedCellX, _selectedCellY);
                        if (!canUpgrade) ImGui::BeginDisabled();
                        if (ImGui::Button("Ameliorer Ville", ImVec2(150, 40))) { 
                            CmdAmeliorer cmd = { _selectedCellX, _selectedCellY };
                            if (_moteur.soumettreCommande(localJIdx, cmd) == ResultatAction::SUCCES) {
                                _popupMsg = "Ville amelioree !";
                                if (isMultiplayer) {
                                    sf::Packet p;
                                    p << static_cast<sf::Int32>(PacketType::ACTION_UPGRADE_CITY) 
                                      << static_cast<sf::Int32>(localJIdx) 
                                      << _selectedCellX 
                                      << _selectedCellY;
                                    _network.sendData(p);
                                }
                            } else {
                                _popupMsg = "Amelioration impossible.";
                            }
                            _showPopup = true;
                        }
                        if (!canUpgrade) ImGui::EndDisabled();
                        
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                            ImGui::BeginTooltip();
                            if (!canUpgrade) ImGui::TextColored(ImVec4(1.0f,0.3f,0.3f,1.0f), "Ressources insuffisantes ou Niveau Max !");
                            ImGui::EndTooltip();
                        }
                    }

                    ImGui::SameLine(0.0f, -1.0f);
                    if (ImGui::Button("Construire Batiment", ImVec2(150, 40))) ImGui::OpenPopup("Menu Construction Batiments");
                    ImGui::SameLine(0.0f, -1.0f);
                    if (ImGui::Button("Recruter Unite", ImVec2(130, 40))) ImGui::OpenPopup("Menu Recrutement");
                }
            } 
            // C. Territoire sans ville (Bâtiments Spéciaux)
            else if (estDansTerritoire) {
                actionPossible = true;
                if (tc->getBatimentSpecial()) {
                    const Batiment* bat = tc->getBatimentSpecial();
                    ImGui::Dummy(ImVec2(0, 10));
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Emplacement occupe par :");
                    
                    ImGui::BeginGroup();
                    ImGui::Text("%s (Niv %d)", bat->getName().c_str(), bat->getLevel());
                    
                    bool hasProd = false;
                    for (auto const& [res, qte] : bat->getProduits()) {
                        std::string resName = res->getName();
                        if (_resourceIcons.count(resName) > 0) {
                            ImGui::Image(_resourceIcons[resName], sf::Vector2f(16.f, 16.f));
                            ImGui::SameLine(0.0f, -1.0f);
                        }
                        ImGui::Text("+%d %s / tour", qte, resName.c_str());
                        hasProd = true;
                    }
                    if (!hasProd) ImGui::TextDisabled("Aucune production.");
                    ImGui::EndGroup();
                } else if (!tc->getRessource().empty()) {
                    if (ImGui::Button("Construire Special", ImVec2(150, 40))) ImGui::OpenPopup("Menu Construction Batiments");
                } else {
                    ImGui::TextDisabled("Aucune ressource a exploiter ici.");
                }
            }
            // D. Acheter Territoire
            else if (_moteur.peutAcheterTerritoire(localJIdx, _selectedCellX, _selectedCellY)) {
                actionPossible = true;
                std::map<const Ressource*, int> coutAchat = _moteur.getCoutAchatTerritoire(localJIdx);
                bool peutPayer = _moteur.peutPayer(localJIdx, coutAchat);
                
                std::string textCout = "Acheter Case : ";
                for (auto const& [res, qte] : coutAchat) textCout += std::to_string(qte) + " " + res->getName() + " ";
                ImGui::TextColored(peutPayer ? ImVec4(0.8f,0.8f,0.8f,1.0f) : ImVec4(1.0f,0.3f,0.3f,1.0f), "%s", textCout.c_str());

                if (!peutPayer) ImGui::BeginDisabled();
                if (ImGui::Button("Acheter Territoire", ImVec2(150, 40))) {
                    CmdAcheterCase cmd = { _selectedCellX, _selectedCellY };
                    if (_moteur.soumettreCommande(localJIdx, cmd) == ResultatAction::SUCCES) {
                        _popupMsg = "Territoire achete !";

                        if (isMultiplayer) {
                            sf::Packet p;
                            p << static_cast<sf::Int32>(PacketType::ACTION_BUY_TILE) << static_cast<sf::Int32>(localJIdx) << _selectedCellX << _selectedCellY;
                            _network.sendData(p);
                        }
                    } else {
                        _popupMsg = "Erreur lors de l'achat.";
                    }
                    _showPopup = true;
                    _casesPossibles.clear();
                    _casesAttaquePossibles.clear();
                }
                if (!peutPayer) ImGui::EndDisabled();
            }

            // --- UNITÉS ---
            Unite* uniteSurCase = _moteur.getPlateau()->getUnite(_selectedCellX, _selectedCellY);
            if (uniteSurCase && cellVisible) {
                int propIdx = _moteur.getProprietaireUnite(_selectedCellX, _selectedCellY);
                bool isMine = (propIdx == localJIdx);

                actionPossible = true;
                ImGui::Separator();
                ImGui::TextColored(isMine ? ImVec4(0.3f, 1.0f, 0.3f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f), 
                                   "UNITE : %s (%s)", uniteSurCase->name().c_str(), isMine ? "Alliee" : "Ennemie");
                
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "HP : %d / %d", uniteSurCase->health_point(), uniteSurCase->health_point_max());
                ImGui::Dummy(ImVec2(0, 5));

                if (isMine) {
                    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "PA : %d / %d", uniteSurCase->point_action(), uniteSurCase->point_action_max());
                    ImGui::Dummy(ImVec2(0, 5));

                    if (ImGui::Button("Deplacer", ImVec2(150, 40))) {
                    _isTargetingMove = true; _isTargetingAttack = false;
                    _unitSourceX = _selectedCellX; _unitSourceY = _selectedCellY;
                    _casesPossibles = _moteur.getDeplacementsPossibles(localJIdx, _selectedCellX, _selectedCellY);
                }
                ImGui::SameLine(0.0f, -1.0f);
                if (ImGui::Button("Attaquer", ImVec2(150, 40))) {
                    _isTargetingAttack = true; _isTargetingMove = false;
                    _unitSourceX = _selectedCellX; _unitSourceY = _selectedCellY;
                    _casesAttaquePossibles = _moteur.getAttaquesPossibles(localJIdx, _selectedCellX, _selectedCellY);
                }

                ImGui::SameLine(0.0f, -1.0f);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                if (ImGui::Button("Detruire", ImVec2(100, 40))) {
                    ImGui::OpenPopup("Confirmation Destruction");
                }
                ImGui::PopStyleColor(3);

                if (ImGui::BeginPopupModal("Confirmation Destruction", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Etes-vous sur de vouloir detruire cette unite ?\nCette action est irreversible !");
                    ImGui::Separator();

                    if (ImGui::Button("OUI, DETRUIRE", ImVec2(120, 0))) {
                        CmdDetruireUnite cmd = { _selectedCellX, _selectedCellY };
                        if (_moteur.soumettreCommande(localJIdx, cmd) == ResultatAction::SUCCES) {
                            _popupMsg = "Unite detruite.";
                            if (isMultiplayer) {
                                sf::Packet p; 
                                p << static_cast<sf::Int32>(PacketType::ACTION_DESTROY_UNIT) 
                                  << static_cast<sf::Int32>(localJIdx) 
                                  << _selectedCellX 
                                  << _selectedCellY;
                                _network.sendData(p);
                            }
                        } else {
                            _popupMsg = "Erreur lors de la destruction.";
                        }
                        _showPopup = true;
                        ImGui::CloseCurrentPopup();

                        _hasSelection = false;
                        _casesPossibles.clear();
                        _casesAttaquePossibles.clear();
                    }
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine(0.0f, -1.0f);
                    if (ImGui::Button("ANNULER", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                }

                if (isMine) {
                    renderUnitActions(uniteSurCase);
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

                // --- DIAGNOSTIC DE CONSTRUCTION ---
                bool peutPayer = _moteur.peutPayer(localJIdx, batimentModele->getResourceConstr());
                bool estSpecial = !batimentModele->getRessourcesSolRequired().empty();
                bool locationValide = false;

                if (tc->getCity()) {
                    // Dans une ville : il faut de la place, et le bâtiment ne doit pas être "Spécial"
                    locationValide = !estSpecial && tc->getCity()->peutAjouterBatiment();
                } else {
                    // Hors d'une ville : le bâtiment DOIT être spécial et la ressource correspondre
                    locationValide = estSpecial && tc->peutConstrBatimentSpecial(*batimentModele);
                }

                bool canBuild = peutPayer && locationValide;

                // Affichage du bouton (gris si impossible)
                if (!canBuild) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                
                if (ImGui::Selectable(label.c_str(), false, canBuild ? 0 : ImGuiSelectableFlags_Disabled)) {
                    if (isMultiplayer && !_network.isHost()) {
                        sf::Packet p; p << static_cast<sf::Int32>(PacketType::ACTION_BUILD) << static_cast<sf::Int32>(_localPlayerIndex) << _selectedCellX << _selectedCellY << nom; 
                        _network.sendData(p);
                        _popupMsg = "Requete envoyee, en attente de validation...";
                        _showPopup = true;
                    } else {
                        CmdConstruction cmd = { _selectedCellX, _selectedCellY, nom };
                        if (_moteur.soumettreCommande(localJIdx, cmd) == ResultatAction::SUCCES) {
                            _popupMsg = nom + " construit avec succes !";
                            if (isMultiplayer) {
                                sf::Packet p; p << static_cast<sf::Int32>(PacketType::SYNC_BUILD) << static_cast<sf::Int32>(localJIdx) << _selectedCellX << _selectedCellY << nom;
                                _network.sendData(p);
                            }
                        } else {
                            _popupMsg = "Erreur critique de construction.";
                        }
                        _showPopup = true;
                    }
                }
                
                if (!canBuild) ImGui::PopStyleColor(1);

                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::BeginTooltip();
                    if (!peutPayer) 
                        ImGui::TextColored(ImVec4(1.0f,0.3f,0.3f,1.0f), "Ressources insuffisantes !");
                    else if (estSpecial && tc->getCity()) 
                        ImGui::TextColored(ImVec4(1.0f,0.3f,0.3f,1.0f), "Batiment special : a construire en dehors d'une ville !");
                    else if (!estSpecial && !tc->getCity()) 
                        ImGui::TextColored(ImVec4(1.0f,0.3f,0.3f,1.0f), "Batiment standard : a construire a l'interieur d'une ville !");
                    else if (tc->getCity() && !tc->getCity()->peutAjouterBatiment()) 
                        ImGui::TextColored(ImVec4(1.0f,0.3f,0.3f,1.0f), "Niveau de ville trop faible (plus d'emplacements) !");
                    else if (estSpecial && !tc->peutConstrBatimentSpecial(*batimentModele)) 
                        ImGui::TextColored(ImVec4(1.0f,0.3f,0.3f,1.0f), "Ressource requise absente sur cette case !");
                    else
                        ImGui::TextColored(ImVec4(0.3f,1.0f,0.3f,1.0f), "Construction possible.");
                    ImGui::EndTooltip();
                }
            }
            if (_moteur.getBatimentFactory().getCatalogue().empty()) ImGui::TextDisabled("Aucun batiment dans le catalogue.");
            ImGui::EndPopup();
        }

        // ========================================================
        // SOUS-MENU : RECRUTEMENT D'UNITÉS
        // ========================================================
        
        if (ImGui::BeginPopup("Menu Recrutement")) {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "RECRUTEMENT D'UNITES");
            ImGui::Separator();
            
            const Joueur& localJ = _moteur.getJoueurs()[localJIdx];
            int capMax = _moteur.getLogicConfig().getCapaciteBase();
            for (City* c : localJ.getCities()) {
                if (c) capMax += c->getLevel() * _moteur.getLogicConfig().getCapaciteVilleNiveau();
            }
            int popActuelle = localJ.getUnites().size();

            ImGui::TextColored(popActuelle >= capMax ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 1.0f, 0.3f, 1.0f), 
                               "Capacite Logistique : %d / %d", popActuelle, capMax);
            
            if (popActuelle >= capMax) {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Ameliorez des villes pour recruter !");
            }
            ImGui::Dummy(ImVec2(0, 5));

            const auto& catalogue = _moteur.getUniteFactory().getCatalogue();
            if (catalogue.empty()) {
                ImGui::TextDisabled("Aucune unite dans le catalogue.");
            } else {
                for (const auto& [nom, uniteModele] : catalogue) {
                    std::string coutText;
                    for (auto const& [res, qte] : uniteModele->cout()) {
                        if (!coutText.empty()) coutText += ", ";
                        coutText += std::to_string(qte) + " " + res->getName();
                    }
                    if (coutText.empty()) coutText = "Gratuit";

                    bool canAfford = _moteur.peutRecruterUnite(localJIdx, nom);
                    bool canRecruit = canAfford && (popActuelle < capMax);

                    if (!canRecruit) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                    std::string label = nom + "  [" + coutText + "]  HP:" + std::to_string(uniteModele->health_point());
                    
                    if (ImGui::Selectable(label.c_str(), false, canRecruit ? 0 : ImGuiSelectableFlags_Disabled)) {
                        CmdRecrutement cmd = { _selectedCellX, _selectedCellY, nom };
                        if (_moteur.soumettreCommande(localJIdx, cmd) == ResultatAction::SUCCES) {
                            _popupMsg = nom + " recrute avec succes !";
                            
                            if (isMultiplayer) {
                                sf::Packet p;
                                p << static_cast<sf::Int32>(PacketType::ACTION_RECRUIT) 
                                  << static_cast<sf::Int32>(localJIdx) 
                                  << _selectedCellX 
                                  << _selectedCellY 
                                  << nom;
                                _network.sendData(p);
                            }
                        } else {
                            _popupMsg = "Recrutement impossible.";
                        }
                        _showPopup = true;
                        ImGui::CloseCurrentPopup();
                    }
                    if (!canRecruit) ImGui::PopStyleColor(1);

                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                        ImGui::BeginTooltip();
                        
                        ImGui::Text("HP: %d | PA: %d", uniteModele->health_point(), uniteModele->point_action());
                        ImGui::Separator();
                        
                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.3f, 1.0f), "Entretien par tour :");
                        auto coutU = uniteModele->getCoutEntretien();
                        if (coutU.empty()) {
                            for (auto const& [nomRes, qte] : _moteur.getLogicConfig().getEntretienCoutDefaut()) {
                                ImGui::Text(" -%d %s", qte, nomRes.c_str());
                            }
                        } else {
                            for (auto const& [resPtr, qte] : coutU) {
                                ImGui::Text(" -%d %s", qte, resPtr->getName().c_str());
                            }
                        }
                        
                        ImGui::Separator();
                        if (!canAfford) ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Ressources insuffisantes !");
                        if (popActuelle >= capMax) ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Limite logistique atteinte !");
                        
                        ImGui::EndTooltip();
                    }
                }
            }
            ImGui::EndPopup();
        }

    } else {
        ImGui::SetCursorPosY(60);
        ImGui::SetCursorPosX((actionPanelWidth - ImGui::CalcTextSize("Selectionnez une case").x) * 0.5f);
        ImGui::TextDisabled("Selectionnez une case");
    }
    ImGui::End();

    // --------------------------------------------------------
    // 5. PANNEAU LATÉRAL (INFOS VILLE ET PANNEAU SPECIAL)
    // --------------------------------------------------------
    if (_hasSelection) {
        const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(_moteur.getPlateau()->getCell(_selectedCellX, _selectedCellY));
        int localJIdx = isMultiplayer ? _localPlayerIndex : _moteur.getCurrentPlayerTurn();
        bool isDiscovered = _moteur.getJoueurs()[localJIdx].estDecouvert(_selectedCellX, _selectedCellY);
        
        bool isVisible = _moteur.getJoueurs()[localJIdx].estVisible(_selectedCellX, _selectedCellY);

        if (tc && isDiscovered && (tc->getCity() || tc->getBatimentSpecial())) {
            bool isMine = (tc->getProprietaire() == &_moteur.getJoueurs()[localJIdx]);

            ImGui::SetNextWindowPos(ImVec2(_window.getSize().x - 260, 50), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_FirstUseEver);
            ImGui::Begin("TileInfo", nullptr, ImGuiWindowFlags_NoTitleBar);
            
            if (!isMine && !isVisible) {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "DONNEES OBSOLETES");
                ImGui::Separator();
                if (tc->getCity()) ImGui::Text("Ville : %s", tc->getCity()->getNom().c_str());
                else ImGui::Text("Batiment special detecte.");
                
                ImGui::Dummy(ImVec2(0, 20));
                ImGui::TextDisabled("Envoyez une unite a proximite\npour actualiser l'intelligence.");
            } else {
                if (tc->getCity()) {
                    City* city = tc->getCity();
                    ImGui::TextColored(isMine ? ImVec4(0.3f, 0.8f, 1.0f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "VILLE %s NIVEAU %d", isMine ? "ALLIEE" : "ENNEMIE", city->getLevel());
                    ImGui::Separator();
                    ImGui::Text("PV: %.0f/%.0f", city->getPv(), city->getPvMax());

                    if (isMine) {
                        ImGui::Text("Degats: %.0f", city->getDegats());
                        ImGui::Dummy(ImVec2(0, 10));

                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "BATIMENTS ACTUELS :");
                        if (city->getBatiments().empty()) {
                            ImGui::TextDisabled("  Aucun batiment.");
                        } else {
                            for (auto& b : city->getBatiments()) {
                                ImGui::BulletText("%s", b->getName().c_str());
                                if (ImGui::IsItemHovered()) {
                                    ImGui::BeginTooltip();
                                    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "Production par tour:");
                                    ImGui::Separator();
                                    bool hasProd = false;
                                    for (auto const& [res, qte] : b->getProduits()) {
                                        std::string resName = res->getName();
                                        if (_resourceIcons.count(resName) > 0) {
                                            ImGui::Image(_resourceIcons[resName], sf::Vector2f(16.f, 16.f));
                                            ImGui::SameLine(0.0f, -1.0f);
                                        }
                                        ImGui::Text("+%d %s", qte, resName.c_str());
                                        hasProd = true;
                                    }
                                    if (!hasProd) ImGui::TextDisabled("Aucune production directe.");
                                    ImGui::EndTooltip();
                                }
                            }
                        }
                    }
                } 
                else if (tc->getBatimentSpecial()) {
                    const Batiment* bat = tc->getBatimentSpecial();
                    ImGui::TextColored(isMine ? ImVec4(0.9f, 0.5f, 0.2f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "BATIMENT SPECIAL");
                    ImGui::Separator();
                    ImGui::Text("%s (Niv %d)", bat->getName().c_str(), bat->getLevel());
                    
                    if (isMine) {
                        ImGui::Dummy(ImVec2(0, 10));
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "COUT CONSTRUCTION :");
                        for (auto const& [res, qte] : bat->getResourceConstr()) {
                            std::string resName = res->getName();
                            if (_resourceIcons.count(resName) > 0) {
                                ImGui::Image(_resourceIcons[resName], sf::Vector2f(16.f, 16.f));
                                ImGui::SameLine(0.0f, -1.0f);
                            }
                            ImGui::Text("%d %s", qte, resName.c_str());
                        }

                        ImGui::Dummy(ImVec2(0, 5));
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "PRODUCTION STATUT :");
                        bool hasProd = false;
                        for (auto const& [res, qte] : bat->getProduits()) {
                            std::string resName = res->getName();
                            if (_resourceIcons.count(resName) > 0) {
                                ImGui::Image(_resourceIcons[resName], sf::Vector2f(16.f, 16.f));
                                ImGui::SameLine(0.0f, -1.0f);
                            }
                            ImGui::Text("+%d %s / tour", qte, resName.c_str());
                            hasProd = true;
                        }
                        if (!hasProd) ImGui::TextDisabled("  Aucune production.");
                    }
                }
                ImGui::End();
            }
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

    // --- POP-UP DE PAUSE RÉSEAU (DÉCONNEXION) ---
    if (isWaitingForPlayer) {
        ImVec2 centerUi = ImVec2(_window.getSize().x * 0.5f, _window.getSize().y * 0.5f);
        ImGui::SetNextWindowPos(centerUi, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Deconnexion", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "/!\\ ALERTE SYSTEME /!\\");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Text("Un Commandant s'est deconnecte.");
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "La partie est en pause en attendant qu'il rejoigne la flotte...");
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::End();
        
        // On annule les actions en cours
        _isDragging = false; 
        _hasSelection = false;
    }

    // RENDU DU COMBAT LOG FLOTTANT
    if (!_combatLogs.empty()) {
        ImGui::SetNextWindowPos(ImVec2(_window.getSize().x - 320, 200), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
        ImGui::Begin("Journal de Combat", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);
        
        float dt = ImGui::GetIO().DeltaTime;
        for (auto it = _combatLogs.begin(); it != _combatLogs.end(); ) {
            it->timer -= dt;
            
            sf::Color c = it->color;
            if (it->timer < 1.0f) c.a = static_cast<sf::Uint8>(std::max(0.0f, it->timer * 255.0f));
            
            ImGui::TextColored(ImVec4(c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f), "⚔ %s", it->message.c_str());
            
            if (it->timer <= 0.0f) it = _combatLogs.erase(it);
            else ++it;
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }

    renderChatWindow();

    // --------------------------------------------------------
    // 7. VÉRIFICATION ET ÉCRAN DE VICTOIRE
    // --------------------------------------------------------

    if (!_gameOver && _moteur.isPartieTerminee()) {
        _winnerName = _moteur.getNomVainqueur();
        _gameOver = true;
        _gameContinuesAfterWin = false;
    }

    // Ecran titre
    if (_gameOver && !_gameContinuesAfterWin) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, _window.getSize().y));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
        ImGui::Begin("GameOverOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::End();
        ImGui::PopStyleColor();

        // Fenêtre de Victoire
        ImVec2 winSize(500, 250);
        ImGui::SetNextWindowPos(ImVec2((_window.getSize().x - winSize.x) / 2.0f, (_window.getSize().y - winSize.y) / 2.0f));
        ImGui::SetNextWindowSize(winSize);
        ImGui::Begin("Victoire", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
        
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::SetCursorPosX((winSize.x - ImGui::CalcTextSize("FIN DE LA PARTIE").x) / 2.0f);
        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "FIN DE LA PARTIE");
        ImGui::Separator();
        
        ImGui::Dummy(ImVec2(0, 20));
        std::string winText = "VICTOIRE DU COMMANDANT " + _winnerName + " !";
        ImGui::SetCursorPosX((winSize.x - ImGui::CalcTextSize(winText.c_str()).x) / 2.0f);
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "%s", winText.c_str());
        
        ImGui::Dummy(ImVec2(0, 40));
        float btnX = (winSize.x - 300) / 2.0f;

        ImGui::SetCursorPosX(btnX);
        if (ImGui::Button("CONTINUER A JOUER", ImVec2(300, 40))) {
            _gameContinuesAfterWin = true;
        }

        ImGui::Dummy(ImVec2(0, 10));
        ImGui::SetCursorPosX(btnX);
        if (ImGui::Button("RESUME DES SCORES", ImVec2(300, 40))) {
            _showSummaryPopup = true;
        }

        ImGui::Dummy(ImVec2(0, 10));
        ImGui::SetCursorPosX((winSize.x - 250) / 2.0f);
        if (ImGui::Button("RETOURNER AU MENU PRINCIPAL", ImVec2(300, 40))) {
            _gameOver = false;
            _winnerName = "";
            _gameContinuesAfterWin = false;
            _network.disconnect();
            _hasSelection = false;
            _casesPossibles.clear();
            _casesAttaquePossibles.clear();
            
            if (std::filesystem::exists("saves/last_save.json")) {
                std::filesystem::remove("saves/last_save.json");
            }
            
            _previousState = GameState::MENU;
            _currentState = GameState::MENU;
        }
        ImGui::End();

        // --- POPUP RÉSUMÉ DES SCORES ---
        if (_showSummaryPopup) ImGui::OpenPopup("Resume des Scores");
        
        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal("Resume des Scores", &_showSummaryPopup, ImGuiWindowFlags_NoSavedSettings)) {
            if (ImGui::BeginTable("ScoreTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
                ImGui::TableSetupColumn("Commandant"); ImGui::TableSetupColumn("Villes");
                ImGui::TableSetupColumn("Armee"); ImGui::TableSetupColumn("Economie");
                ImGui::TableHeadersRow();
                
                for (const Joueur& j : _moteur.getJoueurs()) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", j.getName().c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%d", j.getNbVilles());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("%d", (int)j.getUnites().size());
                    
                    int totalRessources = 0;
                    for (auto const& [resPtr, qte] : j.getInventaire()) totalRessources += qte;
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%d res.", totalRessources);
                }
                ImGui::EndTable();
            }
            ImGui::Dummy(ImVec2(0, 20));
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 120) * 0.5f);
            if (ImGui::Button("Fermer", ImVec2(120, 30))) {
                ImGui::CloseCurrentPopup();
                _showSummaryPopup = false;
            }
            ImGui::EndPopup();
        }
    }
    
    if (_gameOver && _gameContinuesAfterWin) {
        ImGui::SetNextWindowPos(ImVec2((_window.getSize().x - 400) / 2.0f, 50));
        ImGui::SetNextWindowSize(ImVec2(400, 40));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.7f, 0.3f, 1.0f));
        ImGui::Begin("WinnerBanner", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings);
        std::string text = "--- VICTOIRE DE " + _winnerName + " ---";
        ImGui::SetCursorPosX((400 - ImGui::CalcTextSize(text.c_str()).x) / 2.0f);
        ImGui::SetCursorPosY(10);
        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "%s", text.c_str());
        ImGui::End();
        ImGui::PopStyleColor(2);
    }

    // --------------------------------------------------------
    // 8. NOTIFICATION "C'EST VOTRE TOUR"
    // --------------------------------------------------------
    if (_turnNotificationTimer > 0.0f) {
        _turnNotificationTimer -= ImGui::GetIO().DeltaTime;
        
        ImVec2 centerUi = ImVec2(_window.getSize().x * 0.5f, _window.getSize().y * 0.3f);
        ImGui::SetNextWindowPos(centerUi, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        
        float alpha = std::min(1.0f, _turnNotificationTimer); 
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f * alpha));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.7f, 0.3f, alpha));
        ImGui::Begin("TurnNotification", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs);
        
        ImGui::Dummy(ImVec2(20, 10));
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, alpha), ">>> C'EST VOTRE TOUR DE COMMANDEMENT ! <<<");
        ImGui::Dummy(ImVec2(20, 10));
        
        ImGui::End();
        ImGui::PopStyleColor(2);
    }

    // ========================================================
    // POPUP : SAISIE DU NOM DE SAUVEGARDE
    // ========================================================
    if (ImGui::BeginPopupModal("SaveGamePopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "SAUVEGARDE DE LA PARTIE");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 5));

        ImGui::Text("Nom du fichier :");
        ImGui::InputText("##savename", _newSaveName, sizeof(_newSaveName)); 
        ImGui::Dummy(ImVec2(0, 10));

        if (ImGui::Button("Valider", ImVec2(120, 0))) {
            std::filesystem::create_directories("saves");
            
            std::string fullPath = "saves/" + std::string(_newSaveName) + ".json";
            
            if (SaveManager::saveGame(fullPath, _moteur)) {
                _popupMsg = "Partie sauvegardee avec succes !";
                _showPopup = true;
                ImGui::CloseCurrentPopup();
            } else {
                _popupMsg = "Erreur lors de la sauvegarde !";
                _showPopup = true;
            }
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Annuler", ImVec2(120, 0))) { 
            ImGui::CloseCurrentPopup(); 
        }
        
        ImGui::EndPopup();
    }
}


// ---------------------------------------------------------------//
// ------------- Fonctions de textures et themes ---------------- //
// ---------------------------------------------------------------//

void InterfaceManager::loadTextures() {
    // 1. Textures des tuiles de terrain
    if (_espaceJson.contains("tiles")) {
        for (auto& t : _espaceJson["tiles"]) {
            std::string texturePath = t.value("texture", "");
            std::string symboleStr = t.value("symbole", "");
            if (!texturePath.empty() && !symboleStr.empty()) {
                char symb = symboleStr[0];
                if (!_textures[symb].loadFromFile(texturePath)) {
                    std::cerr << "Erreur : Texture terrain introuvable -> " << texturePath << std::endl;
                }
            }
        }
    }
    // Tuile de bordure par défaut
    if (_textures.find('#') == _textures.end()) {
        _textures['#'].loadFromFile("assets/border.png");
    }

    // 2. Textures des Villes (Chargement dynamique)
    for (const auto& [nom, modele] : _moteur.getCityFactory().getCatalogue()) {
        std::string cTex = modele->getTexturePath();
        if (!cTex.empty() && !_cityTextures[nom].loadFromFile(cTex)) {
            std::cerr << "Erreur : Texture Ville introuvable -> " << cTex << " pour " << nom << std::endl;
        }
    }

    // 3. Textures des Bâtiments
    for (const auto& [nom, modele] : _moteur.getBatimentFactory().getCatalogue()) {
        std::string bTex = modele->getTexturePath();
        if (!bTex.empty() && !_buildingTextures[nom].loadFromFile(bTex)) {
            std::cerr << "Erreur : Texture Batiment introuvable -> " << bTex << std::endl;
        }
    }

    // 4. Textures des Unités
    for (const auto& [nom, modele] : _moteur.getUniteFactory().getCatalogue()) {
        std::string uTex = modele->texturePath();
        if (!uTex.empty() && !_unitTextures[nom].loadFromFile(uTex)) {
            std::cerr << "Erreur : Texture Unite introuvable -> " << uTex << std::endl;
        }
    }

    // 5. Textures des ressources
    for (const auto& [resName, resPtr] : _moteur.getRessourceFactory().getCatalogue()) {
        if (resPtr && !resPtr->getIconPath().empty()) {
            sf::Texture tex;
            if (tex.loadFromFile(resPtr->getIconPath())) {
                tex.setSmooth(true);
                _resourceIcons[resName] = tex;
            }
        }
    }

    // 6. Texture des factions
    if (_rulesJson.contains("factions")) {
        for (const auto& f : _rulesJson["factions"]) {
            std::string nomFac = f.value("nom", "");
            std::string imgPath = f.value("image", "");
            if (!nomFac.empty() && !imgPath.empty()) {
                if (!_factionTextures[nomFac].loadFromFile(imgPath)) {
                    std::cerr << "Erreur : Texture Faction introuvable -> " << imgPath << std::endl;
                }
            }
        }
    }
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
    ImGui::SameLine(0.0f, -1.0f);

    // On mémorise la position X exacte après le bouton "<"
    float startX = ImGui::GetCursorPosX();
    float width = 150.0f; 

    const char* text = items[*current_index].c_str();
    float textWidth = ImGui::CalcTextSize(text).x;

    // Centrage mathématique
    ImGui::SetCursorPosX(startX + (width - textWidth) * 0.5f);
    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", text);

    // On force la position du bouton ">"
    ImGui::SameLine(0.0f, -1.0f);
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
    
    ImGui::SameLine(0.0f, -1.0f);
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
    ImGui::PopStyleColor(1);
}

float InterfaceManager::getRotationAngle(direction dir) {
    switch (dir) {
        case direction::est:        return 0.0f;
        case direction::sud_est:    return 60.0f;
        case direction::sud_ouest:  return 120.0f;
        case direction::ouest:      return 180.0f;
        case direction::nord_ouest: return 240.0f;
        case direction::nord_est:   return 300.0f;
        default:                    return 0.0f;
    }
}

void InterfaceManager::renderUnitActions(Unite* u) {
    if (!u) return;

    ImGui::Dummy(ImVec2(0, 10));
    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "ROTATION DE L'UNITE");
    ImGui::Separator();

    bool isMultiplayer = (_network.getState() == NetworkState::CONNECTED || _network.getState() == NetworkState::HOSTING);
    int localJIdx = isMultiplayer ? _localPlayerIndex : _moteur.getCurrentPlayerTurn();

    int coutRot = _moteur.getCoutRotation();
    bool peutTourner = (u->point_action() >= coutRot); 
    
    if (!peutTourner) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "PA Insuffisants (%d requis)", coutRot);
        ImGui::BeginDisabled();
    } else {
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Cout : %d PA", coutRot);
    }

    auto drawRotBtn = [&](const char* label, direction dir) {
        // Coloration : Vert si direction actuelle, Jaune si prévisualisation
        bool isCurrent = (u->regarde() == dir);
        bool isPreview = (_hasPreviewRotation && _previewDirection == dir);
        
        if (isCurrent) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        else if (isPreview) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.7f, 0.3f, 1.0f));
        
        if (ImGui::Button(label, ImVec2(45, 45))) {
            _hasPreviewRotation = true;
            _previewDirection = dir; // On enregistre juste la volonté de tourner
        }

        if (isCurrent || isPreview) ImGui::PopStyleColor(1);
    };

    // --- POSITIONNEMENT EN HEXAGONE DES BOUTONS ---
    ImGui::Dummy(ImVec2(0, 5));
    
    // Ligne du haut (NO, NE)
    ImGui::Indent(65); 
    drawRotBtn("NO", direction::nord_ouest); 
    ImGui::SameLine(0, 15); 
    drawRotBtn("NE", direction::nord_est); 
    ImGui::Unindent(65);
    
    // Ligne du milieu (O, E)
    ImGui::Indent(35);
    drawRotBtn(" O", direction::ouest); 
    ImGui::SameLine(0, 75); 
    drawRotBtn(" E", direction::est);
    ImGui::Unindent(35);
    
    // Ligne du bas (SO, SE)
    ImGui::Indent(65); 
    drawRotBtn("SO", direction::sud_ouest); 
    ImGui::SameLine(0, 15); 
    drawRotBtn("SE", direction::sud_est); 
    ImGui::Unindent(65);

    // --- LE BOUTON DE CONFIRMATION ---
    if (_hasPreviewRotation && _previewDirection != u->regarde()) {
        ImGui::Dummy(ImVec2(0, 10));
        if (ImGui::Button("Confirmer Rotation", ImVec2(150, 40))) {
            
            CmdRotation cmd = { _selectedCellX, _selectedCellY, _previewDirection };
            if (_moteur.soumettreCommande(localJIdx, cmd) == ResultatAction::SUCCES) {
                if (isMultiplayer) {
                    sf::Packet pk;
                    pk << static_cast<sf::Int32>(PacketType::ACTION_ROTATE) << static_cast<sf::Int32>(localJIdx) << _selectedCellX << _selectedCellY << static_cast<sf::Int32>(_previewDirection);
                    _network.sendData(pk);
                }
            }
            _hasPreviewRotation = false;
        }
    }

    if (!peutTourner) ImGui::EndDisabled();
}

void InterfaceManager::sendLobbySync() {
    if (_network.isHost() && _network.getState() == NetworkState::CONNECTED) {
        sf::Packet p;
        p << static_cast<sf::Int32>(PacketType::LOBBY_STATE);
        
        // 1. L'état actuel de l'UI de l'hôte
        p << static_cast<sf::Int32>(_currentState); 
        
        // 2. La liste des joueurs
        p << static_cast<sf::Int32>(_connectedPlayers.size());
        for (size_t i = 0; i < _connectedPlayers.size(); ++i) {
            std::string fac = (i < _playerFactions.size()) ? _playerFactions[i] : "";
            p << _connectedPlayers[i].name << fac;
        }

        // 3. Les paramètres de la carte
        p << static_cast<sf::Int32>(_mapSeed) << static_cast<sf::Int32>(_numPlayers) << static_cast<sf::Int32>(_selectedVictoryIndex);
        p << static_cast<sf::Int32>(_rulesJson["taille_plateau"]["x"].get<int>());
        p << static_cast<sf::Int32>(_rulesJson["taille_plateau"]["y"].get<int>());
        
        // 4. Les poids du générateur (Custom Weights)
        p << static_cast<sf::Int32>(_customWeights.size());
        for (auto const& [symb, weight] : _customWeights) {
            p << static_cast<sf::Int32>(symb) << static_cast<sf::Int32>(weight);
        }

        // 5. Informations de reprise de sauvegarde
        p << static_cast<sf::Int32>(_isHostingLoadedSave ? 1 : 0);
        if (_isHostingLoadedSave) {
            p << static_cast<sf::Int32>(_playerToSlotMapping.size());
            for (int slot : _playerToSlotMapping) {
                p << static_cast<sf::Int32>(slot);
            }
        }

        _network.sendData(p);
    }
}

void InterfaceManager::addCombatLog(const std::string& msg, sf::Color col) {
    _combatLogs.push_back({msg, col, 8.0f});
    if (_combatLogs.size() > 10) {
        _combatLogs.erase(_combatLogs.begin());
    }
}

void InterfaceManager::refreshSaveList() {
    _saveFiles.clear();
    std::filesystem::create_directories("saves");
    for (const auto& entry : std::filesystem::directory_iterator("saves")) {
        if (entry.path().extension() == ".json" && entry.path().filename() != "config_ui.json") {
            _saveFiles.push_back(entry.path().filename().string());
        }
    }
}

void InterfaceManager::renderLoadMenu() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(_window.getSize().x, _window.getSize().y));
    ImGui::Begin("Menu Chargement", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::TextColored(ImVec4(0.8f, 0.7f, 0.3f, 1.0f), "SAUVEGARDES DISPONIBLES");
    ImGui::Separator();

    if (_saveFiles.empty()) {
        ImGui::Text("Aucune sauvegarde trouvee.");
    } else {
        for (const auto& fileName : _saveFiles) {
            bool isSelected = (_selectedSave == fileName);
            if (ImGui::Selectable(fileName.c_str(), isSelected)) {
                _selectedSave = fileName;
                strncpy(_renameBuffer, fileName.c_str(), sizeof(_renameBuffer));
            }
        }
    }

    ImGui::Dummy(ImVec2(0, 20));
    
    if (!_selectedSave.empty()) {
        ImGui::Checkbox("Heberger en Multijoueur", &_loadAsMultiplayer);
        ImGui::Dummy(ImVec2(0, 10));

        if (ImGui::Button("CHARGER", ImVec2(120, 40))) {
            if (SaveManager::loadGame("saves/" + _selectedSave, _moteur)) {
                
                if (_loadAsMultiplayer) {
                    _playerToSlotMapping.clear();
                    _playerToSlotMapping.assign(_maxPlayersBuffer, -1);
                    _playerToSlotMapping[0] = 0;

                    _isHostingLoadedSave = true;
                    _network.startHosting(5000);
                    _currentState = GameState::HOST_LOBBY;
                } else {
                    _isHostingLoadedSave = false;
                    _currentState = GameState::IN_GAME;
                }
            } else {
                _popupMsg = "Erreur lors du chargement !";
                _showPopup = true;
            }
        }
        ImGui::SameLine();
        
        ImGui::SetNextItemWidth(200);
        ImGui::InputText("##rename", _renameBuffer, sizeof(_renameBuffer));
        ImGui::SameLine();
        if (ImGui::Button("RENOMMER")) {
            std::filesystem::rename("saves/" + _selectedSave, "saves/" + std::string(_renameBuffer));
            refreshSaveList();
            _selectedSave = std::string(_renameBuffer);
        }
    }

    if (ImGui::Button("RETOUR", ImVec2(120, 40))) {
        _currentState = GameState::MENU;
    }

    ImGui::End();
}