#include <algorithm>
#include "unite.hh"
#include "comportement.hh"
#include "rank.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

Unite::Unite(const std::string &name, int hp, int point_action, Poids poids, direction dir, Coord loc,  std::shared_ptr<IRank> r, std::list<std::shared_ptr<IComportement>> liste_comportements, std::map<Ressource*, int> cout, char symbole)
    :_name(name),
    _health_point(hp),
    _health_point_max(hp),
    _moral_point(0),
    _point_action(point_action),
    _point_action_max(point_action),
    _poids(poids),
    _regarde(dir),
    _location(loc),
    _rank(r),
    _liste_comportements(liste_comportements),
    _cout(cout),
    _defensif(false),
    _symbol(symbole)
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

int Unite::point_action() const
{
    return _point_action;
}
void Unite::setPoint_action(int newPoint_action)
{
    _point_action = newPoint_action;
}

int Unite::point_action_max() const
{
    return _point_action_max;
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

std::map<Ressource*, int> Unite::cout() const
{
    return _cout;
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


void Unite::setSymbol(char s)
{ 
    _symbol = s; 
}

char Unite::getSymbol() const
{
    return _symbol;
}


bool Unite::defensif() const
{
    return _defensif;
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
        auto evolutif = dynamic_cast<IComportementEvolutif*>(elt.get()); 
        if(evolutif)
        {
            evolutif->update();
        }
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

CompTransport* Unite::Transport() const
{
    for (const auto& comp_ptr : _liste_comportements)
    {
        if (auto* transport = dynamic_cast<CompTransport*>(comp_ptr.get()))
        {
            return transport;
        }
    }
    return nullptr;
}

void Unite::affiche() const
{
    std::cout << "=== [" << _name << "] ===" << std::endl;
    std::cout << "Position: (" << _location.first << "," << _location.second << ")" << std::endl;
    std::cout << "Point d'action restant: " << _point_action << "/" << _point_action_max << std::endl;
    if(_rank)
    {
        std::cout << "Grade: "; _rank->get_role(); std::cout << std::endl;
    }

    for(auto const& comp : _liste_comportements)
    {
        comp->affiche();
    }
}

void Unite::changerDefense()
{
    if(_defensif == false) _defensif = true;
    else _defensif = false;
}

void Unite::resetTemporary_stats()
{
    _temporary_damage = 0;
    _temporary_health = 0;
}

std::shared_ptr<Unite> Unite::clone() const
{
    return std::make_shared<Unite>(*this);
}




// ==========================================
//                  Config
// ==========================================
Poids stringToPoids(const std::string& s) 
{
    if (s == "Leger") return Poids::Leger;
    if (s == "Lourd") return Poids::Lourd;
    return Poids::Moyen;
}

std::shared_ptr<IRank> stringToRank(const std::string& s, int max_unite=0) {
    if (s == "Commandant") return std::make_shared<Rank_Commandant>(max_unite);
    else return std::make_shared<Rank_Regulier>();
}

std::shared_ptr<IBonus> createBuff(const json& jBonus) 
{
    // Le deuxième champ de .value de la bibliothèque de nlohmann sert en valeur de défault si le premier champs ne renvoie rien
    std::string type = jBonus.value("type", "");
    if(type == "BonusDegat") return std::make_shared<BonusDegat>(jBonus.value("multiplicateur", 1));
    if(type == "BonusVie") return std::make_shared<BonusVie>(jBonus.value("soin", 0));
    if(type == "BonusDefense") return std::make_shared<BonusDefense>(jBonus.value("bouclier", 0));
    else return nullptr;
}

std::shared_ptr<IComportement> createComp(const json& jComp) 
{
    std::string type = jComp.value("type", "");

    //COMPORTEMENTS DE MOUVEMENT
    if (type == "MouvementVolant")
    {
        return std::make_shared<CompMouvVolant>(jComp.value("mouvement_par_tour", 3));
    }
    if (type == "MouvementMarin")
    {
        return std::make_shared<CompMouvMarin>(jComp.value("mouvement_par_tour", 2));
    }
    if (type == "MouvementTerrestre")
    {
        return std::make_shared<CompMouvTerrestre>(jComp.value("mouvement_par_tour", 2));
    }

    //COMPORTEMENTS D'ATTAQUE
    if (type == "AttaqueMelee")
    {
        return std::make_shared<CompAttMelee>(jComp.value("degats", 10));
    }
    if (type == "AttaqueDistance")
    {
        return std::make_shared<CompAttDistance>(jComp.value("degats", 10),jComp.value("portee", 3),jComp.value("munitions", 5),jComp.value("portee_mini", 2));
    }
    if (type == "AttaqueIndirect")
    {
        return std::make_shared<CompAttIndirect>(jComp.value("degats", 5), jComp.value("portee", 2), jComp.value("tour_infection", 3));
    }

    //COMPORTEMENTS DE DEFENSE
    if (type == "DefenseArmure")
    {
        return std::make_shared<CompDefArmure>(jComp.value("reduction", 5));
    }
    if (type == "DefenseBouclier")
    {
        return std::make_shared<CompDefBouclier>(jComp.value("nombre_bouclier", 3));
    }

    //COMPORTEMENTS DE SOIN
    if (type == "SoinDirect")
    {
        return std::make_shared<CompSoinDirect>(jComp.value("soin", 20), jComp.value("portee", 2), jComp.value("rayon", 2));
    }
    if (type == "SoinIndirect")
    {
        return std::make_shared<CompSoinIndirect>(jComp.value("soin", 15), jComp.value("portee", 4), jComp.value("tour_regeneration", 2));
    }

    //COMPORTEMENTS SPÉCIAUX 
    if (type == "SpecialTransport")
    {
        return std::make_shared<CompTransport>(jComp.value("capacite", 2));
    }
    if (type == "SpecialFurtif")
    {
        return std::make_shared<CompFurtif>(jComp.value("duree", 2), jComp.value("cooldown", 3));
    }

    return nullptr;
}

void JsonUniteReader::load(const std::string& chemin, std::map<std::string, std::shared_ptr<Unite>>& catalogue, const std::map<std::string, Ressource*>& ressources)
{
    std::ifstream fichier(chemin);
    if (!fichier.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier : " << chemin << std::endl;
        return; 
    }

    json data;
    fichier >> data;

    for (auto& item : data["unites"]) 
    {
        std::string nom = item.value("nom", "Erreur");
        int hp = item.value("hp", 0);
        int nb_action = item.value("action", 0); //

        Poids poids = Poids::Moyen;
        if(item.contains("poids")) poids = stringToPoids(item["poids"]);

        std::string symStr = item.value("symbole", " ");
        char sym = symStr[0];

        std::map<Ressource*, int> coutUnite;
        bool toutesRessourcesExistantes = true;

        if (item.contains("cout")) 
        {
            for (auto it = item["cout"].begin(); it != item["cout"].end(); ++it) 
            {
                std::string nomRes = it.key();
                int quantite = it.value();
                if (ressources.count(nomRes)) 
                {
                    coutUnite[ressources.at(nomRes)] = quantite;
                } 
                else 
                {
                    std::cout << "ERREUR : Ressource '" << nomRes << "' absente. Ressources connues : ";
                    for(auto const& [cle, val] : ressources) 
                    {
                        std::cout << cle << " ";
                    }
                    std::cout << std::endl;
                    toutesRessourcesExistantes = false;
                }
            }
        }

        std::shared_ptr<IRank> rank = stringToRank("Regulier");
        if(item.contains("rank"))
        {
            if(item["rank"] == "Commandant" && item.contains("max_unites"))
            {
                rank = stringToRank(item["rank"], item["max_unites"]);
                if (item.contains("buff_commandant"))
                {
                    for (auto& bonus : item["buff_commandant"])
                    {
                        if (auto commandant = dynamic_cast<Rank_Commandant*>(rank.get()))
                        {
                            commandant->ajout_bonus(createBuff(bonus));
                        }
                    }
                }
            }
            else rank = stringToRank(item["rank"]);
        }

        std::list<std::shared_ptr<IComportement>> listeComp;
        if (item.contains("comportements")) 
        {
            for (auto& comp : item["comportements"]) 
            {
                auto ajout = createComp(comp);
                if(ajout != nullptr) listeComp.push_back(ajout);
            }
        }

        if((nom == "Erreur") || (hp == 0)) 
        {
            std::cout << "Erreur dans la génération de l'unité !" << std::endl;
        }
        else if (!toutesRessourcesExistantes)
        {
            std::cout << "L'unite '" << nom << "' n'a pas ete creee car ses ressources sont invalides." << std::endl;
        }
        else
        {
            catalogue[nom] = std::make_shared<Unite>(nom, hp, nb_action, poids, direction::est, Coord{0,0}, rank, listeComp, coutUnite, sym);
        }
    }   
}


// ==========================================
//                 Factory
// ==========================================
void UniteFactory::chargerConfiguration(const std::string& chemin, UniteConfigReader& lecteur, const std::map<std::string, Ressource*>& ressources)
{
    lecteur.load(chemin, _catalogue, ressources);
}

std::shared_ptr<Unite> UniteFactory::create(std::string type) 
{
    if (_catalogue.count(type))
    {
        return _catalogue[type]->clone();
    }
    return nullptr;
}



