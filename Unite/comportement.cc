#include "comportement.hh"
#include "unite.hh"
#include "orientation.hh"
#include <algorithm>
#include <algorithm>


//====================================================================================================
//                                              Tools
//====================================================================================================
ComportementCooldown::ComportementCooldown(int cooldown):
    _cooldown(cooldown),
    _current_cooldown(0)
{}

int ComportementCooldown::current_cooldown() const
{
    return _current_cooldown;
}

void ComportementCooldown::setCurrent_cooldown(int newNb_tour_cammouflage)
{
    _current_cooldown = newNb_tour_cammouflage;
}

int ComportementCooldown::cooldown() const
{
    return _cooldown;
}

void ComportementCooldown::setCooldown(int newCooldown)
{
    _cooldown = newCooldown;
}

bool ComportementCooldown::estPret() const {
    return _current_cooldown == 0;
}


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

bool CompMouv::EstCaseValide(Coord const& actuel, Coord const& cible)
{
    // Le Test de franchisemant est fait dans board
    //Ici on test la porte
    if((std::abs(actuel.first - cible.first) <= mov_per_laps()) && ((std::abs(actuel.second - cible.second) <= mov_per_laps()))) //std::abs = valeur absolue
    {
        return true;
    }
    else return false;
}

//===================================================================
//                         Mouvement Volant
//===================================================================
CompMouvVolant::CompMouvVolant(int mouvement_par_tour): CompMouv(mouvement_par_tour){}

void CompMouvVolant::affiche() const
{
    std::cout << "[Mouvement] Vol : " << mov_per_laps() << " cases" << std::endl;
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
    std::cout << "[Mouvement] Mer : " << mov_per_laps() << " cases"  << std::endl;
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
    std::cout << "[Mouvement] Terrestre : " << mov_per_laps() << " cases" << std::endl;
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
    std::cout << "[Attaque] Melee : Dgt=" << damage_point() << ", Portée=" << portee() << std::endl;
}


bool CompAttMelee::PeuxAttaquer(Unite const& attaquante, Unite const& cible)const
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
        return false;
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
    std::cout << "[Attaque] Distance : Dgt=" << damage_point() << ", Portée max=" << portee() << "|Portée min=" << _portee_mini<< ", Munitions="  << _munitions << std::endl;
}


bool CompAttDistance::PeuxAttaquer(Unite const& attaquante, Unite const& cible)const
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
    std::cout << "[Attaque] Indirect : Dgt=" << damage_point() << ", Portée=" << portee() << ", Tour d'infection=" << _nombre_de_tour_infection << std::endl;
}
void CompAttIndirect::update()
{
    std::list<infecter> _liste_final;

    for(auto & infect : _liste_infecter)
    {
        auto c = infect.cible.lock();
        if(c != nullptr)
        {
            c->setHealth_point(c->health_point() - _damage_point);
            infect.tour_infection -= 1;

            if(infect.tour_infection > 0 && c->health_point() > 0)
            {
                _liste_final.push_back(infect);
            }
        }
    }
    _liste_infecter = _liste_final; //
}

bool CompAttIndirect::PeuxAttaquer(Unite const& attaquante, Unite const& cible) const
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

void CompDefArmure::affiche() const
{
    std::cout << "[Defense] Armure : Vie armure=" << _armure << std::endl;
}

int CompDefArmure::ReductionDegats(int degat_subit)
{
    return std::max(0, degat_subit - _armure);
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

void CompDefBouclier::affiche() const
{
    std::cout << "[Defense] Bouclier : Nombre bouclier=" << _nombre_bouclier << std::endl;
}
void CompDefBouclier::update()
{
    _nombre_bouclier += 1;
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
//                                              Soin
//====================================================================================================
CompSoin::CompSoin(int healing_point, int portee, int cooldown):
    ComportementCooldown(cooldown),
    _healing_point(healing_point),
    _portee(portee)
{}

int CompSoin::portee() const
{
    return _portee;
}

void CompSoin::setPortee(int newPortee)
{
    _portee = newPortee;
}

int CompSoin::healing_point() const
{
    return _healing_point;
}

void CompSoin::setHealing_point(int newHealing_point)
{
    _healing_point = newHealing_point;
}




//===================================================================
//                        Direct
//===================================================================
CompSoinDirect::CompSoinDirect(int healing_point, int portee, int rayon, int cooldown):
    CompSoin(healing_point, portee, cooldown),
    _rayon(rayon)
{}

int CompSoinDirect::rayon() const
{
    return _rayon;
}

void CompSoinDirect::setRayon(int newRayon)
{
    _rayon = newRayon;
}

void CompSoinDirect::affiche() const
{
    std::cout << "[Soin] Direct : Soin=" << _healing_point << ", Portée=" << _portee << ", Rayon=" << _rayon << std::endl;
}

void CompSoinDirect::update()
{
    if (_current_cooldown > 0) 
    {
        _current_cooldown--;
    }
}

bool CompSoinDirect::PeuxSoigner(Unite const& attaquante, Unite const& cible) const
{
    auto cases_possibles = case_adjascentes(attaquante.location(), _portee);

    if(cases_possibles.count(cible.location()) > 0)
    {
        return true;
    }
    else return false;
}

//===================================================================
//                        Indirect
//===================================================================
CompSoinIndirect::CompSoinIndirect(int healing_point, int portee, int nombre_de_tour_regeneration, int cooldown):
    CompSoin(healing_point, portee, cooldown),
    _nombre_de_tour_regeneration(nombre_de_tour_regeneration)
{}

void CompSoinIndirect::setNombreDeTourRegen(int newNombreDeTourRegen)
{
    _nombre_de_tour_regeneration = newNombreDeTourRegen;
}
int CompSoinIndirect::nombredetourregen() const
{
    return _nombre_de_tour_regeneration;
}

void CompSoinIndirect::affiche() const
{
    std::cout << "[Soin] Indirect : Soin=" << _healing_point << ", Portée=" << _portee << ", Tour régénération=" << _nombre_de_tour_regeneration << std::endl;
}
void CompSoinIndirect::update()
{
    if (_current_cooldown > 0) 
    {
        _current_cooldown--;
    }

    std::list<soigner> _liste_final;

    for(auto & soin : _liste_soigner)
    {
        auto c = soin.cible.lock();
        if(c != nullptr)
        {
            if(c->health_point() + _healing_point <= c->health_point_max())
            {
                c->setHealth_point(c->health_point() + _healing_point);
                soin.tour_soin -= 1;
            }
            else if(soin.tour_soin > 0 && c->health_point() > 0)
            {
                _liste_final.push_back(soin);
            }

        }
    }
    _liste_soigner = _liste_final;
}

bool CompSoinIndirect::PeuxSoigner(Unite const& attaquante, Unite const& cible) const
{
    auto cases_possibles = case_adjascentes(attaquante.location(), _portee);

    if(cases_possibles.count(cible.location()) > 0)
    {
        return true;
    }
    else return false;
}
void CompSoinIndirect::AjoutCibleAtteinte(std::shared_ptr<Unite> const& cible)
{
    _liste_soigner.push_back(soigner{cible, _nombre_de_tour_regeneration});
}
void CompSoinIndirect::RetireCibleAtteinte(soigner const& I)
{
    _liste_soigner.remove(I);
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

void CompTransport::affiche() const
{
    std::cout << "[Special] Transport : " << _liste_unite_transporter.size()<<"/"<< _max_unite_transporter << ", Unitées transportées : [ ";
    for(auto u : _liste_unite_transporter)
    {
        u->affiche();
    }
    std::cout << " ]" <<std::endl;
}

int CompTransport::nb_unite_actuelle()const
{
    return _liste_unite_transporter.size();
}

bool CompTransport::MonterUnite(Unite const& Transport, std::shared_ptr<Unite> const& Voyageur)
{
    auto cases_possibles = Voisins(Voyageur->location());

    auto it = std::find(cases_possibles.begin(), cases_possibles.end(), Transport.location());

    if (it != cases_possibles.end())
    {
        Voyageur->setLocation(Transport.location());
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

//===================================================================
//                        Furtivité
//===================================================================
CompFurtif::CompFurtif(int duree, int cooldown):
    ComportementCooldown(cooldown),
    _camoufler(false),
    _duree_max_camouflage(duree),
    _tours_restants(0)
{}

bool CompFurtif::camoufler() const
{
    return _camoufler;
}

void CompFurtif::affiche() const
{
    std::cout << "[Special] Camouflage : Durée max=" << _duree_max_camouflage<<", Cooldown="<< _cooldown;
    if(_camoufler) std::cout<<"Cammouflage actif"<<std::endl;
    else std::cout<<"Cammouflage non actif"<<std::endl;
}
void CompFurtif::update()
{
    if(_camoufler) 
    {
        _tours_restants--;
        if (_tours_restants <= 0) 
        {
            DesactiveCammouflage();
        }
    }
    
    if (_current_cooldown > 0) 
    {
        _current_cooldown--;
    }
}

void CompFurtif::ActiveCammouflage()
{
    if(estPret() && !_camoufler)
    {
        _camoufler = true;
        _tours_restants = _duree_max_camouflage;
        _current_cooldown = _cooldown; 
    }
}

void CompFurtif::DesactiveCammouflage()
{
    _camoufler = false;
    _tours_restants = 0;
}
