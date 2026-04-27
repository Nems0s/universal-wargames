#pragma once
#include "jeu.hh"
#include <variant>
#include <string>

struct CmdFonderVille { int x, y; std::string nomVille; };
struct CmdAcheterCase { int x, y; };
struct CmdAmeliorer { int x, y; };
struct CmdDeplacement { int xSrc, ySrc, xDest, yDest; };
struct CmdAttaque { int xSrc, ySrc, xDest, yDest; };
struct CmdRotation { int x, y; direction dir; };
struct CmdRecrutement { int x, y; std::string nomUnite; };
struct CmdConstruction { int x, y; std::string nomBatiment; };
struct CmdSoigner { int xSrc, ySrc, xDest, yDest; };
struct CmdCamoufler { int x, y; };
struct CmdCharger { int xPassager, yPassager, xTransport, yTransport; };
struct CmdDecharger { int xTransport, yTransport, indexPassager, xDest, yDest; };
struct CmdEnroler { int xCommandant, yCommandant, xRecrue, yRecrue; };
struct CmdDetruireUnite { int x, y; };
struct CmdFinTour {};

    // ResultatAction actionModifierVision(Joueur& j, const Action& action);
    // ResultatAction actionDebutDefense(Joueur& j, const Action& a);
    // ResultatAction actionArretDefense(Joueur& j, const Action& a);
    // ResultatAction actionDesenrolement(Joueur& j, const Action& a);
    

// variant : contient un seule des structures suivantes
using CommandeJeu = std::variant<
    CmdFonderVille, 
    CmdAcheterCase, 
    CmdAmeliorer,
    CmdDeplacement, 
    CmdAttaque, 
    CmdRotation, 
    CmdRecrutement, 
    CmdConstruction,
    CmdSoigner,
    CmdCamoufler,
    CmdCharger,
    CmdDecharger,
    CmdEnroler,
    CmdDetruireUnite,
    CmdFinTour
>;

enum class ResultatAction {
    SUCCES,
    FIN_TOUR,
    ECHEC_FONDS_INSUFFISANTS,
    ECHEC_PA_INSUFFISANTS,
    ECHEC_COORD_INVALIDE,
    ECHEC_ARBITRE_REFUS
};