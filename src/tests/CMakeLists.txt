add_executable(
    test_bitboard
    test_bitboard.c
)
target_link_libraries(test_bitboard ${LIB_NAME})

add_executable(
    test_eval
    test_eval.c
)
target_link_libraries(test_eval ${LIB_NAME})

add_executable(
    test_moves
    test_moves.c
)
target_link_libraries(test_moves ${LIB_NAME})

add_executable(
    test_performance
    test_performance.c
)
target_link_libraries(test_performance ${LIB_NAME})

add_executable(
    test_perft
    test_perft.c
)
target_link_libraries(test_perft ${LIB_NAME})

add_executable(
    test_polyglot
    test_polyglot.c
)
target_link_libraries(test_polyglot ${LIB_NAME})

add_executable(
    test_see
    test_see.c
)
target_link_libraries(test_see ${LIB_NAME})

add_executable(
    test_state
    test_state.c
)
target_link_libraries(test_state ${LIB_NAME})

