#include "jeu.hh"
#include "comportement.hh"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

//===================================================================
//                     Tuile Configurable
//===================================================================
TuileConfigurable::TuileConfigurable(const TuileData* data) : _d(data) {}

std::string TuileConfigurable::getType() const
{
    return _d->nom;
}

char TuileConfigurable::getSymbole() const
{
    return _d->symbole;
}

int TuileConfigurable::getCoutDeplacement() const
{
    return _d->cout;
}

std::vector<Ressource *> TuileConfigurable::getRessource() const
{
    return _d->ressourceSpeciale;
}


bool TuileConfigurable::estFranchissable(const Unite& u) const
{
    bool peutnager = false;
    bool peutvoler = false;
    bool peutmarcher = false;

    for (auto* mov : u.Mobilite())
    {
        switch(mov->Nature())
        {
        case NatureMouv::TERRE:
            peutmarcher = true;
            break;
        case NatureMouv::MER:
            peutnager = true;
            break;
        case NatureMouv::AIR:
            peutvoler = true;
            break;
        }
    }

    if (_d->mouv.marche && peutmarcher) return true;
    if (_d->mouv.nage && peutnager) return true;
    if (_d->mouv.aerien && peutvoler) return true;
    return false;
}

bool TuileConfigurable::peutConstrVille() const {
    return _d->constructible && !_city;
}

bool TuileConfigurable::peutConstrBatiment(const Batiment & b) const {
    return b.getRessourcesSolRequired().empty();
}

bool TuileConfigurable::peutConstrBatimentSpecial(const Batiment & b) const {
    const BatimentRessource* br = dynamic_cast<const BatimentRessource*>(&b);
    if (!br) return false;

    const auto& requis = br->getRessourcesSolRequired();
    if (requis.empty()) return true;

    for (Ressource* req : requis) {
        bool trouve = false;
        for (Ressource* rTuile : _d->ressourceSpeciale) {
            if (rTuile == req) {
                trouve = true;
                break;
            }
        }
        if (!trouve) return false;
    }

    return true;
}

void TuileConfigurable::constrVille(const GameConfig& config, int x, int y, int max, bool capitale) {
    if (peutConstrVille()) 
    {
        _city = std::make_unique<City>(x, y, config, max, capitale);
    }
}
void TuileConfigurable::constrBatimentSpeciale(std::unique_ptr<Batiment> b) {
    if (peutConstrBatimentSpecial(*b)) {
        _batimentSpecial = std::move(b);
    }
}

City * TuileConfigurable::getCity() const
{
    return _city.get();
}

float TuileConfigurable::getStat(const std::string & key) const
{
    auto itLocal = _localStats.find(key);
    if (itLocal != _localStats.end()) return itLocal->second;

    auto itBase = _d->properties.find(key);
    if (itBase != _d->properties.end()) return itBase->second;

    return 0;
}

void TuileConfigurable::setStat(const std::string & key, float val)
{
    _localStats[key] = val;
}
//===================================================================
//===================================================================
//===================================================================

//===================================================================
//                              Board
//===================================================================
board::board(int size, WorldFactory & world):_size(size)
{
    for (int i = 0; i < size; ++i) {
        std::vector<std::unique_ptr<hexa>> ligne;
        for (int j = 0; j < size; ++j) {
            if (i == 0 || i == size - 1 || j == 0 || j == size - 1)
            {
                ligne.push_back(world.createTile('#'));
            }
            else
            {
                ligne.push_back(world.createRandomTile());
            }
        }
        _matrix.push_back(std::move(ligne));
    }
    world.postGeneration(_matrix, _size);
}

const hexa* board::getCell(int i, int j) const
{
    return _matrix[i][j].get();
}

void board::affichage() const {
    for (int i = 0; i < _size; ++i) {
        if (i%2 == 0) {
            std::cout << " ";
        }
        for (int j = 0; j < _size; ++j) {
            std::cout << _matrix[i][j]->getSymbole() << " ";
        }
        std::cout << std::endl;
    }
}

void board::placerUnite(int x, int y, std::unique_ptr<Unite> u) {
    if (x >= 0 && x < _size && y >= 0 && y < _size) {
        _unites[{x, y}] = std::move(u);
    }
}
Unite * board::getUnite(int x, int y) const {
    auto it = _unites.find({x, y});
    if (it != _unites.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool board::deplacerUnite(Unite& u, int xDest, int yDest) {
    int xSrc = u.location().first;
    int ySrc = u.location().second;

    //Unite selectionner aux Coord
    auto it = _unites.find({xSrc, ySrc});
    if (it == _unites.end()) return false;

    //Test cible valide, avec la portée
    for(auto const& mouv : u.Mobilite())
    {
        if(mouv->EstCaseValide({xSrc, ySrc}, {xDest, yDest}))
        {
            return false;
        }
    }

    //Coord dans la carte
    if (xDest < 0 || xDest >= _size || yDest < 0 || yDest >= _size) return false;

    //Personne aux Coord
    if (_unites.count({xDest, yDest})) return false;

    //Test de franchissement
    if (!_matrix[xDest][yDest]->estFranchissable(*(it->second)))
    {
        return false;
    }

    _unites[{xDest, yDest}] = std::move(it->second);
    _unites.erase(it);
    
    return true;
}

//===================================================================
//===================================================================
//===================================================================

//===================================================================
//                          Factory/Config
//===================================================================
void WorldFactory::ajouterAuCatalogue(char symbole, const TuileData& data)
{
    _catalogue[symbole] = data;
}

std::unique_ptr<hexa> WorldFactory::createTile(char symbole) {
    auto it = _catalogue.find(symbole);
    if (it != _catalogue.end()) {
        return std::make_unique<TuileConfigurable>(&(it->second));
    } else {
        return nullptr;
    }
}

void WorldFactory::initialiserBords() {
    TuileData limite;
    limite.nom = "Limite";
    limite.symbole = '#';
    limite.cout = -1;
    limite.constructible = false;
    
    limite.mouv = {false, false, false};
    limite.gen = {0, 0};
    limite.env = {0.0f, 0.0f, 0.0f};
    
    this->ajouterAuCatalogue('#', limite);
}

std::unique_ptr<hexa> WorldFactory::createRandomTile() {
    if (_catalogue.empty()) return nullptr;

    int poidsTotal = 0;
    for (auto const& [symb, data] : _catalogue) {
        poidsTotal += data.gen.poids;
    }

    // tuile au hasard si poidstotal à 0
    if (poidsTotal == 0) {
        auto it = _catalogue.begin();
        std::advance(it, rand() % _catalogue.size());
        return std::make_unique<TuileConfigurable>(&(it->second));
    } else {
        // par rapport aux poids
        int tirage = rand() % poidsTotal;
        int seuil = 0;
        for (auto & [symb, data] : _catalogue) {
            seuil += data.gen.poids;
            if (tirage < seuil) {
                return std::make_unique<TuileConfigurable>(&data);
            }
        }
    }
    return std::make_unique<TuileConfigurable>(&(_catalogue.begin()->second));
}

void WorldFactory::postGeneration(std::vector<std::vector<std::unique_ptr<hexa>>>& matrix, int size) {
    std::map<char, int> compteurs;

    for (auto & ligne : matrix) {
        for (auto & tuile : ligne) {
            compteurs[tuile->getSymbole()]++;
        }
    }

    for (auto const & [symb, data] : _catalogue) {
        while (compteurs[symb] < data.gen.nbMin) {
            int x = rand() % (size - 2) + 1;
            int y = rand() % (size - 2) + 1;

            if (matrix[x][y]->getSymbole() != '#' && matrix[x][y]->getSymbole() != symb) {
                matrix[x][y] = std::make_unique<TuileConfigurable>(&(_catalogue.at(symb)));
                compteurs[symb]++;
            }
        }
    }
}

bool WorldFactory::estVide() const
{
    return _catalogue.empty();
}

void TxtWorldReader::chargerConfig(std::string chemin, const std::map<std::string, Ressource*>& resDispo, WorldFactory& factory) {
    std::ifstream fichier(chemin);
    std::string mot, ligne;

    while (fichier >> mot) {
        if (mot == "TILE") {
            TuileData d;
            // nom tuile, symbole, cout unite, constructible dessus ou non
            fichier >> d.nom >> d.symbole >> d.cout >> d.constructible;

            while (fichier >> mot && mot != "END") {
                if (mot == "GEN") {
                    // Pourcentage quantité, nombres minimum sur le terrain
                    fichier >> d.gen.poids >> d.gen.nbMin;
                } 
                else if (mot == "MOUV") {
                    // marche, nage, aerien
                    fichier >> d.mouv.marche >> d.mouv.nage >> d.mouv.aerien;
                }
                else if (mot == "RES") {
                    // lis les ressources
                    std::getline(fichier, ligne);
                    std::stringstream ss(ligne);
                    std::string nomRes;

                    while (ss >> nomRes) {
                        if (nomRes == "None" || nomRes.empty()) continue;

                        if (resDispo.count(nomRes)) {
                            d.ressourceSpeciale.push_back(resDispo.at(nomRes));
                        }
                    }
                }
                else if (mot == "ENV") {
                    // température, radiation, gravite
                    fichier >> d.env.temperature >> d.env.radiation >> d.env.gravite;
                }
                else if (mot == "DATA") {
                    // lecture clé-valeur des data
                    std::getline(fichier, ligne);
                    std::stringstream ss(ligne);
                    std::string cle;
                    float val;
                    while (ss >> cle) {
                        if (cle == "None") break;
                        if (ss >> val) {
                            d.properties[cle] = val; 
                        }
                    }
                }
            }
            factory.ajouterAuCatalogue(d.symbole, d);
            std::cout << "Chargé : " << d.nom << " (" << d.symbole << ")" << std::endl;
        }
    }

    if (!fichier.eof() && fichier.fail()) {
        throw std::runtime_error("Erreur dans le fichier : " + chemin);
    }
}

void JsonWorldReader::chargerConfig(std::string chemin, const std::map<std::string, Ressource*>& resDispo, WorldFactory& factory) {
    std::ifstream fichier(chemin);
    if (!fichier.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier JSON : " + chemin);
    }

    json data;
    fichier >> data;

    for (auto& t : data["tiles"]) {
        TuileData d;
        d.nom = t["nom"];
        // Le symbole est un string en JSON, on prend le premier caractère
        std::string s = t["symbole"];
        d.symbole = s[0]; 
        d.cout = t["cout"];
        d.constructible = t["constructible"];

        // Bloc MOUV
        d.mouv.marche = t["mouv"]["marche"];
        d.mouv.nage = t["mouv"]["nage"];
        d.mouv.aerien = t["mouv"]["aerien"];

        // Bloc GEN
        d.gen.poids = t["gen"]["poids"];
        d.gen.nbMin = t["gen"]["nbMin"];

        // Bloc ENV
        d.env.temperature = t["env"]["temperature"];
        d.env.radiation = t["env"]["radiation"];
        d.env.gravite = t["env"]["gravite"];

        // Bloc RES (Ressources multiples)
        for (std::string resNom : t["ressources"]) {
            if (resDispo.count(resNom)) {
                d.ressourceSpeciale.push_back(resDispo.at(resNom));
            }
        }

        // Bloc DATA (Properties)
        if (t.contains("properties")) {
            for (auto& el : t["properties"].items()) {
                d.properties[el.key()] = el.value();
            }
        }

        factory.ajouterAuCatalogue(d.symbole, d);
    }
}

