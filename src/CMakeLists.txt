include_directories(engine)
add_subdirectory(engine)

if(BUILD_EXECUTABLE)
    add_subdirectory(interface)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
    add_subdirectory(tuning)
endif()
