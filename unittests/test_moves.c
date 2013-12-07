/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <assert.h>
#include "engine.h"
#include "defines.h"

void test_invalid_move()
{
    engine_state_t *engine;
    ENGINE_create(&engine);
    assert(ENGINE_apply_move(engine, D2, D3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, G8, F6, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E2, E3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D7, D5, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, B1, D2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, B8, C6, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, G1, F3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C8, F5, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, A2, A3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D8, D7, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, B2, B3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E8, C8, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C1, B2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D8, E8, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C2, C3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E7, E5, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, G2, G3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F8, D6, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F1, E2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F5, H3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, A3, A4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, H7, H6, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, B3, B4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, A7, A5, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, B4, B5, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C6, E7, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C3, C4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E5, E4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D3, E4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D5, E4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F3, D4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, H3, G2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, H1, G1, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D7, H3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E2, F1, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, G2, F1, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D2, F1, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D6, B4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F1, D2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, H3, H2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D4, E2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E8, D8, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, B2, D4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F6, G4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, G1, F1, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, G4, E5, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E2, C3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E5, D3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E1, E2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, H2, H5, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, G3, G4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, H5, G4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D2, F3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, B4, C3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D4, C3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E4, F3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E2, D2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D3, F2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C3, D4, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F2, D1, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, A1, D1, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, E7, F5, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, D2, C3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F5, G3, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C4, C5, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F3, F2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, B5, B6, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, G3, F1, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, B6, C7, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, G4, D1, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C7, D8, ENGINE_PROMOTION_ROOK) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, H8, D8, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C5, C6, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F1, D2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C6, B7, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C8, B7, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, C3, B2, ENGINE_PROMOTION_NONE) == ENGINE_RESULT_NONE);
    assert(ENGINE_apply_move(engine, F2, F1, ENGINE_PROMOTION_QUEEN) == ENGINE_RESULT_NONE);
    ENGINE_destroy(engine);
}

#if 0
void test_bitboard_count_bits()
{
    assert(bitboard_count_bits((bitboard_t)0xFFFFFFFFFFFFFFFF) == 64);
    assert(bitboard_count_bits((bitboard_t)0x0000000000000000) ==  0);
    assert(bitboard_count_bits((bitboard_t)0xAAAAAAAAAAAAAAAA) == 32);
    assert(bitboard_count_bits((bitboard_t)0x8000000000000001) ==  2);
    assert(bitboard_count_bits((bitboard_t)0x0000000000000001) ==  1);
    assert(bitboard_count_bits((bitboard_t)0x8000000000000000) ==  1);
    assert(bitboard_count_bits((bitboard_t)0x0000001000000000) ==  1);
}

void test_bitboard_find_bit()
{
    assert(bitboard_find_bit((bitboard_t)0xFFFFFFFFFFFFFFFF) ==  0);
    assert(bitboard_find_bit((bitboard_t)0xFFFFFFFFFFFFFFFE) ==  1);
    assert(bitboard_find_bit((bitboard_t)0x0000000000000001) ==  0);
    assert(bitboard_find_bit((bitboard_t)0x0000000000000002) ==  1);
    assert(bitboard_find_bit((bitboard_t)0x8000000000000000) == 63);
    assert(bitboard_find_bit((bitboard_t)0x0000000001000000) == 24);
}
#endif

int main()
{
    test_invalid_move();
    
    return 0;
}
