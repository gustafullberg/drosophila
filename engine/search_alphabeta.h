#ifndef _SEARCH_ALPHABETA_H
#define _SEARCH_ALPHABETA_H

#include "state.h"
#include "ttable.h"

int SEARCH_alphabeta(chess_state_t *s1, move_t *stack, ttable_t *ttable, short depth, move_t *move, int alpha, int beta);
int SEARCH_alphabeta_quiescence(chess_state_t *s1, move_t *stack, int alpha, int beta);

#endif
