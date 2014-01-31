#include <stdio.h>
#include "search.h"
#include "search_alphabeta.h"
#include "search_mtdf.h"
#include "search_minimax.h"
#include "eval.h"
#include "time.h"

int SEARCH_perform_search(const chess_state_t *s, ttable_t *ttable, int time_for_move_ms, int *score)
{
    move_t move = 0;
    search_state_t search_state;
    search_state.ttable = ttable;
    search_state.abort_search = 0;
    search_state.next_clock_check = SEARCH_ITERATIONS_BETWEEN_CLOCK_CHECK;
    search_state.start_time_ms = TIME_now();
    search_state.time_for_move_ms = time_for_move_ms;
    
    *score = SEARCH_mtdf_iterative(s, &search_state, &move);
    return move;
}

int SEARCH_is_check(const chess_state_t *s, int color)
{
    int king_bitboard_index = color*NUM_TYPES + KING;
    int king_pos = bitboard_find_bit(s->bitboard[king_bitboard_index]);
    return EVAL_position_is_attacked(s, color, king_pos);
}

int SEARCH_is_mate(const chess_state_t *state)
{
    int num_moves;
    int i;
    chess_state_t s2;
    move_t moves[256];

    num_moves = STATE_generate_moves(state, moves);
    for(i = 0; i < num_moves; i++) {
        s2 = *state;
        STATE_apply_move(&s2, moves[i]);
        if(!SEARCH_is_check(&s2, state->player)) {
            /* A legal move is found => not in mate */
            return 0;
        }
    }
    
    /* No legal moves => mate */
    return 1;
}
