#include "arbitre.hh"
#include <queue>

// ==========================================================
// LOGIQUE COMMUNE
// ==========================================================
bool Arbitre::peutPayer(const std::map<const Ressource*, int>& cout, const Joueur& j) const {
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
    if (config.getVictorySets().empty()) return false;
    
    int activeIndex = config.getActiveVictorySet();
    if (activeIndex < 0 || activeIndex >= config.getVictorySets().size()) return false;
    
    const VictorySet& vSet = config.getVictorySets()[activeIndex];
    
    int conditionsMet = 0;
    for (const auto& cond : vSet.conditions) {
        bool met = false;
        
        if (cond.type == WinType::RESOURCE) {
            int qte = 0;
            for (auto const& [res, val] : j.getInventaire()) {
                if (res->getName() == cond.resourceName) qte = val;
            }
            if (qte >= cond.targetAmount) met = true;
        } 
        else if (cond.type == WinType::CITY_COUNT) {
            if (j.getNbVilles() >= cond.targetAmount) met = true;
        }
        else if (cond.type == WinType::UNIT_COUNT) {
            if ((int)j.getUnites().size() >= cond.targetAmount) met = true;
        }
        else if (cond.type == WinType::CAPITAL_REQ) {
            for (City* v : j.getCities()) {
                if (v && v->estCapitale()) met = true;
            }
        }
        else if (cond.type == WinType::CAPITAL_CONQUEST) {
            int capCount = 0;
            for (City* c : j.getCities()) {
                if (c && c->estCapitale()) capCount++;
            }
            if (capCount >= 2) met = true; 
        }

        if (met) conditionsMet++;
    }
    
    if (vSet.mode == WinMode::ALL && conditionsMet == vSet.conditions.size()) return true;
    if (vSet.mode == WinMode::ANY && conditionsMet > 0) return true;
    
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
    
    // cout de base * niveau actuel
    std::map<const Ressource*, int> coutAmelioration;
    for (const auto& [nomRes, qte] : config.getCoutBaseVille()) {
        for (const auto& [resPtr, invQte] : j.getInventaire()) {
            if (resPtr->getName() == nomRes) {
                coutAmelioration[resPtr] = qte * city.getLevel(); 
            }
        }
    }
    
    return peutPayer(coutAmelioration, j);
}

bool Arbitre::peutAmeliorerBatiment(const Joueur & j, const Batiment & b) const {
    if (b.getLevel() >= b.getMaxLevel()) return false;
    else return peutPayer(b.getResourceConstr(), j);
}

bool Arbitre::estDansTerritoire(const Joueur & j, int x, int y, const board & game, const GameConfig & config) const {
    if (!coordValid(x, y, game)) return false;

    const hexa* cell = game.getCell(x, y);
    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(cell);
    if (tc && tc->getProprietaire() == &j) return true;

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

    // Si déjà dans le territoire, rien à acheter
    if (estDansTerritoire(j, x, y, game, config)) return false;

    // Vérifier si un voisin appartient au territoire du joueur
    bool adjacentATerritoire = false;
    int dx[] = {-1, 1, 0, 0, -1, 1}; // Simplifié pour l'instant (hexagones à gérer proprement)
    int dy[] = {0, 0, -1, 1, (x%2==0?-1:1), (x%2==0?-1:1)};

    for (int i = 0; i < 6; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (coordValid(nx, ny, game) && estDansTerritoire(j, nx, ny, game, config)) {
            adjacentATerritoire = true;
            break;
        }
    }

    if (!adjacentATerritoire) return false;

    return peutPayer(getCostAchatCase(j, config), j);
}

std::map<const Ressource*, int> Arbitre::getCostAchatCase(const Joueur & j, const GameConfig & config) const {
    auto coutBase = config.getCoutBaseVille();
    std::map<const Ressource*, int> coutActuel;
    
    // Application du multiplicateur (progressif selon le nombre de cases déjà achetées)
    float mult = std::pow(config.getMultiplicateurVille(), (float)j.getNbCasesAchetees() / 5.0f);

    for (auto const& [resPtr, qte] : j.getInventaire()) {
        if (coutBase.count(resPtr->getName())) {
            coutActuel[resPtr] = (int)(coutBase.at(resPtr->getName()) * mult);
        }
    }
    return coutActuel;
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
    if (!b) return false;
    if (!coordValid(x,y,game)) return false;
    
    TuileConfigurable* tuile = const_cast<TuileConfigurable*>(dynamic_cast<const TuileConfigurable*>(game.getCell(x, y)));
    if (!tuile) return false;

    const auto & cout = b->getResourceConstr();
    const auto & requisSol = b->getRessourcesSolRequired();

    if (!peutPayer(cout, j)) return false;

    if (!requisSol.empty()) {
        // Bâtiment spécial (ressource au sol)
        if (buildSpecialBuilding(j, game, *b, x, y)) {
            j.payer(cout);
            j.ajouterBatiment(b.get());
            tuile->constrBatimentSpeciale(std::move(b));
            tuile->setProprietaire(&j);
            return true;
        }
    } else {
        // Bâtiment normal (construit dans une ville)
        City* ville = tuile->getCity();
        if (!ville) return false;
        if (!buildBuildingInCity(j, *ville, *b)) return false;
        j.payer(cout);
        ville->creeBatiment(std::move(b));
        return true;
    }

    return false;
}


// ==========================================================
// ZONE UNITÉS
// ==========================================================

bool Arbitre::appartientJoueur(const Joueur& j, const Unite& unite) const {
    return unite.getProprietaire() == &j;
}

bool Arbitre::peutRecruterUnite(const Joueur& j, const std::map<const Ressource*, int>& cout, const Unite& invocation) const 
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

std::vector<std::pair<int, int>> Arbitre::getCasesDeplacementPossibles(const board& game, const Unite& u) const {
    if (u.point_action() <= 0) return {};
    
    std::vector<std::pair<int, int>> casesPossibles;
    std::map<std::pair<int, int>, int> cout_cumule; 
    std::queue<std::pair<int, int>> a_visiter;

    Coord depart = u.location();
    a_visiter.push(depart);
    cout_cumule[depart] = 0;

    while(!a_visiter.empty()) {
        Coord curr = a_visiter.front();
        a_visiter.pop();

        for (auto voisin : Voisins(curr)) {
            if (!coordValid(voisin.first, voisin.second, game)) continue;
            
            const hexa* cell = game.getCell(voisin.first, voisin.second);
            int cost = cell->getCoutDeplacement();
            
            if (cost < 0 || !cell->estFranchissable(u)) continue;
            if (game.getUnite(voisin.first, voisin.second) != nullptr) continue;

            int next_cost = cout_cumule[curr] + cost;
            
            // Si on a assez de PA pour y aller
            if (next_cost <= u.point_action()) {
                // Si on n'y est pas encore allé, ou qu'on a trouvé un chemin plus court
                if (cout_cumule.find(voisin) == cout_cumule.end() || next_cost < cout_cumule[voisin]) {
                    cout_cumule[voisin] = next_cost;
                    a_visiter.push(voisin);
                    
                    // On l'ajoute à la liste finale (sans doublons)
                    if (std::find(casesPossibles.begin(), casesPossibles.end(), voisin) == casesPossibles.end()) {
                        casesPossibles.push_back(voisin);
                    }
                }
            }
        }
    }
    return casesPossibles;
}

std::vector<std::pair<int, int>> Arbitre::getCasesAttaquePossibles(const Joueur& j, const board& game, const Unite& u) const {
    std::vector<std::pair<int, int>> ciblesPossibles;
    if (u.point_action() <= 0) return ciblesPossibles;

    for (int i = 0; i < game.getRows(); ++i) {
        for (int y = 0; y < game.getCols(); ++y) {
            Unite* cible = game.getUnite(i, y);
            
            if (cible && !appartientJoueur(j, *cible)) {
                for (CompAtt* attComp : u.Offensive()) {
                    if (peutAttaquer(j, u, *cible, attComp)) {
                        ciblesPossibles.push_back({i, y});
                        break;
                    }
                }
            }
        }
    }
    return ciblesPossibles;
}

std::map<const Ressource*, int> Arbitre::getCostNouvelleVille(const Joueur & j, const std::map<const Ressource*, int>& coutBase) const {
    std::map<const Ressource*, int> coutActuel;
    
    if (j.getNbVilles() == 0) return coutBase; 
    
    int multiplicateur = j.getNbVilles(); 

    for (const auto& [resPtr, qteBase] : coutBase) {
        coutActuel[resPtr] = qteBase * multiplicateur;
    }
    
    return coutActuel;
}