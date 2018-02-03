#include "moveorder.h"
#include "see.h"
#include "eval.h"

static const int piece_value[6] = { PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE+BISHOP_PAIR, ROOK_VALUE, QUEEN_VALUE, 2*QUEEN_VALUE };

static int MOVEORDER_compute_score(const chess_state_t *s, const move_t move, const move_t *killer, const int history_heuristic[64][64])
{
    unsigned int score = 100;
    
    const int pos_from = MOVE_GET_POS_FROM(move);
    const int pos_to   = MOVE_GET_POS_TO(move);
    const int own_type = MOVE_GET_TYPE(move);

    if(MOVE_IS_CAPTURE_OR_PROMOTION(move)) {
        if(MOVE_IS_CAPTURE(move)) {
            /* MVV-LVA */
            int captured_type = MOVE_GET_CAPTURE_TYPE(move);
            score += piece_value[KING] + piece_value[captured_type] - piece_value[own_type];

            /* Recapture bonus */
            if(MOVE_IS_CAPTURE(s->last_move)) {
                if(pos_to == MOVE_GET_POS_TO(s->last_move)) {
                    score += PAWN_VALUE;
                }
            }
        }

        if(MOVE_IS_PROMOTION(move)) {
            score += piece_value[MOVE_PROMOTION_TYPE(move)] - piece_value[PAWN];
        }
    } else {
        if(killer) {
            if(move == killer[0]) score += 55;
            else if(move == killer[1]) score += 1;
        }
        if(history_heuristic) {
            int hist_val = history_heuristic[pos_from][pos_to];
            score += BITBOARD_find_bit_reversed(hist_val | 1) * 2;
        }
    }

    /* Piece-square tables */
    int pos_mask = s->player * 0x38;
    score += piecesquare[own_type][pos_to^pos_mask] - piecesquare[own_type][pos_from^pos_mask];

    return score;
}

int MOVEORDER_rate_moves(const chess_state_t *s, move_t moves[], int num_moves, const move_t best_guess, const move_t *killer, const int history_heuristic[64][64])
{
    /* Get score for each move */
    for(int i = 0; i < num_moves; i++) {
        int score;
        if(moves[i] == best_guess) {
            /* This move is already tried. Remove it from the list */
            moves[i] = moves[num_moves-1];
            num_moves--;
        }
        score = MOVEORDER_compute_score(s, moves[i], killer, history_heuristic);
        moves[i] |= score << MOVE_SCORE_SHIFT;
    }

    return num_moves;
}

int MOVEORDER_rate_moves_quiescence(const chess_state_t *s, move_t moves[], int num_moves)
{
    /* Get score for each move */
    for(int i = 0; i < num_moves; i++) {
        int score = MOVEORDER_compute_score(s, moves[i], 0, 0);
        moves[i] |= score << MOVE_SCORE_SHIFT;
    }

    return num_moves;
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
