#include <limits.h>
#include "search_alphabeta.h"
#include "search.h"
#include "eval.h"

/* Alpha-Beta search with Nega Max */
int SEARCH_alphabeta(chess_state_t *s1, move_t *stack, ttable_t *ttable, short depth, move_t *move, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    int best_score = -SHRT_MAX - depth;
    move_t next_move;
    chess_state_t s2;
#if USE_TRANSPOSITION_TABLE
    ttable_entry_t *ttentry;
#endif

    if(depth <= 0) {
#if USE_QUIESCENCE
        return SEARCH_alphabeta_quiescence(s1, stack, ttable, alpha, beta);
#else
        return EVAL_evaluate_board(s1);
#endif
    }
    
#if USE_TRANSPOSITION_TABLE
    ttentry = TTABLE_get(ttable, s1->hash, depth);
    if(ttentry) {
        if(ttentry->type == TTABLE_TYPE_EXACT) {
            return ttentry->score;
        } else if(ttentry->type == TTABLE_TYPE_LOWER_BOUND && ttentry->score > alpha) {
            alpha = ttentry->score;
            if(alpha >= beta) {
                return ttentry->score;
            }
        } else if(ttentry->type == TTABLE_TYPE_UPPER_BOUND && ttentry->score < beta) {
            beta = ttentry->score;
        }
    }
#endif

    num_moves = STATE_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        STATE_clone(&s2, s1);
        STATE_apply_move(&s2, stack[i]);
        if(SEARCH_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = -SEARCH_alphabeta(&s2, &stack[num_moves], ttable, depth-1, &next_move, -beta, -alpha);
        if(score > best_score) {
            best_score = score;
            if(best_score > alpha) {
                alpha = best_score;
            }
            if(best_score >= beta) {
                /* Beta-cuttoff */
                break;
            }
            *move = stack[i];
        }
    }
    
    /* Detect checkmate and stalemate */
    if(num_legal_moves == 0) {
        if(SEARCH_is_check(s1, s1->player)) {
            /* Checkmate (worst case) */
        } else {
            /* Stalemate */
            best_score = 0;
        }
    }
    
#if USE_TRANSPOSITION_TABLE
    if(best_score <= alpha) {
        TTABLE_store(ttable, s1->hash, depth, TTABLE_TYPE_LOWER_BOUND, best_score);
    } else if(best_score >= beta) {
        TTABLE_store(ttable, s1->hash, depth, TTABLE_TYPE_UPPER_BOUND, best_score);
    } else {
        TTABLE_store(ttable, s1->hash, depth, TTABLE_TYPE_EXACT, best_score);
    }
#endif
    
    return best_score;
}

/* Alpha-Beta quiescence search with Nega Max */
int SEARCH_alphabeta_quiescence(chess_state_t *s1, move_t *stack, ttable_t *ttable, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    int best_score;
    chess_state_t s2;
#if USE_TRANSPOSITION_TABLE
    ttable_entry_t *ttentry;
#endif

    /* Stand-pat */
    best_score = EVAL_evaluate_board(s1);
    if(best_score > alpha) {
        if(best_score >= beta) {
            return beta;
        }
        alpha = best_score;
    }
    
#if USE_TRANSPOSITION_TABLE
    ttentry = TTABLE_get(ttable, s1->hash, 0);
    if(ttentry) {
        if(ttentry->type == TTABLE_TYPE_EXACT) {
            return ttentry->score;
        } else if(ttentry->type == TTABLE_TYPE_LOWER_BOUND && ttentry->score > alpha) {
            alpha = ttentry->score;
            if(alpha >= beta) {
                return ttentry->score;
            }
        } else if(ttentry->type == TTABLE_TYPE_UPPER_BOUND && ttentry->score < beta) {
            beta = ttentry->score;
        }
    }
#endif
    
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
        score = -SEARCH_alphabeta_quiescence(&s2, &stack[num_moves], ttable, -beta, -alpha);
        if(score > best_score) {
            best_score = score;
            if(best_score > alpha) {
                alpha = best_score;
            }
            if(best_score >= beta) {
                /* Beta-cuttoff */
                break;
            }
        }
    }
    
#if USE_TRANSPOSITION_TABLE
    if(best_score <= alpha) {
        TTABLE_store(ttable, s1->hash, 0, TTABLE_TYPE_LOWER_BOUND, best_score);
    } else if(best_score >= beta) {
        TTABLE_store(ttable, s1->hash, 0, TTABLE_TYPE_UPPER_BOUND, best_score);
    } else {
        TTABLE_store(ttable, s1->hash, 0, TTABLE_TYPE_EXACT, best_score);
    }
#endif
    
    return best_score;
}
