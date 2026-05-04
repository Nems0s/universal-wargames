#include <algorithm>
#include "unite.hh"
#include "comportement.hh"
#include "rank.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::json;
Unite::Unite(const std::string &name, int hp, int point_action, int vision, int fov, Poids poids, direction dir, Coord loc,  std::shared_ptr<IRank> r, std::list<std::shared_ptr<IComportement>> liste_comportements, std::map<const Ressource*, int> cout, std::map<const Ressource*, int> cout_entretien, char symbole, const std::string& texturePath)
    :_name(name),
    _health_point(hp),
    _health_point_max(hp),
    _moral_point(0),
    _point_action(point_action),
    _point_action_max(point_action),
    _visionRange(vision),
    _fov(fov),
    _fov_ref(fov),
    _poids(poids),
    _regarde(dir),
    _location(loc),
    _rank(r),
    _liste_comportements(liste_comportements),
    _cout(cout),
    _cout_entretien(cout_entretien),
    _symbol(symbole),
    _texturePath(texturePath),
    _defensif(false)
{}



void Unite::actualiserVision() 
{
    _tuilesVisibles = ConeVision(_location, _regarde, _visionRange, _fov);
}

/*==============================================================================================*/
/*                                     Getters et Setters                                       */
/*==============================================================================================*/
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
    actualiserVision();
}

Coord Unite::location() const
{
    return _location;
}

void Unite::setLocation(const Coord &newLocation)
{
    _location = newLocation;
    actualiserVision();
}

std::shared_ptr<IRank> Unite::rank() const
{
    return _rank;
}

std::list<std::shared_ptr<IComportement> > Unite::liste_comportements() const
{
    return _liste_comportements;
}

std::map<const Ressource*, int> Unite::cout() const
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


std::map<const Ressource*, int> Unite::getInventaireInterne()const
{
    return _inventaireInterne;
}
void Unite::setInventaireInterne(std::map<const Ressource*, int> inventaire)
{
    _inventaireInterne = inventaire;
}

std::map<const Ressource*, int> Unite::getCapaciteMax()const
{
    return _capaciteMax;
}

const std::list<Coord>&  Unite::getTuilesVisibles() const 
{ 
    return _tuilesVisibles; 
}
void Unite::setTuilesVisibles(const std::list<Coord>& liste) 
{ 
    _tuilesVisibles = liste; 
}


int Unite::getVisionRange() const 
{ 
    return _visionRange; 
}
void Unite::setVisionRange(int r) 
{ 
    _visionRange = r; 
}

int Unite::getFov() const 
{ 
    return _fov; 
}
void Unite::setFov(int f) 
{ 
    _fov = f; 
}

/*==============================================================================================*/
/*==============================================================================================*/
/*==============================================================================================*/

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
        elt->update();
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
    std::cout << "Position: (" << _location.first << "," << _location.second << "), Direction: "<< directionToString(_regarde) << std::endl;
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
    if(_defensif == false)
    {
        _defensif = true;
        _fov = 6;
    }
    else 
    {
        _defensif = false;
        _fov = _fov_ref;
    }
    actualiserVision();
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

void Unite::consommerPourAction(const std::map<const Ressource*, int>& cout) 
{
    for (auto const& [res, qte] : cout) 
    {
        _inventaireInterne[res] -= qte;
    }
}

void Unite::ravitaillement(const Ressource* res, int qte)
{
    auto it = _inventaireInterne.find(res);
    auto it_max = _capaciteMax.find(res);

    if(it != _inventaireInterne.end() && it_max != _capaciteMax.end())
    {
        if(it->second + qte <= it_max->second)
        {
            it->second += qte; // L'itérateur se comporte comme un pointeur dans une map donc si on le modifie on modifie map 
        }
    }
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


template <typename T, typename... Args>
std::shared_ptr<IComportement> buildComportement(bool hasCD, int cd, bool hasCons, const std::map<const Ressource*, int>& cout, Args&&... args) 
{
    if (hasCD && hasCons) 
    {
        return std::make_shared<AvecCooldown<AvecConsommable<T>>>(cd, cout, std::forward<Args>(args)...);
    } 
    else if (hasCD) {
        return std::make_shared<AvecCooldown<T>>(cd, std::forward<Args>(args)...);
    } 
    else if (hasCons) {
        return std::make_shared<AvecConsommable<T>>(cout, std::forward<Args>(args)...);
    }
    return std::make_shared<T>(std::forward<Args>(args)...);
}

std::shared_ptr<IComportement> createComp(const json& jComp, bool hasCD, int cd, bool hasCons, const std::map<const Ressource*, int>& cout) 
{
    std::string type = jComp.value("type", "");

    //COMPORTEMENTS DE MOUVEMENT
    if (type == "MouvementVolant")
    {
        return buildComportement<CompMouvVolant>(hasCD, cd, hasCons, cout, jComp.value("mouvement_par_tour", 0));
    }
    if (type == "MouvementMarin")
    {
        return buildComportement<CompMouvMarin>(hasCD, cd, hasCons, cout, jComp.value("mouvement_par_tour", 0));
    }
    if (type == "MouvementTerrestre")
    {
        return buildComportement<CompMouvTerrestre>(hasCD, cd, hasCons, cout , jComp.value("mouvement_par_tour", 0));
    }

    //COMPORTEMENTS D'ATTAQUE
    if (type == "AttaqueMelee")
    {
        return buildComportement<CompAttMelee>(hasCD, cd, hasCons, cout, jComp.value("degats", 0));
    }
    if (type == "AttaqueDistance")
    {
        return buildComportement<CompAttDistance>(hasCD, cd, hasCons, cout, jComp.value("degats", 0),jComp.value("portee", 0),jComp.value("portee_mini", 0));
    }
    if (type == "AttaqueIndirect")
    {
        return buildComportement<CompAttIndirect>(hasCD, cd, hasCons, cout, jComp.value("degats", 0), jComp.value("portee", 0), jComp.value("tour_infection", 0));
    }

    //COMPORTEMENTS DE DEFENSE
    if (type == "DefenseArmure")
    {
        return buildComportement<CompDefArmure>(hasCD, cd, hasCons, cout, jComp.value("reduction", 0));
    }
    if (type == "DefenseBouclier")
    {
        return buildComportement<CompDefBouclier>(hasCD, cd, hasCons, cout, jComp.value("nombre_bouclier", 0));
    }

    //COMPORTEMENTS DE SOIN
    if (type == "SoinDirect")
    {
        return buildComportement<CompSoinDirect>(hasCD, cd, hasCons, cout, jComp.value("soin", 0), jComp.value("portee", 0), jComp.value("rayon", 0));
    }
    if (type == "SoinIndirect")
    {
        return buildComportement<CompSoinIndirect>(hasCD, cd, hasCons, cout, jComp.value("soin", 0), jComp.value("portee", 0), jComp.value("tour_regeneration", 0));
    }

    //COMPORTEMENTS SPÉCIAUX 
    if (type == "SpecialTransport")
    {
        return buildComportement<CompTransport>(hasCD, cd, hasCons, cout, jComp.value("capacite", 0));
    }
    if (type == "SpecialFurtif")
    {
        return buildComportement<CompFurtif>(hasCD, cd, hasCons, cout, jComp.value("duree", 0));
    }

    return nullptr;
}





void JsonUniteReader::load(const std::string& chemin, std::map<std::string, std::shared_ptr<Unite>>& catalogue, const std::map<std::string, const Ressource*>& ressources)
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

        std::map<const Ressource*, int> coutUnite;
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

        std::map<const Ressource*, int> coutEntretienUnite;
        if (item.contains("cout_entretien")) 
        {
            for (auto it = item["cout_entretien"].begin(); it != item["cout_entretien"].end(); ++it) 
            {
                if (ressources.count(it.key()))
                {
                    coutEntretienUnite[ressources.at(it.key())] = it.value();
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
            std::shared_ptr<IComportement> ajout = nullptr;
            for (auto& comp : item["comportements"]) 
            {
                bool hasCD = false; //A cooldown
                int cd;

                bool hasLC = false; //A liste de consommable
                std::map<const Ressource*, int> coutParActionUnite;

                if(comp.contains("cooldown"))
                {
                    hasCD = true;
                    cd = comp.value("cooldown", 0);
                }
                if(comp.contains("cout_par_action"))
                {
                    hasLC = true;
                    for (auto it = comp["cout_par_action"].begin(); it != comp["cout_par_action"].end(); ++it) 
                    {
                        if (ressources.count(it.key()))
                        {
                            coutParActionUnite[ressources.at(it.key())] = it.value();
                        }
                    }
                }

                ajout = createComp(comp, hasCD, cd, hasLC, coutParActionUnite);
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
            std::string tex = item.value("texture", "");
            int vision = item.value("vision", 2);
            int fov = item.value("fov", 3);
            catalogue[nom] = std::make_shared<Unite>(nom, hp, nb_action, vision, fov, poids, direction::est, Coord{0,0}, rank, listeComp, coutUnite, coutEntretienUnite, sym, tex);
        }   
    }
}


// ==========================================
//                 Factory
// ==========================================
void UniteFactory::chargerConfiguration(const std::string& chemin, UniteConfigReader& lecteur, const std::map<std::string, const Ressource*>& ressources)
{
    lecteur.load(chemin, _catalogue, ressources);
}

// MODIFIE PAR NAIM
std::shared_ptr<Unite> UniteFactory::create(std::string type) const
{
    if (_catalogue.count(type))
    {
        return _catalogue.at(type)->clone();
    }
    return nullptr;
}


