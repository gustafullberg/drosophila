#ifndef _SEARCH_MTDF_H
#define _SEARCH_MTDF_H

#include "state.h"
#include "ttable.h"

int SEARCH_mtdf(chess_state_t *s, move_t *stack, ttable_t *ttable, short depth, move_t *move, int guess);

#endif
