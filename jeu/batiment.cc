#include "batiment.hh"
#include "../joueur/joueur.hh"

//===================================================================
//                          Batiment
//===================================================================
Batiment::Batiment(std::string n, std::map<Ressource*, int> c, int level) :
    _name(n),
    _cout(c),
    _level(level)
{}

std::string Batiment::getName() const
{
    return _name;
}
const std::map<Ressource*, int> & Batiment::getResourceConstr() const
{
    return _cout;
}

Ressource* Batiment::getRessourceRequired() const
{
    return nullptr;
}


//===================================================================
//                      Batiment à Ressource
//===================================================================
BatimentRessource::BatimentRessource(std::string n, std::map<Ressource*, int> c, Ressource* p, int q, Ressource* sol, int l):
    Batiment(n,c,l),
    _produit(p),
    _quantite(q),
    _ressourceSolRequise(sol)
{}

std::unique_ptr<Batiment> BatimentRessource::clone() const
{
    return std::make_unique<BatimentRessource>(*this);
}

Ressource* BatimentRessource::getRessourceRequired() const
{
    return _ressourceSolRequise;
}

void BatimentRessource::action(Joueur & j)
{
    if (_produit != nullptr) {
        j.ajouterRessource(_produit, _quantite);
    }
}


//===================================================================
//                      Factory/Config
//===================================================================
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

        try {
            if (ressourcesDispo.find(nomResCout) == ressourcesDispo.end()) {
                throw std::runtime_error("Ressource inconnue : " + nomResCout + " pour le batiment : " + nomBat);
            }
            Ressource* resCout = ressourcesDispo.at(nomResCout);

            std::map<Ressource*, int> coutMap;
            coutMap[resCout] = qCout;

            if (ss >> nomResProd >> qProd) {
                if (ressourcesDispo.find(nomResProd) == ressourcesDispo.end()) {
                    throw std::runtime_error("Ressource Produite inconnue : " + nomResProd + " pour le batiment : " + nomBat);
                }
                
                Ressource* resProd = ressourcesDispo.at(nomResProd);
                catalogue[nomBat] = std::make_unique<BatimentRessource>(
                    nomBat, coutMap, resProd, qProd, resProd
                );
            } else {
                catalogue[nomBat] = std::make_unique<BatimentRessource>(
                    nomBat, coutMap, nullptr, 0, nullptr
                );
            }
        } catch(const std::exception& e) {
            throw std::runtime_error("Erreur dans le fichier batiment : " + std::string(e.what()));
        }
    }
}


void BatimentFactory::chargerConfiguration(const std::string& chemin, BatimentConfigReader& lecteur, const std::map<std::string, Ressource*>& ressourcesDispo)
{
    lecteur.load(chemin, _catalogue, ressourcesDispo);
}

std::unique_ptr<Batiment> BatimentFactory::create(std::string type)
{
    if (_catalogue.count(type))
    {
        return _catalogue[type]->clone();
    }
    return nullptr;
}
