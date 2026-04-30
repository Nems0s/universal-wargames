#include "NetworkManager.hh"

NetworkManager::NetworkManager() {
    // Mode non-bloquant pour ne pas figer le jeu
    _socket.setBlocking(false);
    _listener.setBlocking(false);
}

void NetworkManager::startHosting(unsigned short port) {
    if (_listener.listen(port) == sf::Socket::Done) {
        _state = NetworkState::HOSTING;
        _isHost = true;
        std::cout << "En attente de connexion sur le port " << port << "..." << std::endl;
    } else {
        std::cerr << "Erreur : Impossible d'écouter sur le port " << port << std::endl;
    }
}

void NetworkManager::connectToHost(const std::string& ip, unsigned short port) {
    _state = NetworkState::CONNECTING;
    _isHost = false;
    _socket.setBlocking(true); // Bloquant juste pour la tentative initiale
    
    if (_socket.connect(ip, port, sf::seconds(5)) == sf::Socket::Done) {
        _state = NetworkState::CONNECTED;
        _socket.setBlocking(false);
        std::cout << "Connecté au serveur !" << std::endl;
    } else {
        _state = NetworkState::DISCONNECTED;
        std::cerr << "Erreur de connexion au serveur." << std::endl;
    }
}

void NetworkManager::update() {
    if (_state == NetworkState::HOSTING) {
        // On vérifie si quelqu'un essaie de se connecter
        if (_listener.accept(_socket) == sf::Socket::Done) {
            _state = NetworkState::CONNECTED;
            std::cout << "Un joueur a rejoint la partie !" << std::endl;
        }
    }
}

void NetworkManager::disconnect() {
    _socket.disconnect();
    _listener.close();
    _state = NetworkState::DISCONNECTED;
}

bool NetworkManager::sendData(sf::Packet& packet) {
    if (_state == NetworkState::CONNECTED) {
        sf::Socket::Status status = _socket.send(packet);
        if (status == sf::Socket::Disconnected) {
            if (_isHost) {
                _socket.disconnect();
                _state = NetworkState::HOSTING;
            } else disconnect();
        }
        return status == sf::Socket::Done;
    }
    return false;
}

bool NetworkManager::receiveData(sf::Packet& packet) {
    if (_state == NetworkState::CONNECTED) {
        sf::Socket::Status status = _socket.receive(packet);
        if (status == sf::Socket::Disconnected) {
            if (_isHost) {
                _socket.disconnect();
                _state = NetworkState::HOSTING;
            } else disconnect();
        }
        return status == sf::Socket::Done;
    }
    return false;
}