#pragma once
#include <vector>
#include "batiment.hh"

class City {
    private:
        int _level;
        int _maxLevel;
        size_t _nbBatiments;
        bool _estCapitale;
        std::vector<std::unique_ptr<Batiment>> _batiments;
        int _x, _y;

    public:
        City(int max=5, bool capitale=false, int x, int y, int level=1);
        //level à la fin car comme ça pas besoin de le mettre lors de la création

        bool peutAjouterBatiment() const;
        void creeBatiment(std::unique_ptr<Batiment> b);
        bool estCapitale() const;
        void product(Joueur & j);

        int getLevel() const { return _level; }
        int getMaxLevel() const { return _maxLevel; }

        int getX() const { return _x; }
        int getY() const { return _y; }
};
