#ifndef _EVAL_H
#define _EVAL_H

#include "state.h"
#include "hashtable.h"

extern const short piecesquare[7][64];
extern const short piece_value[NUM_TYPES];

short EVAL_evaluate_board(const chess_state_t *s, hashtable_t *t);
int   EVAL_position_is_attacked(const chess_state_t *s, const int color, const int pos);
short EVAL_material_midgame(const chess_state_t *s);

static inline short EVAL_get_piecesquare(const int color, const int type, const int pos)
{
    int pos_color = pos ^ (color * 0x38);
    return piecesquare[type][pos_color];
}

#endif
