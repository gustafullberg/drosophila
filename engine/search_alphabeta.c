#include "search_alphabeta.h"
#include "search.h"
#include "eval.h"
#include "moveorder.h"
#include "time.h"

static inline ttable_entry_t *SEARCH_transpositiontable_retrieve(ttable_t *ttable, bitboard_t hash, int depth, int *alpha, int *beta, move_t *best_move);
static inline void SEARCH_transpositiontable_store(ttable_t *ttable, bitboard_t hash, int depth, int best_score, move_t best_move, int alpha, int beta);

/* Alpha-Beta search with Nega Max */
int SEARCH_alphabeta(const chess_state_t *state, search_state_t *search_state, short depth, move_t *move, int inalpha, int inbeta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    int best_score = SEARCH_MIN_RESULT(depth);
    int skip_move_generation = 0;
    move_t next_move;
    chess_state_t next_state;
    move_t moves[256];
    int is_in_check;

    int alpha = inalpha;
    int beta = inbeta;
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

    is_in_check = SEARCH_is_check(state, state->player);
    
    if(is_in_check) {
        /* Check extension */
        depth += 1;
    }

    if(depth <= 0) {
#ifndef DISABLE_QUIESCENCE
        return SEARCH_alphabeta_quiescence(state, search_state, alpha, beta);
#else
        return EVAL_evaluate_board(state);
#endif
    }
    
#ifndef DISABLE_TRANSPOSITION_TABLE
    SEARCH_transpositiontable_retrieve(search_state->ttable, state->hash, depth, &alpha, &beta, move);
    if(alpha >= inbeta) return alpha;
    if(beta <= inalpha) return beta;
    if(alpha >= beta) {
        return alpha;
    }
#endif

#ifndef DISABLE_NULL_MOVE
    if(depth > 4 && state->last_move && !STATE_is_endgame(state)) {
        if(!SEARCH_is_check(state, state->player)) {
            next_state = *state;
            STATE_apply_move(&next_state, 0);
            score = -SEARCH_alphabeta(&next_state, search_state, depth-3, &next_move, -beta, -alpha);
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
            best_score = -SEARCH_alphabeta(&next_state, search_state, depth-1, &next_move, -beta, -alpha);
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

#ifndef DISABLE_LATE_MOVE_REDUCTION
            /* Late move reduction */
            if( num_legal_moves > 4                     && /* Four moves have been searched at full depth   */
                depth >= 3                              && /* No LMR in the last plies                      */
                !MOVE_IS_CAPTURE_OR_PROMOTION(moves[i]) && /* No LMR if capture / promotion                 */
                !is_in_check)                              /* No LMR if in check                            */
            {
                /* Search at reduced depth */
                score = -SEARCH_alphabeta(&next_state, search_state, depth-2, &next_move, -beta, -alpha);
                if(score > alpha) {
                    score = -SEARCH_alphabeta(&next_state, search_state, depth-1, &next_move, -beta, -alpha);
                }
            } else {
                score = -SEARCH_alphabeta(&next_state, search_state, depth-1, &next_move, -beta, -alpha);
            }
#else
            score = -SEARCH_alphabeta(&next_state, search_state, depth-1, &next_move, -beta, -alpha);
#endif
            if(score > best_score) {
                best_score = score;
                *move = moves[i];
                if(best_score >= beta) {
                    /* Beta-cuttoff */
                    break;
                }
                if(best_score > alpha) {
                    alpha = best_score;
                }
            }
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
    SEARCH_transpositiontable_store(search_state->ttable, state->hash, depth, best_score, *move, inalpha, inbeta);
#endif
    return best_score;
}

/* Alpha-Beta quiescence search with Nega Max */
int SEARCH_alphabeta_quiescence(const chess_state_t *state, search_state_t *search_state, int inalpha, int inbeta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    int best_score;
    int skip_move_generation = 0;
    move_t move;
    chess_state_t next_state;
    move_t moves[256];

    int alpha = inalpha;
    int beta = inbeta;
    move = 0;

    /* Stand-pat */
    best_score = EVAL_evaluate_board(state);
    if(best_score > alpha) {
        if(best_score >= beta) {
            return best_score;
        }
        alpha = best_score;
    }
    
#ifndef DISABLE_TRANSPOSITION_TABLE
    SEARCH_transpositiontable_retrieve(search_state->ttable, state->hash, 0, &alpha, &beta, &move);
    if(alpha >= inbeta) return alpha;
    if(beta <= inalpha) return beta;
    if(alpha >= beta) {
        return alpha;
    }
    
    if(move) {
        next_state = *state;
        STATE_apply_move(&next_state, move);
        if(!SEARCH_is_check(&next_state, state->player)) {
            best_score = -SEARCH_alphabeta_quiescence(&next_state, search_state, -beta, -alpha);
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
            score = -SEARCH_alphabeta_quiescence(&next_state, search_state, -beta, -alpha);
            if(score > best_score) {
                best_score = score;
                if(best_score >= beta) {
                    /* Beta-cuttoff */
                    break;
                }
                if(best_score > alpha) {
                    alpha = best_score;
                }
            }
        }
    }
    
#ifndef DISABLE_TRANSPOSITION_TABLE
    SEARCH_transpositiontable_store(search_state->ttable, state->hash, 0, best_score, 0, inalpha, inbeta);
#endif
    
    return best_score;
}

static inline ttable_entry_t *SEARCH_transpositiontable_retrieve(ttable_t *ttable, bitboard_t hash, int depth, int *alpha, int *beta, move_t *best_move)
{
    int score[2];
    ttable_entry_t *ttentry = TTABLE_retrieve(ttable, hash);
    if(ttentry) {
        if(ttentry->depth >= depth) {
            score[0] = SEARCH_clamp_score_to_valid_range(ttentry->score[0], depth);
            score[1] = SEARCH_clamp_score_to_valid_range(ttentry->score[1], depth);
            if(score[0] > *alpha) {
                *alpha = score[0];
            }
            if(score[1] < *beta) {
                *beta = score[1];
            }
        }
        *best_move = ttentry->best_move;
    }
    
    return ttentry;
}

static inline void SEARCH_transpositiontable_store(ttable_t *ttable, bitboard_t hash, int depth, int best_score, move_t best_move, int alpha, int beta)
{
    best_move &= ~MOVE_SCORE_MASK;
    if(best_score <= alpha) {
        TTABLE_store(ttable, hash, depth, TTABLE_TYPE_UPPER_BOUND, best_score, best_move);
    } else if(best_score >= beta) {
        TTABLE_store(ttable, hash, depth, TTABLE_TYPE_LOWER_BOUND, best_score, best_move);
    } else {
        TTABLE_store(ttable, hash, depth, TTABLE_TYPE_EXACT, best_score, best_move);
    }
}
