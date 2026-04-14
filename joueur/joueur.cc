#include "joueur.hh"
#include "combat.hh"

void Joueur::debutTour()
{
    for(auto uni : _unites)
    {
        uni->setPoint_action(uni->point_action_max());
    }

    //Ajouter la recupération de ressources
}

void Joueur::ajouterRessource(Ressource* r, int n) {
    _inventaire[r] += n;
}

bool Joueur::consommerRessource(Ressource* r, int n) {
    if (_inventaire[r] >= n) {
        _inventaire[r] -= n;
        return true;
    } else {
        return false;
    }
}

void Joueur::payer(const std::map<Ressource*, int>& cout) {
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

void Joueur::Attaquer(Unite& attaque, Unite& cible, CompAtt* const& TypeAttaque) 
{

    bool succes = Combat::fight(attaque, TypeAttaque, cible); 
    if (succes) 
    {
        attaque.setPoint_action(attaque.point_action() - 1);
    }
}

void Joueur::Soigner(Unite& healer, Unite& cible, CompSoin* const& TypeSoin) 
{
    bool succes = Combat::heal(healer, TypeSoin, cible);
    
    if (succes) 
    {
        healer.setPoint_action(healer.point_action() - 1);
    }
}

void Joueur::ActiverCamouflage(Unite& unite) 
{
    auto furtif = unite.Cammouflage();
    if (furtif) 
    {
        furtif->ActiveCammouflage();
        unite.setPoint_action(unite.point_action() - 1);
    }
}

void Joueur::Transporter(Unite& transporteur, Unite& passager) 
{
    auto transport = transporteur.Transport();
    if (transport) 
    {
        transport->MonterUnite(transporteur, passager.shared_from_this());
        transporteur.setPoint_action(transporteur.point_action() - 1);
    }
}

void Joueur::RejoindreCommandant(Unite& commandant, Unite& unite) 
{
    auto rankCom = std::dynamic_pointer_cast<Rank_Commandant>(commandant.rank());
    auto rankReg = std::dynamic_pointer_cast<Rank_Regulier>(unite.rank());

    if (rankCom && rankReg)
    {
        rankCom->ajout_unite(unite.shared_from_this()); 
        rankReg->setCommandant(commandant.shared_from_this());
        
        commandant.setPoint_action(commandant.point_action() - 1);
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