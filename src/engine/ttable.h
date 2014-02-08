#ifndef _TTABLE_H
#define _TTABLE_H

#include "bitboard.h"
#include "state.h"

typedef struct {
    bitboard_t  hash;
    move_t      best_move;
    short       score;
    short       depth_and_type;
} ttable_entry_t;

typedef struct {
    ttable_entry_t *entries;
    bitboard_t      key_mask;
} ttable_t;

#define TTABLE_TYPE_LOWER_BOUND     0
#define TTABLE_TYPE_UPPER_BOUND     1

#define TTABLE_GET_DEPTH(depth_and_type) ((depth_and_type) >> 1)
#define TTABLE_GET_TYPE(depth_and_type) ((depth_and_type) & 1)
#define TTABLE_SET_DEPTH_AND_TYPE(depth, type) ((depth << 1) | type)

ttable_t *TTABLE_create(int log2_num_entries);
void TTABLE_destroy(ttable_t *t);
void TTABLE_store(ttable_t *t, bitboard_t hash, short depth, short type, short score, move_t best_move);
ttable_entry_t *TTABLE_retrieve(ttable_t *t, bitboard_t hash);

#endif

