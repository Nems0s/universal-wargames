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

void Joueur::initBrouillard(int w, int h) {
    // Initialise une grille 2D remplie de "false" (non découvert)
    _brouillard.assign(w, std::vector<bool>(h, false));
}

void Joueur::decouvrirZone(int cx, int cy, int rayon, int w, int h) {
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            // Distance de Manhattan (typique des jeux sur grille)
            int distance = std::abs(x - cx) + std::abs(y - cy);
            if (distance <= rayon) {
                _brouillard[x][y] = true;
            }
        }
    }
}

bool Joueur::estDecouvert(int x, int y) const {
    if (x >= 0 && x < _brouillard.size() && y >= 0 && y < _brouillard[0].size()) {
        return _brouillard[x][y];
    }
    return false;
}