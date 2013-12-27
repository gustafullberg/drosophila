#ifndef _EVAL_H
#define _EVAL_H

#include "state.h"

int EVAL_evaluate_board(const chess_state_t *s);
int EVAL_position_is_attacked(const chess_state_t *s, const int color, const int pos);

#endif
