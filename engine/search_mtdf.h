#ifndef _SEARCH_MTDF_H
#define _SEARCH_MTDF_H

#include "state.h"
#include "ttable.h"

int SEARCH_mtdf(const chess_state_t *s, ttable_t *ttable, short depth, move_t *move, int guess);
int SEARCH_mtdf_iterative(const chess_state_t *s, ttable_t *ttable, short max_depth, move_t *move);

#endif
