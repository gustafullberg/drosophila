option(BUILD_EXECUTABLE "Build executable with Xboard interface" ON)
option(BUILD_TESTS "Build test binaries" OFF)

if(BUILD_EXECUTABLE)
set(TARGET_SUFFIX "" CACHE STRING "String to append to name of executables")
endif()
