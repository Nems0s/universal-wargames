#include "comportement.hh"
#include "unite.hh"
#include <typeinfo>
//====================================================================================================
//                                              Mouvement
//====================================================================================================

CompMouv::CompMouv(int mouvement_par_tour): _mov_per_laps(mouvement_par_tour){}

int CompMouv::mov_per_laps() const
{
    return _mov_per_laps;
}

void CompMouv::setMov_per_laps(int newMov_per_laps)
{
    _mov_per_laps = newMov_per_laps;
}

//===================================================================
//                         Mouvement Volant
//===================================================================
CompMouvVolant::CompMouvVolant():CompMouv(2){}
CompMouvVolant::CompMouvVolant(int mouvement_par_tour): CompMouv(mouvement_par_tour){}

void CompMouvVolant::affiche() const
{

}

void CompMouvVolant::update(Unite& proprietaire)
{

}

bool CompMouvVolant::EstCaseValide(Case const& actuel, Case const& cible)
{
    // Finir Quand le plateau sera prêt
    //if(case != inffranchissable)
    //{
    if((std::abs(actuel.x - cible.x) <= mov_per_laps()) && ((std::abs(actuel.y - cible.y) <= mov_per_laps()))) //std::abs = valeur absolue
    {
        return true;
    }
    else return false;
    //}
    //else return false;
}

NatureMouv CompMouvVolant::Nature() const
{
    return NatureMouv::AIR;
}


//===================================================================
//                         Mouvement Marin
//===================================================================
CompMouvMarin::CompMouvMarin():CompMouv(2){}
CompMouvMarin::CompMouvMarin(int mouvement_par_tour): CompMouv(mouvement_par_tour){}

void CompMouvMarin::affiche() const
{

}

void CompMouvMarin::update(Unite& proprietaire)
{

}

bool CompMouvMarin::EstCaseValide(Case const& actuel, Case const& cible)
{
    // Finir Quand le plateau sera prêt
    //if(case != inffranchissable)
    //{
    if((std::abs(actuel.x - cible.x) <= mov_per_laps()) && ((std::abs(actuel.y - cible.y) <= mov_per_laps()))) //std::abs = valeur absolue
    {
        return true;
    }
    else return false;
    //}
    //else return false;
}

NatureMouv CompMouvMarin::Nature() const
{
    return NatureMouv::MER;
}

//===================================================================
//                        Mouvement Terrestre
//===================================================================
CompMouvTerrestre::CompMouvTerrestre():CompMouv(1){}
CompMouvTerrestre::CompMouvTerrestre(int mouvement_par_tour): CompMouv(mouvement_par_tour){}

void CompMouvTerrestre::affiche() const
{

}

void CompMouvTerrestre::update(Unite& proprietaire)
{

}

bool CompMouvTerrestre::EstCaseValide(Case const& actuel, Case const& cible)
{
    // Finir Quand le plateau sera prêt
    //if(case != inffranchissable)
    //{
    if((std::abs(actuel.x - cible.x) <= mov_per_laps()) && ((std::abs(actuel.y - cible.y) <= mov_per_laps()))) //std::abs = valeur absolue
    {
        return true;
    }
    else return false;
    //}
    //else return false;
}

NatureMouv CompMouvTerrestre::Nature() const
{
    return NatureMouv::TERRE;
}

//====================================================================================================
//====================================================================================================
//====================================================================================================
//====================================================================================================


//====================================================================================================
//                                              Attaque
//====================================================================================================
CompAtt::CompAtt(int damage_point, int portee):
    _damage_point(damage_point), _portee(portee)
{}

int CompAtt::damage_point() const
{
    return _damage_point;
}
void CompAtt::setDamage_point(int newDamage_point)
{
    _damage_point = newDamage_point;
}

int CompAtt::portee() const
{
    return _portee;
}
void CompAtt::setPortee(int newPortee)
{
    _portee = newPortee;
}


//===================================================================
//                        Attaque Direct
//===================================================================
CompAttDirect::CompAttDirect(int damage_point):CompAtt(damage_point, 1){}

void CompAttDirect::affiche() const
{

}

void CompAttDirect::update(Unite& proprietaire)
{

}

bool CompAttDirect::PeuxAttaquer(Unite const& attaquante, Unite const& cible)
{
    for(auto const& comp_a : attaquante.liste_comportements())
    {
        if(auto TypeMouvA = std::dynamic_pointer_cast<CompMouv>(comp_a))
        {
            for(auto const& comp_c : cible.liste_comportements())
            {
                auto typemouvA = TypeMouvA->Nature();

                if(auto TypeMouvC = std::dynamic_pointer_cast<CompMouv>(comp_c))
                {
                    auto typemouvC = TypeMouvC->Nature();

                    if(typemouvA == typemouvC)
                    {
                        return true;
                    }

                    if(typemouvA == "Volant")
                    {
                        return true;
                    }

                    if(typemouvA == "Marin" && typemouvC == "Terrestre")
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

//===================================================================
//                        Attaque Distance
//===================================================================


//===================================================================
//                        Attaque Indirect
//===================================================================



//====================================================================================================
//====================================================================================================
//====================================================================================================
//====================================================================================================

