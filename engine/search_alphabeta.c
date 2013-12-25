#include "search_alphabeta.h"
#include "search.h"
#include "eval.h"
#include "moveorder.h"

static inline ttable_entry_t *SEARCH_transpositiontable_retrieve(ttable_t *ttable, bitboard_t hash, int depth, int *alpha, int *beta, move_t *best_move);
static inline void SEARCH_transpositiontable_store(ttable_t *ttable, bitboard_t hash, int depth, int best_score, move_t best_move, int alpha, int beta);

/* Alpha-Beta search with Nega Max */
int SEARCH_alphabeta(chess_state_t *s1, move_t *stack, ttable_t *ttable, short depth, move_t *move, int inalpha, int inbeta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    int best_score = SEARCH_MIN_RESULT(depth);
    move_t next_move;
    chess_state_t s2;

    int alpha = inalpha;
    int beta = inbeta;
    *move = 0;

    if(depth <= 0) {
#if USE_QUIESCENCE
        return SEARCH_alphabeta_quiescence(s1, stack, ttable, alpha, beta);
#else
        return EVAL_evaluate_board(s1);
#endif
    }
    
#if USE_TRANSPOSITION_TABLE
    SEARCH_transpositiontable_retrieve(ttable, s1->hash, depth, &alpha, &beta, move);
    if(alpha >= beta) {
        return alpha;
    }
#endif

    num_moves = STATE_generate_moves(s1, stack);
    
#if USE_MOVE_ORDERING
    MOVEORDER_order_moves(stack, num_moves, *move);
#endif

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
            *move = stack[i];
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
        if(SEARCH_is_check(s1, s1->player)) {
            /* Checkmate (worst case) */
        } else {
            /* Stalemate */
            best_score = 0;
        }
    }
    
#if USE_TRANSPOSITION_TABLE
    SEARCH_transpositiontable_store(ttable, s1->hash, depth, best_score, *move, inalpha, inbeta);
#endif
    return best_score;
}

/* Alpha-Beta quiescence search with Nega Max */
int SEARCH_alphabeta_quiescence(chess_state_t *s1, move_t *stack, ttable_t *ttable, int inalpha, int inbeta)
{
    int num_moves;
    int num_legal_moves;
    int i;
    int score;
    int best_score;
    move_t move;
    chess_state_t s2;

    int alpha = inalpha;
    int beta = inbeta;
    move = 0;

    /* Stand-pat */
    best_score = EVAL_evaluate_board(s1);
    if(best_score > alpha) {
        if(best_score >= beta) {
            return beta;
        }
        alpha = best_score;
    }
    
#if USE_TRANSPOSITION_TABLE
    SEARCH_transpositiontable_retrieve(ttable, s1->hash, 0, &alpha, &beta, &move);
    if(alpha >= beta) {
        return alpha;
    }
#endif
    
    num_moves = STATE_generate_moves(s1, stack);
    
#if USE_MOVE_ORDERING
    MOVEORDER_order_moves(stack, num_moves, move);
#endif
    
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
            if(best_score >= beta) {
                /* Beta-cuttoff */
                break;
            }
            if(best_score > alpha) {
                alpha = best_score;
            }
        }
    }
    
#if USE_TRANSPOSITION_TABLE
    SEARCH_transpositiontable_store(ttable, s1->hash, 0, best_score, 0, inalpha, inbeta);
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
