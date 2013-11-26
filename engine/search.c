#include <limits.h>
#include <stdio.h>
#include "search.h"

static int minimax_min(chess_state_t *s1, move_t *stack, int depth, move_t *move);
static int minimax_max(chess_state_t *s1, move_t *stack, int depth, move_t *move);

int minimax_search(chess_state_t *s, move_t *stack, int depth, int *score)
{
    move_t move;

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
    int i;
    int min_score = INT_MAX;
    int score;
    move_t next_move;
    int checkmate;
    chess_state_t s2;

    if(depth <= 0) {
        return state_evaluate(s1);
    }

    num_moves = state_generate_moves(s1, stack, &checkmate);
    if(checkmate) {
        return INT_MIN;
    }

    for(i = 0; i < num_moves; i++) {
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        score = minimax_max(&s2, &stack[num_moves], depth-1, &next_move);
        if(score < min_score) {
            min_score = score;
            *move = stack[i];
        }
    }
    return min_score;
}

static int minimax_max(chess_state_t *s1, move_t *stack, int depth, move_t *move)
{
    int num_moves;
    int i;
    int max_score = INT_MIN;
    int score;
    move_t next_move;
    int checkmate;
    chess_state_t s2;

    if(depth <= 0) {
        return state_evaluate(s1);
    }
    
    num_moves = state_generate_moves(s1, stack, &checkmate);
    if(checkmate) {
        return INT_MAX;
    }
    
    for(i = 0; i < num_moves; i++) {
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        score = minimax_min(&s2, &stack[num_moves], depth-1, &next_move);
        if(score > max_score) {
            max_score = score;
            *move = stack[i];
        }
    }
    return max_score;
}

int search_is_check(chess_state_t *state, move_t *stack)
{
    /* The king is in check if he can be captured in one move by the opponent */
    chess_state_t check_state;
    int score;
    
    /* Check if opponent is in check */
    
    /* Clone state and switch playing side */
    state_clone(&check_state, state);
    check_state.player = 1 - check_state.player;
    
    /* Can the King be captured? */
    minimax_search(&check_state, stack, 1, &score);
    if(score == INT_MIN || score == INT_MAX) {
        /* In check */
        return 1;
    }
    
    /* Not in check */
    return 0;
}

int search_is_mate(chess_state_t *state, move_t *stack)
{
    /* The king is in mate if he can captured by the opponents
       next move, regardless of the move by the playing side */
    int score;
    
    minimax_search(state, stack, 2, &score);
    if(score == INT_MIN || score == INT_MAX) {
        /* In mate */
        /* 1. Checkmate if the king is also in check */
        /* 2. Stalemate otherwise */
        return 1;
    }
    
    /* Not in mate */
    return 0;
}
