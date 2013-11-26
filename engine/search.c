#include <limits.h>
#include <stdio.h>
#include "search.h"

static int minimax_min(chess_state_t *s1, move_t *stack, int depth, move_t *move);
static int minimax_max(chess_state_t *s1, move_t *stack, int depth, move_t *move);

int minimax_search(chess_state_t *s, move_t *stack, int depth, int *score)
{
    move_t move = 0;

    if(s->player) {
        /* Black player moving */
        *score = minimax_min(s, stack, depth, &move);
    } else {
        /* White player moving */
        *score = minimax_max(s, stack, depth, &move);
    }
    return move;
}

static int minimax_min(chess_state_t *s1, move_t *stack, int depth, move_t *move)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int min_score = INT_MAX;
    int score;
    move_t next_move;
    chess_state_t s2;

    if(depth <= 0) {
        return state_evaluate(s1);
    }

    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(search_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = minimax_max(&s2, &stack[num_moves], depth-1, &next_move);
        if(score < min_score) {
            min_score = score;
            *move = stack[i];
        }
    }
    
    /* Detect stalemate */
    if(num_legal_moves == 0) {
        if(search_is_check(s1, s1->player) == 0) {
            return 0;
        }
    }

    return min_score;
}

static int minimax_max(chess_state_t *s1, move_t *stack, int depth, move_t *move)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int max_score = INT_MIN;
    int score;
    move_t next_move;
    chess_state_t s2;

    if(depth <= 0) {
        return state_evaluate(s1);
    }
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(search_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = minimax_min(&s2, &stack[num_moves], depth-1, &next_move);
        if(score > max_score) {
            max_score = score;
            *move = stack[i];
        }
    }
    
    /* Detect stalemate */
    if(num_legal_moves == 0) {
        if(search_is_check(s1, s1->player) == 0) {
            return 0;
        }
    }
    
    return max_score;
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
    int move = minimax_search(state, stack, 1, &score);
    if(move == 0) {
        /* No legal moves => mate */
        return 1;
    }
    
    /* Not in mate */
    return 0;
}
