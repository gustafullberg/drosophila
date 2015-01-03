#include "search_nullwindow.h"
#include "search.h"
#include "eval.h"
#include "moveorder.h"
#include "time.h"
#include "see.h"

static inline short SEARCH_transpositiontable_retrieve(const hashtable_t *hashtable, const bitboard_t hash, const unsigned char depth, short beta, move_t *best_move, int *cutoff);
static inline void SEARCH_transpositiontable_store(hashtable_t *hashtable, const bitboard_t hash, const unsigned char depth, const short best_score, move_t best_move, const short beta);

/* Alpha-Beta search with Nega Max and null-window */
short SEARCH_nullwindow(const chess_state_t *state, search_state_t *search_state, unsigned char depth, move_t *move, short beta)
{
    int num_moves;
    int num_legal_moves = 0;
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
    int do_futility_pruning = 0;

    *move = 0;
    
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

    HASHTABLE_transition_prefetch(search_state->hashtable, state->hash);

    is_in_check = SEARCH_is_check(state, state->player);
    
    if(is_in_check) {
        /* Check extension */
        depth += 1;
    }

    if(depth == 0) {
        /* Quiescence search */
        return SEARCH_nullwindow_quiescence(state, search_state, beta);
    }
    
    ttable_score = SEARCH_transpositiontable_retrieve(search_state->hashtable, state->hash, depth, beta, move, &cutoff);
    if(cutoff) {
        if(*move || state->last_move) {
            return ttable_score;
        }
    }

    /* Null move pruning */
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

    /* Try hash move */
    if(!skip_move_generation && *move) {
        next_state = *state;
        STATE_apply_move(&next_state, *move);
        if(!SEARCH_is_check(&next_state, state->player)) {
            num_legal_moves++;
            HISTORY_push(search_state->history, next_state.hash);
            best_score = -SEARCH_nullwindow(&next_state, search_state, depth-1, &next_move, -beta+1);
            HISTORY_pop(search_state->history);
            if(best_score >= beta) {
                skip_move_generation = 1;
            }
        }
    }

    if(!skip_move_generation) {
        num_moves = STATE_generate_moves(state, moves);
        num_moves = MOVEORDER_order_moves(state, moves, num_moves, *move);

        /* Check if node is eligible for futility pruning */
        if(depth == 1 && !is_in_check) {
            if(beta > EVAL_evaluate_board(state) + 10) {
                do_futility_pruning = 1;
            }
        }

        for(i = 0; i < num_moves; i++) {

            /* Futility pruning */
            if(do_futility_pruning) {
                if(num_legal_moves > 1 && !MOVE_IS_CAPTURE_OR_PROMOTION(moves[i])) {
                    continue;
                }
            }

            next_state = *state;
            STATE_apply_move(&next_state, moves[i]);
            if(SEARCH_is_check(&next_state, state->player)) {
                continue;
            }
            num_legal_moves++;
            

            HISTORY_push(search_state->history, next_state.hash);
            if(HISTORY_is_repetition(search_state->history, next_state.halfmove_clock)) {
                /* Repetition detected */
                score = 0;
            } else {
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
                    /* Normal search */
                    score = -SEARCH_nullwindow(&next_state, search_state, depth-1, &next_move, -beta+1);
                }
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

            HISTORY_pop(search_state->history);
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

    if(!search_state->abort_search) {
        SEARCH_transpositiontable_store(search_state->hashtable, state->hash, depth, best_score, *move, beta);
    }
    return best_score;
}

/* Alpha-Beta quiescence search with Nega Max and null-window */
short SEARCH_nullwindow_quiescence(const chess_state_t *state, search_state_t *search_state, short beta)
{
    int num_moves;
    int i;
    short score;
    short best_score;
    chess_state_t next_state;
    move_t moves[256];

    /* Stand-pat */
    best_score = EVAL_evaluate_board(state);
    search_state->num_nodes_searched++;
    if(best_score >= beta) {
        return best_score;
    }
    
    num_moves = STATE_generate_moves_quiescence(state, moves);
    num_moves = MOVEORDER_order_moves_quiescence(state, moves, num_moves);
    
    for(i = 0; i < num_moves; i++) {
        /* Only try captures with SEE >= 0 */
        if(MOVE_IS_CAPTURE(moves[i])) {
            if(see(state, moves[i]) < 0) {
                continue;
            }
        }
        
        next_state = *state;
        STATE_apply_move(&next_state, moves[i]);
        if(SEARCH_is_check(&next_state, state->player)) {
            continue;
        }

        score = -SEARCH_nullwindow_quiescence(&next_state, search_state, -beta+1);
        if(score > best_score) {
            best_score = score;
            if(best_score >= beta) {
                /* Beta-cuttoff */
                break;
            }
        }
    }
    
    return best_score;
}

static inline short SEARCH_transpositiontable_retrieve(const hashtable_t *hashtable, const bitboard_t hash, const unsigned char depth, short beta, move_t *best_move, int *cutoff)
{
    transposition_entry_t *ttentry = HASHTABLE_transition_retrieve(hashtable, hash);
    if(ttentry) {
        *best_move = ttentry->best_move;

        if(ttentry->depth >= depth) {
            short score = ttentry->score;
            if(ttentry->type == TTABLE_TYPE_UPPER_BOUND) {
                if(score < beta) {
                    short min = SEARCH_MIN_RESULT(0);
                    *cutoff = 1;
                    return (score <= min) ? score + ttentry->depth - depth : score;
                }
            } else { /* TTABLE_TYPE_LOWER_BOUND */
                if(score >= beta) {
                    short max = SEARCH_MAX_RESULT(0);
                    *cutoff = 1;
                    return (score >= max) ? score - ttentry->depth + depth : score;
                }
            }
        }
        
        *cutoff = 0;
    }
    
    return 0;
}

static inline void SEARCH_transpositiontable_store(hashtable_t *hashtable, const bitboard_t hash, const unsigned char depth, const short best_score, move_t best_move, const short beta)
{
    best_move &= ~MOVE_SCORE_MASK;
    if(best_score < beta) {
        HASHTABLE_transition_store(hashtable, hash, depth, TTABLE_TYPE_UPPER_BOUND, best_score, best_move);
    } else {
        HASHTABLE_transition_store(hashtable, hash, depth, TTABLE_TYPE_LOWER_BOUND, best_score, best_move);
    }
}
