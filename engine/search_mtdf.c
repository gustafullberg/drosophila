#include "search_mtdf.h"
#include "search_alphabeta.h"
#include "search.h"
#include "eval.h"

int SEARCH_mtdf(const chess_state_t *s, move_t *stack, ttable_t *ttable, short depth, move_t *move, int guess)
{
    int bounds[2];
    int beta;
    move_t movetemp;
    
    bounds[0] = SEARCH_MIN_RESULT(depth+1);
    bounds[1] = SEARCH_MAX_RESULT(depth+1);
    
    while(bounds[0] < bounds[1]) {
        if(guess == bounds[0]) {
            beta = guess + 1;
        } else {
            beta = guess;
        }

        guess = SEARCH_alphabeta(s, stack, ttable, depth, &movetemp, beta-1, beta);

        if(guess < beta) {
            bounds[1] = guess;
        } else {
            bounds[0] = guess;
            *move = movetemp;
        }
    }
    
    return guess;
}

int SEARCH_mtdf_iterative(const chess_state_t *s, move_t *stack, ttable_t *ttable, short max_depth, move_t *move)
{
    short depth;
    int result;
    move_t m;
    
    result = EVAL_evaluate_board(s);
    m = 0;
    
    for(depth = 1; depth <= max_depth; depth++) {
        result = SEARCH_mtdf(s, stack, ttable, depth, &m, result);
        *move = m;
    }
    
    return result;
}
