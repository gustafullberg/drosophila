#include "search_nullwindow.h"
#include "search.h"
#include "eval.h"
#include "moveorder.h"
#include "time.h"

static inline short SEARCH_transpositiontable_retrieve(ttable_t *ttable, bitboard_t hash, unsigned char depth, short *beta, move_t *best_move, int *cutoff);
static inline void SEARCH_transpositiontable_store(ttable_t *ttable, bitboard_t hash, unsigned char depth, short best_score, move_t best_move, short beta);

/* Alpha-Beta search with Nega Max and null-window */
short SEARCH_nullwindow(const chess_state_t *state, search_state_t *search_state, unsigned char depth, move_t *move, short beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    short score;
    short best_score = SEARCH_MIN_RESULT(depth);
    int skip_move_generation = 0;
    move_t next_move;
    chess_state_t next_state;
    move_t moves[256];
    int is_in_check;
    short ttable_score;
    int cutoff = 0;

    *move = 0;
    
#ifndef DISABLE_TIME_MANAGEMENT
    search_state->next_clock_check--;
    if(search_state->next_clock_check <= 0) {
        search_state->next_clock_check = SEARCH_ITERATIONS_BETWEEN_CLOCK_CHECK;
        if(TIME_passed(search_state->start_time_ms) >= search_state->time_for_move_ms) {
            search_state->abort_search = 1;
        }
    }
    if(search_state->abort_search) {
        return 0;
    }
#endif

#ifndef DISABLE_TRANSPOSITION_TABLE
    TTABLE_prefetch(search_state->ttable, state->hash);
#endif

    is_in_check = SEARCH_is_check(state, state->player);
    
    if(is_in_check) {
        /* Check extension */
        depth += 1;
    }

    if(depth == 0) {
#ifndef DISABLE_QUIESCENCE
        return SEARCH_nullwindow_quiescence(state, search_state, beta);
#else
        return EVAL_evaluate_board(state, search_state->ttable);
#endif
    }
    
#ifndef DISABLE_TRANSPOSITION_TABLE
    ttable_score = SEARCH_transpositiontable_retrieve(search_state->ttable, state->hash, depth, &beta, move, &cutoff);
    if(cutoff) {
        return ttable_score;
    }
#endif

#ifndef DISABLE_NULL_MOVE
    if(depth > 4 && state->last_move && !STATE_is_endgame(state)) {
        if(!SEARCH_is_check(state, state->player)) {
            next_state = *state;
            STATE_apply_move(&next_state, 0);
            score = -SEARCH_nullwindow(&next_state, search_state, depth-3, &next_move, -beta+1);
            if(score >= beta) {
                best_score = beta;
                skip_move_generation = 1;
            }
        }
    }
#endif

#ifndef DISABLE_TRANSPOSITION_TABLE
    if(!skip_move_generation && *move) {
        next_state = *state;
        STATE_apply_move(&next_state, *move);
        if(!SEARCH_is_check(&next_state, state->player)) {
            best_score = -SEARCH_nullwindow(&next_state, search_state, depth-1, &next_move, -beta+1);
            if(best_score >= beta) {
                skip_move_generation = 1;
            }
        }
    }
#endif

    if(!skip_move_generation) {
        num_moves = STATE_generate_moves(state, moves);
        
#ifndef DISABLE_MOVE_ORDERING
        num_moves = MOVEORDER_order_moves(state, moves, num_moves, *move);
#endif

        num_legal_moves = 0;
        for(i = 0; i < num_moves; i++) {
            next_state = *state;
            STATE_apply_move(&next_state, moves[i]);
            if(SEARCH_is_check(&next_state, state->player)) {
                continue;
            }
            num_legal_moves++;
            
#ifndef DISABLE_HISTORY
            HISTORY_push(search_state->history, next_state.hash);
            if(HISTORY_is_repetition(search_state->history, next_state.halfmove_clock)) {
                score = 0;
            } else
#endif
            {

#ifndef DISABLE_LATE_MOVE_REDUCTION
                /* Late move reduction */
                if( num_legal_moves > 4                     && /* Four moves have been searched at full depth   */
                    depth >= 3                              && /* No LMR in the last plies                      */
                    !MOVE_IS_CAPTURE_OR_PROMOTION(moves[i]) && /* No LMR if capture / promotion                 */
                    !is_in_check)                              /* No LMR if in check                            */
                {
                    /* Search at reduced depth */
                    score = -SEARCH_nullwindow(&next_state, search_state, depth-2, &next_move, -beta+1);
                    if(score >= beta) {
                        score = -SEARCH_nullwindow(&next_state, search_state, depth-1, &next_move, -beta+1);
                    }
                } else {
                    score = -SEARCH_nullwindow(&next_state, search_state, depth-1, &next_move, -beta+1);
                }
#else
                score = -SEARCH_nullwindow(&next_state, search_state, depth-1, &next_move, -beta+1);
#endif
            }
            
            if(score > best_score) {
                best_score = score;
                *move = moves[i];
                if(best_score >= beta) {
                    /* Beta-cuttoff */
                    HISTORY_pop(search_state->history);
                    break;
                }
            }

#ifndef DISABLE_HISTORY
            HISTORY_pop(search_state->history);
#endif
        }
        
        /* Detect checkmate and stalemate */
        if(num_legal_moves == 0) {
            if(SEARCH_is_check(state, state->player)) {
                /* Checkmate (worst case) */
            } else {
                /* Stalemate */
                best_score = 0;
            }
        }
    }
    
#ifndef DISABLE_TRANSPOSITION_TABLE
    SEARCH_transpositiontable_store(search_state->ttable, state->hash, depth, best_score, *move, beta);
#endif
    return best_score;
}

/* Alpha-Beta quiescence search with Nega Max and null-window */
short SEARCH_nullwindow_quiescence(const chess_state_t *state, search_state_t *search_state, short beta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    short score;
    short best_score;
    int skip_move_generation = 0;
    move_t move;
    chess_state_t next_state;
    move_t moves[256];
    int cutoff = 0;
    short ttable_score;

    move = 0;

#ifndef DISABLE_TRANSPOSITION_TABLE
    TTABLE_prefetch(search_state->ttable, state->hash);
#endif

    /* Stand-pat */
    best_score = EVAL_evaluate_board(state, search_state->ttable);
    if(best_score >= beta) {
        return best_score;
    }
    
#ifndef DISABLE_TRANSPOSITION_TABLE
    ttable_score = SEARCH_transpositiontable_retrieve(search_state->ttable, state->hash, 0, &beta, &move, &cutoff);
    if(cutoff) {
        return ttable_score;
    }
    
    if(move) {
        next_state = *state;
        STATE_apply_move(&next_state, move);
        if(!SEARCH_is_check(&next_state, state->player)) {
            best_score = -SEARCH_nullwindow_quiescence(&next_state, search_state, -beta+1);
            if(best_score >= beta) {
                skip_move_generation = 1;
            }
        }
    }
#endif

    if(!skip_move_generation) {    
        num_moves = STATE_generate_moves_quiescence(state, moves);
        
#ifndef DISABLE_MOVE_ORDERING
        num_moves = MOVEORDER_order_moves(state, moves, num_moves, move);
#endif
        
        num_legal_moves = 0;
        for(i = 0; i < num_moves; i++) {
            next_state = *state;
            STATE_apply_move(&next_state, moves[i]);
            if(SEARCH_is_check(&next_state, state->player)) {
                continue;
            }
            num_legal_moves++;
            score = -SEARCH_nullwindow_quiescence(&next_state, search_state, -beta+1);
            if(score > best_score) {
                best_score = score;
                if(best_score >= beta) {
                    /* Beta-cuttoff */
                    break;
                }
            }
        }
    }
    
#ifndef DISABLE_TRANSPOSITION_TABLE
    SEARCH_transpositiontable_store(search_state->ttable, state->hash, 0, best_score, 0, beta);
#endif
    
    return best_score;
}

static inline short SEARCH_transpositiontable_retrieve(ttable_t *ttable, bitboard_t hash, unsigned char depth, short *beta, move_t *best_move, int *cutoff)
{
    ttable_entry_t *ttentry = TTABLE_retrieve(ttable, hash);
    if(ttentry) {
        if(ttentry->depth >= depth) {
            int score = ttentry->score;
            if(ttentry->type == TTABLE_TYPE_UPPER_BOUND) {
                if(score < *beta) {
                    short min = SEARCH_MIN_RESULT(depth);
                    *cutoff = 1;
                    return (score < min) ? min : score;
                }
            } else { /* TTABLE_TYPE_LOWER_BOUND */
                if(score >= *beta) {
                    short max = SEARCH_MAX_RESULT(depth);
                    *cutoff = 1;
                    return (score > max) ? max : score;
                }
            }
        }
        
        *cutoff = 0;
        *best_move = ttentry->best_move;
    }
    
    return 0;
}

static inline void SEARCH_transpositiontable_store(ttable_t *ttable, bitboard_t hash, unsigned char depth, short best_score, move_t best_move, short beta)
{
    best_move &= ~MOVE_SCORE_MASK;
    if(best_score < beta) {
        TTABLE_store(ttable, hash, depth, TTABLE_TYPE_UPPER_BOUND, best_score, best_move);
    } else {
        TTABLE_store(ttable, hash, depth, TTABLE_TYPE_LOWER_BOUND, best_score, best_move);
    }
}
