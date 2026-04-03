cmake_minimum_required(VERSION 3.16)

project(space-wargames LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(space-wargames
    main.cc

    # Unite
    unite/unite.cc
    unite/unite.hh
    unite/comportement.cc
    unite/comportement.hh
    unite/rank.cc
    unite/rank.hh
    unite/orientation.cc
    unite/orientation.hh

    # Combat
    combat/combat.cc
    combat/combat.hh

    # Jeu
    jeu/jeu.cc
    jeu/jeu.hh
    jeu/batiment.cc
    jeu/batiment.hh
    jeu/city.cc
    jeu/city.hh
    jeu/ressource.cc
    jeu/ressource.hh

    # Joueur
    joueur/joueur.cc
    joueur/joueur.hh
)

target_include_directories(space-wargames PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/unite
    ${CMAKE_CURRENT_SOURCE_DIR}/combat
    ${CMAKE_CURRENT_SOURCE_DIR}/jeu
    ${CMAKE_CURRENT_SOURCE_DIR}/joueur
)

include(GNUInstallDirs)
install(TARGETS space-wargames
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
