add_executable(
    tuner
    pgn.cpp
    pgn.hpp
    tuner.cpp
)
target_link_libraries(tuner ${LIB_NAME} m)
