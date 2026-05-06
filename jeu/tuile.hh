#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "ressource.hh"
#include "unite.hh"
#include "batiment.hh"
#include "city.hh"

class Joueur;

//===================================================================
//                              Tools
//===================================================================
struct MouvementData {
    bool marche;
    bool nage;
    bool aerien;
};

struct GenerationData {
    int poids;
    int nbMin;
};

struct EnvironnementData {
    float temperature;
    float radiation;
    float gravite;
};

struct TuileData {
    std::string nom;
    char symbole;
    int cout;
    bool constructible;
    MouvementData mouv;
    GenerationData gen;
    EnvironnementData env;
    std::vector<const Ressource*> ressourceSpeciale;
    std::map<std::string, float> properties;
};

struct PerlinParams {
    float scale;
    int octaves;
    float lacunarity;
    float persistence;
    float redistribution;
    bool inversion;
    std::map<float, char> seuils;
};

//===================================================================
//                              Tuiles
//===================================================================

class hexa {
public:
    virtual ~hexa() = default;
    virtual bool estFranchissable(const Unite & u) const = 0;
    virtual std::string getType() const = 0;
    virtual int getCoutDeplacement() const = 0;

    virtual char getSymbole() const = 0; 
};



class TuileConfigurable : public hexa {
public:
    TuileConfigurable(const TuileData* data);

    std::string getType() const override;
    char getSymbole() const override;
    int getCoutDeplacement() const override;
    std::vector<const Ressource*> getRessource() const;
    
    bool estFranchissable(const Unite& u) const override;

    bool peutConstrVille() const;
    bool peutConstrBatiment(const Batiment & b) const;
    bool peutConstrBatimentSpecial(const Batiment & b) const;

    void placerVille(std::unique_ptr<City> c);
    void constrBatimentSpeciale(std::unique_ptr<Batiment> b);

    City * getCity() const;
    const Batiment* getBatimentSpecial() const { return _batimentSpecial.get(); }

    float getStat(const std::string & key) const;
    void setStat(const std::string & key, float val);

    void setProprietaire(Joueur* j) { _proprietaire = j; }
    Joueur* getProprietaire() const { return _proprietaire; }

private:
    const TuileData* _d; // Pour eviter de dupliquer les même tuiles (comme espace)
    std::map<std::string, float> _localStats; // données modifiés des struct
    std::unique_ptr<City> _city;
    std::unique_ptr<Batiment> _batimentSpecial;
    Joueur* _proprietaire = nullptr;
};


//===================================================================
//                        Factory/Config
//===================================================================
class WorldFactory {
    private:
        std::map<char, TuileData> _catalogue;

        std::string _generationMode = "random";
        std::string _activePresetName = "";
        PerlinParams _activePerlinParams;
        std::map<char, int> _customWeights;

        std::map<std::string, PerlinParams> _presetsPerlin;
        std::map<std::string, std::map<char, int>> _presetsRandom;

    public:
        void ajouterAuCatalogue(char symbole, const TuileData& data);
        void initialiserBords();
        std::unique_ptr<hexa> createTile(char symbole);
        std::unique_ptr<hexa> createRandomTile();
        void postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>>& matrix, int width, int height);
        void overrideWeights(const std::map<char, int>& overrides);
        bool estVide() const;

        // Gestion des presets
        void ajouterPresetPerlin(const std::string& nom, const PerlinParams& p) { _presetsPerlin[nom] = p; }
        void ajouterPresetRandom(const std::string& nom, const std::map<char, int>& w) { _presetsRandom[nom] = w; }
        bool appliquerPreset(const std::string& mode, const std::string& nomPreset);

        // Getters & Setters
        void setGenerationMode(const std::string& mode) { _generationMode = mode; }
        std::string getGenerationMode() const { return _generationMode; }
        
        void setActivePerlinParams(const PerlinParams& p) { _activePerlinParams = p; _activePresetName = "Custom"; }
        const PerlinParams& getActivePerlinParams() const { return _activePerlinParams; }

        std::string getActivePresetName() const { return _activePresetName.empty() ? "Custom" : _activePresetName; }

        const std::map<float, char>& getSeuilsPerlin() const { return _activePerlinParams.seuils; }
        void setCustomWeights(const std::map<char, int>& w) { _customWeights = w; }
        const std::map<char, TuileData>& getCatalogue() const { return _catalogue; }
        
        // Liste Preset pour UI
        const std::map<std::string, PerlinParams>& getPresetsPerlinDispos() const { return _presetsPerlin; }
        const std::map<std::string, std::map<char, int>>& getPresetsRandomDispos() const { return _presetsRandom; }

        std::map<char, int>& getCustomWeightsActuels() { return _customWeights; }
        void setPresetName(const std::string& nom) { _activePresetName = nom; }
};

class WorldConfigReader {
public:
    virtual ~WorldConfigReader() = default;

    virtual void chargerConfig(std::string chemin, const std::map<std::string, const Ressource*>& ressourcesDispo, WorldFactory& factory) = 0;
};

class TxtWorldReader : public WorldConfigReader
{
public:
    void chargerConfig(std::string cheminFichier, const std::map<std::string, const Ressource*> & ressourcesDispo, WorldFactory& factory) override;
};

class JsonWorldReader : public WorldConfigReader
{
public:
    void chargerConfig(std::string cheminFichier, const std::map<std::string, const Ressource*> & ressourcesDispo, WorldFactory& factory) override;
};