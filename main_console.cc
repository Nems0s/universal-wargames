#include <iostream>
#include <memory>
#include <ctime>
#include <filesystem>
#include <map>
#include <vector>
#include <regex>

#include "jeu.hh"
#include "joueur.hh"
#include "arbitre.hh"
#include "moteur.hh"
#include "unite.hh"
#include "config.hh"

std::string trouverConfigs() 
{
    if (std::filesystem::exists("configs")) return "configs/";
    if (std::filesystem::exists("../configs")) return "../configs/";
    throw std::runtime_error("Dossier 'configs' introuvable !");
}

//Regex qui permet de récupérer tout les formes de YES et OUI
// std::regex pattern_validation(R"(^([yY][eE][sS] | ^[oO][uU][iI] | ^[oO] | ^[yY])$)"); //Première version avec o et y mis en brute

// Mais on peut rendre optionnel certain caractère avec ?
// std::regex pattern_validation(R"(^([yY]([eE][sS])? | ^[oO]([uU][iI])?)$)"); //Par contre obligé de mettre les maj et min

// Donc on peut utiliser le drapeau icase qui permet de ne plus distinguer min et maj 
std::regex pattern_validation(R"(^(yes|y|oui|o)$)", std::regex_constants::icase);

int main() {

    std::cout << "\n========================================" << std::endl;
    std::cout << "      Initialisation Du Jeu" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::string cfgDir = trouverConfigs();
    std::string fic;

    /*==================*/
    //Config Ressources
    /*==================*/
    std::cout << "\nFichier de configuration des ressources: ";
    std::cin >> fic;
    std::map<std::string, Ressource*> catalogueRessources;
    JsonRessourceReader resReader;
    try {
        resReader.load(cfgDir + fic + ".json", catalogueRessources);
        std::cout << "[SYSTEME] Ressources chargées." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Erreur critique Ressources : " << e.what() << std::endl;
        return 1;
    }

    /*=============*/
    //Config Règle
    /*=============*/
    std::cout << "\nFichier de configuration des règles: ";
    std::cin >> fic;
    GameConfig config;
    try {
        config.loadRules(cfgDir + fic + ".json");
        std::cout << "[SYSTEME] Règles chargées." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Erreur critique Règle : " << e.what() << std::endl;
        return 1;
    }

    /*=============*/
    //Config Monde
    /*=============*/
    std::cout << "\nFichier de configuration du monde: ";
    std::cin >> fic;
    WorldFactory world;
    JsonWorldReader worldReader;
    try {
        worldReader.chargerConfig(cfgDir + fic + ".json", catalogueRessources, world);
        world.initialiserBords();
        std::cout << "[SYSTEME] Monde chargés." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Erreur critique Monde : " << e.what() << std::endl;
        return 1;
    }

    /*=====================*/
    //Config Factory Unite
    /*=====================*/
    std::cout << "\nFichier de configuration des unités: ";
    std::cin >> fic;
    UniteFactory uniteFactory;
    JsonUniteReader uniteReader;
    try {
        uniteFactory.chargerConfiguration(cfgDir + fic + ".json", uniteReader, catalogueRessources);
        std::cout << "[SYSTEME] Unites chargées." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Erreur critique Unites : " << e.what() << std::endl;
        return 1;
    }

    /*=======================*/
    // Initialisation du jeu
    /*=======================*/
    board plateau(world, config); 
    Arbitre arbitre;

    GameManager moteur(plateau, arbitre, config, uniteFactory);

    /*======================*/
    // Création des joueurs 
    /*======================*/
    int nb_joueur=0;
    std::string nom_joueur;

    std::cout << "\nEntrez nombre de joueur: ";
    std::cin >> nb_joueur;
    for(int i=1; i <= nb_joueur; ++i)
    {
        std::cout << "Entrez nom du joueur "<<i<<": ";
        std::cin >> nom_joueur;

        auto j = std::make_unique<Joueur>(nom_joueur);
        moteur.ajouterJoueur(std::move(j));
    }
    

    /*======================*/
    // Gestion des Factions 
    /*======================*/
    for(auto& joueur : moteur.getJoueurs())
    {
        std::string choix_faction;
        int i=1;

        std::cout << "\nChoisir une faction pour " << joueur->getName() <<": ";
        for(auto& faction : config.get_AllFactions())
        {
            std::cout<<"\n" << i <<". "<< faction.first << " ";
            ++i;
        }
        std::cout <<"\nChoix (Mettre le nom): ";
        std::cin >> choix_faction;

        auto fact = config.getFaction(choix_faction);
        if(fact) 
        {
            joueur->setFaction(fact);
        }
    }
    

    /*======================*/
    // Ressources de départ  
    /*======================*/
    std::string ressources_depart;
    std::cout <<"\nVoulez vous donner des ressources de départ ? (Y/N): ";
    std::cin >> ressources_depart;

    if(std::regex_match(ressources_depart, pattern_validation)) // Vérifie que l'entrer correspond à la regex
    {
        for(auto const& [nom, resPtr] : catalogueRessources) 
        {
            int quantite = 0;
            std::cout <<"Quantité pour "<< nom << ": ";
            std::cin >> quantite;
            for(auto& joueur : moteur.getJoueurs())
            {
                joueur->ajouterRessource(resPtr, quantite);
            }
        }   
    }


    /*========================================================*/
    // Affichage du plateau pour voir ou mettre les capitales 
    /*========================================================*/

    plateau.affichage();


    /*=========================*/
    // Placement des Capitales
    /*=========================*/
    std::cout << "\nGénération aléatoire des Capitales" << std::endl;

    for(auto& joueur : moteur.getJoueurs())
    {
        bool placementValide = false;
        int x, y;

        while(!placementValide)
        {
            x = std::rand() % plateau.getRows(); 
            y = std::rand() % plateau.getCols();

            if(arbitre.coordValid(x, y, plateau)) //
            {
                const hexa* cell = plateau.getCell(x, y);
                TuileConfigurable* tuile = const_cast<TuileConfigurable*>(dynamic_cast<const TuileConfigurable*>(cell));

                if(tuile && tuile->peutConstrVille()) //
                {
                    tuile->constrVille(x, y, config, 5, true); 
                    City* capitale = tuile->getCity();
                    
                    if(capitale)
                    {
                        joueur->ajouterVille(capitale); //
                        placementValide = true;

                        std::cout << "[INFO] La capitale de " << joueur->getName() << " a été établie en (" << x << ", " << y << ")." << std::endl;
                    }
                }
            }
        }
    }






    /*==========================*/
    // Boucle de Jeu Principale
    /*==========================*/
    std::cout << "\n========================================" << std::endl;
    std::cout << "      Début de la Partie" << std::endl;
    std::cout << "========================================\n" << std::endl;
    moteur.lancerPartie(); // Initialise le premier tour
    bool jeuEnCours = true;

    while (jeuEnCours) 
    {
        const auto& listeJoueurs = moteur.getJoueurs();
        int indexActuel = moteur.getIndexJoueurActuel();
        Joueur& joueurActif = *(listeJoueurs.at(indexActuel));

        std::cout << "\n========================================" << std::endl;
        std::cout << "   TOUR DE : " << joueurActif.getName() << std::endl;
        std::cout << "========================================" << std::endl;
        

        std::cout << "\nRessources Actuelle : ";
        for(auto r : joueurActif.getInventaire())
        {
            std::cout << r.first->getName() << ": "<< r.second <<"  ";
        }

        int choixMenu;
        std::cout << "\n1. Voir la carte (Affichage plateau)" << std::endl;
        std::cout << "2. Faire une Action" << std::endl;
        std::cout << "3. Voir Armée" << std::endl;
        std::cout << "4. Voir Ville" << std::endl;
        std::cout << "5. Déclarer Forfait" << std::endl;
        std::cout << "Choix : ";
        std::cin >> choixMenu;

        if (choixMenu == 1) 
        {
            plateau.affichage();
        } 
        else if (choixMenu == 2) 
        {
            std::cout << "\n--- ACTIONS POSSIBLES ---" << std::endl;
            std::cout << " 1. Déplacer             |  2. Attaquer             |  3. Soigner" << std::endl;
            std::cout << " 4. Recruter             |  5. Construire Ville     |  6. Construire Bâtiment" << std::endl;
            std::cout << " 7. Améliorer Ville      |  8. Camoufler            |  9. Début Défense" << std::endl;
            std::cout << "10. Arrêt Défense        | 11. Chargement (Monter)  | 12. Déchargement (Sortir)" << std::endl;
            std::cout << "13. Enrôlement (Cdt)     | 14. Désenrôlement        | 15. FIN DE TOUR" << std::endl;
            
            int choixAction; 
            std::cout << "Choix de l'action : "; 
            std::cin >> choixAction;
            
            Action a;
            bool actionValide = true;

            switch(choixAction) 
            {
                case 1: // DEPLACER
                    std::cout << "\nDéplacement" << std::endl;
                    a.type = TypeAction::DEPLACER;
                    std::cout << "Coord. unité (x y) : ";
                    std::cin >> a.x1 >> a.y1;
                    std::cout << "Coord. destination (x y) : "; 
                    std::cin >> a.x2 >> a.y2;
                    break;
                case 2: // ATTAQUER
                    {
                        std::cout << "\nAttaque" << std::endl;
                        a.type = TypeAction::ATTAQUER;
                        std::cout << "Coord. attaquant (x y) : "; 
                        std::cin >> a.x1 >> a.y1;

                        Unite* att = plateau.getUnite(a.x1, a.y1);
                        if (att) 
                        {
                            auto listeAttaques = att->Offensive();
                            if (!listeAttaques.empty()) 
                            {
                                std::cout << "Attaques disponibles pour " << att->name() << " :" << std::endl;
                                int i = 0;
                                for (auto comp : listeAttaques) 
                                {
                                    std::cout << i << ". ";
                                    comp->affiche();
                                    i++;
                                }
                            } 
                            else 
                            {
                                std::cout << "Cette unité n'a aucune capacité offensive." << std::endl;
                                break;
                            }
                            std::cout << "Coord. cible (x y) : "; 
                            std::cin >> a.x2 >> a.y2;
                            std::cout << "Index de l'attaque : "; 
                            std::cin >> a.data;
                            break;
                        }
                        else
                        {
                            std::cout << "Coordonnées Invalide" << std::endl;
                            break;
                        }
                    }

                case 3: // SOIGNER
                    {
                        std::cout << "\nSoin" << std::endl;
                        std::cout << "Coord. soigneur (x y) : "; 
                        std::cin >> a.x1 >> a.y1;

                        Unite* soin = plateau.getUnite(a.x1, a.y1);
                        if(soin) 
                        {
                            auto listeSoins = soin->Soin();
                            if (!listeSoins.empty()) 
                            {
                                std::cout << "Attaques disponibles pour " << soin->name() << " :" << std::endl;
                                int i = 0;
                                for (auto comp : listeSoins) 
                                {
                                    std::cout << i << ". ";
                                    comp->affiche();
                                    i++;
                                }
                            } 
                            else
                            {
                                std::cout << "Cette unité n'a aucune capacité de soin." << std::endl;
                                break;
                            }
                            std::cout << "Coord. cible (x y) : "; 
                            std::cin >> a.x2 >> a.y2;
                            std::cout << "Index du soin : "; 
                            std::cin >> a.data;
                            break;
                        }
                        else
                        {
                            std::cout << "Coordonnées Invalide" << std::endl;
                            break;
                        }
                    }
                case 4: // RECRUTER
                    {                    
                        std::cout << "\nRecrutement" << std::endl;
                        a.type = TypeAction::RECRUTER_UNITE;
                        std::cout << "Coord. recrutement (x y) : "; 
                        std::cin >> a.x1 >> a.y1;

                        std::cin.ignore(10000, '\n');
                        
                        const FactionParams* faction = joueurActif.getFaction();
                        if (faction) 
                        {
                            for (const std::string& nomUnite : faction->unites_disponibles) 
                            {
                                std::cout << "Unité invocable : " << nomUnite << std::endl;
                            }
                        }
                        else std::cout << "Pas d'unité"<< std::endl;

                        std::cout << "Nom de l'unité : "; 
                        std::getline(std::cin, a.data); // Car certaine troupe on plusieur espace
                        break;
                    }
                case 5: // CONSTRUIRE VILLE
                    std::cout << "\nConstruction de Ville" << std::endl;
                    a.type = TypeAction::CONSTRUIRE_VILLE;
                    std::cout << "Coord. nouvelle ville (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    break;
                case 6: // CONSTRUIRE BATIMENT
                    std::cout << "\nConstruction de Batiment" << std::endl;
                    a.type = TypeAction::CONSTRUIRE_BATIMENT;
                    std::cout << "Coord. tuile (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    std::cout << "Nom du bâtiment : "; 
                    std::cin >> a.data;
                    break;
                case 7: // AMELIORER VILLE
                    std::cout << "\nAmélioration de Ville" << std::endl;
                    a.type = TypeAction::AMELIORER_VILLE;
                    std::cout << "Coord. ville (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    break;
                case 8: // CAMMOUFLER
                    std::cout << "\nCammouflage d'unité" << std::endl; 
                    a.type = TypeAction::CAMMOUFLER;
                    std::cout << "Coord. unité (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    break;
                case 9: // DEBUT_DEFENSSE
                    std::cout << "\nUnité mise en position defensive" << std::endl; 
                    a.type = TypeAction::DEBUT_DEFENSSE;
                    std::cout << "Coord. unité (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    break;
                case 10: // ARRET_DEFENSSE
                    std::cout << "\nUnité retirer de sa position defensive" << std::endl; 
                    a.type = TypeAction::ARRET_DEFENSSE;
                    std::cout << "Coord. unité (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    break;
                case 11: // CHARGEMENT
                    std::cout << "\nChargement d'unité dans un transporteur" << std::endl; 
                    a.type = TypeAction::CHARGEMENT;
                    std::cout << "Coord. transporteur (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    std::cout << "Coord. passager (x y) : "; 
                    std::cin >> a.x2 >> a.y2;
                    break;
                case 12: // DECHARGEMENT
                    std::cout << "\nDechargement d'unité dans un transporteur" << std::endl; 
                    a.type = TypeAction::DECHARGEMENT;
                    std::cout << "Coord. transporteur (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    std::cout << "Coord. arrivée (x y) : "; 
                    std::cin >> a.x2 >> a.y2;
                    std::cout << "Index de l'unité à sortir : "; 
                    std::cin >> a.data;
                    break;
                case 13: // ENROLEMENT
                    std::cout << "\nEnrolement d'une unité par un commandant" << std::endl; 
                    a.type = TypeAction::ENROLEMENT;
                    std::cout << "Coord. Commandant (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    std::cout << "Coord. Recrue (x y) : "; 
                    std::cin >> a.x2 >> a.y2;
                    break;
                case 14: // DESENROLEMENT
                    std::cout << "\nDesenrolement d'une unité par un commandant" << std::endl; 
                    a.type = TypeAction::DESENROLEMENT;
                    std::cout << "Coord. Commandant (x y) : "; 
                    std::cin >> a.x1 >> a.y1;
                    std::cout << "Coord. Recrue (x y) : "; 
                    std::cin >> a.x2 >> a.y2;
                    break;
                case 15: // FIN_TOUR
                    std::cout << "\nFin du tour de "<<joueurActif.getName() << std::endl; 
                    a.type = TypeAction::FIN_TOUR;
                    break;
                default:
                    std::cout << "Action inconnue." << std::endl;
                    actionValide = false;
            }

            if (actionValide) 
            {
                ResultatAction res = moteur.traiterAction(a);
                if (res == ResultatAction::FIN_TOUR) 
                {
                    std::cout << "Tour terminé." << std::endl;
                } 
                else if (res == ResultatAction::SUCCES) 
                {
                    std::cout << "[SUCCÈS] Action effectuée." << std::endl;
                } 
                else 
                {
                    std::cout << "[ÉCHEC] L'arbitre a refusé l'action (Ressources, PA ou portée)." << std::endl;
                }
            }
        }
        else if (choixMenu == 3) 
        {
            std::cout <<"\nVoici le liste de vos unités :"<<std::endl;

            for(auto i : joueurActif.getUnites())
            {
                std::cout <<"\n";
                i->affiche();
            }
        }
        else if (choixMenu == 4) 
        {
            std::cout <<"\nVoici le liste de vos villes :";
            for(auto i : joueurActif.getCities())
            {
               // Créer la fonction d'affichage pour les villes
            }
        }
        else if (choixMenu == 5) 
        {
            std::string forfait;
            std::cout <<"\nÊtes-vous sûr de déclarer forfait ? (Y/N): ";
            std::cin >> forfait;

            if(std::regex_match(forfait, pattern_validation))
            {
                std::cout << "Le joueur " << joueurActif.getName() << " a déclaré forfait !" << std::endl;
                jeuEnCours = false;
            }
        }

        for (auto& j : listeJoueurs) 
        {
            if (arbitre.verifierVictoire(*j, config)) 
            {
                std::cout << "\n****************************************" << std::endl;
                std::cout << " FÉLICITATIONS ! " << j->getName() << " GAGNE !" << std::endl;
                std::cout << "****************************************" << std::endl;
                jeuEnCours = false;
                break;
            }
        }
    }

    
    /*=========================*/
    // Nettoyage de la mémoire
    /*=========================*/
    for (auto const& [nom, res] : catalogueRessources) 
    {
        delete res;
    }
    catalogueRessources.clear();
    return 0;
}