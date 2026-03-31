cmake_minimum_required(VERSION 3.16)

project(unite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(unite
    main.cpp
    unite.hh
    unite.cc
    rank.hh
    rank.cc
    orientation.hh
    orientation.cc
    comportement.hh
    comportement.cc


    ../jeu/jeu.cc
    ../jeu/jeu.hh
)

target_include_directories(unite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}../jeu
)

include(GNUInstallDirs)
install(TARGETS unite
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
