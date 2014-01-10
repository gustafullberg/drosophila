/* Make sure assert is not disabled */
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include "state.h"
#include "search.h"

uint32_t perft(chess_state_t *state, int depth)
{
    chess_state_t next_state;
    move_t moves[512];
    int num_moves;
    uint32_t result = 0;
    
    if(depth == 0) {
        result = 1;
    } else {
        num_moves = STATE_generate_moves(state, moves);
        while(num_moves) {
            next_state = *state;
            STATE_apply_move(&next_state, moves[--num_moves]);
            if(SEARCH_is_check(&next_state, state->player)) {
                continue;
            }
            
            result += perft(&next_state, depth-1);
        }
    }
    
    return result;
}

void test_perft(chess_state_t *s, int depth, uint32_t *expected_results)
{
    int i;
    uint32_t num_moves;
    
    STATE_board_print_debug(s);
    
    for(i = 0; i <= depth; i++) {
        num_moves = perft(s, i);
        printf("%i: num_moves: %d\n", i, num_moves);
        assert(num_moves == expected_results[i]);
    }
}

void test_perft1()
{
    chess_state_t s;
    uint32_t expected_results[7] = { 1, 20, 400, 8902, 197281, 4865609, 119060324 };

    STATE_reset(&s);
    
    test_perft(&s, 6, expected_results);
}

int main()
{
    bitboard_init();
    
    test_perft1();
    
    return 0;
}
