cmake_minimum_required(VERSION 3.16)

project(Projet_L3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Projet_L3 main.cpp unite_base.hh unite_base.cc
    unite_complexe.hh unite_complexe.cc)

include(GNUInstallDirs)
install(TARGETS Projet_L3
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
