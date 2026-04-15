#include "moteur.hh"
#include <cstdlib>

GameManager::GameManager(board & b, Arbitre & a, GameConfig & c, UniteFactory & f):
    _plateau(b),
    _joueurs(), // Liste Vide
    _arbitre(a),
    _config(c),
    _factory(f),
    _indexJoueurActuel(0)
{}

void GameManager::ajouterJoueur(std::unique_ptr<Joueur> j)
{
    _joueurs.push_back(std::move(j));
}

// Vérifie S'il y a un gagant dans les joueurs
int joueurVictorieux(const std::vector<std::unique_ptr<Joueur>> & liste_joueur, const Arbitre & a, const GameConfig & c)
{
    for(size_t i = 0; i < liste_joueur.size(); ++i)
    {
        if(a.verifierVictoire(*(liste_joueur.at(i)), c))
        {
            return static_cast<int>(i); // Gagnant trouver
        }
    }
    return -1; // Pas de Gagnant le jeu continue
}

void GameManager::lancerPartie()
{
    if(_joueurs.empty())
    {
        std::cout<<"STOP !!! Pas de joueur pour jouer !"<<std::endl;
        return;
    }

    int nb_joueurs = _joueurs.size();

    _indexJoueurActuel = std::rand() % nb_joueurs; // Choix aléatoir du joueur qui commence

    while(joueurVictorieux(_joueurs, _arbitre, _config) == -1)
    {
        Joueur &joueurActuel = *(_joueurs.at(_indexJoueurActuel));
        
        //Execution du Tour du joueur
        executerTour(joueurActuel, _arbitre);

        //Changement de joueur
        if(_indexJoueurActuel + 1 == nb_joueurs)
        {
            _indexJoueurActuel = 0;
        }
        else
        {
            ++ _indexJoueurActuel;
        }
    }
    int gagnant = joueurVictorieux(_joueurs, _arbitre, _config);
    std::cout << "La partie est finie ! Victoire de : " << _joueurs.at(gagnant)->getName() << std::endl;
}

void GameManager::executerTour(Joueur & j, const Arbitre & a)
{
    // Récupération de ressource
    for(City* ville : j.getCities()) 
    {
        ville->product(j);
    }
    
    //Reste Buff et point d'action
    for(auto uni : j.getUnites())
    {
        uni->resetTemporary_stats();
        uni->setPoint_action(uni->point_action_max());
    }

    bool fin_tour = false;
    int choix = -1;


    while(fin_tour != true)
    {
        std::cout << "\n========================================" << std::endl;
        std::cout << "       Tour du Joueur "<< j.getName() << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << " 1. -- Construction de Ville -- " << std::endl;
        std::cout << " 2. -- Amélioration de Ville -- " << std::endl;
        std::cout << " 3. -- Création de Unite -- " << std::endl;
        std::cout << " 4. -- Combat -- " << std::endl;
        std::cout << " 5. -- Mise en Position de Defense -- " << std::endl;
        std::cout << " 6. -- Stopper Position de Defense -- " << std::endl;
        std::cout << " 7. -- Transporter Troupe -- " << std::endl;
        std::cout << " 8. -- Decharger Troupe -- " << std::endl;
        std::cout << " 9. -- Mise sous Commandement -- " << std::endl;
        std::cout << " 10. -- Cammoufler -- " << std::endl;
        std::cout << " 11. -- Fin de Tour -- " << std::endl;
        std::cout << "Choix : ";

        if (!(std::cin >> choix)) 
        {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }

        int x, y, targetX, targetY; // Variables pour les coordonnées

        switch(choix) 
        {
            //Construction de Ville
            case 1: 
            {
                break;
            }

            //Amélioration de Ville
            case 2: 
            { 
                break;
            }

            //Création de Unite
            case 3: 
            { 
                std::cout << "--- RECRUTEMENT ---" << std::endl;
    
                const auto& faction = j.getFaction();
                if (!faction) 
                {
                    std::cout << "Erreur : Le joueur n'a pas de faction !" << std::endl;
                    break;
                }

                std::cout << "Unites disponibles pour " << faction->nom << " :" << std::endl;
                for (size_t i = 0; i < faction->unites_disponibles.size(); ++i) 
                {
                    std::cout << i << ". " << faction->unites_disponibles[i] << std::endl;
                }

                int choixUnite;
                std::cout << "Votre choix : "; 
                std::cin >> choixUnite;

                if (choixUnite >= 0 && choixUnite < (int)faction->unites_disponibles.size()) 
                {
                    std::string nomType = faction->unites_disponibles[choixUnite];
                    
                    std::cout << "Coord de deploiement (x y) : ";
                    std::cin >> x >> y;

                    if (a.coordValid(x, y, _plateau) && _plateau.getUnite(x, y) == nullptr) 
                    {

                        std::shared_ptr<Unite> nouvelleUnite = _factory.create(nomType); 

                        if (nouvelleUnite)
                        {
                            if (a.peutRecruterUnite(j, nouvelleUnite->cout(), *nouvelleUnite)) 
                            {
                                j.payer(nouvelleUnite->cout());      
                                
                                nouvelleUnite->setLocation({x, y});
                                j.ajouterUnite(nouvelleUnite.get()); 
                                _plateau.placerUnite(x, y, nouvelleUnite); 
                                
                                std::cout << "[SUCCES] " << nomType << " recrute !" << std::endl;
                            } 
                            else std::cout << "[REFUS] Fonds insuffisants." << std::endl;
                        }
                        else std::cout << "Erreur : Cette unite n'existe pas dans le catalogue." << std::endl;
                    }
                    else std::cout << "[REFUS] Case invalide ou deja occupee." << std::endl;
                }
                break;
            }

            //Combat
            case 4: 
            {   
                std::cout << "Coord Unite active (x y) : "; std::cin >> x >> y;
                std::cout << "Coord Cible (x y) : "; std::cin >> targetX >> targetY;
                
                Unite* active = _plateau.getUnite(x, y);
                Unite* cible = _plateau.getUnite(targetX, targetY);

                if (active && cible) 
                {
                    std::cout << "Action : 1. Attaquer | 2. Soigner : ";
                    int sousChoix; std::cin >> sousChoix;

                    //Attaque
                    if (sousChoix == 1)
                    {
                        auto attaques = active->Offensive();
                        if (attaques.empty()) 
                        {
                            std::cout << "Cette unite n'a aucune competence d'offensive." << std::endl;
                        } 
                        else
                        {
                            std::vector<CompAtt*> vAtt(attaques.begin(), attaques.end());
                            for (size_t i = 0; i < vAtt.size(); ++i) 
                            {
                                std::cout << i << ". "; vAtt[i]->affiche();
                            }

                            std::cout << "Choisissez l'attaque : ";
                            int iAtt; std::cin >> iAtt;

                            if (iAtt >= 0 && iAtt < (int)vAtt.size()) 
                            {
                                if (a.peutAttaquer(j, *active, *cible, vAtt[iAtt])) 
                                {
                                    j.Attaquer(*active, *cible, vAtt[iAtt]);
                                    std::cout << "[SUCCES] Attaque effectuee !" << std::endl;
                                } 
                                else 
                                {
                                    std::cout << "[REFUS] L'arbitre refuse l'attaque (PA insuffisants ou cible invalide)." << std::endl;
                                }
                            }
                        }
                    }

                    //Soin
                    else if (sousChoix == 2)
                    {
                        auto soins = active->Soin();
                        if (soins.empty()) 
                        {
                            std::cout << "Cette unite ne peut pas soigner." << std::endl;
                        } 
                        else 
                        {
                            std::vector<CompSoin*> vSoin(soins.begin(), soins.end());
                            for (size_t i = 0; i < vSoin.size(); ++i) 
                            {
                                std::cout << i << ". "; vSoin[i]->affiche();
                            }

                            std::cout << "Choisissez le soin : ";
                            int iSoin; std::cin >> iSoin;

                            if (iSoin >= 0 && iSoin < (int)vSoin.size()) 
                            {
                                if (a.peutSoigner(j, *active, *cible, vSoin[iSoin])) 
                                {
                                    j.Soigner(*active, *cible, vSoin[iSoin]);
                                    std::cout << "[SUCCES] Soin effectue !" << std::endl;
                                } 
                                else 
                                {
                                    std::cout << "[REFUS] L'arbitre refuse le soin." << std::endl;
                                }
                            }
                        }
                    }
                } 
                else 
                {
                    std::cout << "Case vide ou coordonnees invalides." << std::endl;
                }
                break;
            }

            //Mise en Position de Defense
            case 5: 
            { 
                std::cout << "Coord Unite (x y) : "; std::cin >> x >> y;
                Unite* u = _plateau.getUnite(x, y);
                if (u && a.appartientJoueur(j, *u)) 
                {
                    j.ChangerPositionDefensive(*u);
                    std::cout << "Position defensive activee." << std::endl;
                }
                break;
            }

            //Stopper Position de Defense
            case 6: 
            {
                std::cout << "Coord Unite (x y) : "; std::cin >> x >> y;
                Unite* u = _plateau.getUnite(x, y);
                if (u && a.appartientJoueur(j, *u) && u->defensif()) 
                {
                    j.ChangerPositionDefensive(*u);
                    std::cout << "Position defensive stoppee." << std::endl;
                }
                break;
            }

            //Transporter Troupe
            case 7: 
            {
                std::cout << "Coord Transporteur (x y) : "; std::cin >> x >> y;
                std::cout << "Coord Passager (x y) : "; std::cin >> targetX >> targetY;
                Unite* trans = _plateau.getUnite(x, y);
                Unite* pass = _plateau.getUnite(targetX, targetY);

                if (trans && pass && a.peutTransporter(j, *trans)) { //
                    j.Transporter(*trans, *pass); //
                    std::cout << "Unite embarquee." << std::endl;
                }
                break;
            }
                
            //Decharger Troupe
            case 8: 
            {
                std::cout << "Coord Transporteur (x y) : "; std::cin >> x >> y;
                std::cout << "Coord Destination (x y) : "; std::cin >> targetX >> targetY;
                Unite* trans = _plateau.getUnite(x, y);
                // Note : il faudrait une logique pour choisir quel passager décharger
                break;
            }
                
            //Mise sous Commandement
            case 9: 
            {
                std::cout << "Coord Commandant (x y) : "; std::cin >> x >> y;
                std::cout << "Coord Recrue (x y) : "; std::cin >> targetX >> targetY;
                Unite* com = _plateau.getUnite(x, y);
                Unite* reg = _plateau.getUnite(targetX, targetY);

                if(com && reg && a.peutRejoindreCommandant(j, *com, *reg))
                {
                    j.RejoindreCommandant(*com, *reg);
                    std::cout << "Unite sous commandement." << std::endl;
                }
                break;
            }

            //Cammoufler
            case 10: 
            {
                std::cout << "Coord Unite (x y) : "; std::cin >> x >> y;
                Unite* u = _plateau.getUnite(x, y);
                if (u && a.peutActiverCamouflage(j, *u)) 
                {
                    j.ActiverCamouflage(*u);
                    std::cout << "Camouflage active." << std::endl;
                }
                break;
            }

            case 11:
                fin_tour = true;
                break;

            default: 
                std::cout << "Choix invalide." << std::endl;
                break;
        }
    }
}