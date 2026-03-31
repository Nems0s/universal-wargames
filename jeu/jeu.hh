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

#include "city.hh"
#include "batiment.hh"
#include "../joueur/joueur.hh"

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

    bool constructible;
    Ressource* ressourceSpeciale;
};

class TuileConfigurable : public hexa {
public:
    TuileConfigurable(const TuileData& data) : _d(data) {}

    std::string getType() const override { return _d.nom; }
    char getSymbole() const override { return _d.symbole; }
    int getCoutDeplacement() const override { return _d.cout; }
    Ressource* getRessource() const { return _d.ressourceSpeciale; }
    
    bool estFranchissable(const Unite& u) const;

    bool peutConstrVille() const;
    bool peutConstrBatiment(const Batiment & b) const;
    bool peutConstrBatimentSpecial(const Batiment & b) const;

    void constrVille(int max, bool capitale);
    void constrBatimentSpeciale(std::unique_ptr<Batiment> b);

    City * getCity() const { return _city.get(); }

private:
    TuileData _d;
    std::unique_ptr<City> _city;
    std::unique_ptr<Batiment> _batimentSpecial;
};

class WorldConfigReader {
public:
    virtual ~WorldConfigReader() = default;

    virtual void chargerConfig(std::string chemin, const std::map<std::string, Ressource*>& ressourcesDispo, WorldFactory& factory) = 0;
};

class TxtWorldReader : public WorldConfigReader {
    public:
        void chargerConfig(std::string cheminFichier, const std::map<std::string, Ressource*> & ressourcesDispo, WorldFactory& factory) override;

};


class WorldFactory {
    private:
        std::map<char, TuileData> _catalogue;

    public:
        void ajouterAuCatalogue(char symbole, const TuileData& data) {
            _catalogue[symbole] = data;
        }

        std::unique_ptr<hexa> createTile(char symbole);
        std::unique_ptr<hexa> createRandomTile();

        void postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>>& matrix, int size);

        bool estVide() const { return _catalogue.empty(); }
};


class board {
    public:
        board(int size, WorldFactory & world) : _size(size) {
            for (int i = 0; i < size; ++i) {
                std::vector<std::unique_ptr<hexa>> ligne;
                for (int j = 0; j < size; ++j) {
                    if (i == 0 || i == size - 1 || j == 0 || j == size - 1) {
                        TuileData limiteData{"Limite", '#', -1, false, false, 0, 0, false, nullptr};
                        ligne.push_back(std::make_unique<TuileConfigurable>(limiteData));
                    } else {
                        ligne.push_back(world.createRandomTile());
                    }
                }
                _matrix.push_back(std::move(ligne));
            }

            world.postGeneration(_matrix, _size);
        }

        const hexa* getCell(int i, int j) const { return _matrix[i][j].get(); }
        void affichage() const;

        void placerUnite(int x, int y, std::unique_ptr<Unite> u);
        bool deplacerUnite(int xSrc, int ySrc, int xDest, int yDest);
        Unite * getUnite(int x, int y) const;

        void tenterConstruction(int x, int y, std::unique_ptr<Batiment> b, Joueur & j);

    private:
        int _size;
        std::vector<std::vector<std::unique_ptr<hexa>>> _matrix;
        std::map<std::pair<int, int>, std::unique_ptr<Unite>> _unites;
};
