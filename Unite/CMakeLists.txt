cmake_minimum_required(VERSION 3.16)

# On renomme le projet pour correspondre au nouveau dossier
project(Unite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# J'ai gardé "sunite_complexe.hh" comme dans ton exemple,
# mais vérifie s'il n'y a pas une faute de frappe (souvent c'est "unite_complexe.hh")
add_executable(Unite
    main.cpp
    unite_base.hh
    unite_base.cc
    unite_complexe.hh
    unite_complexe.cc
    rank.hh
    rank.cc
    orientation.hh orientation.cc

)

# Installation (optionnel, mais propre)
include(GNUInstallDirs)
install(TARGETS Unite
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
