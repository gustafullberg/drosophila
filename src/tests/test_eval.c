/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include "fen.h"
#include "eval.h"

int main()
{
    chess_state_t s;
    bitboard_t attack[NUM_COLORS], passedPawns, isolatedPawns;
    BITBOARD_init();

    assert(FEN_read(&s, "1k1r4/2p4p/p7/4p1P1/8/1P6/1PP4P/2K1R3 w - -"));
    EVAL_pawn_types(&s, attack, &passedPawns, &isolatedPawns);
    assert(attack[WHITE] == 0xa000054f0000);
    assert(attack[BLACK] == 0x4a0228000000);
    assert(passedPawns == 0x1000000000);
    assert(isolatedPawns == 0x84011000000000);

    return 0;
}
