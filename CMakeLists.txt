cmake_minimum_required(VERSION 3.20)
project(SFML_Example CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ruta al entorno ucrt64 de MSYS2
set(MSYS2_UCRT64 "C:/msys64/ucrt64")

# Asegura que CMake busque las libs correctas
set(CMAKE_PREFIX_PATH "${MSYS2_UCRT64}")

# Encuentra SFML (usa las libs ya instaladas desde pacman)
find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

# Directorios donde estan los .h
include_directories(include)

# Busca Automaticamente todos los .cpp en src
file(GLOB SRC_FILES src/*.cpp)

# main.cpp está fuera de src/
set(MAIN_FILE main.cpp)

# Ejecutable
add_executable(aed_sfml
    ${MAIN_FILE}
    ${SRC_FILES}
)

# Linkeo de SFML
target_link_libraries(aed_sfml
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
)
