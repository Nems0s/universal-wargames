#pragma once
#include <list>
#include <memory>
#include <string>

class Unite; // Declaration vide pour pouvoir créer la liste d'unité de Commandant

class IRank
{
public:
    virtual ~IRank() = default;
    virtual void get_role()const = 0;
};

class Rank_Regulier : public IRank
{
private:
    std::shared_ptr<Unite> _commandant;
public:
    Unite* commandant() const;
    void setCommandant(std::shared_ptr<Unite> newCommandant);

    void get_role()const override;
    bool PossedeCommandant()const;
};

//===================================================================
//                     Bonus Commandant
//===================================================================
class IBonus {
public:
    virtual ~IBonus() = default;
    virtual std::string nom() const = 0;
    virtual int appliquer(int valeur_initiale) = 0;
};


class BonusDegat : public IBonus {
    int _supplement;
public:
    BonusDegat(int s);
    std::string nom() const override;
    int appliquer(int valeur) override;
};

class BonusVie : public IBonus {
    int _soin;
public:
    BonusVie(int s);
    std::string nom() const override;
    int appliquer(int valeur_initiale) override;
};

class BonusDefense : public IBonus {
    int _shield;
public:
    BonusDefense(int s);
    std::string nom() const override;
    int appliquer(int valeur) override;
};

// A implementer
// class BonusResurection : public IBonus {
//     int _shield;
// public:
//     BonusResurection(int s);
//     std::string nom() const override;
//     int appliquer(int valeur) override;
// };

//===================================================================
//===================================================================

class Rank_Commandant : public IRank
{
private:
    std::list<std::shared_ptr<Unite>> _liste_unites;
    std::list<std::shared_ptr<IBonus>> _liste_bonus;
public:

    void get_role()const override;

    std::list<std::shared_ptr<Unite>> liste_unites() const;
    std::list<std::shared_ptr<IBonus> > liste_bonus() const;

    void reset_liste();

    void ajout_unite(std::shared_ptr<Unite>const & u);
    void supprimer_unite(std::shared_ptr<Unite>const & u);

    void ajout_bonus(std::shared_ptr<IBonus>const & b);
    void supprimer_bonus(std::shared_ptr<IBonus>const & b);
};




