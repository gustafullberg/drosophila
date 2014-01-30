cmake_minimum_required(VERSION 2.8)

project(pawned)
include(cmake/options.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -pedantic -Wall")
elseif(MSVC)
    add_definitions(-Dinline=__inline)
endif()

include_directories(engine)
add_subdirectory(engine)

if(BUILD_EXECUTABLE)
    add_subdirectory(interface)
endif()

if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
