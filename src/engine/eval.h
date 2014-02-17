#ifndef _EVAL_H
#define _EVAL_H

#include "state.h"
#include "hashtable.h"

extern const short piecesquare[7][64];

short EVAL_evaluate_board(const chess_state_t *s, hashtable_t *t);
int   EVAL_position_is_attacked(const chess_state_t *s, const int color, const int pos);

#endif
