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
const std::map<const Ressource*, int> & Batiment::getResourceConstr() const
{
    return _cout;
}


const std::vector<const Ressource*>& Batiment::getRessourcesSolRequired() const
{
    static const std::vector<const Ressource*> vide;
    return vide;
};


//===================================================================
//                      Batiment à Ressource
//===================================================================
std::unique_ptr<Batiment> BatimentRessource::clone() const
{
    return std::make_unique<BatimentRessource>(*this);
}

const std::vector<const Ressource*>& BatimentRessource::getRessourcesSolRequired() const
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
                const std::map<std::string, const Ressource*>& ressourcesDispo) {
            
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

        std::map<const Ressource*, int> coutMap;
        coutMap[ressourcesDispo.at(nomResCout)] = qCout;
        
        std::map<const Ressource*, int> prodMap;
        std::vector<const Ressource*> solReq;
    
        if (ss >> nomResProd >> qProd) {
            if (ressourcesDispo.count(nomResProd)) {
                const Ressource* res = ressourcesDispo.at(nomResProd);
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
                const std::map<std::string, const Ressource*>& ressourcesDispo) {
    
    std::ifstream fichier(chemin);
    json data;
    fichier >> data;

    for (auto& item : data["batiments"]) {
        std::string nom = item["nom"];
        
        std::map<const Ressource*, int> coutMap;
        auto coutObj = item.value("cout", json::object());
        for (const auto& [resName, qty] : coutObj.items()) {
            if (auto it = ressourcesDispo.find(resName); it != ressourcesDispo.end()) {
                coutMap[it->second] = qty.get<int>();
            }
        }

        std::map<const Ressource*, int> prodMap;
        for (const auto& prodItem : item.value("production", json::array())) {
            std::string rNom = prodItem["ressource"];
            if (auto it = ressourcesDispo.find(rNom); it != ressourcesDispo.end()) {
                prodMap[it->second] = prodItem["quantite"].get<int>();
            }
        }

        std::vector<const Ressource*> solsRequis;
        for (const std::string& sNom : item.value("ressources_sol_requises", json::array())) {
            if (auto it = ressourcesDispo.find(sNom); it != ressourcesDispo.end()) {
                solsRequis.push_back(it->second);
            }
        }

        std::string tex = item.value("texture", "");

        catalogue[nom] = std::make_unique<BatimentRessource>(nom, coutMap, prodMap, solsRequis, 1, tex);
    }
}


void BatimentFactory::chargerConfiguration(const std::string& chemin, BatimentConfigReader& lecteur, const std::map<std::string, const Ressource*>& ressourcesDispo)
{
    lecteur.load(chemin, _catalogue, ressourcesDispo);
}

std::unique_ptr<Batiment> BatimentFactory::create(std::string type) const
{
    if (_catalogue.count(type))
    {
        return _catalogue.at(type)->clone();
    }
    return nullptr;
}
