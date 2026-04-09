#include "batiment.hh"
#include "../joueur/joueur.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

//===================================================================
//                          Batiment
//===================================================================

std::string Batiment::getName() const
{
    return _name;
}
const std::map<Ressource*, int> & Batiment::getResourceConstr() const
{
    return _cout;
}


const std::vector<Ressource*>& Batiment::getRessourcesSolRequired() const
{
    static const std::vector<Ressource*> vide;
    return vide;
};


//===================================================================
//                      Batiment à Ressource
//===================================================================
std::unique_ptr<Batiment> BatimentRessource::clone() const
{
    return std::make_unique<BatimentRessource>(*this);
}

const std::vector<Ressource*>& BatimentRessource::getRessourcesSolRequired() const
{
    return _ressourcesSolRequises; 
}

void BatimentRessource::action(Joueur & j) {
    for (auto const& [res, qte] : _produits) {
        j.ajouterRessource(res, qte);
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

        if (ressourcesDispo.count(nomResCout) == 0) continue;

        std::map<Ressource*, int> coutMap;
        coutMap[ressourcesDispo.at(nomResCout)] = qCout;
        
        std::map<Ressource*, int> prodMap;
        std::vector<Ressource*> solReq;
    
        if (ss >> nomResProd >> qProd) {
            if (ressourcesDispo.count(nomResProd)) {
                Ressource* res = ressourcesDispo.at(nomResProd);
                prodMap[res] = qProd;
                solReq.push_back(res);
            }
        }
            
        catalogue[nomBat] = std::make_unique<BatimentRessource>(
            nomBat, coutMap, prodMap, solReq
        );
    }
}

void JsonBatimentReader::load(const std::string& chemin, 
                std::map<std::string, std::unique_ptr<Batiment>>& catalogue,
                const std::map<std::string, Ressource*>& ressourcesDispo) {
    
    std::ifstream fichier(chemin);
    json data;
    fichier >> data;

    for (auto& item : data["batiments"]) {
        std::string nom = item["nom"];
        
        std::map<Ressource*, int> coutMap;
        for (auto& it : item["cout"].items()) {
            if (ressourcesDispo.count(it.key())) {
                coutMap[ressourcesDispo.at(it.key())] = it.value();
            }
        }

        std::map<Ressource*, int> prodMap;
        for (auto& prodItem : item["production"]) {
            std::string rNom = prodItem["ressource"];
            if (ressourcesDispo.count(rNom)) {
                prodMap[ressourcesDispo.at(rNom)] = prodItem["quantite"];
            }
        }

        std::vector<Ressource*> solsRequis;
        for (std::string sNom : item["ressources_sol_requises"]) {
            if (ressourcesDispo.count(sNom)) {
                solsRequis.push_back(ressourcesDispo.at(sNom));
            }
        }

        catalogue[nom] = std::make_unique<BatimentRessource>(nom, coutMap, prodMap, solsRequis);
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
