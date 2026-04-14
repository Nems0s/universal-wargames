#include "joueur.hh"

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