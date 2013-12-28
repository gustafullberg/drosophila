#ifndef _SEARCH_ALPHABETA_H
#define _SEARCH_ALPHABETA_H

#include "state.h"
#include "ttable.h"

int SEARCH_alphabeta(const chess_state_t *state, ttable_t *ttable, short depth, move_t *move, int alpha, int beta);
int SEARCH_alphabeta_quiescence(const chess_state_t *state, ttable_t *ttable, int alpha, int beta);

#endif
