cmake_minimum_required(VERSION 2.8)

project(pawned)
set(CPACK_PACKAGE_VERSION "1.0pre")

include(cmake/options.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()

if(${CMAKE_COMPILER_IS_GNUCC} OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
elseif(MSVC)
    add_definitions(-Dinline=__inline)
endif()

add_subdirectory(src)
if(BUILD_EXECUTABLE)
    add_subdirectory(data)
endif()

install(FILES README.md LICENSE.txt DESTINATION .)
include(CPack)
