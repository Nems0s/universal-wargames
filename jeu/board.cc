#include "board.hh"
#include "FastNoiseLite.hh"

#include <limits>
#include <iostream>

//===================================================================
//                              Board
//===================================================================
board::board(int seed, WorldFactory & world, const GameConfig& config): _config(config) {
    _width = config.getPlateauX();
    _height = config.getPlateauY();

    // Config de FastNoiseLite
    FastNoiseLite noise;
    noise.SetSeed(seed);
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    std::string mode = world.getGenerationMode();
    const PerlinParams& params = world.getActivePerlinParams();
    noise.SetFrequency(params.scale);

    // Activer FBm multi-octaves pour un terrain riche et organique
    if (mode == "perlin") {
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(params.octaves);
        noise.SetFractalLacunarity(params.lacunarity);
        noise.SetFractalGain(params.persistence);
    }

    if (mode == "perlin" && !world.getSeuilsPerlin().empty()) {
        float globalMin = std::numeric_limits<float>::max();
        float globalMax = std::numeric_limits<float>::lowest();
        
        std::vector<std::vector<float>> noiseGrid(_height, std::vector<float>(_width, 0.0f));
        
        for (int i = 1; i < _height - 1; ++i) {
            for (int j = 1; j < _width - 1; ++j) {
                float val = noise.GetNoise((float)j, (float)i);
                noiseGrid[i][j] = val;
                if (val < globalMin) globalMin = val;
                if (val > globalMax) globalMax = val;
            }
        }
        
        float range = globalMax - globalMin;
        if (range < 0.001f) range = 1.0f;
        
        float redistribution = params.redistribution;
        bool inversion = params.inversion;
        
        for (int i = 0; i < _height; ++i) {
            std::vector<std::unique_ptr<hexa>> ligne;
            for (int j = 0; j < _width; ++j) {
                if (i == 0 || i == _height - 1 || j == 0 || j == _width - 1) {
                    ligne.push_back(world.createTile('#'));
                } else {
                    // Normalisation dynamique
                    float normalized = (noiseGrid[i][j] - globalMin) / range;
                    
                    // Redistribution (power curve) pour enrichir la distribution
                    normalized = std::pow(normalized, redistribution);
                    
                    // Inversion pour thèmes îles (hautes valeurs = terre)
                    if (inversion) normalized = 1.0f - normalized;
                    
                    // Clamp
                    if (normalized < 0.0f) normalized = 0.0f;
                    if (normalized > 1.0f) normalized = 1.0f;
                    
                    // Application des seuils
                    char symboleChoisi = '.';
                    for (const auto& [seuil, symb] : world.getSeuilsPerlin()) {
                        if (normalized <= seuil) {
                            symboleChoisi = symb;
                            break;
                        }
                    }
                    ligne.push_back(world.createTile(symboleChoisi));
                }
            }
            _matrix.push_back(std::move(ligne));
        }
    } else {
        // Mode Random (_customWeights)
        for (int i = 0; i < _height; ++i) {
            std::vector<std::unique_ptr<hexa>> ligne;
            for (int j = 0; j < _width; ++j) {
                if (i == 0 || i == _height - 1 || j == 0 || j == _width - 1) {
                    ligne.push_back(world.createTile('#'));
                } else {
                    ligne.push_back(world.createRandomTile());
                }
            }
            _matrix.push_back(std::move(ligne));
        }
    }
    
    world.postGeneration(_matrix, _width, _height);
}

const hexa* board::getCell(int i, int j) const
{
    return _matrix[i][j].get();
}

void board::affichage() const {
    for (int i = 0; i < _height; ++i) {
        if (i%2 == 0) {
            std::cout << " ";
        }
        for (int j = 0; j < _width; ++j) 
        {
            Unite* u = getUnite(i, j);
            if (u != nullptr) 
            {
                std::cout << u->getSymbol() << " ";
            }
            else std::cout << _matrix[i][j]->getSymbole() << " ";
        }
        std::cout << std::endl;
    }
}


void board::placerUnite(int x, int y, std::shared_ptr<Unite> u) {
    if (x >= 0 && x < _height && y >= 0 && y < _width) 
    {
        _unites[{x, y}] = std::move(u);
    }
}
Unite * board::getUnite(int x, int y) const {
    auto it = _unites.find({x, y});
    if (it != _unites.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool board::deplacerUnite(Unite& u, int xDest, int yDest) {
    int xSrc = u.location().first;
    int ySrc = u.location().second;

    //Unite selectionner aux Coord
    auto it = _unites.find({xSrc, ySrc});
    if (it == _unites.end()) return false;

    //Coord dans la carte
    if (xDest < 0 || xDest >= _height || yDest < 0 || yDest >= _width) return false;

    //Personne aux Coord
    if (_unites.count({xDest, yDest})) return false;

    //Test de franchissement
    if (!_matrix[xDest][yDest]->estFranchissable(*(it->second)))
    {
        return false;
    }

    //Test cible valide, avec la portée
    for(auto const& mouv : u.Mobilite())
    {
        if(mouv->EstCaseValide({xSrc, ySrc}, {xDest, yDest}))
        {
            _unites[{xDest, yDest}] = std::move(it->second);
            _unites.erase(it);
            return true;
        }
    }
    return false;
}

void board::retirerUnite(int x, int y) {
    _unites.erase({x, y});
}

std::shared_ptr<Unite> board::extraireUnite(int x, int y) {
    auto it = _unites.find({x, y});
    if (it != _unites.end()) {
        auto u = std::move(it->second);
        _unites.erase(it);
        return u;
    }
    return nullptr;
}
