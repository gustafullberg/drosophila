#include <limits.h>
#include "search_alphabeta.h"
#include "search.h"

int alphabeta_min(chess_state_t *s1, move_t *stack, int depth, move_t *move, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    move_t next_move;
    chess_state_t s2;

    if(depth <= 0) {
        /* return quiescence_max(s1, stack); */
        return state_evaluate(s1);
    }
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(search_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = alphabeta_max(&s2, &stack[num_moves], depth-1, &next_move, alpha, beta);
        if(score <= alpha) {
            /* Alpha-cuttoff */
            return alpha;
        }
        
        if(score < beta) {
            beta = score;
            *move = stack[i];
        }
    }
    
    /* Detect checkmate and stalemate */
    if(num_legal_moves == 0) {
        if(search_is_check(s1, s1->player)) {
            /* Checkmate */
            return SHRT_MAX + depth;
        } else {
            /* Stalemate */
            return 0;
        }
    }
    
    return beta;
}

int alphabeta_max(chess_state_t *s1, move_t *stack, int depth, move_t *move, int alpha, int beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    move_t next_move;
    chess_state_t s2;

    if(depth <= 0) {
        /* return quiescence_max(s1, stack); */
        return state_evaluate(s1);
    }
    
    num_moves = state_generate_moves(s1, stack);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        state_clone(&s2, s1);
        state_apply_move(&s2, stack[i]);
        if(search_is_check(&s2, s1->player)) {
            continue;
        }
        num_legal_moves++;
        score = alphabeta_min(&s2, &stack[num_moves], depth-1, &next_move, alpha, beta);
        if(score >= beta) {
            /* Beta-cuttoff */
            return beta;
        }
        
        if(score > alpha) {
            alpha = score;
            *move = stack[i];
        }
    }
    
    /* Detect checkmate and stalemate */
    if(num_legal_moves == 0) {
        if(search_is_check(s1, s1->player)) {
            /* Checkmate */
            return SHRT_MIN - depth;
        } else {
            /* Stalemate */
            return 0;
        }
    }
    
    return alpha;
}
