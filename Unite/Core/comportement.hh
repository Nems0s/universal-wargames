#pragma once
#include <iostream>
#include <memory>
#include <list>
#include <orientation.hh>

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
};

class CompMouvVolant : public CompMouv
{
public:
    CompMouvVolant(int mouvement_par_tour);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool EstCaseValide(Case const& actuel, Case const& cible) override;
};

class CompMouvMarin : public CompMouv
{
public:
    CompMouvMarin(int mouvement_par_tour);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool EstCaseValide(Case const& actuel, Case const& cible) override;
};

class CompMouvTerrestre : public CompMouv
{
public:
    CompMouvTerrestre(int mouvement_par_tour);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool EstCaseValide(Case const& actuel, Case const& cible) override;
};




