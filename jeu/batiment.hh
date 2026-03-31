#pragma once
#include <string>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>
#include "ressource.hh"

class Joueur;

class Batiment {
    protected:
        int _level;
        std::string _name;
        std::map<Ressource*, int> _cout;

    public:
        Batiment(int level=1, std::string n, std::map<Ressource*, int> c) : _level(level), _name(n), _cout(c) {}
        virtual ~Batiment() = default;

        virtual std::unique_ptr<Batiment> clone() const = 0;
        
        std::string getName() const { return _name; }
        const std::map<Ressource*, int> & getResourceConstr() const { return _cout; }
        virtual Ressource* getRessourceRequired() const { return nullptr; }

        virtual void action(Joueur & j) = 0;
};

class BatimentRessource : public Batiment {
    private:
        Ressource* _produit;
        int _quantite;
        Ressource* _ressourceSolRequise;
    
    public:
        BatimentRessource(int l=1, std::string n, std::map<Ressource*, int> c, Ressource* p, int q, Ressource* sol=nullptr)
            : Batiment(l,n,c), _produit(p), _quantite(q), _ressourceSolRequise(sol) {}

        std::unique_ptr<Batiment> clone() const override {
            return std::make_unique<BatimentRessource>(*this);
        }

        Ressource* getRessourceRequired() const { return _ressourceSolRequise; }

        void action(Joueur & j);
};


class BatimentConfigReader {
    public:
        virtual ~BatimentConfigReader() = default;
        virtual void load(const std::string & chemin, 
            std::map<std::string, std::unique_ptr<Batiment>> & catalogue,
            const std::map<std::string, Ressource*> & ressourcesDispo) = 0;
};

class TxtBatimentReader : public BatimentConfigReader {
    public:
        void load(const std::string& chemin, 
                std::map<std::string, std::unique_ptr<Batiment>>& catalogue,
                const std::map<std::string, Ressource*>& ressourcesDispo) override;
};

class BatimentFactory {
    private:
        std::map<std::string, std::unique_ptr<Batiment>> _catalogue;

    public:
        void chargerConfiguration(const std::string& chemin, 
                                BatimentConfigReader& lecteur,
                                const std::map<std::string, Ressource*>& ressourcesDispo) {
            lecteur.load(chemin, _catalogue, ressourcesDispo);
        }

        std::unique_ptr<Batiment> create(std::string type) {
            if (_catalogue.count(type)) {
                return _catalogue[type]->clone();
            }
            return nullptr;
        }
};