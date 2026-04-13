#include "rank.hh"
#include "unite.hh"
#include <iostream>

// ==========================================
//                    Regulier
// ==========================================
Unite* Rank_Regulier::commandant() const
{
    return _commandant.get();
}

void Rank_Regulier::setCommandant(std::shared_ptr<Unite> newCommandant)
{
    _commandant = std::move(newCommandant);
}

void Rank_Regulier::get_role()const
{
    std::cout<<"Unite reguliere";
}

bool Rank_Regulier::PossedeCommandant() const
{
    return _commandant != nullptr;
}

// ==========================================
//                     Commandant
// ==========================================
Rank_Commandant::Rank_Commandant(int max_unites): _max_unites(max_unites){}

void Rank_Commandant::get_role()const
{
    std::cout<<"Commandant";
}

int Rank_Commandant::get_max_unite()const
{
    return _max_unites;
}

std::list<std::shared_ptr<Unite>> Rank_Commandant::liste_unites() const
{
    return _liste_unites;
}


std::list<std::shared_ptr<IBonus> > Rank_Commandant::liste_bonus() const
{
    return _liste_bonus;
}

void Rank_Commandant::reset_liste()
{
    _liste_unites.clear();
    _liste_bonus.clear();
}

void Rank_Commandant::ajout_unite(std::shared_ptr<Unite>const & u)
{
    _liste_unites.push_back(u);
}

void Rank_Commandant::supprimer_unite(std::shared_ptr<Unite>const & u)
{
    _liste_unites.remove(u);
}

void Rank_Commandant::ajout_bonus(std::shared_ptr<IBonus>const & b)
{
    _liste_bonus.push_back(b);
}

void Rank_Commandant::supprimer_bonus(std::shared_ptr<IBonus>const & b)
{
    _liste_bonus.remove(b);
}

//===================================================================
//                     Bonus Commandant
//===================================================================
BonusDegat::BonusDegat(int s) : _supplement(s) {}
std::string BonusDegat::nom() const
{
    return "Inspiration";
}
int BonusDegat::appliquer(int valeur)
{
    return valeur + _supplement;
}


BonusVie::BonusVie(int s) : _soin(s){}
std::string BonusVie::nom() const
{
    return "Aide";
}


int BonusVie::appliquer(int valeur)
{
    return valeur + _soin;
}


BonusDefense::BonusDefense(int s) : _shield(s) {}
std::string BonusDefense::nom() const
{
    return "Protection";
}
int BonusDefense::appliquer(int valeur)
{
    return valeur + _shield;
}

// BonusResurection::BonusResurection(int s) : _shield(s) {}
// std::string BonusResurection::nom() const
// {
//     return "Protection";
// }
// int BonusResurection::appliquer(int valeur)
// {
//     return valeur + _shield;
// }




