#pragma once
#include <iostream>
#include <string>
#include <list>
#include "rank.hh"
#include "orientation.hh"
#include "comportement.hh"

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
    direction _regarde;
    Case _location;
    std::shared_ptr<IRank> _rank;
    std::list<std::shared_ptr<IComportement>> _liste_comportements;

public:
    Unite(const std::string &name, int hp, int dmg, direction dir, Case loc, std::shared_ptr<IRank> r);
    virtual ~Unite() = default;

    /*Setters*/
    void movement(Case const& c);
    void affiche() const;
    void setHealth_point(int newHealth_point);
    void setDamage_point(int newDamage_point);
    void setMoral_point(int newMoral_point);
    void setRegarde(direction newRegarde);
    void setLocation(const Case &newLocation);

    /*Getters*/
    std::string name() const;
    int health_point() const;
    int damage_point() const;
    int moral_point() const;
    direction regarde() const;
    Case location() const;
    std::shared_ptr<IRank> rank() const;
    std::list<std::shared_ptr<IComportement>> liste_comportements() const;

    /*Méthodes*/
    void ajouterComportement(std::shared_ptr<IComportement> comp);
    void update();

};
