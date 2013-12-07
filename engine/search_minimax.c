#include <limits.h>
#include "search_minimax.h"
#include "search.h"
#include "eval.h"

/* Minimax search with Nega Max - For testing only, too slow to be useful */
int SEARCH_minimax(chess_state_t *s1, move_t *stack, int depth, move_t *move)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    move_t next_move;
    chess_state_t s2;
    int best_score = -SHRT_MAX - depth;

    if(depth <= 0) {
#if USE_QUIESCENCE
        return SEARCH_minimax_quiescence(s1, stack);
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
        score = -SEARCH_minimax(&s2, &stack[num_moves], depth-1, &next_move);
        if(score > best_score) {
            best_score = score;
            *move = stack[i];
        }
    }
    
    /* Detect stalemate */
    if(num_legal_moves == 0) {
        if(!SEARCH_is_check(s1, s1->player)) {
            /* Stalemate */
            best_score = 0;
        }
    }
    
    return best_score;
}

/* Minimax quiescence search with Nega Max - For testing only, too slow to be useful */
int SEARCH_minimax_quiescence(chess_state_t *s1, move_t *stack)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    chess_state_t s2;
    int best_score;

    /* Stand-pat */
    best_score = EVAL_evaluate_board(s1);
    
    num_moves = STATE_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        /* Only look for captures in quiescence search */
        if(!(MOVE_IS_CAPTURE_OR_PROMOTION(stack[i]))) {
            continue;
        }
        
        STATE_clone(&s2, s1);
        STATE_apply_move(&s2, stack[i]);
        if(SEARCH_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = -SEARCH_minimax_quiescence(&s2, &stack[num_moves]);
        if(score > best_score) {
            best_score = score;
        }
    }
    
    return best_score;
}
