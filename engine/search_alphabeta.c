#include <limits.h>
#include "search_alphabeta.h"
#include "search.h"
#include "eval.h"

#define ENABLE_QUIESCENCE

/* Alpha-Beta search with Nega Max */
int SEARCH_alphabeta(chess_state_t *s1, move_t *stack, int depth, move_t *move, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    move_t next_move;
    chess_state_t s2;

    if(depth <= 0) {
#ifdef ENABLE_QUIESCENCE
        return SEARCH_alphabeta_quiescence(s1, stack, alpha, beta);
#else
        return EVAL_evaluate_board(s1);
#endif
    }
    
    num_moves = STATE_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        STATE_clone(&s2, s1);
        STATE_apply_move(&s2, stack[i]);
        if(SEARCH_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = -SEARCH_alphabeta(&s2, &stack[num_moves], depth-1, &next_move, -beta, -alpha);
        if(score > alpha) {
            if(score >= beta) {
                /* Beta-cuttoff */
                return beta;
            }
            alpha = score;
            *move = stack[i];
        }
    }
    
    /* Detect checkmate and stalemate */
    if(num_legal_moves == 0) {
        if(SEARCH_is_check(s1, s1->player)) {
            /* Checkmate (worst case) */
            alpha = -SHRT_MAX - depth;
        } else {
            /* Stalemate */
            alpha = 0;
        }
    }
    
    return alpha;
}

/* Alpha-Beta quiescence search with Nega Max */
int SEARCH_alphabeta_quiescence(chess_state_t *s1, move_t *stack, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    chess_state_t s2;

    /* Stand-pat */
    score = EVAL_evaluate_board(s1);
    if(score > alpha) {
        if(score >= beta) {
            return beta;
        }
        alpha = score;
    }
    
    num_moves = STATE_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        /* Only look for captures in quiescence search */
        if(!(stack[i] & (MOVE_CAPTURE << 18))) {
            continue;
        }
        
        STATE_clone(&s2, s1);
        STATE_apply_move(&s2, stack[i]);
        if(SEARCH_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = -SEARCH_alphabeta_quiescence(&s2, &stack[num_moves], -beta, -alpha);
        if(score > alpha) {
            if(score >= beta) {
                /* Beta-cuttoff */
                return beta;
            }
            alpha = score;
        }
    }
    
    return alpha;
}
