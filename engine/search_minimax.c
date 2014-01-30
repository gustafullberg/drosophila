#include "search_minimax.h"
#include "search.h"
#include "eval.h"

/* Minimax search with Nega Max - For testing only, too slow to be useful */
int SEARCH_minimax(const chess_state_t *state, short depth, move_t *move)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    move_t next_move;
    chess_state_t next_state;
    int best_score = SEARCH_MIN_RESULT(depth);
    move_t moves[256];

    if(depth <= 0) {
#ifndef DISABLE_QUIESCENCE
        return SEARCH_minimax_quiescence(state);
#else
        return EVAL_evaluate_board(state);
#endif
    }
    
    num_moves = STATE_generate_moves(state, moves);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        next_state = *state;
        STATE_apply_move(&next_state, moves[i]);
        if(SEARCH_is_check(&next_state, state->player)) {
            continue;
        }
        num_legal_moves++;
        score = -SEARCH_minimax(&next_state, depth-1, &next_move);
        if(score > best_score) {
            best_score = score;
            *move = moves[i];
        }
    }
    
    /* Detect stalemate */
    if(num_legal_moves == 0) {
        if(!SEARCH_is_check(state, state->player)) {
            /* Stalemate */
            best_score = 0;
        }
    }
    
    return best_score;
}

/* Minimax quiescence search with Nega Max - For testing only, too slow to be useful */
int SEARCH_minimax_quiescence(const chess_state_t *state)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    chess_state_t next_state;
    int best_score;
    move_t moves[256];

    /* Stand-pat */
    best_score = EVAL_evaluate_board(state);
    
    num_moves = STATE_generate_moves(state, moves);
    num_legal_moves = 0;
    for(i = 0; i < num_moves; i++) {
        /* Only look for captures in quiescence search */
        if(!(MOVE_IS_CAPTURE_OR_PROMOTION(moves[i]))) {
            continue;
        }
        
        next_state = *state;
        STATE_apply_move(&next_state, moves[i]);
        if(SEARCH_is_check(&next_state, state->player)) {
            continue;
        }
        num_legal_moves++;
        score = -SEARCH_minimax_quiescence(&next_state);
        if(score > best_score) {
            best_score = score;
        }
    }
    
    return best_score;
}
