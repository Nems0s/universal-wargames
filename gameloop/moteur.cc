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

void GameManager::passerAuJoueurSuivant() 
{
    if (_joueurs.empty()) return;

    _indexJoueurActuel = (_indexJoueurActuel + 1) % _joueurs.size();

    Joueur& suivant = *(_joueurs.at(_indexJoueurActuel));
    suivant.debutTour();
}

void GameManager::lancerPartie() 
{
    if (_joueurs.empty()) return;

    _indexJoueurActuel = std::rand() % _joueurs.size(); 

    Joueur& premier = *(_joueurs.at(_indexJoueurActuel));
    premier.debutTour();
}

ResultatAction GameManager::actionFinTour() 
{
    passerAuJoueurSuivant();
    return ResultatAction::FIN_TOUR;
}

ResultatAction GameManager::actionConstruireVille(Joueur& j, const Action& action) 
{
    if(!_arbitre.coordValid(action.x1, action.y1, _plateau)) return ResultatAction::ECHEC_COORD_INVALIDE;
    if(_arbitre.buildCity(j, _plateau, action.x1, action.y1)) 
    {
        City* c = new City(action.x1, action.y1, _config);
        j.ajouterVille(c);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionConstruireBatiment(Joueur& j, const Action& action) 
{
    // Faire la logique pour batiment
    return ResultatAction::SUCCES;
}

ResultatAction GameManager::actionAmeliorerVille(Joueur& j, const Action& action) 
{
    const hexa* cell = _plateau.getCell(action.x1, action.y1);
    const TuileConfigurable* tuile = dynamic_cast<const TuileConfigurable*>(cell);
    if(tuile && tuile->getCity() && _arbitre.peutAmeliorerVille(j, *(tuile->getCity()), _config)) 
    {
        tuile->getCity()->upgrade();
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionRecruterUnite(Joueur& j, const Action& action) 
{
    if(!_arbitre.coordValid(action.x1, action.y1, _plateau) || _plateau.getUnite(action.x1, action.y1) != nullptr)
        return ResultatAction::ECHEC_COORD_INVALIDE;

    std::shared_ptr<Unite> nouvelleUnite = _factory.create(action.data);
    if(!nouvelleUnite) return ResultatAction::ECHEC_ARBITRE_REFUS;

    if(_arbitre.peutRecruterUnite(j, nouvelleUnite->cout(), *nouvelleUnite)) 
    {
        j.payer(nouvelleUnite->cout());
        nouvelleUnite->setLocation({action.x1, action.y1});
        j.ajouterUnite(nouvelleUnite.get());
        _plateau.placerUnite(action.x1, action.y1, nouvelleUnite);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_FONDS_INSUFFISANTS;
}

ResultatAction GameManager::actionDeplacer(Joueur& j, const Action& action) 
{
    Unite* u = _plateau.getUnite(action.x1, action.y1);
    if(!u || !_arbitre.appartientJoueur(j, *u)) return ResultatAction::ECHEC_ARBITRE_REFUS;
    
    if(_arbitre.moveUnite(j, _plateau, *u, action.x2, action.y2)) 
    {
        if(_plateau.deplacerUnite(*u, action.x2, action.y2)) 
        {
            u->setLocation({action.x2, action.y2});
            u->setPoint_action(u->point_action() - 1);
            return ResultatAction::SUCCES;
        }
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionAttaquer(Joueur& j, const Action& action) 
{
    Unite* att = _plateau.getUnite(action.x1, action.y1);
    Unite* cible = _plateau.getUnite(action.x2, action.y2);
    if(!att || !cible) return ResultatAction::ECHEC_COORD_INVALIDE;

    auto attaques = att->Offensive();
    if(attaques.empty()) return ResultatAction::ECHEC_ARBITRE_REFUS;

    int index = 0;
    try{ index = std::stoi(action.data); } catch(...){ index = 0; }
    
    auto it = attaques.begin();
    std::advance(it, std::min((int)attaques.size() - 1, std::max(0, index)));

    if(_arbitre.peutAttaquer(j, *att, *cible, *it)) 
    {
        j.Attaquer(*att, *cible, *it);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionSoigner(Joueur& j, const Action& action) 
{
    Unite* healer = _plateau.getUnite(action.x1, action.y1);
    Unite* cible = _plateau.getUnite(action.x2, action.y2);
    if(!healer || !cible) return ResultatAction::ECHEC_COORD_INVALIDE;

    auto soins = healer->Soin();
    if(soins.empty()) return ResultatAction::ECHEC_ARBITRE_REFUS;

    int index = -1;
    try{ index = std::stoi(action.data); } catch(...){ index = -1; }

    auto it = soins.begin();
    std::advance(it, std::min((int)soins.size() - 1, std::max(0, index)));

    if(_arbitre.peutSoigner(j, *healer, *cible, *it)) 
    {
        j.Soigner(*healer, *cible, *it);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionCamoufler(Joueur& j, const Action& action) 
{
    Unite* u = _plateau.getUnite(action.x1, action.y1);
    if (u && _arbitre.peutActiverCamouflage(j, *u)) 
    {
        j.ActiverCamouflage(*u);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionDebutDefense(Joueur& j, const Action& action) 
{
    Unite* u = _plateau.getUnite(action.x1, action.y1);
    if(u && _arbitre.appartientJoueur(j, *u) && !u->defensif()) 
    {
        j.ChangerPositionDefensive(*u);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionArretDefense(Joueur& j, const Action& action) 
{
    Unite* u = _plateau.getUnite(action.x1, action.y1);
    if(u && _arbitre.appartientJoueur(j, *u) && u->defensif()) 
    {
        u->changerDefense();
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionChargement(Joueur& j, const Action& action) 
{
    Unite* trans = _plateau.getUnite(action.x1, action.y1);
    Unite* pass = _plateau.getUnite(action.x2, action.y2);
    if(trans && pass && _arbitre.peutTransporter(j, *trans)) 
    {
        j.Transporter(*trans, *pass);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionDechargement(Joueur& j, const Action& action) 
{
    Unite* trans = _plateau.getUnite(action.x1, action.y1);
    if(!trans || !trans->Transport()) return ResultatAction::ECHEC_COORD_INVALIDE;

    const auto& liste = trans->Transport()->liste_unite_transporter();
    int index = 0;
    try{ index = std::stoi(action.data); } catch(...){ index = 0; }
    
    if(index >= 0 && index < (int)liste.size()) 
    {
        auto it = liste.begin();
        std::advance(it, index);
        std::shared_ptr<Unite> pass = *it;

        if(_arbitre.peutDechargerTransport(j, *trans, *pass, action.x2, action.y2)) 
        {
            j.DechargerTransport(*trans, *pass, action.x2, action.y2);
            _plateau.placerUnite(action.x2, action.y2, pass);
            return ResultatAction::SUCCES;
        }
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionEnrolement(Joueur& j, const Action& action) 
{
    Unite* com = _plateau.getUnite(action.x1, action.y1);
    Unite* reg = _plateau.getUnite(action.x2, action.y2);
    if (com && reg && _arbitre.peutRejoindreCommandant(j, *com, *reg)) 
    {
        j.RejoindreCommandant(*com, *reg);
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}

ResultatAction GameManager::actionDesenrolement(Joueur& j, const Action& action) 
{
    Unite* com = _plateau.getUnite(action.x1, action.y1);
    Unite* reg = _plateau.getUnite(action.x2, action.y2);
    if (com && reg && _arbitre.peutRejoindreCommandant(j, *com, *reg)) 
    {
        j.QuitterCommandant(*com, *reg); // Assurez-vous que cette méthode existe dans Joueur !
        return ResultatAction::SUCCES;
    }
    return ResultatAction::ECHEC_ARBITRE_REFUS;
}


ResultatAction GameManager::traiterAction(const Action& action) 
{
    if(_joueurs.empty()) return ResultatAction::ECHEC_ARBITRE_REFUS;

    Joueur& j = *(_joueurs.at(_indexJoueurActuel));

    switch(action.type) 
    {
        case TypeAction::FIN_TOUR:            return actionFinTour();
        case TypeAction::CONSTRUIRE_VILLE:    return actionConstruireVille(j, action);
        case TypeAction::CONSTRUIRE_BATIMENT: return actionConstruireBatiment(j, action);
        case TypeAction::AMELIORER_VILLE:     return actionAmeliorerVille(j, action);
        case TypeAction::RECRUTER_UNITE:      return actionRecruterUnite(j, action);
        case TypeAction::DEPLACER:            return actionDeplacer(j, action);
        case TypeAction::ATTAQUER:            return actionAttaquer(j, action);
        case TypeAction::SOIGNER:             return actionSoigner(j, action);
        case TypeAction::CAMMOUFLER:          return actionCamoufler(j, action);
        case TypeAction::DEBUT_DEFENSSE:      return actionDebutDefense(j, action);
        case TypeAction::ARRET_DEFENSSE:      return actionArretDefense(j, action);
        case TypeAction::CHARGEMENT:          return actionChargement(j, action);
        case TypeAction::DECHARGEMENT:        return actionDechargement(j, action);
        case TypeAction::ENROLEMENT:          return actionEnrolement(j, action);
        case TypeAction::DESENROLEMENT:       return actionDesenrolement(j, action);
        default:                              return ResultatAction::ECHEC_ARBITRE_REFUS;
    }
}