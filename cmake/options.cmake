option(DISABLE_QUIESCENCE "Disable quiscence search" OFF)
if(DISABLE_QUIESCENCE)
    add_definitions(-DDISABLE_QUIESCENCE)
endif()

option(DISABLE_TRANSPOSITION_TABLE "Disable transposition table" OFF)
if(DISABLE_TRANSPOSITION_TABLE)
    add_definitions(-DDISABLE_TRANSPOSITION_TABLE)
endif()

option(DISABLE_MOVE_ORDERING "Disable move ordering" OFF)
if(DISABLE_MOVE_ORDERING)
    add_definitions(-DDISABLE_MOVE_ORDERING)
endif()

option(DISABLE_NULL_MOVE "Disable null move pruning" OFF)
if(DISABLE_NULL_MOVE)
    add_definitions(-DDISABLE_NULL_MOVE)
endif()

option(DISABLE_TIME_MANAGEMENT "Disable time management" OFF)
if(DISABLE_TIME_MANAGEMENT)
    add_definitions(-DDISABLE_TIME_MANAGEMENT)
endif()

option(DISABLE_OPENING_BOOK "Disable opening book" OFF)
if(DISABLE_OPENING_BOOK)
    add_definitions(-DDISABLE_OPENING_BOOK)
endif()

option(DISABLE_LATE_MOVE_REDUCTION "Disable late move reductions" OFF)
if(DISABLE_LATE_MOVE_REDUCTION)
    add_definitions(-DDISABLE_LATE_MOVE_REDUCTION)
endif()

option(DISABLE_HISTORY "Disable history (logging of past moves to avoid repetition moves)" OFF)
if(DISABLE_HISTORY)
    add_definitions(-DDISABLE_HISTORY)
endif()

option(BUILD_EXECUTABLE "Build executable with Xboard interface" ON)
option(BUILD_TOOLS "Build tools" OFF)
option(BUILD_TESTS "Build test binaries" OFF)

if(BUILD_EXECUTABLE)
set(TARGET_SUFFIX "" CACHE STRING "String to append to name of executables")
endif()
