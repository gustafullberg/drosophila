#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include "bitboard.h"
#include "state.h"

typedef struct {
    bitboard_t      hash;
    move_t          best_move;
    short           score;
    unsigned char   depth;
    unsigned char   type;
} transposition_entry_t;

typedef struct {
    uint32_t        hash;
    int             score;
} pawn_entry_t;

typedef struct {
    transposition_entry_t   *entries;
    bitboard_t              key_mask;
    pawn_entry_t            *pawn_entries;
    uint32_t                pawn_key_mask;
} hashtable_t;

#define TTABLE_TYPE_LOWER_BOUND     0
#define TTABLE_TYPE_UPPER_BOUND     1

hashtable_t *HASHTABLE_create(int log2_num_entries);
void HASHTABLE_destroy(hashtable_t *h);
void HASHTABLE_transition_store(hashtable_t *h, bitboard_t hash, unsigned char depth, unsigned char type, short score, move_t best_move);
transposition_entry_t *HASHTABLE_transition_retrieve(hashtable_t *h, bitboard_t hash);
void HASHTABLE_pawn_store(hashtable_t *h, uint32_t hash, int score);
int HASHTABLE_pawn_retrieve(hashtable_t *h, uint32_t hash, int *score);

static inline void HASHTABLE_transition_prefetch(hashtable_t *h, bitboard_t hash)
{
#if __GNUC__
    int index = (int)(hash & h->key_mask);
    __builtin_prefetch(&h->entries[index]);
#endif
}

#endif

