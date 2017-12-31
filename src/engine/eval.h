#ifndef EVAL_H
#define EVAL_H

#include "state.h"

/* Material value */
#define PAWN_VALUE      20
#define KNIGHT_VALUE    65
#define BISHOP_VALUE    65
#define ROOK_VALUE     100
#define QUEEN_VALUE    195
#define KING_VALUE    2000 /* Only used for SEE */
#define BISHOP_PAIR     10

extern const short piecesquare[7][64];

void  EVAL_pawn_types(const chess_state_t *s, bitboard_t attack[NUM_COLORS], bitboard_t *passedPawns, bitboard_t *isolatedPawns);
short EVAL_evaluate_board(const chess_state_t *s);
int   EVAL_position_is_attacked(const chess_state_t *s, const int color, const int pos);
int   EVAL_insufficient_material(const chess_state_t *s);
int   EVAL_fifty_move_rule(const chess_state_t *s);

#endif
