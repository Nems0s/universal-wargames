#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cstdlib>
#include <ctime>
#include <map>

#include "moteur.hh"
#include "commandes.hh"
#include "orientation.hh"

// ===========================================================
// Regex de validation oui/non
// ===========================================================
std::regex pattern_validation(R"(^(yes|y|oui|o)$)", std::regex_constants::icase);

// ===========================================================
// Conversion texte → direction
// ===========================================================
bool stringToDirection(const std::string& s, direction& out) {
    static const std::map<std::string, direction> table = {
        {"nord_ouest", direction::nord_ouest},
        {"nord_est",   direction::nord_est},
        {"ouest",      direction::ouest},
        {"est",        direction::est},
        {"sud_ouest",  direction::sud_ouest},
        {"sud_est",    direction::sud_est},
    };
    auto it = table.find(s);
    if (it != table.end()) { out = it->second; return true; }
    return false;
}

// ===========================================================
// Utilitaires d'affichage
// ===========================================================

void afficherSeparateur(const std::string& titre = "") {
    std::cout << "\n========================================" << std::endl;
    if (!titre.empty())
        std::cout << "   " << titre << std::endl;
    std::cout << "========================================" << std::endl;
}

void afficherResultat(ResultatAction res) {
    switch (res) {
        case ResultatAction::SUCCES:
            std::cout << "[SUCCÈS] Action effectuée." << std::endl;
            break;
        case ResultatAction::FIN_TOUR:
            std::cout << "[FIN DE TOUR]" << std::endl;
            break;
        case ResultatAction::ECHEC_FONDS_INSUFFISANTS:
            std::cout << "[ÉCHEC] Ressources insuffisantes." << std::endl;
            break;
        case ResultatAction::ECHEC_PA_INSUFFISANTS:
            std::cout << "[ÉCHEC] Points d'action insuffisants (coût rotation : "
                      << "voir config)." << std::endl;
            break;
        case ResultatAction::ECHEC_COORD_INVALIDE:
            std::cout << "[ÉCHEC] Coordonnées invalides." << std::endl;
            break;
        case ResultatAction::ECHEC_ARBITRE_REFUS:
            std::cout << "[ÉCHEC] Action refusée par l'arbitre." << std::endl;
            break;
    }
}

// ===========================================================
// Affichage du cône de vision d'une unité
// ===========================================================
void afficherConeVision(const Unite* u) {
    if (!u) return;
    const auto& visibles = u->getTuilesVisibles();
    std::cout << "  Regarde     : " << directionToString(u->regarde()) << std::endl;
    std::cout << "  Vision      : range=" << u->getVisionRange()
              << "  fov=" << u->getFov() << " cases" << std::endl;
    std::cout << "  Cases vues  : ";
    if (visibles.empty()) {
        std::cout << "(aucune)";
    } else {
        for (const auto& c : visibles) {
            std::cout << "(" << c.first << "," << c.second << ") ";
        }
    }
    std::cout << std::endl;
}

// ===========================================================
// Affichage de l'état du joueur actif
// ===========================================================
void afficherEtatJoueur(const MoteurDeJeu& moteur, int pIdx) {
    const Joueur& j = moteur.getJoueurs()[pIdx];

    afficherSeparateur("TOUR DE : " + j.getName()
        + "  (Tour " + std::to_string(moteur.getTourActuel()) + ")");

    // Ressources
    std::cout << "\nRessources : ";
    if (j.getInventaire().empty()) {
        std::cout << "(aucune)";
    } else {
        for (auto const& [res, qty] : j.getInventaire()) {
            std::cout << res->getName() << ": " << qty << "  ";
        }
    }
    std::cout << "\n" << std::endl;

    // Unités (résumé une ligne par unité avec direction)
    int idx = 0;
    for (auto* u : j.getUnites()) {
        if (!u) continue;
        std::cout << "  [" << idx << "] " << u->name()
                  << "  pos=(" << u->location().first << "," << u->location().second << ")"
                  << "  dir=" << directionToString(u->regarde())
                  << "  PA=" << u->point_action() << "/" << u->point_action_max()
                  << "  PV=" << u->health_point() << "/" << u->health_point_max()
                  << std::endl;
        ++idx;
    }

    // Villes
    std::cout << "\nVilles : ";
    for (auto* c : j.getCities()) {
        if (c) std::cout << "(" << c->getX() << "," << c->getY()
                         << ") Niv." << c->getLevel() << "  ";
    }
    std::cout << std::endl;
}

// ===========================================================
// Sous-menu "Faire une Action"
// ===========================================================
bool menuAction(MoteurDeJeu& moteur, int pIdx) {

    const Joueur& j = moteur.getJoueurs()[pIdx];

    std::cout << "\n--- ACTIONS POSSIBLES ---" << std::endl;
    std::cout << " 1. Déplacer              |  2. Pivoter (changer direction)   |  3. Attaquer"    << std::endl;
    std::cout << " 4. Soigner               |  5. Recruter                      |  6. Fonder Ville" << std::endl;
    std::cout << " 7. Acheter une Case      |  8. Améliorer Ville               |  9. Construire Bâtiment" << std::endl;
    std::cout << "10. Camoufler             | 11. Charger (transport)           | 12. Décharger (transport)" << std::endl;
    std::cout << "13. Enrôler (commandant)  | 14. Détruire une unité            | 15. Ravitailler sur ville" << std::endl;
    std::cout << "16. Ravitailler           | 17. FIN DE TOUR" << std::endl;

    int choix = 0;
    std::cout << "Choix : ";
    std::cin >> choix;

    CommandeJeu cmd = CmdFinTour{};

    switch (choix) {

        // ---------------------------
        case 1: { // DEPLACEMENT
            std::cout << "\nUnités disponibles :" << std::endl;
            for (auto* u : j.getUnites()) {
                if (u) {
                    std::cout << "  " << u->name()
                              << " pos=(" << u->location().first << "," << u->location().second << ")"
                              << " dir=" << directionToString(u->regarde())
                              << " PA=" << u->point_action() << std::endl;

                    // Afficher les déplacements possibles
                    auto possibles = moteur.getDeplacementsPossibles(pIdx, u->location().first, u->location().second);
                    if (!possibles.empty()) {
                        std::cout << "    Déplacements possibles : ";
                        for (auto& p : possibles) std::cout << "(" << p.first << "," << p.second << ") ";
                        std::cout << std::endl;
                    }
                }
            }
            int xSrc, ySrc, xDest, yDest;
            std::cout << "Coord. unité source (x y) : ";
            std::cin >> xSrc >> ySrc;
            std::cout << "Coord. destination  (x y) : ";
            std::cin >> xDest >> yDest;
            cmd = CmdDeplacement{xSrc, ySrc, xDest, yDest};
            break;
        }

        // ---------------------------
        case 2: { // PIVOTER (Rotation / Orientation)
            std::cout << "\nPivoter une unité — coût : " << moteur.getCoutRotation() << " PA" << std::endl;
            std::cout << "Unités disponibles :" << std::endl;
            for (auto* u : j.getUnites()) {
                if (u) {
                    std::cout << "  " << u->name()
                              << " pos=(" << u->location().first << "," << u->location().second << ")"
                              << " dir=" << directionToString(u->regarde())
                              << " PA=" << u->point_action() << std::endl;
                }
            }
            int x, y;
            std::string dirStr;
            std::cout << "Coord. unité (x y) : ";
            std::cin >> x >> y;
            std::cout << "Nouvelle direction (nord_ouest / nord_est / ouest / est / sud_ouest / sud_est) : ";
            std::cin >> dirStr;

            direction dir;
            if (!stringToDirection(dirStr, dir)) {
                std::cout << "[ERREUR] Direction invalide." << std::endl;
                return false;
            }

            // Afficher le cône avant/après pour info
            const board* plateau = moteur.getPlateau();
            Unite* u = plateau->getUnite(x, y);
            if (u) {
                std::cout << "  Cône actuel  → ";
                afficherConeVision(u);
            }

            cmd = CmdRotation{x, y, dir};
            break;
        }

        // ---------------------------
        case 3: { // ATTAQUE
            int xSrc, ySrc, xDest, yDest;
            std::cout << "\nCoord. attaquant (x y) : ";
            std::cin >> xSrc >> ySrc;

            const board* plateau = moteur.getPlateau();
            Unite* att = plateau->getUnite(xSrc, ySrc);
            int choix_attaque = 0;

            if (att) {
                std::cout << "\nAttaquant : " << att->name()
                          << "  dir=" << directionToString(att->regarde()) << std::endl;
                afficherConeVision(att);

                int choix_attaque;
                auto attaques = att->Offensive();
                if (!attaques.empty()) {
                    std::cout << "Attaques disponibles :" << std::endl;
                    int i = 0;
                    for (auto* comp : attaques) {
                        std::cout << "  " << i << ". ";
                        comp->affiche();
                        ++i;
                    }
                    std::cout << "Choisissez l'index de l'attaque : ";
                    std::cin >> choix_attaque;

                    if (choix_attaque < 0 || choix_attaque >= static_cast<int>(attaques.size())) {
                        std::cout << "Index d'attaque invalide." << std::endl;
                        return false;
                    }

                } else {
                    std::cout << "Cette unité n'a aucune capacité offensive." << std::endl;
                    return false;
                }

                auto itAtt = attaques.begin();
                std::advance(itAtt, choix_attaque);
                CompAtt* attaque_choisie = *itAtt;

                auto attaquables = moteur.getAttaquesPossibles(pIdx, xSrc, ySrc);
                if (!attaquables.empty()) {
                    std::cout << "Cibles valides pour cette attaque : ";
                    bool auMoinsUneCible = false;
                    
                    for (auto& p : attaquables) {
                        Unite* ciblePotentielle = plateau->getUnite(p.first, p.second);
                        if (ciblePotentielle != nullptr) {
                            bool dansLaPortee = attaque_choisie->PeuxAttaquer(*att, *ciblePotentielle);
                            
                            if (dansLaPortee) {
                                std::cout << "(" << p.first << "," << p.second << ") ";
                                auMoinsUneCible = true;
                            }
                        }
                    }
                    
                    if (!auMoinsUneCible)
                    {
                        std::cout << "(Aucune cible valide pour cette attaque)";
                        return false;
                    }
                    std::cout << std::endl;

                } else {
                    std::cout << "(Aucune cible globale trouvée)" << std::endl;
                    return false;
                }
            } else {
                std::cout << "Aucune unité à ces coordonnées." << std::endl;
                return false;
            }

            std::cout << "Coord. cible (x y) : ";
            std::cin >> xDest >> yDest;

            // Informer sur l'avantage d'attaque
            Unite* def = plateau->getUnite(xDest, yDest);
            if (def) {
                bool avantage = avantage_attaque(att->location(), def->location(), def->regarde(), def->getFov());
                if (avantage) {
                    std::cout << " AVANTAGE : vous attaquez dans le DOS du défenseur !" << std::endl;
                } else {
                    std::cout << " Attaque de face : le défenseur vous voit." << std::endl;
                }
            }

            cmd = CmdAttaque{xSrc, ySrc, xDest, yDest, choix_attaque};
            break;
        }

        // ---------------------------
        case 4: { // SOIGNER
            int xSrc, ySrc, xDest, yDest;
            std::cout << "\nCoord. soigneur (x y) : ";
            std::cin >> xSrc >> ySrc;

            const board* plateau = moteur.getPlateau();
            Unite* healer = plateau->getUnite(xSrc, ySrc);
            int choix_soin = 0;

            if (healer) {
                auto soins = healer->Soin();
                if (!soins.empty()) {
                    std::cout << "Soins disponibles pour " << healer->name() << " :" << std::endl;
                    int i = 0;
                    for (auto* comp : soins) {
                        std::cout << "  " << i << ". ";
                        comp->affiche();
                        ++i;
                    }
                    std::cout << "Choisissez l'index du soin : ";
                    std::cin >> choix_soin;
                    if (choix_soin < 0 || choix_soin >= static_cast<int>(soins.size())) {
                        std::cout << "Index de soin invalide." << std::endl;
                        return false;
                    }

                    auto itSoin = soins.begin();
                    std::advance(itSoin, choix_soin);
                    CompSoin* soin_choisi = *itSoin;

                    std::cout << "Cibles valides pour ce soin : ";
                    bool auMoinsUneCibleSoin = false;

                    // On génère les cases adjacentes selon la portée du soin
                    int portee = soin_choisi->portee();
                    auto cases_possibles = case_adjascentes(healer->location(), portee); 

                    for (const auto& p : cases_possibles) {
                        Unite* ciblePotentielle = plateau->getUnite(p.first, p.second);
                        
                        if (ciblePotentielle != nullptr) {
                            // On vérifie avec la méthode définie dans ton comportement
                            if (soin_choisi->PeuxSoigner(*healer, *ciblePotentielle)) {
                                std::cout << "(" << p.first << "," << p.second << ") ";
                                auMoinsUneCibleSoin = true;
                            }
                        }
                    }
                    
                    if (!auMoinsUneCibleSoin) 
                    {
                        std::cout << "(Aucune cible valide pour ce soin)";
                        return false;
                    }
                    std::cout << std::endl;
                } else {
                    std::cout << "(Aucune cible globale trouvée)" << std::endl;
                    return false;
                }
            } else {
                std::cout << "Aucune unité à ces coordonnées." << std::endl;
                return false;
            }

            std::cout << "Coord. cible (x y) : ";
            std::cin >> xDest >> yDest;
            cmd = CmdSoigner{xSrc, ySrc, xDest, yDest, choix_soin};
            break;
        }

        // ---------------------------
        case 5: { // RECRUTEMENT
            std::cout << "\nVilles disponibles pour le recrutement :" << std::endl;
            for (auto* c : j.getCities()) {
                if (c) {
                    std::cout << "  (" << c->getX() << "," << c->getY() << ")  Niv." << c->getLevel() << std::endl;
                    std::cout << "    Cases adjacentes : ";
                    for (auto& voisin : Voisins({c->getX(), c->getY()}))
                        std::cout << "(" << voisin.first << "," << voisin.second << ") ";
                    std::cout << std::endl;
                }
            }

            const FactionParams* faction = j.getFaction();
            if (faction) {
                std::cout << "Unités disponibles dans votre faction :" << std::endl;
                for (const auto& nomU : faction->unites_disponibles)
                    std::cout << "  - " << nomU << std::endl;
            }

            int x, y;
            std::string nomUnite;
            std::cout << "Coord. de spawn (x y) : ";
            std::cin >> x >> y;
            std::cin.ignore(10000, '\n');
            std::cout << "Nom de l'unité : ";
            std::getline(std::cin, nomUnite);
            cmd = CmdRecrutement{x, y, nomUnite};
            break;
        }

        // ---------------------------
        case 6: { // FONDER VILLE
            int x, y;
            std::string nomVille;
            std::cout << "\nTypes de ville disponibles :" << std::endl;
            for (const auto& [nom, city] : moteur.getCityFactory().getCatalogue()) {
                std::cout << "  - " << nom
                          << (city->estCapitale() ? " (Capitale)" : "") << std::endl;
            }
            std::cout << "Coord. nouvelle ville (x y) : ";
            std::cin >> x >> y;
            std::cout << "Nom du type de ville : ";
            std::cin >> nomVille;
            cmd = CmdFonderVille{x, y, nomVille};
            break;
        }

        // ---------------------------
        case 7: { // ACHETER CASE
            int x, y;
            std::cout << "\nCoord. de la case à acheter (x y) : ";
            std::cin >> x >> y;
            cmd = CmdAcheterCase{x, y};
            break;
        }

        // ---------------------------
        case 8: { // AMELIORER VILLE
            int x, y;
            std::cout << "\nCoord. ville à améliorer (x y) : ";
            std::cin >> x >> y;
            cmd = CmdAmeliorer{x, y};
            break;
        }

        // ---------------------------
        case 9: { // CONSTRUIRE BATIMENT
            int x, y;
            std::string nomBat;
            std::cout << "\nBâtiments disponibles :" << std::endl;
            for (const auto& [nom, bat] : moteur.getBatimentFactory().getCatalogue())
                std::cout << "  - " << nom << std::endl;
            std::cout << "Coord. ville (x y) : ";
            std::cin >> x >> y;
            std::cout << "Nom du bâtiment : ";
            std::cin >> nomBat;
            cmd = CmdConstruction{x, y, nomBat};
            break;
        }

        // ---------------------------
        case 10: { // CAMOUFLER
            int x, y;
            std::cout << "\nCoord. unité à camoufler (x y) : ";
            std::cin >> x >> y;
            cmd = CmdCamoufler{x, y};
            break;
        }

        // ---------------------------
        case 11: { // CHARGER (transport)
            int xTrans, yTrans, xPass, yPass;
            std::cout << "\nCoord. transporteur (x y) : ";
            std::cin >> xTrans >> yTrans;
            std::cout << "Coord. passager    (x y) : ";
            std::cin >> xPass >> yPass;
            cmd = CmdCharger{xPass, yPass, xTrans, yTrans};
            break;
        }

        // ---------------------------
        case 12: { // DECHARGER (transport)
            int xTrans, yTrans, xDest, yDest, idx;
            std::cout << "\nCoord. transporteur (x y) : ";
            std::cin >> xTrans >> yTrans;

            const board* plateau = moteur.getPlateau();
            Unite* trans = plateau->getUnite(xTrans, yTrans);
            if (trans && trans->Transport()) {
                int i = 0;
                for (auto& pass : trans->Transport()->liste_unite_transporter()) {
                    std::cout << "  " << i << ". " << pass->name() << std::endl;
                    ++i;
                }
            } else {
                std::cout << "Pas de transporteur à ces coordonnées." << std::endl;
                return false;
            }

            std::cout << "Index du passager à décharger : ";
            std::cin >> idx;
            std::cout << "Coord. de dépose (x y) : ";
            std::cin >> xDest >> yDest;
            cmd = CmdDecharger{xTrans, yTrans, idx, xDest, yDest};
            break;
        }

        // ---------------------------
        case 13: { // ENROLER (commandant)
            int xCom, yCom, xRec, yRec;
            std::cout << "\nCoord. Commandant (x y) : ";
            std::cin >> xCom >> yCom;
            std::cout << "Coord. Recrue     (x y) : ";
            std::cin >> xRec >> yRec;
            cmd = CmdEnroler{xCom, yCom, xRec, yRec};
            break;
        }

        // ---------------------------
        case 14: { // DETRUIRE UNITE
            int x, y;
            std::cout << "\nCoord. unité à détruire (x y) : ";
            std::cin >> x >> y;
            std::string confirm;
            std::cout << "Confirmer la destruction ? (Y/N) : ";
            std::cin >> confirm;
            if (!std::regex_match(confirm, pattern_validation)) {
                std::cout << "Annulé." << std::endl;
                return false;
            }
            cmd = CmdDetruireUnite{x, y};
            break;
        }

        // ---------------------------
        case 15: { // RAVITAILLER SUR VILLE
            int x, y;
            std::cout << "\nCoord. unité à ravitailler (x y) : ";
            std::cin >> x >> y;

            const board* plateau = moteur.getPlateau();
            Unite* u = plateau->getUnite(x, y);
            if (u) {
                std::cout << "  Inventaire actuel : ";
                for (auto const& [res, qte] : u->getInventaireInterne())
                    std::cout << res->getName() << ": " << qte << "  ";
                std::cout << std::endl;
            } else {
                std::cout << "Aucune unité à ces coordonnées." << std::endl;
                return false;
            }
            cmd = CmdRavitaillerSurVille{x, y};
            break;
        }

        // ---------------------------
        case 16: { // RAVITAILLER (unité → unité)
            int xSrc, ySrc, xDest, yDest;
            std::cout << "\nCoord. ravitailleur (x y) : ";
            std::cin >> xSrc >> ySrc;
            std::cout << "Coord. cible        (x y) : ";
            std::cin >> xDest >> yDest;

            const board* plateau = moteur.getPlateau();
            Unite* source = plateau->getUnite(xSrc, ySrc);
            if (source && source->Ravitaillement()) {
                std::cout << "  Stock disponible : ";
                for (auto const& [res, qte] : source->getInventaireInterne())
                    std::cout << res->getName() << ": " << qte << "  ";
                std::cout << std::endl;
            } else {
                std::cout << "Cette unité n'a pas de comportement Ravitaillement." << std::endl;
                return false;
            }
            cmd = CmdRavitailler{xSrc, ySrc, xDest, yDest};
            break;
        }

        // ---------------------------
        case 17: { // FIN DE TOUR  ← était case 15
            std::cout << "\n[FIN DE TOUR]" << std::endl;
            cmd = CmdFinTour{};
            break;
        }

        default: {
            std::cout << "Action inconnue." << std::endl;
            return false;
        }
    }

    ResultatAction res = moteur.soumettreCommande(pIdx, cmd);
    afficherResultat(res);

    // Si rotation réussie : afficher le nouveau cône
    if (res == ResultatAction::SUCCES && choix == 2) 
    {
        std::cout << "  → Utilisez 'Voir mes unités' pour voir le nouveau cône de vision." << std::endl;
    }

    return (res == ResultatAction::FIN_TOUR);
}

// ===========================================================
// MAIN
// ===========================================================
int main() {

    afficherSeparateur("Initialisation Du Jeu");

    // -------------------------------------------------------
    // 1. Chargement de la configuration
    // -------------------------------------------------------

    std::cout << "\nChargement des configurations depuis configs/..." << std::endl;

    MoteurDeJeu moteur;

    // Config
    GameConfigFiles mesConfigs;
    std::string jeu = "space";

    // Paths
    mesConfigs.rulesPath = "configs/" + jeu + "/config_rules.json";
    mesConfigs.ressourcesPath = "configs/" + jeu + "/config_ressources.json";
    mesConfigs.batimentsPath = "configs/" + jeu + "/config_batiments.json";
    mesConfigs.villesPath = "configs/" + jeu + "/config_villes.json";
    mesConfigs.unitesPath = "configs/" + jeu + "/config_unites.json";
    mesConfigs.tuilesPath = "configs/" + jeu + "/config_tuiles.json";
    mesConfigs.winsPath = "configs/" + jeu + "/config_wins.json";

    // Chargement
    try {
        moteur.chargerConfiguration(mesConfigs);
        std::cout << "[SYSTEME] Configuration chargée." << std::endl;
        std::cout << "  Coût de rotation : " << moteur.getCoutRotation() << " PA" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erreur critique lors du chargement : " << e.what() << std::endl;
        return 1;
    }

    // -------------------------------------------------------
    // 2. Joueurs
    // -------------------------------------------------------
    int nbJoueurs = 0;
    std::cout << "\nNombre de joueurs : ";
    std::cin >> nbJoueurs;

    if (nbJoueurs < 1) {
        std::cerr << "Il faut au moins 1 joueur." << std::endl;
        return 1;
    }

    std::vector<std::string> noms;
    std::vector<std::string> factions;

    std::cout << "\nFactions disponibles :" << std::endl;
    for (const auto& [nom, fp] : moteur.getFactionsAvailable()) {
        std::cout << "  - " << nom << std::endl;
        if (!fp.unites_disponibles.empty()) {
            std::cout << "    Unités : ";
            for (const auto& u : fp.unites_disponibles) std::cout << u << "  ";
            std::cout << std::endl;
        }
    }

    for (int i = 0; i < nbJoueurs; ++i) {
        std::string nom, faction;
        std::cout << "\nNom du joueur " << (i + 1) << " : ";
        std::cin >> nom;
        std::cin.ignore(10000, '\n'); //Vide le buffer
        std::cout << "Faction pour " << nom << " : ";
        std::getline(std::cin, faction);
        noms.push_back(nom);
        factions.push_back(faction);
    }

    // -------------------------------------------------------
    // 3. Graine de génération
    // -------------------------------------------------------
    int seed = 0;
    std::cout << "\nGraine de génération (0 = aléatoire) : ";
    std::cin >> seed;
    if (seed == 0) seed = static_cast<int>(std::time(nullptr));

    // -------------------------------------------------------
    // 4. Conditions de victoire
    // -------------------------------------------------------
    std::cout << "Index des conditions de victoire (0 par défaut) : ";
    int victorySet = 0;
    std::cin >> victorySet;

    // -------------------------------------------------------
    // 5. Initialisation
    // -------------------------------------------------------
    try {
        moteur.initGame(seed, noms, factions);
        moteur.setActiveVictorySet(victorySet);
        std::cout << "[SYSTEME] Partie initialisée (graine : " << seed << ")." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'initialisation : " << e.what() << std::endl;
        return 1;
    }

    // -------------------------------------------------------
    // 6. Boucle de jeu
    // -------------------------------------------------------
    afficherSeparateur("DÉBUT DE LA PARTIE");

    while (!moteur.isPartieTerminee()) {

        int pIdx = moteur.getCurrentPlayerTurn();
        afficherEtatJoueur(moteur, pIdx);

        std::cout << "\n1. Voir la carte" << std::endl;
        std::cout << "2. Faire une action" << std::endl;
        std::cout << "3. Voir mes unités (détail + cône de vision)" << std::endl;
        std::cout << "4. Voir mes villes" << std::endl;
        std::cout << "5. Déclarer forfait" << std::endl;
        std::cout << "Choix : ";

        int choixMenu = 0;
        std::cin >> choixMenu;

        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "[ERREUR] Entrée invalide." << std::endl;
            continue;
        }

        if (choixMenu == 1) {
            const board* plateau = moteur.getPlateau();
            if (plateau) plateau->affichage();

        } else if (choixMenu == 2) {
            menuAction(moteur, pIdx);

        } else if (choixMenu == 3) {
            const Joueur& j = moteur.getJoueurs()[pIdx];
            std::cout << "\n--- VOS UNITÉS ---" << std::endl;
            for (auto* u : j.getUnites()) {
                if (!u) continue;
                std::cout << "\n";
                u->affiche();
                // Affichage du cône de vision
                afficherConeVision(u);
            }

        } else if (choixMenu == 4) {
            /// Détail villes
            const auto& j = moteur.getJoueurs()[pIdx];
            std::cout << "\n--- VOS VILLES ---" << std::endl;

            for (auto* c : j.getCities()) {
                if (c) {
                    std::cout << "\n========================================" << std::endl;
                    // Utilisation de l'opérateur -> car 'c' est un pointeur
                    std::cout << " CITY : " << c->getNom() << (c->estCapitale() ? " [CAPITALE]" : "") << std::endl;
                    std::cout << "========================================" << std::endl;

                    // Informations de position et niveau
                    std::cout << "Localisation : (" << c->getX() << ", " << c->getY() << ")" << std::endl;
                    std::cout << "Niveau       : " << c->getLevel() << "/" << c->getMaxLevel() << std::endl;
                    std::cout << "Territoire   : Rayon de " << c->getRayonTerritoire() << " cases" << std::endl;

                    // État de santé et combat
                    std::cout << "Points de Vie: " << c->getPv() << " / " << c->getPvMax() << std::endl;
                    std::cout << "Puissance    : " << c->getDegats() << " dégâts" << std::endl;
                    std::cout << "Vision       : " << c->getVisionRange() << " cases" << std::endl;

                    std::cout << "----------------------------------------" << std::endl;

                    // Affichage de la production de ressources
                    std::cout << "Production par tour :" << std::endl;
                    const auto& produits = c->getProduits();
                    if (produits.empty()) {
                        std::cout << "  - Aucune production" << std::endl;
                    } else {
                        for (const auto& [ressource, quantite] : produits) {
                            // Utilisation de getNom() sur la ressource (si défini dans ressource.hh)
                            std::cout << "  * " << ressource->getName() << " : +" << quantite << std::endl;
                        }
                    }

                    std::cout << "----------------------------------------" << std::endl;

                    // Affichage des bâtiments
                    const auto& batiments = c->getBatiments();
                    std::cout << "Bâtiments (" << batiments.size() << ") :" << std::endl;
                    if (batiments.empty()) {
                        std::cout << "  - Aucun bâtiment construit" << std::endl;
                    } else {
                        for (const auto& b : batiments) {
                            // 'b' est un std::unique_ptr<Batiment>, on utilise ->
                            std::cout << "  [B] " << b->getName() << std::endl;
                        }
                    }

                    std::cout << "========================================" << std::endl;
                }
            }

        } else if (choixMenu == 5) {
            std::string confirm;
            std::cout << "\nÊtes-vous sûr de déclarer forfait ? (Y/N) : ";
            std::cin >> confirm;
            if (std::regex_match(confirm, pattern_validation)) {
                std::cout << "Le joueur " << moteur.getJoueurs()[pIdx].getName()
                          << " a déclaré forfait !" << std::endl;
                break;
            }
        } else {
            std::cout << "Choix invalide." << std::endl;
        }
    }

    // -------------------------------------------------------
    // 7. Fin de partie
    // -------------------------------------------------------
    afficherSeparateur("FIN DE PARTIE");

    if (moteur.isPartieTerminee()) {
        std::cout << "\n★★★  FÉLICITATIONS  ★★★" << std::endl;
        std::cout << moteur.getNomVainqueur() << " REMPORTE LA PARTIE !" << std::endl;
    } else {
        std::cout << "\nLa partie s'est terminée sans vainqueur déclaré." << std::endl;
    }

    std::cout << "\nTours joués : " << moteur.getTourActuel() << std::endl;
    std::cout << "Merci d'avoir joué à Space Wargames !\n" << std::endl;

    return 0;
}
