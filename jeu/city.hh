#pragma once
#include <vector>
#include "batiment.hh"
#include "config.hh"
#include "config.hh"

class City {
    private:
        int _x, _y;
        const GameConfig & _config;
        int _maxLevel;
        bool _estCapitale;
        int _level;

        float _pvCurrent;
        float _pvMax;
        float _damage;
        size_t _nbBatiments;
        
        std::vector<std::unique_ptr<Batiment>> _batiments;


    public:
        City(int x, int y, const GameConfig& config, int max=5, bool capitale=false, int level=1);

        bool peutAjouterBatiment() const;
        void creeBatiment(std::unique_ptr<Batiment> b);
        bool estCapitale() const;
        void product(Joueur & j);

        void upgrade();

        int getLevel() const { return _level; }
        int getMaxLevel() const { return _maxLevel; }

        float getPv() const { return _pvCurrent; }
        float getPvMax() const { return _pvMax; }
        float getDegats() const { return _damage; }

        void takeDamage(int d);

        int getX() const { return _x; }
        int getY() const { return _y; }
};
