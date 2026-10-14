cmake_minimum_required(VERSION 3.14)
project(CpuRenderer CXX)

# On impose un C++ moderne (C++17 ou C++20, parfait pour la suite)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Gestion automatique de SDL2 via FetchContent ---
include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.30.x  # On cible une version stable de SDL2
)

# On force SDL2 à se compiler discrètement sans faire de bruit inutile
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_SHARED ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL2)

# --- 2. Déclaration de ton exécutable ---
add_executable(CpuRenderer main.cpp)

# --- 3. Liaison avec SDL2 ---
# C'est ici qu'on dit à notre binaire qu'il a besoin de SDL2 pour vivre
target_link_libraries(CpuRenderer PRIVATE SDL2::SDL2main SDL2::SDL2)