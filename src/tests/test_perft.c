/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "state.h"
#include "search.h"
#include "fen.h"

uint64_t perft(chess_state_t *state, int depth)
{
    chess_state_t next_state;
    move_t moves[512];
    int num_moves;
    uint64_t result = 0;
    
    if(depth == 0) {
        result = 1;
    } else {
        num_moves = STATE_generate_moves_simple(state, moves);

        while(num_moves) {
            next_state = *state;
            STATE_apply_move(&next_state, moves[--num_moves]);
            result += perft(&next_state, depth-1);
        }
    }
    
    return result;
}

void test_perft(chess_state_t *s, int depth, uint64_t *expected_results)
{
    int i;
    uint64_t num_moves;
    
    STATE_board_print_debug(s);
    
    for(i = 0; i <= depth; i++) {
        num_moves = perft(s, i);
        printf("%i: num_moves: %ld\n", i, num_moves);
        assert(num_moves == expected_results[i]);
    }
}

void test_perft1()
{
    chess_state_t s;
    uint64_t expected_results[7] = { 1, 20, 400, 8902, 197281, 4865609, 119060324 };

    FEN_read(&s, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    test_perft(&s, 6, expected_results);
}

void test_perft2()
{
    chess_state_t s;
    uint64_t expected_results[6] = { 1, 48, 2039, 97862, 4085603, 193690690 };

    FEN_read(&s, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    
    test_perft(&s, 5, expected_results);
}

void test_perft3()
{
    chess_state_t s;
    uint64_t expected_results[8] = { 1, 14, 191, 2812, 43238, 674624, 11030083, 178633661 };

    FEN_read(&s, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
    
    test_perft(&s, 7, expected_results);
}

void test_perft4()
{
    chess_state_t s;
    uint64_t expected_results[7] = { 1, 6, 264, 9467, 422333, 15833292, 706045033 };

    FEN_read(&s, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    test_perft(&s, 6, expected_results);
}

void test_perft5()
{
    chess_state_t s;
    uint64_t expected_results[4] = { 1, 42, 1352, 53392 };

    FEN_read(&s, "rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6");
    
    test_perft(&s, 3, expected_results);
}

void test_perft6()
{
    chess_state_t s;
    uint64_t expected_results[6] = { 1, 46, 2079, 89890, 3894594, 164075551 };

    FEN_read(&s, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
    
    test_perft(&s, 5, expected_results);
}

int main()
{
    BITBOARD_init();
    
    test_perft1();
    test_perft2();
    test_perft3();
    test_perft4();
    test_perft5();
    test_perft6();
    
    return 0;
}
