/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <assert.h>
#include "see.h"
#include "fen.h"

void test_see()
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
    assert(FEN_read(&s, "1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -"));
    result = see(&s, move);
    printf("result: %d\n", result);
}



int main()
{
    BITBOARD_init();

    test_see();
    
    return 0;
}
