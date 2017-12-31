#ifndef SEARCH_MTDF_H
#define SEARCH_MTDF_H

#include "state.h"
#include "search.h"

short SEARCH_mtdf(const chess_state_t *s, search_state_t *search_state, const unsigned char depth, move_t *move, short guess);
short SEARCH_mtdf_iterative(const chess_state_t *s, search_state_t *search_state, move_t *move);

#endif
