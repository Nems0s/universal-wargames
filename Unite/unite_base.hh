#pragma once
#include <iostream>
#include <string>
#include <list>

enum class direction{nord_ouest, nord_est, ouest, est, sud_ouest, sud_est};
std::string directionToString(direction dir);

struct Case{
    int x;
    int y;
};

// ==========================================
//                     Unite
// ==========================================
class Unite
{
protected:
    std::string _name;
    int _health_point;
    int _damage_point;
    int _moral_point;
    int _move_per_laps;
    direction _regarde;
    Case _location;

public:
    Unite(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location);
    virtual ~Unite() = default;

    /*Getters*/
    const std::string &name() const;
    int health_point() const;
    int damage_point() const;
    int moral_point() const;
    int move_per_laps() const;
    direction regarde() const;

    /*Setters*/
    void setHealth_point(int newHealth_point);
    void setDamage_point(int newDamage_point);
    void setMoral_point(int newMoral_point);
    void setMove_per_laps(int newMove_per_laps);
    void setRegarde(direction newRegarde);

    /*Méthodes*/
    virtual void movement(Case const& c);
    virtual void affiche() const;
};

// ==========================================
//                 Unite Legere
// ==========================================
class Unite_legere : virtual public Unite {
public:
    Unite_legere(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location);
    // void movement(Case const& c) override; Pour dire les types de cases visitable
    void affiche() const override;
};

// ==========================================
//                 Unite Volante
// ==========================================
class Unite_volante : virtual public Unite {
protected:
    bool _vole;
public:
    Unite_volante(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location);
    // void movement(Case const& c) override;
    void affiche() const override;
};

// ==========================================
//                 Unite Maritime
// ==========================================
class Unite_maritime : virtual public Unite {
protected:
    bool _navigue;
public:
    Unite_maritime(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location);
    // void movement(Case const& c) override;
    void affiche() const override;
};

// ==========================================
//                 Unite Lourde
// ==========================================
class Unite_lourde : virtual public Unite {
public:
    Unite_lourde(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location);
    // void movement(Case const& c) override;
    void affiche() const override;
};

// ==========================================
//                 Unite Transport
// ==========================================
class Unite_transport : virtual public Unite {
private:
    std::list<std::shared_ptr<Unite>> _liste_unite;
    void affiche_unites() const;
public:
    Unite_transport(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location);
    // void movement(Case const& c) override;
    void affiche() const override;
};
