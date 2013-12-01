add_executable(
    unittest_bitboard
    test_bitboard.c
)
target_link_libraries(unittest_bitboard pawnedengine)

add_executable(
    unittest_state
    test_state.c
)
target_link_libraries(unittest_state pawnedengine)

