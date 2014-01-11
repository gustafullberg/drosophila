#include "search_mtdf.h"
#include "search_alphabeta.h"
#include "search.h"
#include "eval.h"

#define abs(x) ((x) >= 0 ? (x) : -(x))

int SEARCH_mtdf(const chess_state_t *s, search_state_t *search_state, short depth, move_t *move, int guess)
{
    int bounds[2];
    int beta;
    move_t movetemp;
    chess_state_t state = *s;
    
    /* Trick to disable null-move pruning on the first level of search */
    state.last_move = 0;
    
    bounds[0] = SEARCH_MIN_RESULT(depth+1);
    bounds[1] = SEARCH_MAX_RESULT(depth+1);
    
    while(bounds[0] < bounds[1]) {
        if(guess == bounds[0]) {
            beta = guess + 1;
        } else {
            beta = guess;
        }

        guess = SEARCH_alphabeta(&state, search_state, depth, &movetemp, beta-1, beta);

        if(guess < beta) {
            bounds[1] = guess;
        } else {
            bounds[0] = guess;
            *move = movetemp;
        }
    }
    
    return guess;
}

int SEARCH_mtdf_iterative(const chess_state_t *s, search_state_t *search_state, short max_depth, move_t *move)
{
#define MAX_DEPTH 100
    short depth;
    int results[MAX_DEPTH+1];
    int guess;
    move_t m;
    m = 0;
    
    if(max_depth > MAX_DEPTH) {
        max_depth = MAX_DEPTH;
    }
    
    results[0] = SEARCH_mtdf(s, search_state, 0, &m, 0);
    
    for(depth = 1; depth <= max_depth; depth++) {
        
        /* If results oscillate between depths, let guess be the result from two depths back */ 
        guess = results[depth-1];
        if(depth >= 3) {
            int r1 = results[depth-1] - results[depth-2];
            int r2 = results[depth-1] - results[depth-3];
            if(abs(r2) < abs(r1)) {
                guess = results[depth-2];
            }
        }
        
        results[depth] = SEARCH_mtdf(s, search_state, depth, &m, guess);
        *move = m;
    }
    
    return results[depth];
}
