CXX = g++
# On ajoute les dossiers au path d'inclusion (-I) pour que les #include "fichier.hh" fonctionnent
CXXFLAGS = -Wall -Wextra -std=c++17 -g -I./jeu -I./joueur

BUILD_DIR = build
BIN_DIR = bin

# On cherche tous les .cc dans les deux sous-dossiers
SRC_JEU = $(wildcard jeu/*.cc)
SRC_JOUEUR = $(wildcard joueur/*.cc)
SRC = $(SRC_JEU) $(SRC_JOUEUR)

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

# Règle générique pour compiler les fichiers de jeu/
$(BUILD_DIR)/%.o: jeu/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle générique pour compiler les fichiers de joueur/
$(BUILD_DIR)/%.o: joueur/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean