#include "arbitre.hh"

// ==========================================================
// LOGIQUE COMMUNE
// ==========================================================
bool Arbitre::peutPayer(const std::map<Ressource*, int>& cout, const Joueur& j) const {
    for (auto const& [res, qte] : cout) {
        auto it = j.getInventaire().find(res);
        if (it == j.getInventaire().end() || it->second < qte) return false;
    }
    return true;
}






// ==========================================================
// ZONE PLATEAU - Travail de [TON NOM]
// ==========================================================
// (Tu écris tout ton code ici)










// ==========================================================
// ZONE UNITÉS - Travail de [NOM COLLÈGUE]
// ==========================================================
// (Lui écrira tout son code ici, bien plus bas dans le fichier)






