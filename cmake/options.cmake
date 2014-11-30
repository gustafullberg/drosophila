option(BUILD_EXECUTABLE "Build executable with Xboard interface" ON)
option(BUILD_TOOLS "Build tools" OFF)
option(BUILD_TESTS "Build test binaries" OFF)
option(USE_PAWN_STRUCTURE "Use pawn structure assessment" OFF)

if(USE_PAWN_STRUCTURE)
add_definitions(-DPAWN_STRUCTURE)
endif()

if(BUILD_EXECUTABLE)
set(TARGET_SUFFIX "" CACHE STRING "String to append to name of executables")
endif()
