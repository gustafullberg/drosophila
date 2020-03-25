#ifndef SEE_H
#define SEE_H

#include "state.h"

short see(const chess_state_t *s, const move_t move);
int SEE_capture_less_valuable(const move_t move);

#endif
