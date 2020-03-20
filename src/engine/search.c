#include <stdio.h>
#include "search.h"
#include "search_mtdf.h"
#include "eval.h"
#include "clock.h"

move_t SEARCH_perform_search(const chess_state_t *s, search_state_t *search_state, short *score)
{
    move_t move = 0;
    *score = SEARCH_mtdf_iterative(s, search_state, &move);
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
    int num_moves, num_checkers;
    int i;
    chess_state_t s2;
    move_t moves[256];
    bitboard_t block_check, pinners, pinned;

    num_checkers = STATE_checkers_and_pinners(state, &block_check, &pinners, &pinned);
    num_moves = STATE_generate_legal_moves(state, num_checkers, block_check, pinners, pinned, moves);

    return num_moves == 0;
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
