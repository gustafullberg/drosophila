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
    search_state.num_nodes_searched = 0;
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

int SEARCH_find_pv(const chess_state_t *state, hashtable_t *hashtable, int depth, int *pos_from, int *pos_to, int *promotion_type)
{
    if(depth > 0) {
        transposition_entry_t *entry = HASHTABLE_transition_retrieve(hashtable, state->hash);
        if(entry) {
            if(entry->best_move && entry->depth >= depth) {
                move_t move = entry->best_move;
                chess_state_t next_state = *state;
                STATE_apply_move(&next_state, move);
                *pos_from       = MOVE_GET_POS_FROM(move);
                *pos_to         = MOVE_GET_POS_TO(move);
                *promotion_type = MOVE_PROMOTION_TYPE(move);
                return 1 + SEARCH_find_pv(&next_state, hashtable, depth-1, pos_from+1, pos_to+1, promotion_type+1);
            }
        }
    }

    return 0;
}
