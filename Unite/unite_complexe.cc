#include "unite_complexe.hh"

// ==========================================
//                 Unite Cuirrasse
// ==========================================
Unite_cuirrasse::Unite_cuirrasse(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location)
    : Unite(name, health_point, damage_point, move_per_laps, regarde, location),
    Unite_maritime(name, health_point, damage_point, move_per_laps, regarde, location),
    Unite_lourde(name, health_point, damage_point, move_per_laps, regarde, location)
{}

void Unite_cuirrasse::affiche() const
{
    std::cout<<"====================================="<<std::endl;
    std::cout<<"Unite Cuirasse (Maritime & Lourde) : "<<_name<<std::endl;
    Unite::affiche();
}

void Unite_cuirrasse::movement(Case const& c)
{
    Unite::movement(c);
}





// ==========================================
//                 Unite Gunship
// ==========================================
Unite_gunship::Unite_gunship(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc)
    : Unite(name, hp, dmg, mov, dir, loc),
    Unite_volante(name, hp, dmg, mov, dir, loc),
    Unite_lourde(name, hp, dmg, mov, dir, loc)
{}

void Unite_gunship::movement(Case const& c)
{
    Unite::movement(c);
}

void Unite_gunship::affiche() const
{
    std::cout << "=== GUNSHIP (Volant + Lourd) : " << _name << " ===" << std::endl;
    Unite::affiche();
}






// ==========================================
//                 Unite Amphibi
// ==========================================
Unite_amphibi::Unite_amphibi(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc)
    : Unite(name, hp, dmg, mov, dir, loc),
    Unite_legere(name, hp, dmg, mov, dir, loc),
    Unite_maritime(name, hp, dmg, mov, dir, loc)
{}

void Unite_amphibi::movement(Case const& c)
{
    Unite::movement(c);
}
void Unite_amphibi::affiche() const
{
    std::cout << "=== UNITE AMPHIBIE (Infanterie + Marine) : " << _name << " ===" << std::endl;
    Unite::affiche();
}





// ==========================================
//             Unite Cargo Volant
// ==========================================
Unite_cargo_volant::Unite_cargo_volant(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc)
    : Unite(name, hp, dmg, mov, dir, loc),
    Unite_volante(name, hp, dmg, mov, dir, loc),
    Unite_transport(name, hp, dmg, mov, dir, loc)
{}

void Unite_cargo_volant::movement(Case const& c)
{
    Unite::movement(c);
}
void Unite_cargo_volant::affiche() const
{
    std::cout << "=== CARGO AERIEN (Volant + Transport) : " << _name << " ===" << std::endl;
    Unite::affiche();
}






// ==========================================
//             Unite Embarcation
// ==========================================
Unite_embarcation::Unite_embarcation(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc)
    : Unite(name, hp, dmg, mov, dir, loc),
    Unite_maritime(name, hp, dmg, mov, dir, loc),
    Unite_transport(name, hp, dmg, mov, dir, loc)
{}

void Unite_embarcation::movement(Case const& c)
{
    Unite::movement(c);
}
void Unite_embarcation::affiche() const
{
    std::cout << "=== EMBARCATION ARMEE (Marine + Transport) : " << _name << " ===" << std::endl;
    Unite::affiche();
}
