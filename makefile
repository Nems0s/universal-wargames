CXX = g++

# Ajout de -I./UI pour tes headers de rendu et ImGui
CXXFLAGS = -Wall -Wextra -std=c++17 -g -I./jeu -I./joueur -I./unite -I./combat -I./lib -I./UI -I./UI/imgui -I./UI/imgui-sfml -I./configs -I./arbitre -I./gameloop

# Bibliothèques à lier (SFML et OpenGL)
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network -lGL

BUILD_DIR = build
BIN_DIR = bin

# On cherche tous les .cc et les .cpp (pour ImGui)
SRC_CC = 	$(wildcard jeu/*.cc) $(wildcard joueur/*.cc) \
        	$(wildcard unite/*.cc) $(wildcard combat/*.cc) \
            $(wildcard configs/*.cc) $(wildcard arbitre/*.cc) \
            $(wildcard UI/*.cc) $(wildcard lib/*.cc)
SRC_CPP = 	$(wildcard UI/imgui/*.cpp) $(wildcard UI/imgui-sfml/*.cpp)

# Transformation en fichiers .o
OBJ = $(patsubst %.cc, $(BUILD_DIR)/%.o, $(SRC_CC))
OBJ += $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC_CPP))

# Les 2 executables (terminal et gui)
EXEC_CLI = $(BIN_DIR)/wargame_cli
EXEC_GUI = $(BIN_DIR)/wargame_gui


# -- Raccourcis -- #

# par défaut : utiliser "make" pour tout compiler
all: $(BUILD_DIR) $(BIN_DIR) $(EXEC_CLI) $(EXEC_GUI)

# "make cli" pour compiler la version console
cli: $(EXEC_CLI)

# "make gui" pour compiler la version graphique
gui: $(EXEC_GUI)


# -- règles des builds -- #

# Règle pour la version Terminal
$(EXEC_CLI): $(OBJ) $(BUILD_DIR)/main_console.o | $(BIN_DIR)
	$(CXX) $(OBJ) $(BUILD_DIR)/main_console.o -o $(EXEC_CLI) $(LIBS)

# Règle pour la version Graphique
$(EXEC_GUI): $(OBJ) $(BUILD_DIR)/main_gui.o | $(BIN_DIR)
	$(CXX) $(OBJ) $(BUILD_DIR)/main_gui.o -o $(EXEC_GUI) $(LIBS)


# --- règles génériques --- #

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compilation des fichiers .cc (ton code)
$(BUILD_DIR)/%.o: %.cc | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle spéciale pour les fichiers .cpp imgui
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean cli gui

# Ajouter j suivi d'un nombre pour du multithreading
# make gui -j4


# --- Pour Debug --- #
# which gdb
# sudo apt install gdb
# lancé via mode debug de vs code

# fichier launch.json dans .vscode :
# {
#   "version": "0.2.0",
#   "configurations": [
#     {
#       "name": "Debug Space Wargames",
#       "type": "cppdbg",
#       "request": "launch",
#       "program": "${workspaceFolder}/bin/wargame_gui",
#       "args": [],
#       "stopAtEntry": false,
#       "cwd": "${workspaceFolder}",
#       "environment": [],
#       "externalConsole": false,
#       "MIMode": "gdb",
#       "miDebuggerPath": "/usr/bin/gdb",
#       "setupCommands": [
#           {
#               "description": "Arrêter sur les exceptions C++",
#               "text": "catch throw",
#               "ignoreFailures": false
#           }
#       ]
#     }
#   ]
# }