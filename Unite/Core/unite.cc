#include "unite.hh"

Unite::Unite(const std::string &name, int hp, int dmg, direction dir, Case loc, std::shared_ptr<IRank> r)
    :_name(name),
    _health_point(hp),
    _damage_point(dmg),
    _moral_point(0),
    _regarde(dir),
    _location(loc),
    _rank(r)
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

int Unite::damage_point() const
{
    return _damage_point;
}

void Unite::setDamage_point(int newDamage_point)
{
    _damage_point = newDamage_point;
}

int Unite::moral_point() const
{
    return _moral_point;
}

void Unite::setMoral_point(int newMoral_point)
{
    _moral_point = newMoral_point;
}


direction Unite::regarde() const
{
    return _regarde;
}

void Unite::setRegarde(direction newRegarde)
{
    _regarde = newRegarde;
}

Case Unite::location() const
{
    return _location;
}

void Unite::setLocation(const Case &newLocation)
{
    _location = newLocation;
}

std::shared_ptr<IRank> Unite::rank() const
{
    return _rank;
}

std::list<std::unique_ptr<IComportement> > Unite::liste_comportements() const
{
    return _liste_comportements;
}


void Unite::ajouterComportement(std::unique_ptr<IComportement> comp)
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

void Unite::affiche() const
{
    std::cout << "=== [" << _name << "] ===" << std::endl;
    std::cout << "Position: (" << _location.x << "," << _location.y << ")" << std::endl;
    if(_rank)
    {
        std::cout << "Grade: "; _rank->get_role(); std::cout << std::endl;
    }

    for(auto const& comp : _liste_comportements)
    {
        comp->affiche();
    }
}

// void Unite::movement(Case const& c) {
//     for(auto const& elt : _liste_comportements)
//     {
//         if(auto TypeMouv = std::dynamic_pointer_cast<CompMouv>(elt))
//         {
//             if(TypeMouv->EstCaseValide(_location, c))
//             {
//                 _location = c;
//                 return;
//             }
//         }
//     }
// }
