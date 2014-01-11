#ifndef _SEARCH_ALPHABETA_H
#define _SEARCH_ALPHABETA_H

#include "state.h"
#include "search.h"

int SEARCH_alphabeta(const chess_state_t *state, search_state_t *search_state, short depth, move_t *move, int alpha, int beta);
int SEARCH_alphabeta_quiescence(const chess_state_t *state, search_state_t *search_state, int alpha, int beta);

#endif
