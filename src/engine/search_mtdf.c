#include "search_mtdf.h"
#include "search_nullwindow.h"
#include "search.h"
#include "eval.h"
#include "time.h"

#define abs(x) ((x) >= 0 ? (x) : -(x))

short SEARCH_mtdf(const chess_state_t *s, search_state_t *search_state, short depth, move_t *move, short guess)
{
    short bounds[2];
    short beta;
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

        guess = SEARCH_nullwindow(&state, search_state, depth, &movetemp, beta);

        if(guess < beta) {
            bounds[1] = guess;
        } else {
            bounds[0] = guess;
            *move = movetemp;
        }
        
        if(search_state->abort_search) {
            return 0;
        }
    }
    
    return guess;
}

short SEARCH_mtdf_iterative(const chess_state_t *s, search_state_t *search_state, move_t *move)
{
#define MAX_DEPTH 100
    short depth;
    short results[MAX_DEPTH+1];
    short guess;
    move_t m;
    m = 0;
    
    results[0] = SEARCH_mtdf(s, search_state, 0, &m, 0);
    *move = m;
    
    for(depth = 1; depth <= search_state->max_depth; depth++) {
        
        /* If results oscillate between depths, let guess be the result from two depths back */ 
        guess = results[depth-1];
        if(depth >= 3) {
            short r1 = results[depth-1] - results[depth-2];
            short r2 = results[depth-1] - results[depth-3];
            if(abs(r2) < abs(r1)) {
                guess = results[depth-2];
            }
        }
        
        results[depth] = SEARCH_mtdf(s, search_state, depth, &m, guess);
        
        if(search_state->abort_search) {
            depth--;
            break;
        }
        
        *move = m;

#ifndef DISABLE_TIME_MANAGEMENT
        if(2 * TIME_passed(search_state->start_time_ms) > search_state->time_for_move_ms) {
            break;
        }
#endif
    }
    
    return results[depth];
}
