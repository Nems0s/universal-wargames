CXX = g++
# Ajout de -I./UI pour tes headers de rendu et ImGui
CXXFLAGS = -Wall -Wextra -std=c++17 -g -I./jeu -I./joueur -I./unite -I./combat -I./lib -I./UI -I./UI/imgui -I./UI/imgui-sfml -I./configs -I./arbitre

# Bibliothèques à lier (SFML et OpenGL)
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lGL

BUILD_DIR = build
BIN_DIR = bin

# On cherche tous les .cc et les .cpp (pour ImGui)
SRC_CC  = $(wildcard *.cc) $(wildcard jeu/*.cc) $(wildcard joueur/*.cc) \
          $(wildcard unite/*.cc) $(wildcard combat/*.cc) \
          $(wildcard configs/*.cc) $(wildcard arbitre/*.cc) $(wildcard UI/*.cc)

SRC_CPP = $(wildcard UI/imgui/*.cpp) $(wildcard UI/imgui-sfml/*.cpp)

# Transformation en fichiers .o
OBJ = $(patsubst %.cc, $(BUILD_DIR)/%.o, $(SRC_CC))
OBJ += $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC_CPP))

EXEC = $(BIN_DIR)/main

all: $(BUILD_DIR) $(BIN_DIR) $(EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LIBS)

# Compilation des fichiers .cc (ton code)
$(BUILD_DIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Règle spéciale pour les fichiers .cpp imgui
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean