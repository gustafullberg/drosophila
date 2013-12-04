#ifndef _SEARCH_MINIMAX_H
#define _SEARCH_MINIMAX_H

#include "state.h"

int SEARCH_minimax(chess_state_t *s1, move_t *stack, int depth, move_t *move);
int SEARCH_minimax_quiescence(chess_state_t *s1, move_t *stack);

#endif
