#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "batiment.hh"
#include "ressource.hh"

class City {
    private:
        int _x, _y;
        std::string _nom;
        bool _estCapitale;
        int _level;
        int _maxLevel;

        float _pvMaxBase;
        float _pvCurrent;
        float _pvMax;

        float _damageBase;
        float _damage;

        int _visionRangeBase;
        int _visionRange;

        int _rayonBase;
        std::map<const Ressource*, int> _coutBase;
        std::string _texturePath;

        size_t _nbBatiments;
        std::vector<std::unique_ptr<Batiment>> _batiments;

    public:
        City(int x, int y, const std::string & nom, bool capitale, int maxLvl, float pvB, float dmgB, int visB, int rayB, const std::map<const Ressource*, int> & coutB, const std::string & tex);

        std::unique_ptr<City> clone(int x, int y) const;

        // Getters
        int getX() const { return _x; }
        int getY() const { return _y; }
        std::string getNom() const { return _nom; }
        bool estCapitale() const { return _estCapitale; }
        int getLevel() const { return _level; }
        int getMaxLevel() const { return _maxLevel; }
        float getPv() const { return _pvCurrent; }
        float getPvMax() const { return _pvMax; }
        float getDegats() const { return _damage; }
        int getVisionRange() const { return _visionRange; }
        int getRayonTerritoire() const { return _rayonBase + _level - 1; }
        std::string getTexturePath() const { return _texturePath; }
        const std::map<const Ressource*, int>& getCoutBase() const { return _coutBase; }

        bool peutAjouterBatiment() const;
        void creeBatiment(std::unique_ptr<Batiment> b);
        const std::vector<std::unique_ptr<Batiment>>& getBatiments() const { return _batiments; }

        void takeDamage(int d);
        void upgrade();
        void product(Joueur & j);
};

class JsonCityReader {
public:
    void load(const std::string & chemin, std::map<std::string, std::shared_ptr<City>> & catalogue, const std::map<std::string, const Ressource*> & ressourcesDispo);
};

class CityFactory {
private:
    std::map<std::string, std::shared_ptr<City>> _catalogue;
public:
    void chargerConfiguration(const std::string & chemin, JsonCityReader & lecteur, const std::map<std::string, const Ressource*> & ressourcesDispo);
    std::unique_ptr<City> create(const std::string & nom, int x, int y) const;
    const std::map<std::string, std::shared_ptr<City>> & getCatalogue() const { return _catalogue; }
};