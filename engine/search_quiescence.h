#ifndef _SEARCH_QUIESCENCE_H
#define _SEARCH_QUIESCENCE_H

#include "state.h"

int SEARCH_quiescence_min(chess_state_t *s1, move_t *stack, int alpha, int beta);
int SEARCH_quiescence_max(chess_state_t *s1, move_t *stack, int alpha, int beta);

#endif
