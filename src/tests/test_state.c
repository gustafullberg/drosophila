/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "eval.h"

void test_position_is_attacked()
{
    chess_state_t s;
    
    /* Start out with fresh state */
    STATE_reset(&s);
    
    /* Move white pawn B2->B4 */
    s.bitboard[WHITE_PIECES+ALL] ^= s.bitboard[WHITE_PIECES+PAWN];
    s.bitboard[WHITE_PIECES+PAWN] ^= BITBOARD_POSITION(B2);
    s.bitboard[WHITE_PIECES+PAWN] ^= BITBOARD_POSITION(B4);
    s.bitboard[WHITE_PIECES+ALL] ^= s.bitboard[WHITE_PIECES+PAWN];

    /* Move black pawn E7->E5 */
    s.bitboard[BLACK_PIECES+ALL] ^= s.bitboard[BLACK_PIECES+PAWN];
    s.bitboard[BLACK_PIECES+PAWN] ^= BITBOARD_POSITION(E7);
    s.bitboard[BLACK_PIECES+PAWN] ^= BITBOARD_POSITION(E5);
    s.bitboard[BLACK_PIECES+ALL] ^= s.bitboard[BLACK_PIECES+PAWN];

    s.bitboard[OCCUPIED] = s.bitboard[WHITE_PIECES+ALL] | s.bitboard[BLACK_PIECES+ALL];
    
    assert(EVAL_position_is_attacked(&s, WHITE, A3) ==  0);
    assert(EVAL_position_is_attacked(&s, WHITE, E4) ==  0);
    assert(EVAL_position_is_attacked(&s, WHITE, H3) ==  0);
    assert(EVAL_position_is_attacked(&s, WHITE, H3) ==  0);
    assert(EVAL_position_is_attacked(&s, BLACK, H6) ==  0);
    assert(EVAL_position_is_attacked(&s, WHITE, B4) ==  1);
    assert(EVAL_position_is_attacked(&s, WHITE, C5) ==  1);
    assert(EVAL_position_is_attacked(&s, WHITE, H4) ==  1);
    assert(EVAL_position_is_attacked(&s, WHITE, F4) ==  1);
    
    /* Remove white pawn H2 */
    s.bitboard[WHITE_PIECES+ALL] ^= s.bitboard[WHITE_PIECES+PAWN];
    s.bitboard[WHITE_PIECES+PAWN] ^= BITBOARD_POSITION(H2);
    s.bitboard[WHITE_PIECES+ALL] ^= s.bitboard[WHITE_PIECES+PAWN];
    s.bitboard[OCCUPIED] = s.bitboard[WHITE_PIECES+ALL] | s.bitboard[BLACK_PIECES+ALL];
    
    assert(EVAL_position_is_attacked(&s, BLACK, H6) ==  1);
}

int main()
{
    BITBOARD_init();
    
    test_position_is_attacked();
    return 0;
}

