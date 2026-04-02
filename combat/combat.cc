#include "combat.hh"
#include "comportement.hh"
#include "rank.hh"
#include <algorithm>
//==============================================================================
//                             Compétences Commandant
//==============================================================================
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
                u.setHealth_point(soin->appliquer_soin(u.health_point(), u));
            }
        }
    }
}
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================
//                             Gestion du Moral
//==============================================================================
int MAX_MORAL = 20;
int MIN_MORAL = -20;

void AugmentationMoral(Unite &u, int x_point)
{
    if(x_point > 0)
    {
        int x = u.moral_point() + x_point;
        if(x <= MAX_MORAL)
        {
            u.setMoral_point(x);
        }
        else
        {
            u.setMoral_point(MAX_MORAL);
        }
    }
}

void DiminussionMoral(Unite &u, int x_point)
{
    if(x_point > 0)
    {
        int x = u.moral_point() - x_point;
        if(x <= MIN_MORAL)
        {
            u.setMoral_point(x);
        }
        else
        {
            u.setMoral_point(MIN_MORAL);
        }
    }
}

void EffetMoral(Unite & u)
{
    int moral = u.moral_point();
    int baseDmg = u.damage_point_start();
    float coeff = 1.0;

    // Fatigue
    if (moral > MAX_MORAL*0.75)
    {
        coeff = 0.9;
    }

    // Héroïsme
    else if (moral > MAX_MORAL*0.5)
    {
        coeff = 1.5;
    }

    // Courage
    else if (moral > MAX_MORAL*0.25)
    {
        coeff = 1.2;
    }

    // Peur
    else if (moral < MIN_MORAL*0.25)
    {
        coeff = 0.8;
    }

    // Panique
    else if (moral < MIN_MORAL*0.5)
    {
        coeff = 0.5;
        if (u.health_point() > u.health_point_max() * 0.8)
        {
            u.setHealth_point(u.health_point() * 0.8);
        }
    }

    // Fuite
    else if (moral < MIN_MORAL*0.75)
    {
        u.setHealth_point(0);
    }

    u.setTemporary_damage(static_cast<int>(baseDmg * coeff));
}

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================
//                                  Combat
//==============================================================================
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

    EffetMoral(attaquant);
    EffetMoral(defenseur);


    BuffCommandant(attaquant, attaquantASoin);
    BuffCommandant(defenseur, defenseurASoin);

    int puissance_attaque = std::max(attaquant.damage_point(), attaquant.temporary_damage());

    int degats_finals;

    int newmoralAtt = 0;
    int newmoralDef = 0;

    if(avantage_attaque(attaquant.location(), defenseur.location(), defenseur.regarde()))
    {
        degats_finals = puissance_attaque * 1.5;
        newmoralDef = 2;
        newmoralAtt = 2;
    }
    else
    {
        degats_finals = puissance_attaque;
        newmoralDef = 1;
        newmoralAtt = 1;
    }

    auto defenses = defenseur.Defensif();

    for(auto const& def : defenses)
    {
        if(degats_finals > 0) {
            degats_finals = def->ReductionDegats(degats_finals);
            newmoralDef = 1;
        }
    }


    if(degats_finals <= 0)
    {
        AugmentationMoral(defenseur, 1);
        DiminussionMoral(attaquant, 1);
    }
    else
    {
        DiminussionMoral(defenseur, newmoralDef);
        AugmentationMoral(attaquant, newmoralAtt);
    }

    defenseur.setHealth_point(defenseur.health_point() - degats_finals);


    if(defenseurASoin && defenseur.health_point() > 0)
    {
        SoinDuCommandant(defenseur);
    }
    if(attaquantASoin)
    {
        SoinDuCommandant(attaquant);
    }

    attaquant.resetTemporary_stats();
    defenseur.resetTemporary_stats();

    return true;
}




// bool Combat::heal(const Unite &healer, const Unite & cible)
// {

// }

