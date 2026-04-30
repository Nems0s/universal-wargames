#pragma once
#include <vector>
#include <memory>
#include <map>
#include "unite.hh"
#include "batiment.hh"
#include "city.hh"
#include "joueur.hh"
#include "config.hh"

class Unite;

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
        std::map<char, int> _customWeights;

        std::string _generationMode = "random";
        float _perlinScale = 0.15f;
        int _perlinOctaves = 4;
        float _perlinLacunarity = 2.0f;
        float _perlinPersistence = 0.5f;
        float _perlinRedistribution = 1.0f;
        bool _perlinInversion = false;
        std::map<float, char> _seuilsPerlin;
        std::map<std::string, std::map<char, int>> _presetsWorld;

    public:
        void ajouterAuCatalogue(char symbole, const TuileData& data);

        void initialiserBords();

        std::unique_ptr<hexa> createTile(char symbole);
        std::unique_ptr<hexa> createRandomTile();

        void postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>>& matrix, int width, int height);
        void overrideWeights(const std::map<char, int>& overrides);

        bool estVide() const;

        const std::map<char, TuileData>& getCatalogue() const { return _catalogue; }

        // Setters pour la génération
        void setGenerationMode(const std::string& mode) { _generationMode = mode; }
        void setPerlinScale(float scale) { _perlinScale = scale; }
        void setPerlinOctaves(int o) { _perlinOctaves = o; }
        void setPerlinLacunarity(float l) { _perlinLacunarity = l; }
        void setPerlinPersistence(float p) { _perlinPersistence = p; }
        void setPerlinRedistribution(float r) { _perlinRedistribution = r; }
        void setPerlinInversion(bool inv) { _perlinInversion = inv; }
        void ajouterSeuilPerlin(float seuil, char symb) { _seuilsPerlin[seuil] = symb; }
        void ajouterPreset(const std::string& nom, const std::map<char, int>& poids) { _presetsWorld[nom] = poids; }
        void setCustomWeights(const std::map<char, int>& w) { _customWeights = w; }

        // Getters
        std::string getGenerationMode() const { return _generationMode; }
        float getPerlinScale() const { return _perlinScale; }
        int getPerlinOctaves() const { return _perlinOctaves; }
        float getPerlinLacunarity() const { return _perlinLacunarity; }
        float getPerlinPersistence() const { return _perlinPersistence; }
        float getPerlinRedistribution() const { return _perlinRedistribution; }
        bool getPerlinInversion() const { return _perlinInversion; }
        const std::map<float, char>& getSeuilsPerlin() const { return _seuilsPerlin; }
        const std::map<std::string, std::map<char, int>>& getPresetsWorld() const { return _presetsWorld; }
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

//===================================================================
//                        Board
//===================================================================

class board {
public:
    board(int seed, WorldFactory & world, const GameConfig& c);

    const hexa* getCell(int i, int j) const;
    void affichage() const;

    void placerUnite(int x, int y, std::shared_ptr<Unite> u);
    bool deplacerUnite(Unite& u, int xDest, int yDest);
    Unite * getUnite(int x, int y) const;
    std::shared_ptr<Unite> extraireUnite(int x, int y);
    void retirerUnite(int x, int y);

    int getRows() const { return _matrix.size(); }
    int getCols() const { return _matrix.empty() ? 0 : _matrix[0].size(); }

private:
    int _width;
    int _height;
    const GameConfig& _config;
    std::vector<std::vector<std::unique_ptr<hexa>>> _matrix;
    std::map<std::pair<int, int>, std::shared_ptr<Unite>> _unites;
};

