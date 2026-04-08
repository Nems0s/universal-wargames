#include "joueur.hh"

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