#include "unite.hh"
#include "comportement.hh"

Unite::Unite(const std::string &name, int hp,Poids poids, direction dir, Coord loc, std::shared_ptr<IRank> r, std::list<std::shared_ptr<IComportement>> liste_comportements)
    :_name(name),
    _health_point(hp),
    _health_point_max(hp),
    _moral_point(0),
    _poids(poids),
    _regarde(dir),
    _location(loc),
    _rank(r),
    _liste_comportements(liste_comportements)
{}

std::string Unite::name() const
{
    return _name;
}

int Unite::health_point() const
{
    return _health_point;
}

void Unite::setHealth_point(int newHealth_point)
{
    _health_point = newHealth_point;
}

int Unite::health_point_max() const
{
    return _health_point_max;
}

int Unite::moral_point() const
{
    return _moral_point;
}

void Unite::setMoral_point(int newMoral_point)
{
    _moral_point = newMoral_point;
}

Poids Unite::poids() const
{
    return _poids;
}

void Unite::setPoids(Poids newPoids)
{
    _poids = newPoids;
}


direction Unite::regarde() const
{
    return _regarde;
}

void Unite::setRegarde(direction newRegarde)
{
    _regarde = newRegarde;
}

Coord Unite::location() const
{
    return _location;
}

void Unite::setLocation(const Coord &newLocation)
{
    _location = newLocation;
}

std::shared_ptr<IRank> Unite::rank() const
{
    return _rank;
}

std::list<std::shared_ptr<IComportement> > Unite::liste_comportements() const
{
    return _liste_comportements;
}

int Unite::temporary_health() const
{
    return _temporary_health;
}

void Unite::setTemporary_health(int newTemporary_health)
{
    _temporary_health = newTemporary_health;
}

int Unite::temporary_damage() const
{
    return _temporary_damage;
}

void Unite::setTemporary_damage(int newTemporary_damage)
{
    _temporary_damage = newTemporary_damage;
}

void Unite::ajouterComportement(std::shared_ptr<IComportement> comp)
{
    if (comp)
    {
        _liste_comportements.push_back(comp);
    }
}

void Unite::update()
{
    for(auto const& elt : _liste_comportements)
    {
        elt->update(*this);
    }
}

std::list<CompMouv*> Unite::Mobilite() const
{
    std::list<CompMouv*> liste_CompMouv;

    for (const auto& comp_ptr : _liste_comportements)
    {
        if (auto* typeMouv = dynamic_cast<CompMouv*>(comp_ptr.get()))
        {
            liste_CompMouv.push_back(typeMouv);
        }
    }
    return liste_CompMouv;
}
std::list<CompAtt*> Unite::Offensive() const
{
    std::list<CompAtt*> liste_CompAtt;

    for (const auto& comp_ptr : _liste_comportements)
    {
        if (auto* typeMouv = dynamic_cast<CompAtt*>(comp_ptr.get()))
        {
            liste_CompAtt.push_back(typeMouv);
        }
    }
    return liste_CompAtt;
}

std::list<CompDef*> Unite::Defensif() const
{
    std::list<CompDef*> liste_CompDef;

    for (const auto& comp_ptr : _liste_comportements)
    {
        if (auto* typeMouv = dynamic_cast<CompDef*>(comp_ptr.get()))
        {
            liste_CompDef.push_back(typeMouv);
        }
    }
    return liste_CompDef;
}

std::list<CompSoin *> Unite::Soin() const
{
    std::list<CompSoin*> liste_CompSoin;

    for (const auto& comp_ptr : _liste_comportements)
    {
        if (auto* typeMouv = dynamic_cast<CompSoin*>(comp_ptr.get()))
        {
            liste_CompSoin.push_back(typeMouv);
        }
    }
    return liste_CompSoin;
}

CompFurtif* Unite::Cammouflage() const
{
    for (const auto& comp_ptr : _liste_comportements)
    {
        if (auto* furtif = dynamic_cast<CompFurtif*>(comp_ptr.get()))
        {
            return furtif;
        }
    }
    return nullptr;
}

void Unite::affiche() const
{
    std::cout << "=== [" << _name << "] ===" << std::endl;
    std::cout << "Position: (" << _location.first << "," << _location.second << ")" << std::endl;
    if(_rank)
    {
        std::cout << "Grade: "; _rank->get_role(); std::cout << std::endl;
    }

    for(auto const& comp : _liste_comportements)
    {
        comp->affiche();
    }
}

void Unite::resetTemporary_stats()
{
    _temporary_damage = 0;
    _temporary_health = 0;
}
