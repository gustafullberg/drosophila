#include "search_quiescence.h"
#include "search.h"
#include "eval.h"

int SEARCH_quiescence_min(chess_state_t *s1, move_t *stack, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    chess_state_t s2;

    /* Stand-pat */
    score = EVAL_evaluate_board(s1);
    if(score <= alpha) {
        return alpha;
    }
    if(score < beta) {
        beta = score;
    }
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        /* Only look for captures in quiescence search */
        if(!(stack[i] & (MOVE_CAPTURE << 18))) {
            continue;
        }
        
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(SEARCH_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = SEARCH_quiescence_max(&s2, &stack[num_moves], alpha, beta);
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
        return EVAL_evaluate_board(s1);
    }
    
    return beta;
}

int SEARCH_quiescence_max(chess_state_t *s1, move_t *stack, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    chess_state_t s2;

    /* Stand-pat */
    score = EVAL_evaluate_board(s1);
    if(score >= beta) {
        return beta;
    }
    if(score > alpha) {
        alpha = score;
    }
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        /* Only look for captures in quiescence search */
        if(!(stack[i] & (MOVE_CAPTURE << 18))) {
            continue;
        }
        
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(SEARCH_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = SEARCH_quiescence_min(&s2, &stack[num_moves], alpha, beta);
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
        return EVAL_evaluate_board(s1);
    }

    
    return alpha;
}
