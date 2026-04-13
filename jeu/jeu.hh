#pragma once
#include <vector>
#include <memory>
#include <map>
#include "unite.hh"
#include "batiment.hh"
#include "city.hh"
#include "unite.hh"
#include "joueur.hh"
#include "config.hh"

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
    std::vector<Ressource*> ressourceSpeciale;
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
    std::vector<Ressource*> getRessource() const;
    
    bool estFranchissable(const Unite& u) const override;

    bool peutConstrVille() const;
    bool peutConstrBatiment(const Batiment & b) const;
    bool peutConstrBatimentSpecial(const Batiment & b) const;

    void constrVille(int x, int y, const GameConfig& config, int max, bool capitale);
    void constrBatimentSpeciale(std::unique_ptr<Batiment> b);

    City * getCity() const;

    float getStat(const std::string & key) const;
    void setStat(const std::string & key, float val);

private:
    const TuileData* _d; // Pour eviter de dupliquer les même tuiles (comme espace)
    std::map<std::string, float> _localStats; // données modifiés des struct
    std::unique_ptr<City> _city;
    std::unique_ptr<Batiment> _batimentSpecial;
};


//===================================================================
//                        Factory/Config
//===================================================================
class WorldFactory {
    private:
        std::map<char, TuileData> _catalogue;
    public:
        void ajouterAuCatalogue(char symbole, const TuileData& data);

        void initialiserBords();

        std::unique_ptr<hexa> createTile(char symbole);
        std::unique_ptr<hexa> createRandomTile();

        void postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>>& matrix, int width, int height);

        bool estVide() const;
};

class WorldConfigReader {
public:
    virtual ~WorldConfigReader() = default;

    virtual void chargerConfig(std::string chemin, const std::map<std::string, Ressource*>& ressourcesDispo, WorldFactory& factory) = 0;
};

class TxtWorldReader : public WorldConfigReader
{
public:
    void chargerConfig(std::string cheminFichier, const std::map<std::string, Ressource*> & ressourcesDispo, WorldFactory& factory) override;
};

class JsonWorldReader : public WorldConfigReader
{
public:
    void chargerConfig(std::string cheminFichier, const std::map<std::string, Ressource*> & ressourcesDispo, WorldFactory& factory) override;
};

//===================================================================
//                        Board
//===================================================================

class board {
public:
    board(WorldFactory & world, const GameConfig& c);

    const hexa* getCell(int i, int j) const;
    void affichage() const;

    void placerUnite(int x, int y, std::unique_ptr<Unite> u);
    bool deplacerUnite(Unite& u, int xDest, int yDest);
    Unite * getUnite(int x, int y) const;

    int getRows() const { return _matrix.size(); }
    int getCols() const { return _matrix.empty() ? 0 : _matrix[0].size(); }

private:
    int _width;
    int _height;
    const GameConfig& _config;
    std::vector<std::vector<std::unique_ptr<hexa>>> _matrix;
    std::map<std::pair<int, int>, std::unique_ptr<Unite>> _unites;
};

