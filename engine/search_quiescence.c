#include "search_quiescence.h"
#include "search.h"

int quiescence_min(chess_state_t *s1, move_t *stack, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    chess_state_t s2;
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        /* Only look for captures in quiescence search */
        if(!(stack[i] & (MOVE_CAPTURE << 18))) {
            continue;
        }
        
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(search_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = quiescence_max(&s2, &stack[num_moves], alpha, beta);
        if(score <= alpha) {
            /* Alpha-cuttoff */
            return alpha;
        }
        
        if(score < beta) {
            beta = score;
        }
    }
    
    /* No possible captures */
    if(num_legal_moves == 0) {
        return state_evaluate(s1);
    }
    
    return beta;
}

int quiescence_max(chess_state_t *s1, move_t *stack, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    chess_state_t s2;
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        /* Only look for captures in quiescence search */
        if(!(stack[i] & (MOVE_CAPTURE << 18))) {
            continue;
        }
        
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(search_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = quiescence_min(&s2, &stack[num_moves], alpha, beta);
        if(score >= beta) {
            /* Beta-cuttoff */
            return beta;
        }
        
        if(score > alpha) {
            alpha = score;
        }
    }
    
    /* No possible captures */
    if(num_legal_moves == 0) {
        return state_evaluate(s1);
    }

    
    return alpha;
}

#if 0
static int quiescence_min(chess_state_t *s1, move_t *stack)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int min_score = SHRT_MAX;
    int score;
    chess_state_t s2;
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        /* Only look for captures in quiescence search */
        if(!(stack[i] & (MOVE_CAPTURE << 18))) {
            continue;
        }
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(search_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = quiescence_max(&s2, &stack[num_moves]);
        if(score < min_score) {
            min_score = score;
        }
    }
    
    /* No more possible captures */
    if(num_legal_moves == 0) {
        /* Evaluate position */
        return state_evaluate(s1);
    }
    
    return min_score;
}

static int quiescence_max(chess_state_t *s1, move_t *stack)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int max_score = SHRT_MIN;
    int score;
    chess_state_t s2;
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        /* Only look for captures in quiescence search */
        if(!(stack[i] & (MOVE_CAPTURE << 18))) {
            continue;
        }
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(search_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = quiescence_min(&s2, &stack[num_moves]);
        if(score > max_score) {
            max_score = score;
        }
    }
    
    /* No more possible captures */
    if(num_legal_moves == 0) {
        /* Evaluate position */
        return state_evaluate(s1);
    }
    
    return max_score;
}
#endif
