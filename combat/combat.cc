#include "combat.hh"
#include "comportement.hh"
#include "rank.hh"
#include <algorithm>

void BuffCommandant(Unite & u, bool & aSoin)
{
    auto r = u.rank();
    auto regulier = std::dynamic_pointer_cast<Rank_Regulier>(r);

    if(regulier && regulier->PossedeCommandant())
    {
        auto c = regulier->commandant()->rank();
        auto com = std::dynamic_pointer_cast<Rank_Commandant>(c);

        if(com)
        {
            for(auto const& buff : com->liste_bonus())
            {
                if(std::dynamic_pointer_cast<BonusVie>(buff))
                {
                    aSoin = true;
                }
                else if(auto att = std::dynamic_pointer_cast<BonusDegat>(buff))
                {
                    u.setTemporary_damage(att->appliquer(u.damage_point()));
                }
                else if(auto def = std::dynamic_pointer_cast<BonusDefense>(buff))
                {
                    u.setTemporary_health(def->appliquer(u.health_point()));
                }
            }
        }
    }
}

void SoinDuCommandant(Unite & u)
{
    auto r = u.rank();
    auto regulier = std::dynamic_pointer_cast<Rank_Regulier>(r);
    if(regulier)
    {
        auto com = std::dynamic_pointer_cast<Rank_Commandant>(regulier->commandant()->rank());
        for(auto const& buff : com->liste_bonus())
        {
            if(auto soin = std::dynamic_pointer_cast<BonusVie>(buff))
            {
                u.setHealth_point(soin->appliquer(u.health_point()));
            }
        }
    }
}

bool Combat::fight(Unite &attaquant, CompAtt* const& TypeAttaque, Unite &defenseur)
{
    auto styles_attaque = attaquant.Offensive();
    auto it = std::find(styles_attaque.begin(), styles_attaque.end(), TypeAttaque);

    if (it == styles_attaque.end() || !((*it)->PeuxAttaquer(attaquant, defenseur)))
    {
        return false;
    }

    if(auto* infect = dynamic_cast<CompAttIndirect*>(*it))
    {
        if(infect)
        {
            // A Corriger problème de pointeur
           // infect->AjoutCibleAtteinte(defenseur);
        }
    }

    bool attaquantASoin = false;
    bool defenseurASoin = false;

    BuffCommandant(attaquant, attaquantASoin);
    BuffCommandant(defenseur, defenseurASoin);

    int puissance_attaque = std::max(attaquant.damage_point(), attaquant.temporary_damage());

    int degats_finals;

    if(avantage_attaque(attaquant.location(), defenseur.location(), defenseur.regarde()))
    {
        degats_finals = puissance_attaque * 1.5;
    }
    else
    {
        degats_finals = puissance_attaque;
    }
    auto defenses = defenseur.Defensif();

    for(auto const& def : defenses)
    {
        if(degats_finals > 0) {
            degats_finals = def->ReductionDegats(degats_finals);
        }
    }

    defenseur.setHealth_point(defenseur.health_point() - degats_finals);

    // Revoir le soin pour ne pas dépasser la vie de départ de l'unité
    if(defenseurASoin && defenseur.health_point() > 0) SoinDuCommandant(defenseur);
    if(attaquantASoin) SoinDuCommandant(attaquant);

    attaquant.resetTemporary_stats();
    defenseur.resetTemporary_stats();

    return true;
}




// bool Combat::heal(const Unite &healer, const Unite & cible)
// {

// }
