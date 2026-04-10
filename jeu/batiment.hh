#pragma once
#include <string>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>
#include "ressource.hh"

class Joueur;

class Batiment
{
    protected:
        int _level=1;
        int _maxLevel;
        std::string _name;
        std::map<Ressource*, int> _cout;

    public:
        Batiment(std::string n, std::map<Ressource*, int> c, int level=1)
            : _level(level), _name(n), _cout(c) {}
        virtual ~Batiment() = default;

        virtual std::unique_ptr<Batiment> clone() const = 0;
        
        std::string getName() const;
        const std::map<Ressource*, int> & getResourceConstr() const;
        
        virtual const std::vector<Ressource*>& getRessourcesSolRequired() const;

        virtual void action(Joueur & j) = 0;

        int getLevel() const { return _level; }
        int getMaxLevel() const { return _maxLevel; }
};

class BatimentRessource : public Batiment
{
    private:
        std::map<Ressource*, int> _produits;
        std::vector<Ressource*> _ressourcesSolRequises;
    
    public:
        BatimentRessource(std::string n, std::map<Ressource*, int> c, std::map<Ressource*, int> p, std::vector<Ressource*> sols, int l=1)
            : Batiment(n, c, l), _produits(p), _ressourcesSolRequises(sols) {}

        std::unique_ptr<Batiment> clone() const override;

        const std::vector<Ressource*>& getRessourcesSolRequired() const override;

        void action(Joueur & j) override;
};


class BatimentConfigReader
{
    public:
        virtual ~BatimentConfigReader() = default;
        virtual void load(const std::string & chemin,std::map<std::string, std::unique_ptr<Batiment>> & catalogue,
        const std::map<std::string, Ressource*> & ressourcesDispo) = 0;
};

class TxtBatimentReader : public BatimentConfigReader
{
    public:
        void load(const std::string& chemin, std::map<std::string, std::unique_ptr<Batiment>>& catalogue, const std::map<std::string, Ressource*>& ressourcesDispo) override;
};

class JsonBatimentReader : public BatimentConfigReader {
    public:
        void load(const std::string& chemin, std::map<std::string, std::unique_ptr<Batiment>>& catalogue, const std::map<std::string, Ressource*>& ressourcesDispo) override;
};

class BatimentFactory
{
    private:
        std::map<std::string, std::unique_ptr<Batiment>> _catalogue;

    public:
        void chargerConfiguration(const std::string& chemin, BatimentConfigReader& lecteur, const std::map<std::string, Ressource*>& ressourcesDispo);
        std::unique_ptr<Batiment> create(std::string type);
};
