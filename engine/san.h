#ifndef _SAN_H
#define _SAN_H

#include "state.h"

move_t SAN_parse_move(const chess_state_t *state, const char *san);

#endif

