cmake_minimum_required(VERSION 2.8)

project(pawned)

option(USE_QUIESCENCE "Use quiscence search" ON)
if(USE_QUIESCENCE)
    add_definitions(-DUSE_QUIESCENCE=1)
else()
    add_definitions(-DUSE_QUIESCENCE=0)
endif()

option(USE_TRANSPOSITION_TABLE "Use transposition table" ON)
if(USE_TRANSPOSITION_TABLE)
    add_definitions(-DUSE_TRANSPOSITION_TABLE=1)
else()
    add_definitions(-DUSE_TRANSPOSITION_TABLE=0)
endif()

option(USE_MOVE_ORDERING "Use move ordering" ON)
if(USE_MOVE_ORDERING)
    add_definitions(-DUSE_MOVE_ORDERING=1)
else()
    add_definitions(-DUSE_MOVE_ORDERING=0)
endif()

option(USE_NULL_MOVE "Use null move pruning" ON)
if(USE_NULL_MOVE)
    add_definitions(-DUSE_NULL_MOVE=1)
else()
    add_definitions(-DUSE_NULL_MOVE=0)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS "-pedantic -Wall")
elseif(MSVC)
    add_definitions(-Dinline=__inline)
endif()

include_directories(engine)
add_subdirectory(engine)
add_subdirectory(unittests)
add_subdirectory(interface)
