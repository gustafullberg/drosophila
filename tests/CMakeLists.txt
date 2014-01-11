add_executable(
    test_bitboard
    test_bitboard.c
)
target_link_libraries(test_bitboard pawnedengine)

add_executable(
    test_moves
    test_moves.c
)
target_link_libraries(test_moves pawnedengine)

add_executable(
    test_perft
    test_perft.c
)
target_link_libraries(test_perft pawnedengine)

add_executable(
    test_search
    test_search.c
)
target_link_libraries(test_search pawnedengine)

add_executable(
    test_state
    test_state.c
)
target_link_libraries(test_state pawnedengine)

