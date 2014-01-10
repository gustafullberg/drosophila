add_executable(
    unittest_bitboard
    test_bitboard.c
)
target_link_libraries(unittest_bitboard pawnedengine)

add_executable(
    unittest_moves
    test_moves.c
)
target_link_libraries(unittest_moves pawnedengine)

add_executable(
    unittest_perft
    test_perft.c
)
target_link_libraries(unittest_perft pawnedengine)

add_executable(
    unittest_search
    test_search.c
)
target_link_libraries(unittest_search pawnedengine)

add_executable(
    unittest_state
    test_state.c
)
target_link_libraries(unittest_state pawnedengine)

