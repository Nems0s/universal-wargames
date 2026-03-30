#pragma once
#include <iostream>
#include <memory>
#include <list>
#include <orientation.hh>

enum class NatureMouv { TERRE, MER, AIR };

class Unite;

class IComportement
{
public:
    virtual ~IComportement() = default;
    virtual void affiche() const = 0;
    virtual void update(Unite& proprietaire) = 0;
};

//===================================================================
//                     Comportement Mouvement
//===================================================================
class CompMouv : public IComportement
{
protected:
    int _mov_per_laps;
public:
    CompMouv(int mouvement_par_tour);

    int mov_per_laps() const;
    void setMov_per_laps(int newMov_per_laps);

    virtual bool EstCaseValide(Case const& actuel, Case const& cible) = 0;
    virtual NatureMouv Nature()const=0;
};

class CompMouvVolant : public CompMouv
{
public:
    CompMouvVolant();
    CompMouvVolant(int mouvement_par_tour);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool EstCaseValide(Case const& actuel, Case const& cible) override;
    NatureMouv Nature() const override;
};

class CompMouvMarin : public CompMouv
{
public:
    CompMouvMarin();
    CompMouvMarin(int mouvement_par_tour);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool EstCaseValide(Case const& actuel, Case const& cible) override;
    NatureMouv Nature() const override;
};

class CompMouvTerrestre : public CompMouv
{
public:
    CompMouvTerrestre();
    CompMouvTerrestre(int mouvement_par_tour);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool EstCaseValide(Case const& actuel, Case const& cible) override;
    NatureMouv Nature() const override;
};


//===================================================================
//                   Comportement Attaque
//===================================================================
class CompAtt : public IComportement
{
protected:
    int _damage_point;
    int _portee;
public:
    CompAtt(int damage_point, int portee);

    int damage_point() const;
    void setDamage_point(int newDamage_point);
    int portee() const;
    void setPortee(int newPortee);

    virtual bool PeuxAttaquer(Unite const& attaquante, Unite const& cible)=0;
};

class CompAttDirect : public CompAtt
{
public:
    CompAttDirect(int damage_point);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool PeuxAttaquer(Unite const& attaquante, Unite const& cible) override;
};



