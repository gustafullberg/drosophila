#include "moveorder.h"
#include "see.h"
#include "eval.h"

#define HASH_BONUS 0x3FF
#define GOOD_CAPTURE 0x300
#define BAD_CAPTURE 0x100
#define RECAPTURE 1
#define PROMOTION 0x200
#define KILLER1 0x100
#define KILLER2 0x0F0

static const int piece_value[6] = { 1, 3, 3, 5, 9, 20 };

void MOVEORDER_rate_moves(const chess_state_t *s, move_t moves[], int num_moves, const move_t hash_move, const move_t *killer, const int history_heuristic[64][64])
{
    /* Get score for each move */
    for(int i = 0; i < num_moves; i++) {
        int score = 0;
        if(moves[i] == hash_move) {
            /* Hash move */
            score = HASH_BONUS;
        } else {
            const int pos_from = MOVE_GET_POS_FROM(moves[i]);
            const int pos_to   = MOVE_GET_POS_TO(moves[i]);
            const int own_type = MOVE_GET_TYPE(moves[i]);

            if(MOVE_IS_CAPTURE_OR_PROMOTION(moves[i])) {
                /* Captures */
                if(MOVE_IS_CAPTURE(moves[i])) {
                    /* SEE */
                    int see_val = see(s, moves[i]);
                    if(see_val >= 0) {
                        score = GOOD_CAPTURE + see_val;
                    } else {
                        score = BAD_CAPTURE + see_val;
                    }

                    /* Recapture bonus */
                    if(MOVE_IS_CAPTURE(s->last_move)) {
                        if(pos_to == MOVE_GET_POS_TO(s->last_move)) {
                            score += RECAPTURE;
                        }
                    }
                }

                /* Promotions */
                if(MOVE_IS_PROMOTION(moves[i]) && MOVE_PROMOTION_TYPE(moves[i]) == QUEEN) {
                    score += PROMOTION;
                }
            } else { /* Quiet moves */
                /* Killer moves */
                if(moves[i] == killer[0]) score += KILLER1;
                else if(moves[i] == killer[1]) score += KILLER2;

                /* History heuristic */
                int hist_val = history_heuristic[pos_from][pos_to];
                score += BITBOARD_find_bit_reversed(hist_val | 1);
            }
        }

        moves[i] |= score << MOVE_SCORE_SHIFT;
    }
}

void MOVEORDER_rate_moves_quiescence(const chess_state_t *s, move_t moves[], int num_moves)
{
    /* Get score for each move */
    for(int i = 0; i < num_moves; i++) {
        int score = 0;

        const int pos_to   = MOVE_GET_POS_TO(moves[i]);
        const int own_type = MOVE_GET_TYPE(moves[i]);

        /* Captures */
        if(MOVE_IS_CAPTURE(moves[i])) {
            /* MVV-LVA */
            int captured_type = MOVE_GET_CAPTURE_TYPE(moves[i]);
            int mvv_lva = 20*piece_value[captured_type] - piece_value[own_type];
            score = GOOD_CAPTURE + mvv_lva;

            /* Recapture bonus */
            if(MOVE_IS_CAPTURE(s->last_move)) {
                if(pos_to == MOVE_GET_POS_TO(s->last_move)) {
                    score += RECAPTURE;
                }
            }
        }

        if(MOVE_IS_PROMOTION(moves[i]) && MOVE_PROMOTION_TYPE(moves[i]) == QUEEN) {
            score += PROMOTION;
        }

        moves[i] |= score << MOVE_SCORE_SHIFT;
    }
}

void MOVEORDER_best_move_first(move_t moves[], int num_moves)
{
    move_t *best = moves + 0;
    move_t *last = moves + num_moves;
    move_t *p = best;
    while(++p < last) {
        if(*p > *best) best = p;
    }
    move_t tmp = *best & ~MOVE_SCORE_MASK;
    *best = moves[0];
    moves[0] = tmp;
}
