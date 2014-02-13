#ifndef _EVAL_H
#define _EVAL_H

#include "state.h"
#include "ttable.h"

extern const int piecesquare[7][64];

int EVAL_evaluate_board(const chess_state_t *s, ttable_t *t);
int EVAL_position_is_attacked(const chess_state_t *s, const int color, const int pos);

#endif
