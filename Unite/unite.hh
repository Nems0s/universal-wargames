#pragma once
#include <iostream>
#include <string>
#include <list>
#include <memory>
#include "comportement.hh"
#include "rank.hh"
#include "orientation.hh"

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

    int _health_point;
    int _health_point_max; //Qui servira à savoir les points de vie de l'unité initialiser
    int _moral_point;

    Poids _poids;
    direction _regarde;
    Coord _location;
    std::shared_ptr<IRank> _rank;
    std::list<std::shared_ptr<IComportement>> _liste_comportements;

    int _temporary_health;
    int _temporary_damage;
public:
    Unite(const std::string &name, int hp,Poids poids, direction dir, Coord loc, std::shared_ptr<IRank> r, std::list<std::shared_ptr<IComportement>> liste_comportements);
    virtual ~Unite() = default;

    /*Setters*/
    void setHealth_point(int newHealth_point);
    void setMoral_point(int newMoral_point);
    void setPoids(Poids newPoids);
    void setRegarde(direction newRegarde);
    void setLocation(const Coord &newLocation);
    void setTemporary_health(int newTemporary_health);
    void setTemporary_damage(int newTemporary_damage);

    /*Getters*/
    std::string name() const;
    int health_point() const;
    int health_point_max() const;
    int moral_point() const;
    Poids poids() const;
    direction regarde() const;
    Coord location() const;
    std::shared_ptr<IRank> rank() const;
    std::list<std::shared_ptr<IComportement>> liste_comportements() const;
    int temporary_health() const;
    int temporary_damage() const;

    /*Méthodes*/
    // void movement(Case const& c);
    void affiche() const;
    void ajouterComportement(std::shared_ptr<IComportement> comp);
    void update();
    std::list<CompMouv*> Mobilite() const;
    std::list<CompAtt*> Offensive() const;
    std::list<CompDef*> Defensif() const;
    std::list<CompSoin*> Soin() const;
    CompFurtif* Cammouflage() const;

    void resetTemporary_stats();
};
