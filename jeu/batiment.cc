#include "batiment.hh"
#include "../joueur/joueur.hh"

void BatimentRessource::action(Joueur & j) {
    if (_produit != nullptr) {
        j.ajouterRessource(_produit, _quantite);
    }
}

void TxtBatimentReader::load(const std::string& chemin, 
                std::map<std::string, std::unique_ptr<Batiment>>& catalogue,
                const std::map<std::string, Ressource*>& ressourcesDispo) {
            
    std::ifstream fichier(chemin);
    std::string ligne;

    // Format avec ressource : ExtracteurEtoile EX Or 10 poussiereDEtoile 10
    // Format sans ressource : Muraille M Or 15
    while (std::getline(fichier, ligne)) {
        if (ligne.empty()) continue;

        std::stringstream ss(ligne);
        std::string nomBat, symbBat, nomResProd, nomResCout;
        int qProd, qCout;

        if (!(ss >> nomBat >> symbBat >> nomResCout >> qCout)) continue;

        Ressource* resCout = ressourcesDispo.at(nomResCout);
        std::map<Ressource*, int> coutMap;
        coutMap[resCout] = qCout;

        if (ss >> nomResProd >> qProd) {
            Ressource* resProd = ressourcesDispo.at(nomResProd);
            Ressource* sol = resProd;
            catalogue[nomBat] = std::make_unique<BatimentRessource>(
                nomBat, coutMap, resProd, qProd, sol
            );
        } else {
            catalogue[nomBat] = std::make_unique<BatimentRessource>(
                nomBat, coutMap, nullptr, 0, nullptr
            );
        }
        
    }
}