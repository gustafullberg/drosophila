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
