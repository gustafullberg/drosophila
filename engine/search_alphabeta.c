#include <limits.h>
#include "search_alphabeta.h"
#include "search_quiescence.h"
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
        return SEARCH_quiescence(s1, stack, alpha, beta);
#else
        return EVAL_evaluate_board(s1);
#endif
    }
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
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
