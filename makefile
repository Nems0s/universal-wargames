CXX = g++
# On ajoute les dossiers au path d'inclusion (-I) pour que les #include "fichier.hh" fonctionnent
CXXFLAGS = -Wall -Wextra -std=c++17 -g -I./jeu -I./joueur -I./unite -I./combat -I./lib -I./arbitre -I./configs

BUILD_DIR = build
BIN_DIR = bin

# On cherche tous les .cc dans les sous-dossiers
SRC_JEU = $(wildcard jeu/*.cc)
SRC_JOUEUR = $(wildcard joueur/*.cc)
SRC_UNITE = $(wildcard unite/*.cc)
SRC_COMBAT = $(wildcard combat/*.cc)
SRC_ARBITRE = $(wildcard arbitre/*.cc)
SRC = main.cc $(SRC_JEU) $(SRC_JOUEUR) $(SRC_UNITE) $(SRC_COMBAT) $(SRC_ARBITRE)

# On transforme "jeu/main.cc" en "build/main.o"
OBJ = $(patsubst %.cc, $(BUILD_DIR)/%.o, $(notdir $(SRC)))

EXEC = $(BIN_DIR)/main

all: $(BUILD_DIR) $(BIN_DIR) $(EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC)

$(BUILD_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle générique pour compiler les fichiers de jeu/
$(BUILD_DIR)/%.o: jeu/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle générique pour compiler les fichiers de joueur/
$(BUILD_DIR)/%.o: joueur/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle générique pour compiler les fichiers de unite/
$(BUILD_DIR)/%.o: unite/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle générique pour compiler les fichiers de combat/
$(BUILD_DIR)/%.o: combat/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: arbitre/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean