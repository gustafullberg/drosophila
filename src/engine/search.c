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
    int num_moves;
    move_t moves[256];

    num_moves = STATE_generate_moves_simple(state, moves);

    return num_moves == 0;
}

