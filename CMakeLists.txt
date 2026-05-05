cmake_minimum_required(VERSION 3.14)
project(WargameEnv)

# On impose le C++17 (ou 20 selon ce que tu utilises)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On demande à CMake de trouver pybind11 (installé via apt)
find_package(pybind11 REQUIRED)

# Liste ici tous tes fichiers sources C++ (sauf main_console.cc !)
# Le main_console.cc[cite: 1] est remplacé par Python.
set(SOURCES
    arbitre/arbitre.cc
    combat/combat.cc
    configs/config.cc
    jeu/batiment.cc
    jeu/city.cc
    jeu/jeu.cc
    jeu/moteur.cc
    jeu/ressource.cc
    joueur/joueur.cc
    Unite/comportement.cc
    Unite/orientation.cc
    Unite/rank.cc
    Unite/unite.cc
    bindings.cc
    # Ajoute les autres fichiers nécessaires : combat.cc, arbitre.cc, etc.
)

# On crée le module Python
pybind11_add_module(wargame_env ${SOURCES})

# Ajoute tes répertoires d'en-têtes si besoin (là où sont tes .hh)
target_include_directories(wargame_env PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/arbitre
    ${CMAKE_CURRENT_SOURCE_DIR}/combat
    ${CMAKE_CURRENT_SOURCE_DIR}/configs
    ${CMAKE_CURRENT_SOURCE_DIR}/jeu
    ${CMAKE_CURRENT_SOURCE_DIR}/joueur
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/Unite
)

# mkdir build
# cd build
# cmake ..
# make -j4
