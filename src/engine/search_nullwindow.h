#ifndef _SEARCH_NULLWINDOW_H
#define _SEARCH_NULLWINDOW_H

#include "state.h"
#include "search.h"

short SEARCH_nullwindow(const chess_state_t *state, search_state_t *search_state, unsigned char depth, int allow_nullmove, move_t *move, short beta);
short SEARCH_nullwindow_quiescence(const chess_state_t *state, search_state_t *search_state, short beta);

#endif
