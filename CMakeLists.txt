cmake_minimum_required(VERSION 2.8)

project(drosophila)
add_definitions(-D_VERSION="1.5.1")

include(cmake/options.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
    add_definitions(-Dinline=__inline)
endif()

set(PROGRAM_NAME drosophila)
set(LIB_NAME drosophilaengine)

find_package(Threads REQUIRED)

add_subdirectory(src)
if(BUILD_EXECUTABLE)
    add_subdirectory(data)
endif()

install(FILES README.md LICENSE.txt DESTINATION .)
