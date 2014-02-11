#ifndef _TTABLE_H
#define _TTABLE_H

#include "bitboard.h"
#include "state.h"

typedef struct {
    bitboard_t      hash;
    move_t          best_move;
    short           score;
    unsigned char   depth;
    unsigned char   type;
} ttable_entry_t;

typedef struct {
    ttable_entry_t *entries;
    bitboard_t      key_mask;
} ttable_t;

#define TTABLE_TYPE_LOWER_BOUND     0
#define TTABLE_TYPE_UPPER_BOUND     1

ttable_t *TTABLE_create(int log2_num_entries);
void TTABLE_destroy(ttable_t *t);
void TTABLE_store(ttable_t *t, bitboard_t hash, unsigned char depth, unsigned char type, short score, move_t best_move);
ttable_entry_t *TTABLE_retrieve(ttable_t *t, bitboard_t hash);

static inline void TTABLE_prefetch(ttable_t *t, bitboard_t hash)
{
#if __GNUC__
    int index = (int)(hash & t->key_mask);
    __builtin_prefetch(&t->entries[index]);
#endif
}

#endif

