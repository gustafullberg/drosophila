#ifndef MOVEORDER_H
#define MOVEORDER_H

#include "state.h"

int MOVEORDER_order_moves(const chess_state_t *s, move_t moves[], int num_moves, const move_t best_guess);
int MOVEORDER_order_moves_quiescence(const chess_state_t *s, move_t moves[], int num_moves);

#endif

