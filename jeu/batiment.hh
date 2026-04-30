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
        std::map<const Ressource*, int> _cout;
        std::string _texturePath;

    public:
        Batiment(std::string n, std::map<const Ressource*, int> c, int level=1, const std::string& tex = "")
            : _level(level), _name(n), _cout(c), _texturePath(tex) {}
        virtual ~Batiment() = default;

        virtual std::unique_ptr<Batiment> clone() const = 0;
        
        std::string getName() const;
        const std::map<const Ressource*, int> & getResourceConstr() const;
        virtual const std::vector<const Ressource*>& getRessourcesSolRequired() const;
        virtual const std::map<const Ressource*, int> getProduits() const { return {}; }

        virtual void action(Joueur & j) = 0;

        int getLevel() const { return _level; }
        int getMaxLevel() const { return _maxLevel; }
        std::string getTexturePath() const { return _texturePath; }
};

class BatimentRessource : public Batiment
{
    private:
        std::map<const Ressource*, int> _produits;
        std::vector<const Ressource*> _ressourcesSolRequises;
    
    public:
        BatimentRessource(std::string n, std::map<const Ressource*, int> c, std::map<const Ressource*, int> p, std::vector<const Ressource*> sols, int l=1, const std::string& tex = "")
            : Batiment(n, c, l, tex), _produits(p), _ressourcesSolRequises(sols) {}

        std::unique_ptr<Batiment> clone() const override;

        const std::vector<const Ressource*>& getRessourcesSolRequired() const override;
        const std::map<const Ressource*, int> getProduits() const override { return _produits; }

        void action(Joueur & j) override;
};


class BatimentConfigReader
{
    public:
        virtual ~BatimentConfigReader() = default;
        virtual void load(const std::string & chemin,std::map<std::string, std::unique_ptr<Batiment>> & catalogue,
        const std::map<std::string, const Ressource*> & ressourcesDispo) = 0;
};

class TxtBatimentReader : public BatimentConfigReader
{
    public:
        void load(const std::string& chemin, std::map<std::string, std::unique_ptr<Batiment>>& catalogue, const std::map<std::string, const Ressource*>& ressourcesDispo) override;
};

class JsonBatimentReader : public BatimentConfigReader {
    public:
        void load(const std::string& chemin, std::map<std::string, std::unique_ptr<Batiment>>& catalogue, const std::map<std::string, const Ressource*>& ressourcesDispo) override;
};

class BatimentFactory
{
    private:
        std::map<std::string, std::unique_ptr<Batiment>> _catalogue;

    public:
        void chargerConfiguration(const std::string& chemin, BatimentConfigReader& lecteur, const std::map<std::string, const Ressource*>& ressourcesDispo);
        std::unique_ptr<Batiment> create(std::string type) const;

        const std::map<std::string, std::unique_ptr<Batiment>>& getCatalogue() const { return _catalogue; }
};
