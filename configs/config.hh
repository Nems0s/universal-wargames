#include <string>
#include <map>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Data-Driven Design pour avoir une map
struct FactionParams {
    std::string nom;
    std::map<std::string, float> params;
};

class GameConfig {
    private:
        int _coutBaseVille;
        float _multiplicateurVille;
        int _maxLevelVille;
        int _rayonBaseVille;
        int _distanceMinVilles;

        std::map<std::string, FactionParams> _factions;

    public:
        GameConfig() : _coutBaseVille(100), _multiplicateurVille(1.5f), _distanceMinVilles(3) {}

        void load(const std::string& chemin);

        int getCoutBaseVille() const { return _coutBaseVille; }
        float getMultiplicateurVille() const { return _multiplicateurVille; }
        int getMaxLevelVille() const { return _maxLevelVille; }
        int getRayonBaseVille() const { return _rayonBaseVille; }
        int getDistanceMinVilles() const { return _distanceMinVilles; }
        
        const FactionParams* getFaction(const std::string& nom) const;
};