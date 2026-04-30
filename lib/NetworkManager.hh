#pragma once
#include <SFML/Network.hpp>
#include <iostream>
#include <string>

enum class NetworkState { DISCONNECTED, HOSTING, CONNECTING, CONNECTED };

enum class PacketType : sf::Int32 {
    PLAYER_INFO,   // Envoi du nom au lobby
    LOBBY_STATE,
    LOAD_SAVE_STATE,
    CHAT,
    GAME_START,    // L'hôte lance la partie (Seed + Factions)
    ACTION_MOVE,   // Déplacement d'unité
    ACTION_BUILD,  // Construction de bâtiment
    ACTION_BUILD_CITY,
    ACTION_RECRUIT,
    ACTION_ATTACK, // Attaque d'unité
    ACTION_ROTATE, // Changement d'orientation d'unité
    ACTION_UPGRADE_CITY, 
    ACTION_BUY_TILE,
    ACTION_DESTROY_UNIT,
    SYNC_BUILD,     // Envoie des informations de build PEUT CHEAT AVEC CHEAT ENGINE CAR DANS LA RAM
    END_TURN       // Fin de tour
};

class NetworkManager {
private:
    sf::TcpSocket _socket;
    sf::TcpListener _listener;
    NetworkState _state = NetworkState::DISCONNECTED;
    bool _isHost = false;

public:
    NetworkManager();

    // Lancement de la connexion
    void startHosting(unsigned short port);
    void connectToHost(const std::string& ip, unsigned short port);
    
    // À appeler à chaque frame dans ton interface pour vérifier si on est connecté
    void update(); 

    // Accesseurs
    NetworkState getState() const { return _state; }
    bool isHost() const { return _isHost; }
    
    // Envoyer / Recevoir (On verra les détails des paquets plus tard)
    bool sendData(sf::Packet& packet);
    bool receiveData(sf::Packet& packet);
    
    void disconnect();
};