#ifndef _SEARCH_ALPHABETA_H
#define _SEARCH_ALPHABETA_H

#include "move.h"
#include "state.h"

int SEARCH_alphabeta(chess_state_t *s1, move_t *stack, int depth, move_t *move, int alpha, int beta);

#endif
