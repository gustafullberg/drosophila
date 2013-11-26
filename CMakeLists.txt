cmake_minimum_required(VERSION 2.8)

project(pawned)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_C_FLAGS "-Wpedantic -Wall")

include_directories(engine)
add_subdirectory(engine)
add_subdirectory(unittests)
add_subdirectory(interface)
