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
CompAttIndirect::CompAttIndirect(int damage_point, int portee, int nombre_de_tour_infection):
    CompAtt(damage_point, portee),
    _nombre_de_tour_infection(nombre_de_tour_infection)
{}

void CompAttIndirect::setNombreDeTourInfection(int newNombreDeTourInfection)
{
    _nombre_de_tour_infection = newNombreDeTourInfection;
}
int CompAttIndirect::nombredetourinfection() const
{
    return _nombre_de_tour_infection;
}

void CompAttIndirect::affiche() const
{

}
void CompAttIndirect::update(Unite& proprietaire)
{

}

bool CompAttIndirect::PeuxAttaquer(Unite const& attaquante, Unite const& cible)
{
    auto cases_possibles = case_adjascentes(attaquante.location(), _portee);

    if(cases_possibles.count(cible.location()) > 0)
    {
        return true;
    }
    else return false;
}
void CompAttIndirect::AjoutCibleAtteinte(std::shared_ptr<Unite> const& cible)
{
    _liste_infecter.push_back(infecter{cible, _nombre_de_tour_infection});
}
void CompAttIndirect::RetireCibleAtteinte(infecter const& I)
{
    _liste_infecter.remove(I);
}


//====================================================================================================
//====================================================================================================
//====================================================================================================
//====================================================================================================


//====================================================================================================
//                                              Defense
//====================================================================================================

//===================================================================
//                        Defense Armure
//===================================================================
CompDefArmure::CompDefArmure(int armure): _armure(armure){}

int CompDefArmure::armure() const
{
    return _armure;
}
void CompDefArmure::setArmure(int newArmure)
{
    _armure = newArmure;
}

int CompDefArmure::ReductionDegats(int degat_subit)
{
    return std::abs(degat_subit - _armure);
}

//===================================================================
//                        Defense Bouclier
//===================================================================
CompDefBouclier::CompDefBouclier(int nombre_bouclier): _nombre_bouclier(nombre_bouclier){}

int CompDefBouclier::nombre_bouclier() const
{
    return _nombre_bouclier;
}

void CompDefBouclier::setNombre_bouclier(int newNombre_bouclier)
{
    _nombre_bouclier = newNombre_bouclier;
}

int CompDefBouclier::ReductionDegats(int degat_subit)
{
    if(_nombre_bouclier > 0)
    {
        -- _nombre_bouclier;
        return 0;
    }
    else
    {
        return degat_subit;
    }
}

//====================================================================================================
//====================================================================================================
//====================================================================================================
//====================================================================================================


//====================================================================================================
//                                              Spéciaux
//====================================================================================================
//===================================================================
//                        Transport
//===================================================================

CompTransport::CompTransport(int max_unite_transporter): _max_unite_transporter(max_unite_transporter){}

std::list<std::shared_ptr<Unite> > CompTransport::liste_unite_transporter() const
{
    return _liste_unite_transporter;
}

void CompTransport::setListe_unite_transporter(const std::list<std::shared_ptr<Unite>> &newListe_unite_transporter)
{
    _liste_unite_transporter = newListe_unite_transporter;
}

int CompTransport::max_unite_transporter() const
{
    return _max_unite_transporter;
}

void CompTransport::setMax_unite_transporter(int newMax_unite_transporter)
{
    _max_unite_transporter = newMax_unite_transporter;
}


bool CompTransport::MonterUnite(Unite const& Transport, std::shared_ptr<Unite> const& Voyageur)
{
    auto cases_possibles = Voisins(Voyageur->location());

    auto it = std::find(cases_possibles.begin(), cases_possibles.end(), Transport.location());

    if (it != cases_possibles.end())
    {
        _liste_unite_transporter.push_back(Voyageur);
        return true;
    }
    else return false;
}

bool CompTransport::DescenteUniteUnite(Unite const& Transport, std::shared_ptr<Unite> const& Voyageur)
{
    bool present = false;
    for(auto const& U : _liste_unite_transporter)
    {
        if(U == Voyageur) present = true;
    }

    if(present)
    {
        auto cases_possibles = Voisins(Transport.location());
        auto mouv = Voyageur->Mobilite();

        for(auto const& C : cases_possibles)
        {
            for(auto const& M : mouv)
            {
                if(M->EstCaseValide(Transport.location(), C))
                {
                    _liste_unite_transporter.remove(Voyageur);
                    return true;
                }
            }
        }
        return false;
    }
    else return false;
}
