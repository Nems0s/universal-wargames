#include "arbitre.hh"

// ==========================================================
// LOGIQUE COMMUNE
// ==========================================================
bool Arbitre::peutPayer(const std::map<Ressource*, int>& cout, const Joueur& j) const {
    for (auto const& [res, qte] : cout) 
    {
        auto it = j.getInventaire().find(res);
        if (it == j.getInventaire().end() || it->second < qte) return false;
    }
    return true;
}

bool Arbitre::coordValid(int x, int y, const board & game) const {
    // Sécurité mémoire
    if (x < 0 || x >= game.getRows() || y < 0 || y >= game.getCols()) return false;
    
    // Sécurité logique
    if (game.getCell(x, y)->getSymbole() == '#') return false;

    return true;
}

bool Arbitre::checkWin(const Joueur& j, const WinConditions & win) const {
    switch (win.type) {
        case WinType::RESOURCE: {
            int qte = 0;
            for (auto const& [res, val] : j.getInventaire()) {
                if (res->getName() == win.resourceName) qte = val;
            }
            return qte >= win.targetAmount;
        }

        case WinType::CITY_COUNT:
            return j.getNbVilles() >= win.targetAmount;

        case WinType::CAPITAL_REQ:
            for (City* v : j.getCities()) {
                if (v->estCapitale()) return true;
            }
            return false;

        case WinType::UNIT_COUNT:
            return (int)j.getUnites().size() >= win.targetAmount;

        default: return false;
    }
}

bool Arbitre::verifierVictoire(const Joueur& j, const GameConfig & config) const {
    const auto & wins = config.getVictorySets();

    for (const auto & win : wins) {
        bool winValide;

        if (win.mode == WinMode::ALL) {
            winValide = true;
            for (const auto & cond : win.conditions) {
                if (!checkWin(j,cond)) {
                    winValide = false;
                    break;
                }
            }
        } else {
            winValide = false;
            for (const auto & cond : win.conditions) {
                if (checkWin(j,cond)) {
                    winValide = true;
                    break;
                }
            }
        }

        if (winValide) {
            std::cout << "Victoire par " << win.name << std::endl;
            return true;
        }
    }

    return false;
}


// ==========================================================
// ZONE PLATEAU
// ==========================================================

bool Arbitre::buildCity(const Joueur & j, const board & game, int x, int y) const {
    // vérification tuile existante et non un bord
    if (!coordValid(x,y,game)) return false;
    
    // récupération des infos de la tuile
    const hexa* cell = game.getCell(x,y);
    const TuileConfigurable* tuile = dynamic_cast<const TuileConfigurable*>(cell);

    // prérequis de la tuile
    if (!tuile->peutConstrVille()) return false;

    return true;
}

bool Arbitre::buildBuildingInCity(const Joueur & j, const City & city, const Batiment & b) const {
    if (!b.getRessourcesSolRequired().empty()) return false;
    if (!city.peutAjouterBatiment()) return false;
    if (!peutPayer(b.getResourceConstr(), j)) return false;

    return true;
}

bool Arbitre::buildSpecialBuilding(const Joueur & j, const board & game, const Batiment & b, int x, int y) const {
    if (!coordValid(x,y,game)) return false;
    
    const hexa* cell = game.getCell(x,y);
    const TuileConfigurable* tuile = dynamic_cast<const TuileConfigurable*>(cell);

    if (!tuile->peutConstrBatimentSpecial(b)) return false;
    if (!peutPayer(b.getResourceConstr(), j)) return false;
    if (tuile->getCity() != nullptr) return false;

    return true;
}

bool Arbitre::moveUnite(const Joueur & j, const board & game, const Unite & u, int xDest, int yDest) const {
    if (!coordValid(xDest,yDest,game)) return false;
    if (game.getUnite(xDest, yDest) != nullptr) return false;

    const hexa* cell = game.getCell(xDest,yDest);
    const TuileConfigurable* tuile = dynamic_cast<const TuileConfigurable*>(cell);

    if (!tuile->estFranchissable(u)) return false;

    for(auto mov : u.Mobilite())
    {
        if(!mov->EstCaseValide(u.location(),Coord(xDest,yDest)))
        {
            return false;
        }
    }

    return true;
}

bool Arbitre::validPayRessource(Joueur & j, const Batiment & b) {
    if (peutPayer(b.getResourceConstr(), j)) {
        j.payer(b.getResourceConstr());
        return true;
    } else {
        return false;
    }
}

bool Arbitre::peutAmeliorerVille(const Joueur & j, const City & city, const GameConfig & config) const {
    if (city.getLevel() >= config.getMaxLevelVille()) return false;
    else return true;
}

bool Arbitre::peutAmeliorerBatiment(const Joueur & j, const Batiment & b) const {
    if (b.getLevel() >= b.getMaxLevel()) return false;
    else return peutPayer(b.getResourceConstr(), j);
}

bool Arbitre::estDansTerritoire(const Joueur & j, int x, int y, const board & game, const GameConfig & config) const {
    if (!coordValid(x, y, game)) return false;

    for (City* ville : j.getCities()) {
        int vx = ville->getX(); 
        int vy = ville->getY();
        int rayon = config.getRayonBaseVille() + ville->getLevel();

        int distance = std::abs(x - vx) + std::abs(y - vy);
        
        if (distance <= rayon) return true;
    }
    return false;
}

bool Arbitre::peutAcheterCase(const Joueur & j, int x, int y, const board & game, const GameConfig & config) const {
    if (!coordValid(x, y, game)) return false;

    if (estDansTerritoire(j,x,y,game,config)) return false;

    return true;
}

bool Arbitre::estCaseHabitable(const TuileConfigurable & t, const Joueur & j) const {
    const FactionParams* f = j.getFaction();
    if (!f) return true;

    for (auto const& [statName, limite] : f->params) {
        float valeurTuile = t.getStat(statName);
        if (valeurTuile > limite) return false;
    }
    return true;
}

bool Arbitre::peutDetruireBatiment(const Joueur & j, const Batiment & b) const {
    for (Batiment* bat : j.getBatiments()) {
        if (bat == &b) return true;
    }
    return false;
}


bool Arbitre::tenterConstruction(int x, int y, std::unique_ptr<Batiment> b, Joueur & j, board & game) {
    if (!coordValid(x,y,game)) return false;
    
    TuileConfigurable* tuile = const_cast<TuileConfigurable*>(dynamic_cast<const TuileConfigurable*>(game.getCell(x, y)));
    if (!tuile) return false;

    const auto & cout = b->getResourceConstr();
    const auto & requisSol = b->getRessourcesSolRequired();

    if (!peutPayer(cout, j)) return false;

    if (!requisSol.empty()) {
        if (buildSpecialBuilding(j,game,*b,x,y)) {
            j.payer(cout);
            tuile->constrBatimentSpeciale(std::move(b));
            return true;
        }
    } else {
        City* ville = tuile->getCity();
        if (buildCity(j,game,x,y)) {
            j.payer(cout);
            ville->creeBatiment(std::move(b));
            return true;
        }
    }

    return false;
}


// ==========================================================
// ZONE UNITÉS
// ==========================================================

bool Arbitre::appartientJoueur(const Joueur& j, const Unite& unite)const
{
    auto unites_joueur = j.getUnites();
    if(std::find(unites_joueur.begin(), unites_joueur.end(), &unite) != unites_joueur.end())
    {
        return true;
    }
    else return false;
}

bool Arbitre::peutRecruterUnite(const Joueur& j, const std::map<Ressource*, int>& cout, const Unite& invocation) const 
{
    if (!peutPayer(cout, j)) {
        return false;
    }
    if (invocation.health_point() <= 0) {
        return false;
    }
    return true;
}

bool Arbitre::peutAttaquer(const Joueur& j, const Unite& attaque, const Unite& cible, CompAtt* const& TypeAttaque)const
{
    if(attaque.point_action() <= 0)
    {
        return false;
    }
    if(appartientJoueur(j,attaque)==false || appartientJoueur(j,cible)==true)
    {
        return false;
    }

    auto styles_attaque = attaque.Offensive();
    auto it = std::find(styles_attaque.begin(), styles_attaque.end(), TypeAttaque);

    if (it != styles_attaque.end() || (*it)->PeuxAttaquer(attaque, cible))
    {
        return true;
    }
    else return false;

}

bool Arbitre::peutSoigner(const Joueur& j, const Unite& healer, const Unite& cible, CompSoin* const& TypeSoin)const
{
    if(healer.point_action() <= 0)
    {
        return false;
    }
    if(appartientJoueur(j,healer)==false || appartientJoueur(j,cible)==true)
    {
        return false;
    }
    auto styles_healer = healer.Soin();
    auto it = std::find(styles_healer.begin(), styles_healer.end(), TypeSoin);

    if (it != styles_healer.end() || (*it)->PeuxSoigner(healer, cible))
    {
        if((*it)->estPret())
        {
            return true;
        }
        else return false;
        return true;
    }
    else return false;
}

bool Arbitre::peutActiverCamouflage(const Joueur& j, const Unite& unite)const
{
    if(unite.point_action() <= 0)
    {
        return false;
    }
    if(appartientJoueur(j,unite)==false)
    {
        return false;
    }
    auto cammouflage = unite.Cammouflage();
    if(cammouflage)
    {
        if(cammouflage->estPret() && cammouflage->camoufler() == false)
        {
            return true;
        }
        else return false;
    }
    else return false;
}

bool Arbitre::peutTransporter(const Joueur& j, const Unite& unite)const
{
    if(unite.point_action() <= 0)
    {
        return false;
    }
    if(appartientJoueur(j,unite)==false)
    {
        return false;
    }
    auto transport = unite.Transport();
    if(transport)
    {
        if(transport->nb_unite_actuelle() < transport->max_unite_transporter())
        {
            return true;
        }
        else return false;
    }
    else return false;
}

bool Arbitre::peutRejoindreCommandant(const Joueur& j, const Unite& commandant, const Unite& unite)const
{
    if(commandant.point_action() <= 0)
    {
        return false;
    }
    if(appartientJoueur(j,commandant)==false || appartientJoueur(j,unite)==false)
    {
        return false;
    }

    auto r = commandant.rank();
    auto com = std::dynamic_pointer_cast<Rank_Commandant>(r);
    if (com) 
    {
        if(com->liste_unites().size() < com->get_max_unite())
        {
            return true;
        }
        else return false;
    }
    else return false;
}
