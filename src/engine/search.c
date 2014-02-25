#include <stdio.h>
#include "search.h"
#include "search_mtdf.h"
#include "eval.h"
#include "time.h"

int SEARCH_perform_search(const chess_state_t *s, hashtable_t *hashtable, history_t *history, const int time_for_move_ms, const unsigned char max_depth, short *score, thinking_output_cb think_cb)
{
    move_t move = 0;
    search_state_t search_state;
    search_state.hashtable = hashtable;
    search_state.history = history;
    search_state.abort_search = 0;
    search_state.next_clock_check = SEARCH_ITERATIONS_BETWEEN_CLOCK_CHECK;
    search_state.start_time_ms = TIME_now();
    search_state.time_for_move_ms = time_for_move_ms;
    search_state.max_depth = max_depth;
    search_state.think_cb = think_cb;
    
    *score = SEARCH_mtdf_iterative(s, &search_state, &move);
    return move;
}

int SEARCH_is_check(const chess_state_t *s, const int color)
{
    const int king_bitboard_index = color*NUM_TYPES + KING;
    const int king_pos = BITBOARD_find_bit(s->bitboard[king_bitboard_index]);
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
