#include "comportement.hh"
#include "unite.hh"
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


//===================================================================
//                         Mouvement Marin
//===================================================================
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


//===================================================================
//                        Mouvement Terrestre
//===================================================================
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


//====================================================================================================
//====================================================================================================
//====================================================================================================
//====================================================================================================


