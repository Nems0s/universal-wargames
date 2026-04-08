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
        std::string _name;
        std::map<Ressource*, int> _cout;
        int _level;

    public:
        Batiment(std::string n, std::map<Ressource*, int> c, int level=1);
        virtual ~Batiment() = default;

        virtual std::unique_ptr<Batiment> clone() const = 0;
        
        std::string getName() const;
        const std::map<Ressource*, int> & getResourceConstr() const;
        virtual Ressource* getRessourceRequired() const;

        virtual void action(Joueur & j) = 0;
};

class BatimentRessource : public Batiment
{
    private:
        Ressource* _produit;
        int _quantite;
        Ressource* _ressourceSolRequise;
    
    public:
        BatimentRessource(std::string n, std::map<Ressource*, int> c, Ressource* p, int q, Ressource* sol=nullptr, int l=1);

        std::unique_ptr<Batiment> clone() const override;

        Ressource* getRessourceRequired() const override;

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

class BatimentFactory
{
    private:
        std::map<std::string, std::unique_ptr<Batiment>> _catalogue;

    public:
        void chargerConfiguration(const std::string& chemin, BatimentConfigReader& lecteur, const std::map<std::string, Ressource*>& ressourcesDispo);
        std::unique_ptr<Batiment> create(std::string type);
};
