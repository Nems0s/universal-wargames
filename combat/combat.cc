#include "combat.hh"
#include "comportement.hh"
#include "rank.hh"

bool Combat::fight(Unite &attaquant,CompAtt* const& TypeAttaque, Unite &defenseur)
{
    auto styles_attaque = attaquant.Offensive();
    auto attaque_autorise = attaquant.Offensive();

    int degat = attaquant.damage_point();

    for(auto const& style : styles_attaque)
    {
        if(!(style->PeuxAttaquer(attaquant, defenseur)))
        {
            attaque_autorise.remove(style);
        }
    }

    auto it = std::find(attaque_autorise.begin(), attaque_autorise.end(), TypeAttaque);

    if (it != attaque_autorise.end())
    {
        auto r = attaquant.rank();
        auto regulier = std::dynamic_pointer_cast<Rank_Regulier>(r);
        //Buff
        if(regulier && regulier->PossedeCommandant())
        {
            auto com = regulier->commandant();
            //lancer buff
        }

        //Ajouter test Moral

        //Debuf
        auto defenses_possible = defenseur.Defensif();

        for(auto const& defense : defenses_possible)
        {
            if(degat != 0)
            {
                degat = defense->ReductionDegats(degat);
            }
        }

        defenseur.setHealth_point(defenseur.health_point() - degat);
        return true;
    }
    else
    {
        return false;
    }
}




// bool Combat::heal(const Unite &healer, const Unite & cible)
// {

// }
