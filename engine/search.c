#include <stdio.h>
#include "search.h"
#include "search_alphabeta.h"
#include "search_mtdf.h"
#include "search_minimax.h"
#include "eval.h"

int SEARCH_perform_search(chess_state_t *s, move_t *stack, ttable_t *ttable, short depth, int *score)
{
    move_t move = 0;
    *score = SEARCH_mtdf_iterative(s, stack, ttable, depth, &move);
    return move;
}

int SEARCH_is_check(chess_state_t *s, int color)
{
    int king_bitboard_index = color*NUM_TYPES + KING;
    int king_pos = bitboard_find_bit(s->bitboard[king_bitboard_index]);
    return EVAL_position_is_attacked(s, color, king_pos);
}

int SEARCH_is_mate(chess_state_t *state, move_t *stack)
{
    int num_moves;
    int i;
    chess_state_t s2;

    num_moves = STATE_generate_moves(state, stack);
    for(i = 0; i < num_moves; i++) {
        STATE_clone(&s2, state);
        STATE_apply_move(&s2, stack[i]);
        if(!SEARCH_is_check(&s2, state->player)) {
            /* A legal move is found => not in mate */
            return 0;
        }
    }
    
    /* No legal moves => mate */
    return 1;
}

