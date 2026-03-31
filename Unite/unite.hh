#pragma once
#include <iostream>
#include <string>
#include <list>
#include <memory>
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
class Unite
{
private:
    std::string _name;
    int _health_point;
    int _damage_point;
    int _moral_point;
    Poids _poids;
    direction _regarde;
    Case _location;
    std::shared_ptr<IRank> _rank;
    std::list<std::shared_ptr<IComportement>> _liste_comportements;


public:
    Unite(const std::string &name, int hp, int dmg,Poids poids, direction dir, Case loc, std::shared_ptr<IRank> r);
    virtual ~Unite() = default;

    /*Setters*/
    void setHealth_point(int newHealth_point);
    void setDamage_point(int newDamage_point);
    void setMoral_point(int newMoral_point);
    void setPoids(Poids newPoids);
    void setRegarde(direction newRegarde);
    void setLocation(const Case &newLocation);

    /*Getters*/
    std::string name() const;
    int health_point() const;
    int damage_point() const;
    int moral_point() const;
    Poids poids() const;
    direction regarde() const;
    Case location() const;
    std::shared_ptr<IRank> rank() const;
    std::list<std::shared_ptr<IComportement>> liste_comportements() const;

    /*Méthodes*/
    // void movement(Case const& c);
    void affiche() const;
    void ajouterComportement(std::shared_ptr<IComportement> comp);
    void update();
    std::list<CompMouv*> Mobilite() const;
    std::list<CompAtt*> Offensive() const;
    std::list<CompDef*> Defensif() const;
};
