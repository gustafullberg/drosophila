cmake_minimum_required(VERSION 2.8)

project(pawned)
add_definitions(-D_VERSION="1.0")

include(cmake/options.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
elseif(MSVC)
    add_definitions(-Dinline=__inline)
endif()

add_subdirectory(src)
if(BUILD_EXECUTABLE)
    add_subdirectory(data)
endif()

install(FILES README.md LICENSE.txt DESTINATION .)
