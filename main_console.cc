#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cstdlib>
#include <ctime>

#include "moteur.hh"
#include "commandes.hh"

// ===========================================================
// Regex de validation oui/non
// ===========================================================
std::regex pattern_validation(R"(^(yes|y|oui|o)$)", std::regex_constants::icase);

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
            std::cout << "[ÉCHEC] Points d'action insuffisants." << std::endl;
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
// Affichage de l'état du joueur actif
// ===========================================================
void afficherEtatJoueur(const MoteurDeJeu& moteur, int pIdx) {
    const auto& joueurs = moteur.getJoueurs();
    const Joueur& j = joueurs[pIdx];

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
    std::cout << std::endl;

    // Unités
    std::cout << "Unités    : " << j.getUnites().size() << "   ";
    for (auto* u : j.getUnites()) {
        if (u) {
            std::cout << u->name()
                      << "(" << u->location().first << "," << u->location().second << ")"
                      << " PA:" << u->point_action()
                      << " PV:" << u->health_point()
                      << "   ";
        }
    }
    std::cout << std::endl;

    // Villes
    std::cout << "Villes    : " << j.getCities().size() << "   ";
    for (auto* c : j.getCities()) {
        if (c) {
            std::cout << "(" << c->getX() << "," << c->getY() << ") Niv." << c->getLevel() << "   ";
        }
    }
    std::cout << std::endl;
}

// ===========================================================
// Sous-menu "Faire une Action"
// ===========================================================
bool menuAction(MoteurDeJeu& moteur, int pIdx) {

    const auto& joueurs = moteur.getJoueurs();
    const Joueur& j = joueurs[pIdx];

    std::cout << "\n--- ACTIONS POSSIBLES ---" << std::endl;
    std::cout << " 1. Déplacer              |  2. Attaquer             |  3. Soigner" << std::endl;
    std::cout << " 4. Recruter              |  5. Fonder une Ville     |  6. Acheter une Case" << std::endl;
    std::cout << " 7. Améliorer Ville       |  8. Construire Bâtiment  |  9. Camoufler" << std::endl;
    std::cout << "10. Charger (transport)   | 11. Décharger (transport)| 12. Enrôler (commandant)" << std::endl;
    std::cout << "13. Détruire une unité    | 14. FIN DE TOUR" << std::endl;

    int choix = 0;
    std::cout << "Choix : ";
    std::cin >> choix;

    CommandeJeu cmd = CmdFinTour{}; // valeur par défaut
    bool valide = true;

    switch (choix) {

        // ---------------------------
        case 1: { // DEPLACEMENT
            std::cout << "\nUnités disponibles : ";
            for (auto* u : j.getUnites()) {
                if (u) std::cout << u->name()
                                 << "(" << u->location().first << "," << u->location().second << ")  ";
            }
            int xSrc, ySrc, xDest, yDest;
            std::cout << "\nCoord. unité (x y) : ";
            std::cin >> xSrc >> ySrc;
            std::cout << "Coord. destination (x y) : ";
            std::cin >> xDest >> yDest;
            cmd = CmdDeplacement{xSrc, ySrc, xDest, yDest};
            break;
        }

        // ---------------------------
        case 2: { // ATTAQUE
            int xSrc, ySrc, xDest, yDest;
            std::cout << "\nCoord. attaquant (x y) : ";
            std::cin >> xSrc >> ySrc;

            // Afficher les attaques disponibles si l'unité existe
            const board* plateau = moteur.getPlateau();
            Unite* att = plateau->getUnite(xSrc, ySrc);
            if (att) {
                auto attaques = att->Offensive();
                if (!attaques.empty()) {
                    std::cout << "Attaques disponibles pour " << att->name() << " :" << std::endl;
                    int i = 0;
                    for (auto* comp : attaques) {
                        std::cout << "  " << i << ". ";
                        comp->affiche();
                        ++i;
                    }
                } else {
                    std::cout << "Cette unité n'a aucune capacité offensive." << std::endl;
                    return false;
                }
            } else {
                std::cout << "Aucune unité à ces coordonnées." << std::endl;
                return false;
            }

            std::cout << "Coord. cible (x y) : ";
            std::cin >> xDest >> yDest;
            cmd = CmdAttaque{xSrc, ySrc, xDest, yDest};
            break;
        }

        // ---------------------------
        case 3: { // SOIGNER
            int xSrc, ySrc, xDest, yDest;
            std::cout << "\nCoord. soigneur (x y) : ";
            std::cin >> xSrc >> ySrc;

            const board* plateau = moteur.getPlateau();
            Unite* healer = plateau->getUnite(xSrc, ySrc);
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
                } else {
                    std::cout << "Cette unité n'a aucune capacité de soin." << std::endl;
                    return false;
                }
            } else {
                std::cout << "Aucune unité à ces coordonnées." << std::endl;
                return false;
            }

            std::cout << "Coord. cible (x y) : ";
            std::cin >> xDest >> yDest;
            cmd = CmdSoigner{xSrc, ySrc, xDest, yDest};
            break;
        }

        // ---------------------------
        case 4: { // RECRUTEMENT
            std::cout << "\nVilles disponibles pour le recrutement : ";
            for (auto* c : j.getCities()) {
                if (c) std::cout << "(" << c->getX() << "," << c->getY() << ")  ";
            }

            // Afficher les unités de la faction
            const FactionParams* faction = j.getFaction();
            if (faction) {
                std::cout << "\nUnités disponibles dans votre faction :" << std::endl;
                for (const auto& nomU : faction->unites_disponibles) {
                    std::cout << "  - " << nomU << std::endl;
                }
            } else {
                std::cout << "\n(Aucune faction assignée)" << std::endl;
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
        case 5: { // FONDER VILLE
            int x, y;
            std::string nomVille;
            std::cout << "\nCoord. nouvelle ville (x y) : ";
            std::cin >> x >> y;

            // Afficher les types de villes disponibles
            for (const auto& [nom, city] : moteur.getCityFactory().getCatalogue()) {
                std::cout << "  - " << nom
                          << (city->estCapitale() ? " (Capitale)" : "") << std::endl;
            }
            std::cout << "Nom du type de ville : ";
            std::cin >> nomVille;
            cmd = CmdFonderVille{x, y, nomVille};
            break;
        }

        // ---------------------------
        case 6: { // ACHETER CASE
            int x, y;
            std::cout << "\nCoord. de la case à acheter (x y) : ";
            std::cin >> x >> y;
            cmd = CmdAcheterCase{x, y};
            break;
        }

        // ---------------------------
        case 7: { // AMELIORER VILLE
            int x, y;
            std::cout << "\nCoord. ville à améliorer (x y) : ";
            std::cin >> x >> y;
            cmd = CmdAmeliorer{x, y};
            break;
        }

        // ---------------------------
        case 8: { // CONSTRUIRE BATIMENT
            int x, y;
            std::string nomBat;
            std::cout << "\nCoord. ville (x y) : ";
            std::cin >> x >> y;

            // Afficher les bâtiments disponibles
            for (const auto& [nom, bat] : moteur.getBatimentFactory().getCatalogue()) {
                std::cout << "  - " << nom << std::endl;
            }
            std::cout << "Nom du bâtiment : ";
            std::cin >> nomBat;
            cmd = CmdConstruction{x, y, nomBat};
            break;
        }

        // ---------------------------
        case 9: { // CAMOUFLER
            int x, y;
            std::cout << "\nCoord. unité à camoufler (x y) : ";
            std::cin >> x >> y;
            cmd = CmdCamoufler{x, y};
            break;
        }

        // ---------------------------
        case 10: { // CHARGER (transport)
            int xTrans, yTrans, xPass, yPass;
            std::cout << "\nCoord. transporteur (x y) : ";
            std::cin >> xTrans >> yTrans;
            std::cout << "Coord. passager (x y) : ";
            std::cin >> xPass >> yPass;
            cmd = CmdCharger{xPass, yPass, xTrans, yTrans};
            break;
        }

        // ---------------------------
        case 11: { // DECHARGER (transport)
            int xTrans, yTrans, xDest, yDest, idx;
            std::cout << "\nCoord. transporteur (x y) : ";
            std::cin >> xTrans >> yTrans;

            // Afficher les passagers
            const board* plateau = moteur.getPlateau();
            Unite* trans = plateau->getUnite(xTrans, yTrans);
            if (trans && trans->Transport()) {
                int i = 0;
                for (auto& pass : trans->Transport()->liste_unite_transporter()) {
                    std::cout << "  " << i << ". " << pass->name() << std::endl;
                    ++i;
                }
            } else {
                std::cout << "Pas de transport à ces coordonnées." << std::endl;
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
        case 12: { // ENROLER (commandant)
            int xCom, yCom, xRec, yRec;
            std::cout << "\nCoord. Commandant (x y) : ";
            std::cin >> xCom >> yCom;
            std::cout << "Coord. Recrue (x y) : ";
            std::cin >> xRec >> yRec;
            cmd = CmdEnroler{xCom, yCom, xRec, yRec};
            break;
        }

        // ---------------------------
        case 13: { // DETRUIRE UNITE
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
        case 14: { // FIN DE TOUR
            std::cout << "\n[FIN DE TOUR]" << std::endl;
            cmd = CmdFinTour{};
            break;
        }

        default:
            std::cout << "Action inconnue." << std::endl;
            return false;
    }

    ResultatAction res = moteur.soumettreCommande(pIdx, cmd);
    afficherResultat(res);
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
    MoteurDeJeu moteur;

    std::cout << "\nChargement des configurations depuis configs/..." << std::endl;
    try {
        moteur.chargerConfiguration("configs/config_rules.json");
        std::cout << "[SYSTEME] Configuration chargée." << std::endl;
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

    // Afficher les factions disponibles
    std::cout << "\nFactions disponibles :" << std::endl;
    for (const auto& [nom, fp] : moteur.getFactionsAvailable()) {
        std::cout << "  - " << nom << std::endl;
    }

    for (int i = 0; i < nbJoueurs; ++i) {
        std::string nom, faction;
        std::cout << "\nNom du joueur " << (i + 1) << " : ";
        std::cin >> nom;
        std::cout << "Faction pour " << nom << " : ";
        std::cin >> faction;
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
    // 4. Initialisation du jeu
    // -------------------------------------------------------
    try {
        moteur.initGame(seed, noms, factions);
        std::cout << "[SYSTEME] Partie initialisée (graine : " << seed << ")." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de l'initialisation : " << e.what() << std::endl;
        return 1;
    }

    // -------------------------------------------------------
    // 5. Choix des conditions de victoire
    // -------------------------------------------------------
    std::cout << "\nIndex des conditions de victoire (0 par défaut) : ";
    int victorySet = 0;
    std::cin >> victorySet;
    moteur.setActiveVictorySet(victorySet);

    // -------------------------------------------------------
    // 6. Boucle de jeu
    // -------------------------------------------------------
    afficherSeparateur("DÉBUT DE LA PARTIE");

    while (!moteur.isPartieTerminee()) {

        int pIdx = moteur.getCurrentPlayerTurn();
        afficherEtatJoueur(moteur, pIdx);

        std::cout << "\n1. Voir la carte" << std::endl;
        std::cout << "2. Faire une action" << std::endl;
        std::cout << "3. Voir mes unités (détail)" << std::endl;
        std::cout << "4. Voir mes villes (détail)" << std::endl;
        std::cout << "5. Déclarer forfait" << std::endl;
        std::cout << "Choix : ";

        int choixMenu = 0;
        std::cin >> choixMenu;

        // Sécurité : si cin échoue (ex: entrée non entière), réinitialiser
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "[ERREUR] Entrée invalide." << std::endl;
            continue;
        }

        if (choixMenu == 1) {
            // Affichage plateau
            const board* plateau = moteur.getPlateau();
            if (plateau) plateau->affichage();

        } else if (choixMenu == 2) {
            // Sous-menu actions
            bool finTour = menuAction(moteur, pIdx);
            (void)finTour; // le moteur gère lui-même le changement de joueur

        } else if (choixMenu == 3) {
            // Détail unités
            const auto& j = moteur.getJoueurs()[pIdx];
            std::cout << "\n--- VOS UNITÉS ---" << std::endl;
            for (auto* u : j.getUnites()) {
                if (u) {
                    std::cout << "\n";
                    u->affiche();
                }
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
            // Forfait
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
        std::cout << "\n★★★ FÉLICITATIONS ★★★" << std::endl;
        std::cout << moteur.getNomVainqueur() << " REMPORTE LA PARTIE !" << std::endl;
    } else {
        std::cout << "\nLa partie s'est terminée sans vainqueur." << std::endl;
    }

    std::cout << "\nTours joués : " << moteur.getTourActuel() << std::endl;
    std::cout << "\nMerci d'avoir joué à Space Wargames !" << std::endl;

    return 0;
}