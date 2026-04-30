#include "joueur.hh"
#include "combat.hh"

void Joueur::ajouterRessource(const Ressource* r, int n) {
    _inventaire[r] += n;
}

void Joueur::payer(const std::map<const Ressource*, int>& cout) {
    for (auto const& [res, qte] : cout) {
        _inventaire[res] -= qte;
    }
}

void Joueur::ajouterVille(City* c) {
    if (c) _cities.push_back(c);
}

void Joueur::ajouterBatiment(Batiment* b) {
    if (b) _batiments.push_back(b);
}

void Joueur::ajouterUnite(Unite* u) {
    if (u) _unites.push_back(u);
}

void Joueur::perdreVille(City* c) {
    _cities.remove(c);
}

void Joueur::perdreBatiment(Batiment* b) {
    _batiments.remove(b);
}

void Joueur::perdreUnite(Unite* u) {
    _unites.remove(u);
}


// Brouillard
void Joueur::resetVision() {
    for (auto& row : _visible) {
        std::fill(row.begin(), row.end(), false);
    }
}

// Modifier par Simon pour coller au nouvel FOV
void Joueur::decouvrirZoneVision(int startX, int startY, int rayon, int fov, int w, int h, direction dir, bool circulaire) {
    if (startX < 0 || startX >= w || startY < 0 || startY >= h) return;

    _decouvert[startX][startY] = true;
    _visible[startX][startY] = true;

    if (rayon <= 0) return;

    Coord origine = {startX, startY};
    
    if (circulaire) 
    {
    
        std::set<Coord> zone = case_adjascentes(origine, rayon);

        for (const auto& c : zone) 
        {
            if (c.first >= 0 && c.first < w && c.second >= 0 && c.second < h) 
            {
                _decouvert[c.first][c.second] = true;
                _visible[c.first][c.second] = true;
            }
        }
    } 
    else 
    {
        std::list<Coord> zone = ConeVision(origine, dir, rayon, fov);
        
        for (const auto& c : zone) 
        {
            if (c.first >= 0 && c.first < w && c.second >= 0 && c.second < h) {
                _decouvert[c.first][c.second] = true;
                _visible[c.first][c.second] = true;
            }
        }
    }
}

void Joueur::Attaquer(Unite& attaque, Unite& cible, CompAtt* const& TypeAttaque) 
{

    bool succes = Combat::fight(attaque, TypeAttaque, cible); 
    if(succes) 
    {
        attaque.setPoint_action(attaque.point_action() - 1);
    }
}

void Joueur::Soigner(Unite& healer, Unite& cible, CompSoin* const& TypeSoin) 
{
    bool succes = Combat::heal(healer, TypeSoin, cible);
    
    if(succes) 
    {
        healer.setPoint_action(healer.point_action() - 1);
    }
}

void Joueur::ActiverCamouflage(Unite& unite) 
{
    auto furtif = unite.Cammouflage();
    if(furtif) 
    {
        furtif->ActiveCammouflage();
        unite.setPoint_action(unite.point_action() - 1);
    }
}

void Joueur::Transporter(Unite& transporteur, Unite& passager) 
{
    auto transport = transporteur.Transport();
    if(transport) 
    {
        transport->MonterUnite(transporteur, passager.shared_from_this());
        transporteur.setPoint_action(transporteur.point_action() - 1);
    }
}

void Joueur::RejoindreCommandant(Unite& commandant, Unite& unite) 
{
    auto rankCom = std::dynamic_pointer_cast<Rank_Commandant>(commandant.rank());
    auto rankReg = std::dynamic_pointer_cast<Rank_Regulier>(unite.rank());

    if(rankCom && rankReg)
    {
        rankCom->ajout_unite(unite.shared_from_this()); 
        rankReg->setCommandant(commandant.shared_from_this());
        
        commandant.setPoint_action(commandant.point_action() - 1);
    }
}

void Joueur::QuitterCommandant(Unite& commandant, Unite& unite)
{
    auto rankCom = std::dynamic_pointer_cast<Rank_Commandant>(commandant.rank());
    auto rankReg = std::dynamic_pointer_cast<Rank_Regulier>(unite.rank());

    if(rankCom && rankReg)
    {
        rankCom->supprimer_unite(unite.shared_from_this()); 
        rankReg->setCommandant(nullptr);
        
        unite.setPoint_action(unite.point_action() - 1);
    }
}

void Joueur::DechargerTransport(Unite& transporteur, Unite& passager, int xDest, int yDest) 
{
    auto transport = transporteur.Transport();
    if (transport) 
    {
        transport->DescenteUniteUnite(transporteur, passager.shared_from_this());
        passager.setLocation({xDest, yDest});
        transporteur.setPoint_action(transporteur.point_action() - 1);
    }
}


void Joueur::ChangerPositionDefensive(Unite& unite)
{
    unite.changerDefense();
    unite.setPoint_action(0);
}