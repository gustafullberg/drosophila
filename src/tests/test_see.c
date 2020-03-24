/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <assert.h>
#include "see.h"
#include "fen.h"
#include "eval.h"

move_t get_move(const chess_state_t *s, int pos_from, int pos_to)
{
    move_t moves[256];
    int num_moves, i;
    
    num_moves = STATE_generate_moves_simple(s, moves);
    
    for(i = 0; i < num_moves; i++) {
        if(MOVE_GET_POS_FROM(moves[i]) == pos_from && MOVE_GET_POS_TO(moves[i]) == pos_to)
        {
            return moves[i];
        }
    }
    
    return 0;
}

void test_see(const char *fen, int pos_from, int pos_to, short expected_result)
{
    chess_state_t s;
    int result;
    move_t move;
    assert(FEN_read(&s, fen));
    move = get_move(&s, pos_from, pos_to);
    assert(move);
    result = see(&s, move);
    printf("%s\n", fen);
    printf("\tresult %d (%d centipawns)\n", result, result*100);
    assert(result == expected_result);
}



int main()
{
    BITBOARD_init();

    test_see("1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -", E1, E5, 1);
    test_see("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -", D3, E5, 1-3);
    test_see("1k1r3q/1pp4p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 b - -", D8, D3, 3-5);
    test_see("k7/8/1q1r4/2p5/3P4/2P1K2/8/8 b - - 0 1", C5, D4, 1);
    test_see("8/8/1k6/2p5/3K4/4B3/8/8 w - - 0 1", D4, C5, 1-20);
    
    return 0;
}
