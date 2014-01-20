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

option(USE_TIME_MANAGEMENT "Use time management" ON)
if(USE_TIME_MANAGEMENT)
    add_definitions(-DUSE_TIME_MANAGEMENT=1)
else()
    add_definitions(-DUUSE_TIME_MANAGEMENT=0)
endif()

option(USE_OPENING_BOOK "Use opening book" ON)
if(USE_OPENING_BOOK)
    add_definitions(-DUSE_OPENING_BOOK=1)
else()
    add_definitions(-DUSE_OPENING_BOOK=0)
endif()

option(BUILD_TESTS "Build test binaries" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -pedantic -Wall")
elseif(MSVC)
    add_definitions(-Dinline=__inline)
endif()

include_directories(engine)
add_subdirectory(engine)
add_subdirectory(interface)
add_subdirectory(tools)

if(BUILD_TESTS)
add_subdirectory(tests)
endif()
