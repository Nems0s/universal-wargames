#include "combat.hh"
#include "comportement.hh"
#include "rank.hh"
#include <algorithm>
//==============================================================================
//                             Compétences Commandant
//==============================================================================
void BuffCommandant(Unite & u, bool & aSoin, int degatsArme)
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
                    u.setTemporary_damage(att->appliquer(degatsArme));
                }
                else if(auto def = std::dynamic_pointer_cast<BonusDefense>(buff))
                {
                    u.setTemporary_health(def->appliquer(u.health_point_max()));
                }
                std::cout << "[BUFF] " << buff->nom() << " de " << regulier->commandant()->name() << " active sur " << u.name() << std::endl;
            }
        }
    }
}

void SoinDuCommandant(Unite & u)
{
    auto r = u.rank();
    auto regulier = std::dynamic_pointer_cast<Rank_Regulier>(r);
    if(regulier && regulier->PossedeCommandant())
    {
        auto com = std::dynamic_pointer_cast<Rank_Commandant>(regulier->commandant()->rank());
        if(com)
        {
            for(auto const& buff : com->liste_bonus())
            {
                if(auto soin = std::dynamic_pointer_cast<BonusVie>(buff))
                {
                    if(soin->appliquer(u.health_point()) <= u.health_point_max())
                    {
                        u.setHealth_point(soin->appliquer(u.health_point()));
                    }
                }
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
        std::cout << "[MORAL] " << u.name() << " : " << x-x_point << " -> " << u.moral_point() << " (+" << x_point << ")" << std::endl;
    }
}

void DiminussionMoral(Unite &u, int x_point)
{
    if(x_point > 0)
    {
        int x = u.moral_point() - x_point;
        if(x >= MIN_MORAL)
        {
            u.setMoral_point(x);
        }
        else
        {
            u.setMoral_point(MIN_MORAL);
        }
        std::cout << "[MORAL] " << u.name() << " : " << x+x_point << " -> " << u.moral_point() << " (-" << x_point << ")" << std::endl;
    }
}

void EffetMoral(Unite & u, int degatsArme)
{
    int moral = u.moral_point();
    float coeff = 1.0;

    float ratio = (moral - MIN_MORAL) / (MAX_MORAL - MIN_MORAL);

    std::string etat = "Neutre";

    // Fuite
    if (ratio < 0.14)
    {
        std::cout << "[ALERTE] " << u.name() << " s'enfuit du champ de bataille !" << std::endl;
        u.setHealth_point(0);
        return;
        }

    // Panique
    else if (ratio < 0.28)
    {
        coeff = 0.5;
        if (u.health_point() > u.health_point_max() * 0.8)
        {
            u.setHealth_point(u.health_point() * 0.8);
        }
        etat = "Panique (Malus)";
    }

    // Peur
    else if (ratio < 0.42)
    {
        coeff = 0.8;
        etat = "Peur (Malus)";
    }

    // Fatigue
    else if (ratio > 0.86)
    {
        coeff = 0.9;
        etat = "Fatigue (Malus)";
    }

    // Héroïsme
    else if (ratio > 0.72)
    {
        coeff = 1.5;
        etat = "Heroisme (Bonus)";
    }

    // Courage
    else if (ratio > 0.58)
    {
        coeff = 1.2;
        etat = "Courage (Bonus)";
    }
    
    if(etat != "Neutre")
    {
        std::cout << "[ETAT] " << u.name() << " est en etat : " << etat << std::endl;
    }
    u.setTemporary_damage(static_cast<int>(degatsArme * coeff));
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
    if(attaquant.defensif()) return false;

    auto styles_attaque = attaquant.Offensive();
    auto it = std::find(styles_attaque.begin(), styles_attaque.end(), TypeAttaque);

    if (it == styles_attaque.end() || !((*it)->PeuxAttaquer(attaquant, defenseur)))
    {
        return false;
    }

    bool attaquantASoin = false;
    bool defenseurASoin = false;

    auto furtifDef = defenseur.Cammouflage(); //evite l'erreur si nullptr
    if(furtifDef && furtifDef->camoufler())
    {
        return false;
    }

    if(auto* infect = dynamic_cast<CompAttIndirect*>(*it))
    {
        if(infect)
        {
           infect->AjoutCibleAtteinte(defenseur.shared_from_this());
        }
    }
    
    // Sert à eviter un test sur cammouflage alors que l'unite en à pas
    bool estCamoufle = false;
    auto furtifAtt = attaquant.Cammouflage();
    if(furtifAtt && furtifAtt->camoufler()) 
    {
        estCamoufle = true;
    }

    EffetMoral(attaquant, TypeAttaque->damage_point());
    EffetMoral(defenseur);


    BuffCommandant(attaquant, attaquantASoin, TypeAttaque->damage_point());
    BuffCommandant(defenseur, defenseurASoin);

    int puissance_attaque = std::max(TypeAttaque->damage_point(), attaquant.temporary_damage());

    int degats_finals;

    int newmoralAtt = 0;
    int newmoralDef = 0;

    if(((avantage_attaque(attaquant.location(), defenseur.location(), defenseur.regarde())) && (defenseur.defensif() == false))|| estCamoufle == true )
    {
        degats_finals = puissance_attaque * 1.5;
        newmoralDef = MAX_MORAL * (GROS_CHANGE / 100.0);
        newmoralAtt = MAX_MORAL * (GROS_CHANGE / 100.0);
        if(estCamoufle == true)
        {
            attaquant.Cammouflage()->DesactiveCammouflage();
        }
    }
    else
    {
        degats_finals = puissance_attaque;
        newmoralDef = MAX_MORAL * (PETIT_CHANGE / 100.0);
        newmoralAtt = MAX_MORAL * (PETIT_CHANGE / 100.0);
    }

    auto defenses = defenseur.Defensif();

    for(auto const& def : defenses)
    {
        if(degats_finals > 0) 
        {
            degats_finals = def->ReductionDegats(degats_finals);
        }
    }


    if(degats_finals <= 0)
    {
        AugmentationMoral(defenseur, MAX_MORAL * (PETIT_CHANGE / 100.0));
        DiminussionMoral(attaquant, MAX_MORAL * (PETIT_CHANGE / 100.0));
    }
    else
    {
        DiminussionMoral(defenseur, newmoralDef);
        AugmentationMoral(attaquant, newmoralAtt);
    }

    // Vérifier si une des deux unitées fuit
    EffetMoral(defenseur);
    EffetMoral(attaquant);

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



bool Combat::heal(Unite &healer, CompSoin* const& TypeSoin,Unite & cible)
{
    auto styles_healer = healer.Soin();
    auto it = std::find(styles_healer.begin(), styles_healer.end(), TypeSoin);

    if (it == styles_healer.end() || !((*it)->PeuxSoigner(healer, cible)))
    {
        return false;
    }

    if(TypeSoin->PeuxSoigner(healer, cible))
    {
        if(auto* soin = dynamic_cast<CompSoinIndirect*>(*it))
        {
            if(soin)
            {
                soin->AjoutCibleAtteinte(cible.shared_from_this());
                AugmentationMoral(healer, MAX_MORAL * (PETIT_CHANGE / 100.0));
                AugmentationMoral(cible, MAX_MORAL * (PETIT_CHANGE / 100.0));
            }
        }

        if(cible.health_point() + TypeSoin->healing_point() <= cible.health_point_max())
        {
            cible.setHealth_point(cible.health_point() + TypeSoin->healing_point());
            AugmentationMoral(healer, MAX_MORAL * (PETIT_CHANGE / 100.0));
            AugmentationMoral(cible, MAX_MORAL * (GROS_CHANGE / 100.0));
        }
        return true;
    }

    return false;
}

