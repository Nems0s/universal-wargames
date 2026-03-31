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

bool Joueur::peutPayer(const std::map<Ressource*, int>& cout) const {
    for (auto const& [res, qte] : cout) {
        auto it = _inventaire.find(res);
        if (it == _inventaire.end() || it->second < qte) return false;
    }
    return true;
}

void Joueur::payer(const std::map<Ressource*, int>& cout) {
    for (auto const& [res, qte] : cout) {
        _inventaire[res] -= qte;
    }
}