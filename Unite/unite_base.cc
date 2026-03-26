#include "unite_base.hh"
#include <iostream>

std::string directionToString(direction dir) {
    switch (dir) {
    case direction::nord_ouest: return "Nord-Ouest";
    case direction::nord_est:   return "Nord-Est";
    case direction::ouest:      return "Ouest";
    case direction::est:        return "Est";
    case direction::sud_ouest:  return "Sud-Ouest";
    case direction::sud_est:    return "Sud-Est";
    default:                    return "Direction inconnue";
    }
}

// ==========================================
//                     Unite
// ==========================================
Unite::Unite(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location)
    : _name(name),
    _health_point(health_point),
    _damage_point(damage_point),
    _moral_point(0),
    _move_per_laps(move_per_laps),
    _regarde(regarde),
    _location(location)
{}

const std::string &Unite::name() const
{
    return _name;
}
int Unite::health_point() const
{
    return _health_point;
}
int Unite::damage_point() const
{
    return _damage_point;
}
int Unite::moral_point() const
{
    return _moral_point;
}
int Unite::move_per_laps() const
{
    return _move_per_laps;
}
direction Unite::regarde() const
{
    return _regarde;
}

void Unite::setHealth_point(int newHealth_point)
{
    _health_point = newHealth_point;
}
void Unite::setDamage_point(int newDamage_point)
{
    _damage_point = newDamage_point;
}
void Unite::setMoral_point(int newMoral_point)
{
    _moral_point = newMoral_point;
}
void Unite::setMove_per_laps(int newMove_per_laps)
{
    _move_per_laps = newMove_per_laps;
}
void Unite::setRegarde(direction newRegarde)
{
    _regarde = newRegarde;
}

void Unite::movement(Case const& c)
{
    if((_location.x+_move_per_laps >= c.x) && (_location.y+_move_per_laps >= c.y)) {
        _location = c;
    }
}

void Unite::affiche() const {
    std::cout<<"    HP: "<<_health_point<<std::endl;
    std::cout<<"    Dmg: "<<_damage_point<<std::endl;
    std::cout<<"    Mov: "<<_move_per_laps<<std::endl;
    std::cout<<"    Mor: "<<_moral_point<<std::endl;
    std::cout<<"    Loc: ("<<_location.x<<","<<_location.y<<")/"<<directionToString(_regarde)<<std::endl;
    std::cout<<"====================================="<<std::endl;
}






// ==========================================
//                 Unite Legere
// ==========================================
Unite_legere::Unite_legere(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location)
    : Unite(name, health_point, damage_point, move_per_laps, regarde, location)
{}

void Unite_legere::affiche() const
{
    std::cout<<"====================================="<<std::endl;
    std::cout<<"Unite Legere: "<<_name<<std::endl;
    Unite::affiche();
}






// ==========================================
//                Unite Volante
// ==========================================
Unite_volante::Unite_volante(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location)
    : Unite(name, health_point, damage_point, move_per_laps, regarde, location), _vole(true)
{}

void Unite_volante::affiche() const
{
    std::cout<<"====================================="<<std::endl;
    std::cout<<"Unite Volante: "<<_name<<std::endl;
    Unite::affiche();
}





// ==========================================
//                Unite Maritime
// ==========================================
Unite_maritime::Unite_maritime(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location)
    : Unite(name, health_point, damage_point, move_per_laps, regarde, location), _navigue(true)
{}

void Unite_maritime::affiche() const
{
    std::cout<<"====================================="<<std::endl;
    std::cout<<"Unite Maritime: "<<_name<<std::endl;
    Unite::affiche();
}





// ==========================================
//                Unite Lourde
// ==========================================
Unite_lourde::Unite_lourde(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location)
    : Unite(name, health_point, damage_point, move_per_laps, regarde, location)
{}

void Unite_lourde::affiche() const
{
    std::cout<<"====================================="<<std::endl;
    std::cout<<"Unite Lourde: "<<_name<<std::endl;
    Unite::affiche();
}




// ==========================================
//                Unite Transport
// ==========================================
Unite_transport::Unite_transport(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location)
    : Unite(name, health_point, damage_point, move_per_laps, regarde, location)
{}

void Unite_transport::affiche() const
{
    std::cout<<"====================================="<<std::endl;
    std::cout<<"Unite de Transport: "<<_name<<std::endl;
    std::cout<<"unites transporter: ";
    affiche_unites();
    Unite::affiche();
}

void Unite_transport::affiche_unites() const
{
    std::cout<<"[ "<<std::endl;
    for(auto &elt : _liste_unite)
    {
        std::cout<<elt->name()<<"/";
    }
    std::cout<<" ]"<<std::endl;
}
