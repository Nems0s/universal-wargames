cmake_minimum_required(VERSION 3.16)

project(Unite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Unite
    main.cpp
    Core/unite.hh
    Core/unite.cc
    Core/rank.hh
    Core/rank.cc
    Core/orientation.hh
    Core/orientation.cc
    Core/comportement.hh
    Core/comportement.cc

    Game/joueur.hh
    Game/joueur.cc
)

target_include_directories(Unite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Game
)

include(GNUInstallDirs)
install(TARGETS Unite
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
