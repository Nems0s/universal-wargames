#pragma once
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <random>
#include <fstream>
#include <sstream>
#include <map>


class Unite {
    public:
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
};

class FileFactory : public WorldGenerator {
    public:
        void chargerConfig(std::string cheminFichier) {
            std::ifstream fichier(cheminFichier);
            if (!fichier.is_open()) {
                std::cerr << "Erreur : Impossible d'ouvrir " << cheminFichier << std::endl;
                return;
            }

            std::string nom;
            char symb;
            int cout, p;
            bool m, n;

            // Format attendu : Plaine T 1 1 0
            while (fichier >> nom >> symb >> cout >> m >> n >> p) {
                TuileData nouvelleTuile;
                nouvelleTuile.nom = nom;
                nouvelleTuile.symbole = symb;
                nouvelleTuile.cout = cout;
                nouvelleTuile.marche = m;
                nouvelleTuile.nage = n;
                nouvelleTuile.poids = p;

                // TuileData nouvelleTuile = {nom, symb, cout, m, n, p}

                _catalogue[symb] = nouvelleTuile;
                
                std::cout << "Chargé : " << nom << " (" << symb << ")" << std::endl;
            }
        }

        std::unique_ptr<hexa> createTile(int, int) override {
            if (_catalogue.empty()) return nullptr;

            int poidsTotal = 0;
            for (auto const& [symb, data] : _catalogue) {
                poidsTotal += data.poids;
            }

            int tirage = rand() % poidsTotal;

            int seuil = 0;
            for (auto const& [symb, data] : _catalogue) {
                seuil += data.poids;
                if (tirage < seuil) {
                    return std::make_unique<TuileConfigurable>(data);
                }
            }

            return std::make_unique<TuileConfigurable>(_catalogue.begin()->second);
        }






private:
    std::map<char, TuileData> _catalogue;
};




class board {
    public:
        board(int size, WorldGenerator & gen) : _size(size) {
            for (int i = 0; i < size; ++i) {
                std::vector<std::unique_ptr<hexa>> ligne;
                for (int j = 0; j < size; ++j) {
                    ligne.push_back(gen.createTile(i,j));
                }
                _matrix.push_back(std::move(ligne));
            }
        }

        const hexa* getCell(int i, int j) const { return _matrix[i][j].get(); }

        void affichage() const;

    private:
        int _size;
        std::vector<std::vector<std::unique_ptr<hexa>>> _matrix;
};






