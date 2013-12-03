#include "search_quiescence.h"
#include "search.h"
#include "eval.h"

/* Alpha-Beta quiescence search with Nega Max */
int SEARCH_quiescence(chess_state_t *s1, move_t *stack, int alpha, int beta)
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
        score = -SEARCH_quiescence(&s2, &stack[num_moves], -beta, -alpha);
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
