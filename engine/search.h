#ifndef _SEARCH_H
#define _SEARCH_H

#include "defines.h"
#include "state.h"
#include "ttable.h"

int SEARCH_perform_search(chess_state_t *s, move_t *stack, ttable_t *ttable, short depth, int *score);
int SEARCH_is_check(chess_state_t *s, int color);
int SEARCH_is_mate(chess_state_t *state, move_t *stack);

#endif

