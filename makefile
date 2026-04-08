CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g -I./jeu -I./joueur -I./unite -I./combat

BUILD_DIR = build
BIN_DIR = bin

# Sources de chaque module
SRC_JEU = $(filter-out jeu/main.cc, $(wildcard jeu/*.cc))
SRC_JOUEUR = $(wildcard joueur/*.cc)
SRC_UNITE = $(filter-out unite/main.cpp, $(wildcard unite/*.cc))
SRC_COMBAT = $(wildcard combat/*.cc)
SRC_MAIN = main.cc

SRC = $(SRC_MAIN) $(SRC_JEU) $(SRC_JOUEUR) $(SRC_UNITE) $(SRC_COMBAT)

# Transforme les chemins en objets dans build/
OBJ = $(patsubst %.cc, $(BUILD_DIR)/%.o, $(SRC))

EXEC = $(BIN_DIR)/space-wargames

all: $(EXEC)

$(EXEC): $(OBJ) | $(BIN_DIR)
	$(CXX) $(OBJ) -o $(EXEC)

# Règle générique : compile tout .cc en gardant la structure de dossiers dans build/
$(BUILD_DIR)/%.o: %.cc
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

clean:
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@if exist "$(BIN_DIR)" rmdir /s /q "$(BIN_DIR)"

.PHONY: all clean