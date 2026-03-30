#include "comportement.hh"
#include "unite.hh"
#include "orientation.hh"

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
CompAttMelee::CompAttMelee(int damage_point):CompAtt(damage_point, 1){}

void CompAttMelee::affiche() const
{

}

void CompAttMelee::update(Unite& proprietaire)
{

}

bool CompAttMelee::PeuxAttaquer(Unite const& attaquante, Unite const& cible)
{
    auto cases_possibles = Voisins(attaquante.location());

    auto it = std::find(cases_possibles.begin(), cases_possibles.end(), cible.location());

    if (it != cases_possibles.end())
    {
        auto listMouvA = attaquante.Mobilite();
        auto listMouvC = cible.Mobilite();

        for (auto* mouvA : listMouvA)
        {
            auto natureA = mouvA->Nature();

            for (auto* mouvC : listMouvC)
            {
                auto natureC = mouvC->Nature();

                if (natureA == natureC) return true;

                if (natureA == NatureMouv::AIR) return true;

                if (natureA == NatureMouv::MER && natureC == NatureMouv::TERRE) return true;
            }
        }
    }
    else return false;
}

//===================================================================
//                        Attaque Distance
//===================================================================
CompAttDistance::CompAttDistance(int damage_point):
    CompAtt(damage_point, 2),
    _munitions(10),
    _portee_mini(2)
{}

CompAttDistance::CompAttDistance(int damage_point, int portee, int munitions, int portee_mini):
    CompAtt(damage_point, portee),
    _munitions(munitions),
    _portee_mini(portee_mini)
{}

void CompAttDistance::setMunitions(int newMunitions)
{
    _munitions = newMunitions;
}
int CompAttDistance::munitions() const
{
    return _munitions;
}
int CompAttDistance::portee_mini() const
{
    return _portee_mini;
}

void CompAttDistance::setPortee_mini(int newPortee_mini)
{
    _portee_mini = newPortee_mini;
}

void CompAttDistance::affiche()const
{

}
void CompAttDistance::update(Unite& proprietaire)
{

}

bool CompAttDistance::PeuxAttaquer(Unite const& attaquante, Unite const& cible)
{
    if(_munitions<=0) return false;
    auto cases_possibles = case_adjascentes(attaquante.location(), _portee);
    auto cases_impossibles = case_adjascentes(attaquante.location(), _portee_mini-1);

    if((cases_possibles.count(cible.location()) > 0) && (cases_impossibles.count(cible.location()) == 0)) //On peut utiliser .contains(cible) en C++
    {
        return true; //Chaque unité distance peut toucher n'importe quel unité
    }
    else return false;
}

//===================================================================
//                        Attaque Indirect
//===================================================================



//====================================================================================================
//====================================================================================================
//====================================================================================================
//====================================================================================================

