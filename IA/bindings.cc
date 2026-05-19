#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 
#include "../jeu/moteur.hh"

namespace py = pybind11;

PYBIND11_MODULE(wargame_env, m) {
    m.doc() = "Binding Python pour le Moteur de Jeu Space Wargames";

    // ------------------------------------------------------------------------
    // EXPOSITION DES STRUCTURES DE CONFIGURATION
    // ------------------------------------------------------------------------
    py::class_<GameConfigFiles>(m, "GameConfigFiles")
        .def(py::init<>())
        .def_readwrite("ressourcesPath", &GameConfigFiles::ressourcesPath)
        .def_readwrite("batimentsPath", &GameConfigFiles::batimentsPath)
        .def_readwrite("villesPath", &GameConfigFiles::villesPath)
        .def_readwrite("rulesPath", &GameConfigFiles::rulesPath)
        .def_readwrite("winsPath", &GameConfigFiles::winsPath)
        .def_readwrite("unitesPath", &GameConfigFiles::unitesPath)
        .def_readwrite("tuilesPath", &GameConfigFiles::tuilesPath);
        
    py::class_<GameConfig>(m, "GameConfig")
        .def("getPlateauX", &GameConfig::getPlateauX)
        .def("getPlateauY", &GameConfig::getPlateauY);

    
    // ------------------------------------------------------------------------
    // MOTEUR DE JEU ET LOGIQUE PRINCIPALE
    // ------------------------------------------------------------------------
    py::class_<MoteurDeJeu>(m, "MoteurDeJeu")
        .def(py::init<>()) 
        .def("chargerConfiguration", &MoteurDeJeu::chargerConfiguration)
        .def("initGame", &MoteurDeJeu::initGame)
        .def("passerTour", &MoteurDeJeu::passerTour)
        .def("getTourActuel", &MoteurDeJeu::getTourActuel)
        .def("getCurrentPlayerTurn", &MoteurDeJeu::getCurrentPlayerTurn)
        .def("isPartieTerminee", &MoteurDeJeu::isPartieTerminee)
        .def("setActiveVictorySet", &MoteurDeJeu::setActiveVictorySet)
        .def("getLogicConfig", &MoteurDeJeu::getLogicConfig, py::return_value_policy::reference)
        .def("getNomVainqueur", &MoteurDeJeu::getNomVainqueur)
        
        // --- FONCTION EXECUTION ACTION ---
        // Ne calcule aucune récompense, renvoie juste un code de statut à Python
        .def("step_ai", [](MoteurDeJeu& self, int pIdx, std::vector<int> action_array) {
            if (action_array.size() < 5) return -1; 
            int typeAction = action_array[0] % 15; 
            int x1 = action_array[1], y1 = action_array[2];
            int x2 = action_array[3], y2 = action_array[4];

            const board* plateau = self.getPlateau();
            if (!plateau || plateau->getRows() <= 0 || plateau->getCols() <= 0) return 5; 

            int rows = plateau->getRows();
            int cols = plateau->getCols();
            x1 = x1 % rows; x2 = x2 % rows;
            y1 = y1 % cols; y2 = y2 % cols;

            CommandeJeu cmd = CmdFinTour{}; 
            
            // Lecture dynamique des catalogues
            std::vector<std::string> catUnites, catBatiments, catVilles;
            for (const auto& pair : self.getUniteFactory().getCatalogue()) catUnites.push_back(pair.first);
            for (const auto& pair : self.getBatimentFactory().getCatalogue()) catBatiments.push_back(pair.first);
            for (const auto& pair : self.getCityFactory().getCatalogue()) catVilles.push_back(pair.first);

            // Sécurité anti-crash si json vide
            if(catUnites.empty()) catUnites.push_back("Dummy");
            if(catBatiments.empty()) catBatiments.push_back("Dummy");
            if(catVilles.empty()) catVilles.push_back("Dummy");

            switch (typeAction) {
                case 0: cmd = CmdFonderVille{x1, y1, catVilles[x2 % catVilles.size()]}; break;
                case 1: cmd = CmdAcheterCase{x1, y1}; break;
                case 2: cmd = CmdAmeliorer{x1, y1}; break;
                case 3: cmd = CmdDeplacement{x1, y1, x2, y2}; break;
                case 4: cmd = CmdAttaque{x1, y1, x2, y2}; break;
                case 5: cmd = CmdRotation{x1, y1, static_cast<direction>(x2 % 6)}; break; 
                case 6: cmd = CmdRecrutement{x1, y1, catUnites[x2 % catUnites.size()]}; break;
                case 7: cmd = CmdConstruction{x1, y1, catBatiments[x2 % catBatiments.size()]}; break;
                case 8: cmd = CmdSoigner{x1, y1, x2, y2}; break;
                case 9: cmd = CmdCamoufler{x1, y1}; break;
                case 10: cmd = CmdCharger{x1, y1, x2, y2}; break;
                case 11: { 
                    Unite* trans = plateau->getUnite(x1, y1);
                    if (!trans || !trans->Transport()) cmd = CmdFinTour{}; 
                    else cmd = CmdDecharger{x1, y1, x2 % 5, (x1 + 1) % rows, y1}; break;
                }
                case 12: cmd = CmdEnroler{x1, y1, x2, y2}; break;
                case 13: cmd = CmdDetruireUnite{x1, y1}; break;
                case 14: cmd = CmdFinTour{}; break;
            }
            return static_cast<int>(self.soumettreCommande(pIdx, cmd));
        })

        // --- FONCTION OBSERVATION (LES YEUX DE L'IA) ---
        // 8 Couches (Channels) pour une compréhension parfaite du jeu.
        
        // Couche 0 : Brouillard de guerre (0 = Inconnu, 1 = Découvert)
        // Couche 1 : Coût du terrain (pour qu'elle apprenne à contourner les montagnes)
        // Couche 2 : Présence Alliée (Où sont mes troupes ?)
        // Couche 3 : Points de Vie (PV) Alliés (Normalisés de 0.0 à 1.0)
        // Couche 4 : Points d'Action (PA) Alliés (Qui peut encore jouer ?)
        // Couche 5 : Présence Ennemie (Les cibles)
        // Couche 6 : PV Ennemis (Pour qu'elle apprenne à achever les faibles)
        // Couche 7 : Bâtiments et Villes (Ses objectifs stratégiques)
        
        .def("get_state_ai", [](MoteurDeJeu& self, int pIdx) {
            const board* plateau = self.getPlateau();
            int max_x = self.getLogicConfig().getPlateauX();
            int max_y = self.getLogicConfig().getPlateauY();
            
            int channels = 8; // Nombres de couches d'analyses
            std::vector<float> obs(channels * max_x * max_y, 0.0f);
            if (!plateau) return obs;

            int real_rows = plateau->getRows();
            int real_cols = plateau->getCols();
            int area = max_x * max_y;

            for (int r = 0; r < std::min(max_x, real_rows); ++r) {
                for (int c = 0; c < std::min(max_y, real_cols); ++c) {
                    int idx = r * max_y + c; 
                    
                    // COUCHE 0 : Brouillard de Guerre (Très important pour l'exploration)
                    // 1.0 si découvert, 0.0 si inconnu. Si inconnu, on ne remplit pas le reste !
                    bool decouvert = self.getJoueurs()[pIdx].estDecouvert(r, c);
                    obs[0 * area + idx] = decouvert ? 1.0f : 0.0f;
                    if (!decouvert) continue; 

                    // COUCHE 1 : Terrain (Coût de déplacement normalisé grossièrement)
                    const hexa* cell = plateau->getCell(r, c);
                    if (cell) obs[1 * area + idx] = std::min(1.0f, static_cast<float>(cell->getCoutDeplacement()) / 10.0f);

                    // Analyse des Unités
                    Unite* u = plateau->getUnite(r, c);
                    if (u) {
                        int uProp = self.getProprietaireUnite(r, c);
                        if (uProp == pIdx) {
                            obs[2 * area + idx] = 1.0f; // COUCHE 2 : Présence Unité Alliée
                            obs[3 * area + idx] = static_cast<float>(u->health_point()) / u->health_point_max(); // COUCHE 3 : PV Alliés
                            obs[4 * area + idx] = static_cast<float>(u->point_action()) / std::max(1, u->point_action_max()); // COUCHE 4 : PA Alliés
                        } else if (uProp != -1) {
                            obs[5 * area + idx] = 1.0f; // COUCHE 5 : Présence Unité Ennemie
                            obs[6 * area + idx] = static_cast<float>(u->health_point()) / u->health_point_max(); // COUCHE 6 : PV Ennemis
                        }
                    }

                    // Analyse des Villes / Bâtiments Spéciaux
                    const TuileConfigurable* tc = dynamic_cast<const TuileConfigurable*>(cell);
                    if (tc && tc->getCity()) {
                        // COUCHE 7 : Villes (1.0 = Alliée, -1.0 = Ennemie)
                        obs[7 * area + idx] = (tc->getProprietaire() == &self.getJoueurs()[pIdx]) ? 1.0f : -1.0f;
                        // Si c'est une capitale, on augmente l'intensité pour que l'IA la reconnaisse
                        if (tc->getCity()->estCapitale()) obs[7 * area + idx] *= 2.0f; 
                    }
                }
            }
            return obs; 
        });
}