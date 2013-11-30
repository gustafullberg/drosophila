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
        *score = alphabeta_min(s, stack, depth, &move, INT_MIN, INT_MAX);
    }
    printf("#SCORE: %d\n", *score);
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
    int score;
    int move = search(state, stack, 1, &score);
    if(move == 0) {
        /* No legal moves => mate */
        return 1;
    }
    
    /* Not in mate */
    return 0;
}
