#pragma once
#include "board.hh"
#include <variant>
#include <string>

struct CmdFonderVille { int x, y; std::string nomVille; };
struct CmdAcheterCase { int x, y; };
struct CmdAmeliorer { int x, y; };
struct CmdDeplacement { int xSrc, ySrc, xDest, yDest; };
struct CmdAttaque { int xSrc, ySrc, xDest, yDest, indexAtt; };
struct CmdRotation { int x, y; direction dir; };
struct CmdRecrutement { int x, y; std::string nomUnite; };
struct CmdConstruction { int x, y; std::string nomBatiment; };
struct CmdSoigner { int xSrc, ySrc, xDest, yDest, indexSoin; };
struct CmdCamoufler { int x, y; };
struct CmdCharger { int xPassager, yPassager, xTransport, yTransport; };
struct CmdDecharger { int xTransport, yTransport, indexPassager, xDest, yDest; };
struct CmdEnroler { int xCommandant, yCommandant, xRecrue, yRecrue; };
struct CmdDesenroler { int xCommandant, yCommandant, xRecrue, yRecrue; };
struct CmdChangerDefense { int x, y; };
struct CmdRavitaillerSurVille { int x, y; };
struct CmdRavitailler { int xSrc, ySrc, xDest, yDest; };
struct CmdDetruireUnite { int x, y; };
struct CmdFinTour {};

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
    CmdDesenroler,
    CmdChangerDefense,
    CmdRavitaillerSurVille,
    CmdRavitailler,
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