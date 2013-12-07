cmake_minimum_required(VERSION 2.8)

project(pawned)
option(USE_QUIESCENCE "Use quiscence search" ON)

if(USE_QUIESCENCE)
add_definitions(-DUSE_QUIESCENCE=1)
else(USE_QUIESCENCE)
add_definitions(-DUSE_QUIESCENCE=0)
endif(USE_QUIESCENCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_C_FLAGS "-Wpedantic -Wall")

include_directories(engine)
add_subdirectory(engine)
add_subdirectory(unittests)
add_subdirectory(interface)
