#pragma once
#include <vector>
#include <memory>
#include <map>

#include "unite.hh"
#include "tuile.hh"
#include "config.hh"

class Unite;

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

