cmake_minimum_required(VERSION 3.10)

# Project name and version
project(PuertoRicoBot VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add source files
set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/game.cpp
    ${CMAKE_SOURCE_DIR}/src/mayor.cpp
    ${CMAKE_SOURCE_DIR}/src/craftsman.cpp
    ${CMAKE_SOURCE_DIR}/src/settler.cpp
    ${CMAKE_SOURCE_DIR}/src/builder.cpp
    ${CMAKE_SOURCE_DIR}/src/captain.cpp
    ${CMAKE_SOURCE_DIR}/src/trader.cpp
    ${CMAKE_SOURCE_DIR}/src/prospector.cpp
    ${CMAKE_SOURCE_DIR}/src/integrity_checker.cpp
    ${CMAKE_SOURCE_DIR}/src/basic_heuristic.cpp
    ${CMAKE_SOURCE_DIR}/src/console_strategy.cpp
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

# Add executable
add_executable(main ${SOURCES})

# Add compile options
target_compile_options(main PRIVATE
    -g
    -fdiagnostics-color=always
    -Wall
)

# Add optimized build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# Set output directory
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)