#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <random>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

// Exemple actuel à supprimer quand class unité créer
class Unite {
    public:
        virtual ~Unite() = default;
        virtual bool peutMarcher() const { return false; }
        virtual bool peutNager() const { return false; }
};

class terre : virtual public Unite {
    public:
        bool peutMarcher() const override { return true; }
};

class mer : virtual public Unite {
    public:
        bool peutNager() const override { return true; }
};

class amphibie : public terre, public mer {};











class hexa {
public:
    virtual ~hexa() = default;
    virtual bool estFranchissable(const Unite & u) const = 0;
    virtual std::string getType() const = 0;
    virtual int getCoutDeplacement() const = 0;

    virtual char getSymbole() const = 0; 
};

struct TuileData {
    std::string nom;
    char symbole;
    int cout;
    bool marche;
    bool nage;
    int poids;
    int nbMin;
};

class TuileConfigurable : public hexa {
public:
    TuileConfigurable(const TuileData& data) : _d(data) {}

    std::string getType() const override { return _d.nom; }
    char getSymbole() const override { return _d.symbole; }
    int getCoutDeplacement() const override { return _d.cout; }
    
    bool estFranchissable(const Unite& u) const override {
        if (_d.marche && u.peutMarcher()) return true;
        if (_d.nage && u.peutNager()) return true;
        return false;
    }

private:
    TuileData _d;
};

class WorldGenerator {
public:
    virtual ~WorldGenerator() = default;
    virtual std::unique_ptr<hexa> createTile(int i, int j) = 0;

    virtual void postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>> & matrix, int size) = 0;
};

class FileFactory : public WorldGenerator {
    public:
        void chargerConfig(std::string cheminFichier);
        std::unique_ptr<hexa> createTile(int, int);
        const std::map<char, TuileData>& getCatalogue() const {
            return _catalogue;
        }

        void postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>>& matrix, int size);

private:
    std::map<char, TuileData> _catalogue;
};


class board {
    public:
        board(int size, WorldGenerator & gen) : _size(size) {
            for (int i = 0; i < size; ++i) {
                std::vector<std::unique_ptr<hexa>> ligne;
                for (int j = 0; j < size; ++j) {
                    if (i == 0 || i == size - 1 || j == 0 || j == size - 1) {
                        TuileData limiteData{"Limite", '#', -1, false, false, 0, 0};
                        ligne.push_back(std::make_unique<TuileConfigurable>(limiteData));
                    } else {
                        ligne.push_back(gen.createTile(i, j));
                    }
                }
                _matrix.push_back(std::move(ligne));
            }

            gen.postGeneration(_matrix, _size);
        }

        const hexa* getCell(int i, int j) const { return _matrix[i][j].get(); }
        void affichage() const;

        void placerUnite(int x, int y, std::unique_ptr<Unite> u);
        bool deplacerUnite(int xSrc, int ySrc, int xDest, int yDest);
        Unite * getUnite(int x, int y) const;

    private:
        int _size;
        std::vector<std::vector<std::unique_ptr<hexa>>> _matrix;
        std::map<std::pair<int, int>, std::unique_ptr<Unite>> _unites;
};
