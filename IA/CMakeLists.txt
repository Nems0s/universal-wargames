cmake_minimum_required(VERSION 3.14)
project(WargameEnv)

# On impose le C++17 (ou 20 selon ce que tu utilises)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On demande à CMake de trouver pybind11 (installé via apt)
find_package(pybind11 REQUIRED)

# On demande à CMake de chercher automatiquement tous les .cc dans tes dossiers
file(GLOB SOURCES
    "../arbitre/*.cc"
    "../combat/*.cc"
    "../configs/*.cc"
    "../jeu/*.cc"
    "../joueur/*.cc"
    "../lib/*.cc"
    "../Unite/*.cc"
    "bindings.cc"
)

# Sécurité vitale : On retire les fichiers "main" de la liste
# car un module Python (.so) ne doit pas avoir de fonction int main() !
list(REMOVE_ITEM SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../main.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../main_console.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../main_gui.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../lib/NetworkManager.cc"
)

# On crée le module Python
pybind11_add_module(wargame_env ${SOURCES})

# Ajoute tes répertoires d'en-têtes (là où sont tes .hh)
target_include_directories(wargame_env PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../arbitre
    ${CMAKE_CURRENT_SOURCE_DIR}/../combat
    ${CMAKE_CURRENT_SOURCE_DIR}/../configs
    ${CMAKE_CURRENT_SOURCE_DIR}/../jeu
    ${CMAKE_CURRENT_SOURCE_DIR}/../joueur
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../Unite
)


# --- depuis le dossier IA ---#

# mkdir build
# cmake -B build -S .
# cmake --build build -j4
# python3 train_ai.py

# rm -rf build && cmake -B build -S . && cmake --build build -j6 && python3 train_ai.py