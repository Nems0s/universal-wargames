#pragma once
#include <string>
#include <nlohmann/json.hpp>

class MoteurDeJeu;

class SaveManager {
public:
    // Retourne true si succès, false si erreur
    static bool saveGame(const std::string& filename, MoteurDeJeu& moteur);
    static bool loadGame(const std::string& filename, MoteurDeJeu& moteur);
};