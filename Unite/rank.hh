#pragma once
#include <list>
#include <memory>

class Unite; // Declaration vide pour pouvoir créer la liste d'unité de Commandant

class IRank
{
public:
    virtual ~IRank() = default;
    virtual void get_role()const = 0;
};

class Rank_Commandant : public IRank
{
private:
    std::list<std::shared_ptr<Unite>> _liste_unites;
public:

    void get_role()const override;
    std::list<std::shared_ptr<Unite>> liste_unites() const;
    void reset_liste();
    void ajout_unite(std::shared_ptr<Unite>const & u);
    void supprimer_unite(std::shared_ptr<Unite>const & u);

};

class Rank_Regulier : public IRank
{
private:
    std::unique_ptr<Unite> _commandant;
public:
    Unite* commandant() const;
    void setCommandant(std::unique_ptr<Unite> newCommandant);

    void get_role()const override;
    bool PossedeCommandant()const;
};
