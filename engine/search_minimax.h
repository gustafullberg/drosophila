#ifndef _SEARCH_MINIMAX_H
#define _SEARCH_MINIMAX_H

#include "state.h"

int SEARCH_minimax(const chess_state_t *s1, move_t *stack, short depth, move_t *move);
int SEARCH_minimax_quiescence(const chess_state_t *s1, move_t *stack);

#endif
