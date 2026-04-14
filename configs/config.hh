#pragma once
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Data-Driven Design pour avoir une map
struct FactionParams {
    std::string nom;
    std::map<std::string, float> params;
};

enum class WinType { RESOURCE, CITY_COUNT, UNIT_COUNT, CAPITAL_REQ };
enum class WinMode { ALL, ANY };

struct WinConditions {
    WinType type;
    std::string resourceName;
    int targetAmount;
    bool required;
};

struct VictorySet {
    std::string name;
    WinMode mode;
    std::vector<WinConditions> conditions;
};

class GameConfig {
    private:
        int _plateauX;
        int _plateauY;

        int _coutBaseVille;
        float _multiplicateurVille;
        int _distanceMinVilles;
        int _pvMaxVille;
        int _degatsVille;

        int _maxLevelVille;
        int _rayonBaseVille;

        std::vector<WinConditions> _winConds;
        std::vector<VictorySet> _victorySets;

        std::map<std::string, FactionParams> _factions;

    public:
        GameConfig() : _plateauX(10), _plateauY(10), _coutBaseVille(100), _multiplicateurVille(1.5f), _distanceMinVilles(3), _pvMaxVille(200), _degatsVille(20), _maxLevelVille(5), _rayonBaseVille(2) {}

        void loadRules(const std::string& chemin);
        void loadWins(const std::string& chemin);

        const std::vector<WinConditions>& getWinConditions() const { return _winConds; }
        const std::vector<VictorySet>& getVictorySets() const { return _victorySets; }

        int getPlateauX() const { return _plateauX; }
        int getPlateauY() const { return _plateauY; }

        int getCoutBaseVille() const { return _coutBaseVille; }
        float getMultiplicateurVille() const { return _multiplicateurVille; }
        int getPvMaxVille() const { return _pvMaxVille; }
        int getDegatsVille() const { return _degatsVille; }
        
        int getMaxLevelVille() const { return _maxLevelVille; }

        int getRayonBaseVille() const { return _rayonBaseVille; }
        int getDistanceMinVilles() const { return _distanceMinVilles; }
        
        const FactionParams* getFaction(const std::string& nom) const;
};