#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 
#include "moteur.hh"

namespace py = pybind11;

PYBIND11_MODULE(wargame_env, m) {
    m.doc() = "Binding Python pour le Moteur de Jeu Space Wargames";

    // On expose la Config pour que Python lise la taille de la carte
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
            std::vector<std::string> catalogueUnites;
            for (const auto& pair : self.getUniteFactory().getCatalogue()) catalogueUnites.push_back(pair.first);

            std::vector<std::string> catalogueBatiments;
            for (const auto& pair : self.getBatimentFactory().getCatalogue()) catalogueBatiments.push_back(pair.first);

            std::vector<std::string> catalogueVilles;
            for (const auto& pair : self.getCityFactory().getCatalogue()) catalogueVilles.push_back(pair.first);

            // Sécurité anti-crash si json vide
            if(catalogueUnites.empty()) catalogueUnites.push_back("Dummy");
            if(catalogueBatiments.empty()) catalogueBatiments.push_back("Dummy");
            if(catalogueVilles.empty()) catalogueVilles.push_back("Dummy");

            switch (typeAction) {
                case 0: cmd = CmdFonderVille{x1, y1, catalogueVilles[x2 % catalogueVilles.size()]}; break;
                case 1: cmd = CmdAcheterCase{x1, y1}; break;
                case 2: cmd = CmdAmeliorer{x1, y1}; break;
                case 3: cmd = CmdDeplacement{x1, y1, x2, y2}; break;
                case 4: cmd = CmdAttaque{x1, y1, x2, y2}; break;
                case 5: cmd = CmdRotation{x1, y1, static_cast<direction>(x2 % 6)}; break; 
                case 6: cmd = CmdRecrutement{x1, y1, catalogueUnites[x2 % catalogueUnites.size()]}; break;
                case 7: cmd = CmdConstruction{x1, y1, catalogueBatiments[x2 % catalogueBatiments.size()]}; break;
                case 8: cmd = CmdSoigner{x1, y1, x2, y2}; break;
                case 9: cmd = CmdCamoufler{x1, y1}; break;
                case 10: cmd = CmdCharger{x1, y1, x2, y2}; break;
                case 11: { 
                    Unite* trans = plateau->getUnite(x1, y1);
                    if (!trans || !trans->Transport()) cmd = CmdFinTour{}; 
                    else cmd = CmdDecharger{x1, y1, x2 % 5, (x1 + 1) % rows, y1}; 
                    break;
                }
                case 12: cmd = CmdEnroler{x1, y1, x2, y2}; break;
                case 13: cmd = CmdDetruireUnite{x1, y1}; break;
                case 14: cmd = CmdFinTour{}; break;
                default: cmd = CmdFinTour{}; break;
            }
            return static_cast<int>(self.soumettreCommande(pIdx, cmd)); 
        })

        .def("get_state_ai", [](MoteurDeJeu& self, int pIdx) {
            const board* plateau = self.getPlateau();
            
            int max_x = self.getLogicConfig().getPlateauX();
            int max_y = self.getLogicConfig().getPlateauY();
            
            int channels = 4; // 4 Couches !
            std::vector<float> obs(channels * max_x * max_y, 0.0f);
            
            if (!plateau) return obs;

            int real_rows = plateau->getRows();
            int real_cols = plateau->getCols();
            int scan_rows = std::min(max_x, real_rows);
            int scan_cols = std::min(max_y, real_cols);
            int area = max_x * max_y;

            for (int r = 0; r < scan_rows; ++r) {
                for (int c = 0; c < scan_cols; ++c) {
                    int index_base = r * max_y + c; 
                    
                    // Couche 3: Terrain (Coût)
                    const hexa* cell = plateau->getCell(r, c);
                    if (cell) obs[3 * area + index_base] = static_cast<float>(cell->getCoutDeplacement());

                    // Couches 0, 1, 2: Unités
                    Unite* u = plateau->getUnite(r, c);
                    if (u) {
                        int uProp = self.getProprietaireUnite(r, c);
                        if (uProp == pIdx) {
                            obs[0 * area + index_base] = 1.0f; // Alliés
                            obs[2 * area + index_base] = static_cast<float>(u->health_point()) / u->health_point_max(); // PV
                        } else if (uProp != -1) {
                            obs[1 * area + index_base] = 1.0f; // Ennemis
                        }
                    }
                }
            }
            return obs; 
        });
}