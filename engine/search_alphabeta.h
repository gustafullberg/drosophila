#ifndef _SEARCH_ALPHABETA_H
#define _SEARCH_ALPHABETA_H

#include "move.h"
#include "state.h"

int SEARCH_alphabeta_min(chess_state_t *s1, move_t *stack, int depth, move_t *move, int alpha, int beta);
int SEARCH_alphabeta_max(chess_state_t *s1, move_t *stack, int depth, move_t *move, int alpha, int beta);

#endif
