add_executable(
    unittest_bitboard
    test_bitboard.c
    ../engine/bitboard.h
)

target_link_libraries(unittest_bitboard pawnedengine)
