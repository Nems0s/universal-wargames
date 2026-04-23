#include "joueur.hh"
#include <set>
#include <cmath>

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


void Joueur::decouvrirZoneVision(int startX, int startY, int rayon, int fov, int w, int h, direction dir, bool circulaire) {    if (startX < 0 || startX >= w || startY < 0 || startY >= h) return;
    if (startX < 0 || startX >= w || startY < 0 || startY >= h) return;

    // case où on est
    _decouvert[startX][startY] = true;
    _visible[startX][startY] = true;
    if (rayon <= 0) return;

    if (circulaire) {
        // VISION VILLES (cercle)
        for (int i = std::max(0, startX - rayon); i <= std::min(w - 1, startX + rayon); ++i) {
            for (int j = std::max(0, startY - rayon); j <= std::min(h - 1, startY + rayon); ++j) {
                int q1 = startY - (startX - (startX & 1)) / 2;
                int r1 = startX;
                int q2 = j - (i - (i & 1)) / 2;
                int r2 = i;
                int dist = (std::abs(q1 - q2) + std::abs(q1 + r1 - q2 - r2) + std::abs(r1 - r2)) / 2;
                
                if (dist <= rayon) {
                    _decouvert[i][j] = true;
                    _visible[i][j] = true;
                }
            }
        }
    } else {
        // --- VISION UNITÉS (cone avec arg fov) ---
        float angleDirDeg = 0.0f;
        switch(dir) {
            case direction::est:        angleDirDeg = 0.0f; break;
            case direction::sud_est:    angleDirDeg = 60.0f; break;
            case direction::sud_ouest:  angleDirDeg = 120.0f; break;
            case direction::ouest:      angleDirDeg = 180.0f; break;
            case direction::nord_ouest: angleDirDeg = 240.0f; break;
            case direction::nord_est:   angleDirDeg = 300.0f; break;
        }
        float PI = 3.14159265f;
        float angleDirRad = angleDirDeg * PI / 180.0f;

        float R = 1.0f; 
        float W = std::sqrt(3.0f) * R;
        float ux = W * startY + W * 0.5f * (std::abs(startX) % 2);
        float uy = 1.5f * R * startX;

        for (int i = std::max(0, startX - rayon); i <= std::min(w - 1, startX + rayon); ++i) {
            for (int j = std::max(0, startY - rayon); j <= std::min(h - 1, startY + rayon); ++j) {
                float tx = W * j + W * 0.5f * (std::abs(i) % 2);
                float ty = 1.5f * R * i;
                float distPx = std::sqrt(std::pow(tx - ux, 2) + std::pow(ty - uy, 2));
                
                if (distPx <= (rayon * W * 1.15f)) {
                    if (i == startX && j == startY) continue;

                    float angleCible = std::atan2(ty - uy, tx - ux);
                    if (angleCible < 0) angleCible += 2 * PI;
                    float angleDirPos = angleDirRad;
                    if (angleDirPos < 0) angleDirPos += 2 * PI;

                    float diff = std::abs(angleCible - angleDirPos);
                    if (diff > PI) diff = 2 * PI - diff;

                    if (diff <= (fov / 2.0f) * PI / 180.0f) {
                        _decouvert[i][j] = true;
                        _visible[i][j] = true;
                    }
                }
            }
        }
    }
}