#pragma once
#include <string>
#include <nlohmann/json.hpp>

// Forward declaration pour éviter les inclusions circulaires
class InterfaceManager; 

class SaveManager {
public:
    // Retourne true si succès, false si erreur
    static bool saveGame(const std::string& filename, InterfaceManager* ui);
    static bool loadGame(const std::string& filename, InterfaceManager* ui);
};