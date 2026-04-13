#pragma once
#include <vector>
#include "batiment.hh"

class City {
    private:
        int _level;
        int _maxLevel;
        float _pvCurrent;
        float _pvMax;
        float _damage;
        size_t _nbBatiments;
        bool _estCapitale;
        std::vector<std::unique_ptr<Batiment>> _batiments;
        int _x, _y;

    public:
        City(int max=5, bool capitale=false, int x, int y, const GameConfig & config, int level=1);
        //level à la fin car comme ça pas besoin de le mettre lors de la création

        bool peutAjouterBatiment() const;
        void creeBatiment(std::unique_ptr<Batiment> b);
        bool estCapitale() const;
        void product(Joueur & j);

        int getLevel() const { return _level; }
        int getMaxLevel() const { return _maxLevel; }

        float getPv() const { return _pvCurrent; }
        float getPvMax() const { return _pvMax; }
        float getDegats() const { return _damage; }

        void takeDamage(int d);

        int getX() const { return _x; }
        int getY() const { return _y; }
};
