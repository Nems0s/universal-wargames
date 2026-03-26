#include "rank.hh"
#include <iostream>

// ==========================================
//                     Commandant
// ==========================================
void Rank_Commandant::get_role()const
{
    std::cout<<"Commandant";
}

std::list<std::shared_ptr<Unite>> Rank_Commandant::liste_unites() const
{
    return _liste_unites;
}

void Rank_Commandant::ajout_unite(std::shared_ptr<Unite>const & u)
{
    _liste_unites.push_back(u);
}

void Rank_Commandant::reset_liste()
{
    _liste_unites.clear();
}

void Rank_Commandant::supprimer_unite(std::shared_ptr<Unite>const & u)
{
    _liste_unites.remove(u);
}

// ==========================================
//                    Regulier
// ==========================================
void Rank_Regulier::get_role()const
{
    std::cout<<"Unite régulière";
}
