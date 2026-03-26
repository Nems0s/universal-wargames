#pragma once
#include "unite_base.hh"

// ==========================================
//               Unite Cuirrasse
// ==========================================
class Unite_cuirrasse : public Unite_maritime, public Unite_lourde
{
public:
    Unite_cuirrasse(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location);
    void movement(Case const& c) override;
    void affiche() const override;
};


// ==========================================
//                Unite Gunship
// ==========================================
class Unite_gunship : public Unite_volante, public Unite_lourde
{
public:
    Unite_gunship(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc);
    void movement(Case const& c) override;
    void affiche() const override;
};


// ==========================================
//                 Unite Amphibi
// ==========================================
class Unite_amphibi : public Unite_legere, public Unite_maritime
{
public:
    Unite_amphibi(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc);
    void movement(Case const& c) override;
    void affiche() const override;
};


// ==========================================
//            Unite Cargo
// ==========================================
class Unite_cargo : public Unite_volante, public Unite_transport
{
public:
    Unite_cargo(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc);
    void movement(Case const& c) override;
    void affiche() const override;
};


// ==========================================
//            Unite Embarcation
// ==========================================
class Unite_embarcation : public Unite_maritime, public Unite_transport
{
public:
    Unite_embarcation(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc);
    void movement(Case const& c) override;
    void affiche() const override;
};
