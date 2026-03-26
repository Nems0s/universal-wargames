#pragma once
#include "unite_base.hh"

class Unite_cuirrasse : public Unite_maritime, public Unite_lourde
{
public:
    Unite_cuirrasse(const std::string &name, int health_point, int damage_point, int move_per_laps, direction regarde, Case location);
    void movement(Case const& c) override;
    void affiche() const override;
};


class Unite_gunship : public Unite_volante, public Unite_lourde
{
public:
    Unite_gunship(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc);
    void movement(Case const& c) override;
    void affiche() const override;
};

class Unite_amphibi : public Unite_legere, public Unite_maritime
{
public:
    Unite_amphibi(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc);
    void movement(Case const& c) override;
    void affiche() const override;
};

class Unite_cargo_volant : public Unite_volante, public Unite_transport
{
public:
    Unite_cargo_volant(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc);
    void movement(Case const& c) override;
    void affiche() const override;
};


class Unite_embarcation : public Unite_maritime, public Unite_transport
{
public:
    Unite_embarcation(const std::string &name, int hp, int dmg, int mov, direction dir, Case loc);
    void movement(Case const& c) override;
    void affiche() const override;
};
