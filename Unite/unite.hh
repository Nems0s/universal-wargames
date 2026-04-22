#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <memory>
#include <map>
#include "comportement.hh"
#include "rank.hh"
#include "orientation.hh"
#include "ressource.hh"

class Joueur;

enum class Poids{Leger, Moyen, Lourd};
class IComportement;
class CompMouv;
class CompAtt;
class CompDef;

// ==========================================
//                     Unite
// ==========================================
class Unite : public std::enable_shared_from_this<Unite>
{
private:
    std::string _name;

    Joueur* _proprietaire = nullptr;

    int _health_point;
    int _health_point_max; //Qui servira à savoir les points de vie de l'unité initialiser
    int _moral_point;
    int _point_action;
    int _point_action_max;

    Poids _poids;
    direction _regarde;
    Coord _location;
    std::shared_ptr<IRank> _rank;
    std::list<std::shared_ptr<IComportement>> _liste_comportements;
    std::map<Ressource*, int> _cout;

    int _temporary_health;
    int _temporary_damage;
    int _visionRange;
    int _fov;
    std::string _texturePath;
public:
    Unite(const std::string &name, int hp, int point_action, int vision, int fov, Poids poids, direction dir, Coord loc, std::shared_ptr<IRank> r, std::list<std::shared_ptr<IComportement>> liste_comportements, std::map<Ressource*, int> cout, const std::string& texturePath = "");
    virtual ~Unite() = default;

    /*Setters*/
    void setHealth_point(int newHealth_point);
    void setMoral_point(int newMoral_point);
    void setPoids(Poids newPoids);
    void setRegarde(direction newRegarde);
    void setLocation(const Coord &newLocation);
    void setTemporary_health(int newTemporary_health);
    void setTemporary_damage(int newTemporary_damage);

    // MODIFIE PAR NAIM
    void setPoint_action(int pa) { _point_action = pa; }
    std::string texturePath() const { return _texturePath; }
    void setProprietaire(Joueur* j) { _proprietaire = j; }
    Joueur* getProprietaire() const { return _proprietaire; }

    /*Getters*/
    std::string name() const;
    int health_point() const;
    int health_point_max() const;
    int moral_point() const;
    int point_action() const;
    int point_action_max() const;
    Poids poids() const;
    direction regarde() const;
    Coord location() const;
    std::shared_ptr<IRank> rank() const;
    std::list<std::shared_ptr<IComportement>> liste_comportements() const;
    std::map<Ressource*, int> cout() const;
    int temporary_health() const;
    int temporary_damage() const;

    // Vision
    int visionRange() const { return _visionRange; }
    void setVisionRange(int v) { _visionRange = v; }
    int fov() const { return _fov; }
    void setFov(int f) { _fov = f; }
    
    /*Méthodes*/
    void affiche() const;
    void ajouterComportement(std::shared_ptr<IComportement> comp);
    void update();

    std::list<CompMouv*> Mobilite() const;
    std::list<CompAtt*> Offensive() const;
    std::list<CompDef*> Defensif() const;
    std::list<CompSoin*> Soin() const;
    CompFurtif* Cammouflage() const;
    CompTransport* Transport() const;

    void resetTemporary_stats();
    std::shared_ptr<Unite> clone() const;
};


// ==========================================
//              Config/Factory
// ==========================================
class UniteConfigReader {
public:
    virtual ~UniteConfigReader() = default;
    virtual void load(const std::string& chemin,std::map<std::string, std::shared_ptr<Unite>>& catalogue,const std::map<std::string, Ressource*>& ressources) = 0;
};

class JsonUniteReader : public UniteConfigReader {
public:
    void load(const std::string& chemin,std::map<std::string, std::shared_ptr<Unite>>& catalogue,const std::map<std::string, Ressource*>& ressources) override;
};

class UniteFactory {
private:
    std::map<std::string, std::shared_ptr<Unite>> _catalogue;
public:
    void chargerConfiguration(const std::string& chemin, UniteConfigReader& lecteur,const std::map<std::string, Ressource*>& ressources);
    // MODIFIE PAR NAIM
    std::shared_ptr<Unite> create(std::string type) const;
    const std::map<std::string, std::shared_ptr<Unite>>& getCatalogue() const { return _catalogue; }
};