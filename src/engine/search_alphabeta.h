#ifndef _SEARCH_ALPHABETA_H
#define _SEARCH_ALPHABETA_H

#include "state.h"
#include "search.h"

short SEARCH_alphabeta(const chess_state_t *state, search_state_t *search_state, unsigned char depth, move_t *move, short alpha, short beta);
short SEARCH_alphabeta_quiescence(const chess_state_t *state, search_state_t *search_state, short alpha, short beta);

#endif
