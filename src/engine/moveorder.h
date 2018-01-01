#ifndef MOVEORDER_H
#define MOVEORDER_H

#include "state.h"

int MOVEORDER_rate_moves(const chess_state_t *s, move_t moves[], int num_moves, const move_t best_guess);
int MOVEORDER_rate_moves_quiescence(const chess_state_t *s, move_t moves[], int num_moves);
void MOVEORDER_best_move_first(move_t moves[], int num_moves);

#endif

