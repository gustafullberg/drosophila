cmake_minimum_required(VERSION 2.8)

project(pawned)
option(USE_QUIESCENCE "Use quiscence search" ON)
if(USE_QUIESCENCE)
add_definitions(-DUSE_QUIESCENCE=1)
else(USE_QUIESCENCE)
add_definitions(-DUSE_QUIESCENCE=0)
endif(USE_QUIESCENCE)

option(USE_TRANSPOSITION_TABLE "Use transposition table" ON)
if(USE_TRANSPOSITION_TABLE)
add_definitions(-DUSE_TRANSPOSITION_TABLE=1)
else(USE_TRANSPOSITION_TABLE)
add_definitions(-DUSE_TRANSPOSITION_TABLE=0)
endif(USE_TRANSPOSITION_TABLE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_C_FLAGS "-Wpedantic -Wall")

include_directories(engine)
add_subdirectory(engine)
add_subdirectory(unittests)
add_subdirectory(interface)
