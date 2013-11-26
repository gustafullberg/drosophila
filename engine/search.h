#ifndef _SEARCH_H
#define _SEARCH_H

#include "defines.h"
#include "state.h"

int minimax_search(chess_state_t *s, move_t *stack, int depth, int *score);
int search_is_check(chess_state_t *s, int color);
int search_is_mate(chess_state_t *state, move_t *stack);

#endif

