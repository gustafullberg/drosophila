#include <limits.h>
#include <stdio.h>
#include "search.h"
#include "search_alphabeta.h"

int search(chess_state_t *s, move_t *stack, int depth, int *score)
{
    move_t move = 0;

    if(s->player) {
        /* Black player moving */
        *score = alphabeta_min(s, stack, depth, &move, INT_MIN, INT_MAX);
    } else {
        /* White player moving */
        *score = alphabeta_max(s, stack, depth, &move, INT_MIN, INT_MAX);
    }
    return move;
}

int search_is_check(chess_state_t *s, int color)
{
    int king_bitboard_index = color*NUM_TYPES + KING;
    int king_pos = bitboard_find_bit(s->bitboard[king_bitboard_index]);
    return state_position_is_attacked(s, color, king_pos);
}

int search_is_mate(chess_state_t *state, move_t *stack)
{
    int num_moves;
    int i;
    chess_state_t s2;

    num_moves = state_generate_moves(state, stack);
    for(i = 0; i < num_moves; i++) {
        state_clone(&s2, state);
        state_apply_move(&s2, stack[i]);
        if(!search_is_check(&s2, state->player)) {
            /* A legal move is found => not in mate */
            return 0;
        }
    }
    
    /* No legal moves => mate */
    return 1;
}
