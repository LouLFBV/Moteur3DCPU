cmake_minimum_required(VERSION 3.16)
project(CpuRenderer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. On déclare ton exécutable
add_executable(CpuRenderer main.cpp)

# 2. Chemins absolus vers les includes et les libs
target_include_directories(CpuRenderer PRIVATE "${CMAKE_SOURCE_DIR}/external/include")
target_link_directories(CpuRenderer PRIVATE "${CMAKE_SOURCE_DIR}/external/lib")

# 3. Liaison de la bibliothèque statique SDL3
target_link_libraries(CpuRenderer PRIVATE 
    SDL3-static 
    winmm 
    setupapi 
    version 
    imm32 
    cfgmgr32
)