#pragma once
#include <iostream>
#include <memory>
#include <list>
#include "orientation.hh"

enum class NatureMouv {TERRE, MER, AIR};

class Unite;

class IComportement
{
public:
    virtual ~IComportement() = default;
    virtual void affiche() const = 0;
    virtual void update(Unite& proprietaire) = 0;
};

//===================================================================
//                     Comportement Mouvement
//===================================================================
class CompMouv : public IComportement
{
protected:
    int _mov_per_laps;
public:
    CompMouv(int mouvement_par_tour);

    int mov_per_laps() const;
    void setMov_per_laps(int newMov_per_laps);

    virtual bool EstCaseValide(Coord const& actuel, Coord const& cible) = 0;
    virtual NatureMouv Nature()const=0;
};

class CompMouvVolant : public CompMouv
{
public:
    CompMouvVolant(int mouvement_par_tour = 2);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    NatureMouv Nature() const override;
};

class CompMouvMarin : public CompMouv
{
public:
    CompMouvMarin(int mouvement_par_tour = 1);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    NatureMouv Nature() const override;
};

class CompMouvTerrestre : public CompMouv
{
public:
    CompMouvTerrestre(int mouvement_par_tour = 1);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    NatureMouv Nature() const override;
};


//===================================================================
//                   Comportement Attaque
//===================================================================
class CompAtt : public IComportement
{
protected:
    int _damage_point;
    int _portee;
public:
    CompAtt(int damage_point, int portee);

    int damage_point() const;
    void setDamage_point(int newDamage_point);
    int portee() const;
    void setPortee(int newPortee);

    virtual bool PeuxAttaquer(Unite const& attaquante, Unite const& cible) const=0;
};

class CompAttMelee : public CompAtt
{
public:
    CompAttMelee(int damage_point);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool PeuxAttaquer(Unite const& attaquante, Unite const& cible) const override;
};

class CompAttDistance : public CompAtt
{
private:
    int _munitions;
    int _portee_mini;
public:
    CompAttDistance(int damage_point, int portee = 2, int munitions = 10, int portee_mini = 2);

    int munitions() const;
    void setMunitions(int newMunitions);
    int portee_mini() const;
    void setPortee_mini(int newPortee_mini);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool PeuxAttaquer(Unite const& attaquante, Unite const& cible) const override;
};

class CompAttIndirect : public CompAtt
{
private:
    struct infecter
    {
        std::weak_ptr<Unite> cible;
        int tour_infection;

        bool operator==(const infecter& other) const {
            bool memeCible = !cible.owner_before(other.cible) && !other.cible.owner_before(cible);
            return memeCible && (tour_infection == other.tour_infection);
        }
    };

    int _nombre_de_tour_infection;
    std::list<infecter> _liste_infecter;
public:
    CompAttIndirect(int damage_point, int portee = 1, int nombre_de_tour_infection = 2);

    void setNombreDeTourInfection(int newNombreDeTourInfection);
    int nombredetourinfection() const;

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool PeuxAttaquer(Unite const& attaquante, Unite const& cible) const override;
    void AjoutCibleAtteinte(std::shared_ptr<Unite> const& cible);
    void RetireCibleAtteinte(infecter const& I);
};


//===================================================================
//                   Comportement Defense
//===================================================================
class CompDef : public IComportement
{
public:
    CompDef() = default;
    virtual int ReductionDegats(int degat_subit) = 0;
};

class CompDefArmure : public CompDef
{
private:
    int _armure;
public:
    CompDefArmure(int armure);

    int armure() const;
    void setArmure(int newArmure);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    int ReductionDegats(int degat_subit) override;
};

class CompDefBouclier : public CompDef
{
private:
    int _nombre_bouclier;
public:
    CompDefBouclier(int nombre_bouclier);

    int nombre_bouclier() const;
    void setNombre_bouclier(int newNombre_bouclier);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    int ReductionDegats(int degat_subit) override;
};

//===================================================================
//                   Comportement Soin
//===================================================================
class CompSoin: public IComportement
{
protected:
    int _healing_point;
    int _portee;
public:
    CompSoin(int healing_point, int portee);

    int healing_point() const;
    void setHealing_point(int newHealing_point);
    int portee() const;
    void setPortee(int newPortee);

    virtual bool PeuxSoigner(Unite const& attaquante, Unite const& cible) const =0;
};

class CompSoinDirect: public CompSoin
{
private:
    int _rayon;
public:
    CompSoinDirect(int healing_point = 10, int portee = 2, int rayon = 2);

    int rayon() const;
    void setRayon(int newRayon);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool PeuxSoigner(Unite const& attaquante, Unite const& cible) const override;
};

class CompSoinIndirect: public CompSoin
{
private:
    struct soigner
    {
        std::weak_ptr<Unite> cible;
        int tour_soin;

        bool operator==(const soigner& other) const {
            bool memeCible = !cible.owner_before(other.cible) && !other.cible.owner_before(cible);
            return memeCible && (tour_soin == other.tour_soin);
        }
    };

    int _nombre_de_tour_regeneration;
    std::list<soigner> _liste_soigner;
public:
    CompSoinIndirect(int healing_point = 10, int portee = 2, int nombre_de_tour_regeneration = 2);

    void setNombreDeTourRegen(int newNombreDeTourRegen);
    int nombredetourregen() const;

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool PeuxSoigner(Unite const& attaquante, Unite const& cible) const override;
    void AjoutCibleAtteinte(std::shared_ptr<Unite> const& cible);
    void RetireCibleAtteinte(soigner const& I);
};

//===================================================================
//                   Comportement Spéciaux
//===================================================================
class CompTransport : public IComportement
{
private:
    std::list<std::shared_ptr<Unite>> _liste_unite_transporter;
    int _max_unite_transporter;
public:
    CompTransport(int max_unite_transporter = 3);

    std::list<std::shared_ptr<Unite> > liste_unite_transporter() const;
    void setListe_unite_transporter(const std::list<std::shared_ptr<Unite> > &newListe_unite_transporter);
    int max_unite_transporter() const;
    void setMax_unite_transporter(int newMax_unite_transporter);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    bool MonterUnite(Unite const& Transport, std::shared_ptr<Unite> const& Voyageur);
    bool DescenteUniteUnite(Unite const& Transport, std::shared_ptr<Unite> const& Voyageur);
};

class CompFurtif : public IComportement
{
private:
    bool _camoufler;
    int _nb_tour_cammouflage;
    int _tour_cooldown;

    int _nb_max_cammouflage;
    int _cooldown;
public:
    CompFurtif(int nb_tour_cammouflage = 3, int cooldown = 2);

    bool camoufler() const;
    void setCamoufler(bool newCamoufler);
    int nb_tour_cammouflage() const;
    void setNb_tour_cammouflage(int newNb_tour_cammouflage);
    int cooldown() const;
    void setCooldown(int newCooldown);

    void affiche() const override;
    void update(Unite& proprietaire) override;

    void ActiveCammouflage();
    void DesactiveCammouflage();
};

