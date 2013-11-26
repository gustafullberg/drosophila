#ifndef _STATE_H
#define _STATE_H

#include "defines.h"
#include "bitboard.h"
#include "move.h"

#define WHITE_PIECES    0
#define BLACK_PIECES    (NUM_TYPES)
#define OCCUPIED        (NUM_COLORS*NUM_TYPES)

#define STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_SHIFT     0
#define STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_MASK      (1<<(STATE_FLAGS_QUEEN_CASTLE_POSSIBLE_SHIFT))

#define STATE_FLAGS_KING_CASTLE_POSSIBLE_SHIFT      1
#define STATE_FLAGS_KING_CASTLE_POSSIBLE_MASK       (1<<(STATE_FLAGS_KING_CASTLE_POSSIBLE_SHIFT))

#define STATE_FLAGS_EN_PASSANT_POSSIBLE_SHIFT       2
#define STATE_FLAGS_EN_PASSANT_POSSIBLE_MASK        (1<<(STATE_FLAGS_EN_PASSANT_POSSIBLE_SHIFT))

#define STATE_FLAGS_EN_PASSANT_FILE_SHIFT           3
#define STATE_FLAGS_EN_PASSANT_FILE_MASK            (0x7<<(STATE_FLAGS_EN_PASSANT_FILE_SHIFT))


typedef struct chess_state_t {
    bitboard_t  bitboard[NUM_COLORS*NUM_TYPES+1];
    int         flags[2];
    char	    player;
} chess_state_t;

void state_reset(chess_state_t *s);
int state_generate_moves(chess_state_t *s, move_t *stack);
void state_clone(chess_state_t *s_dst, const chess_state_t *s_src);
int state_apply_move(chess_state_t *s, const move_t move);
int state_position_is_attacked(const chess_state_t *s, const int color, const int pos);
int state_evaluate(chess_state_t *s);

#endif
