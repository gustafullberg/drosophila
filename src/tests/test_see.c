/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <assert.h>
#include "see.h"
#include "fen.h"

void test_see(const char *fen, short expected_result)
{
    chess_state_t s;
    int result;
    int pos_from = D3;
    int pos_to = E5;
    int type = KNIGHT;
    int capture_type = PAWN;
    int special = MOVE_CAPTURE;
    move_t move =
        pos_from << MOVE_POS_FROM_SHIFT |
        pos_to << MOVE_POS_TO_SHIFT |
        type << MOVE_TYPE_SHIFT |
        capture_type << MOVE_CAPTURE_TYPE_SHIFT |
        special << MOVE_SPECIAL_FLAGS_SHIFT;
    assert(FEN_read(&s, fen));
    result = see(&s, move);
    printf("%s\n", fen);
    printf("\tresult %d (%d centipawns)\n", result, result*5);
    assert(result == expected_result);
}



int main()
{
    BITBOARD_init();

    test_see("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -", -225/5);
    
    return 0;
}
