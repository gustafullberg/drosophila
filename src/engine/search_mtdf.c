#include "search_mtdf.h"
#include "search_nullwindow.h"
#include "search.h"
#include "eval.h"
#include "time.h"

#define abs(x) ((x) >= 0 ? (x) : -(x))

short SEARCH_mtdf(const chess_state_t *s, search_state_t *search_state, const unsigned char depth, move_t *move, short guess)
{
    short bounds[2];
    short beta;
    move_t movetemp;
    chess_state_t state = *s;
    
    /* Trick to disable null-move pruning on the first level of search */
    state.last_move = 0;
    
    bounds[0] = SEARCH_MIN_RESULT(0);
    bounds[1] = SEARCH_MAX_RESULT(0);
    
    while(bounds[0] < bounds[1]) {
        if(guess == bounds[0]) {
            beta = guess + 1;
        } else {
            beta = guess;
        }

        guess = SEARCH_nullwindow(&state, search_state, depth, 0, &movetemp, beta);

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
    unsigned char depth;
    short results[MAX_SEARCH_DEPTH+1];
    short guess;
    move_t m;
    int64_t time_passed_ms = 0;
    m = 0;
    
    /* Limit maximum search depth */
    if(search_state->max_depth > MAX_SEARCH_DEPTH) search_state->max_depth = MAX_SEARCH_DEPTH;
    
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
        
        time_passed_ms = TIME_passed(search_state->start_time_ms);
        if(search_state->think_cb) {
            int pos_from[100];
            int pos_to[100];
            int promotion_type[100];
            int pv_length = SEARCH_find_pv(s, search_state->hashtable, depth, pos_from, pos_to, promotion_type);
            (*search_state->think_cb)(depth, 5 * (int)results[depth], (int)time_passed_ms, search_state->num_nodes_searched, pv_length, pos_from, pos_to, promotion_type);
        }

        /* No need to search deeper if checkmate is detected */
        if(results[depth] <= SEARCH_MIN_RESULT(0) || results[depth] >= SEARCH_MAX_RESULT(0)) {
            break;
        }

        if(2 * time_passed_ms > search_state->time_for_move_ms) {
            break;
        }
    }
    
    /* If maximum search depth is reached */
    if(depth > search_state->max_depth) depth = search_state->max_depth;
    
    return results[depth];
}
