#include "moveorder.h"
#include "see.h"
#include "eval.h"

static const int piece_value[6] = { PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE+BISHOP_PAIR, ROOK_VALUE, QUEEN_VALUE, 2*QUEEN_VALUE };

void MOVEORDER_rate_moves(const chess_state_t *s, move_t moves[], int num_moves, const move_t hash_move, const move_t *killer, const int history_heuristic[64][64])
{
    /* Get score for each move */
    for(int i = 0; i < num_moves; i++) {
        int score = 100;
        if(moves[i] == hash_move) {
            /* Hash move */
            score = 0x3FF;
        } else {
            const int pos_from = MOVE_GET_POS_FROM(moves[i]);
            const int pos_to   = MOVE_GET_POS_TO(moves[i]);
            const int own_type = MOVE_GET_TYPE(moves[i]);

            if(MOVE_IS_CAPTURE_OR_PROMOTION(moves[i])) {
                /* Captures */
                if(MOVE_IS_CAPTURE(moves[i])) {
                    /* MVV-LVA */
                    int captured_type = MOVE_GET_CAPTURE_TYPE(moves[i]);
                    score += piece_value[KING] + piece_value[captured_type] - piece_value[own_type];

                    /* Recapture bonus */
                    if(MOVE_IS_CAPTURE(s->last_move)) {
                        if(pos_to == MOVE_GET_POS_TO(s->last_move)) {
                            score += PAWN_VALUE;
                        }
                    }
                }

                /* Promotions */
                if(MOVE_IS_PROMOTION(moves[i])) {
                    score += piece_value[MOVE_PROMOTION_TYPE(moves[i])] - piece_value[PAWN];
                }
            } else { /* Quiet moves */
                /* Killer moves */
                if(moves[i] == killer[0]) score += 55;
                else if(moves[i] == killer[0]) score += 1;

                /* History heuristic */
                int hist_val = history_heuristic[pos_from][pos_to];
                score += BITBOARD_find_bit_reversed(hist_val | 1) * 2;
            }
        }

        moves[i] |= score << MOVE_SCORE_SHIFT;
    }
}

void MOVEORDER_rate_moves_quiescence(const chess_state_t *s, move_t moves[], int num_moves)
{
    /* Get score for each move */
    for(int i = 0; i < num_moves; i++) {
        int score = 100;

        const int pos_to   = MOVE_GET_POS_TO(moves[i]);
        const int own_type = MOVE_GET_TYPE(moves[i]);

        /* Captures */
        if(MOVE_IS_CAPTURE(moves[i])) {
            /* MVV-LVA */
            int captured_type = MOVE_GET_CAPTURE_TYPE(moves[i]);
            score += piece_value[KING] + piece_value[captured_type] - piece_value[own_type];

            /* Recapture bonus */
            if(MOVE_IS_CAPTURE(s->last_move)) {
                if(pos_to == MOVE_GET_POS_TO(s->last_move)) {
                    score += PAWN_VALUE;
                }
            }
        }

        /* Promotions */
        if(MOVE_IS_PROMOTION(moves[i])) {
            score += piece_value[MOVE_PROMOTION_TYPE(moves[i])] - piece_value[PAWN];
        }

        moves[i] |= score << MOVE_SCORE_SHIFT;
    }
}

void MOVEORDER_best_move_first(move_t moves[], int num_moves)
{
    int best = 0;
    for(int i = 1; i < num_moves; i++) {
        if(moves[i] > moves[best]) best = i;
    }
    move_t tmp = moves[best] & ~MOVE_SCORE_MASK;
    moves[best] = moves[0];
    moves[0] = tmp;
}
