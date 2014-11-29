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
    short           score;
} pawn_entry_t;

typedef struct {
    transposition_entry_t   *entries;
    bitboard_t              key_mask;
    //pawn_entry_t            *pawn_entries;
    //uint32_t                pawn_key_mask;
} hashtable_t;

#define TTABLE_TYPE_LOWER_BOUND     0
#define TTABLE_TYPE_UPPER_BOUND     1

hashtable_t *HASHTABLE_create(const int size_mb);
void HASHTABLE_destroy(hashtable_t *h);
void HASHTABLE_transition_store(hashtable_t *h, const bitboard_t hash, const unsigned char depth, const unsigned char type, const short score, const move_t best_move);
transposition_entry_t *HASHTABLE_transition_retrieve(const hashtable_t *h, const bitboard_t hash);
//void HASHTABLE_pawn_store(hashtable_t *h, const uint32_t hash, const short score);
//int HASHTABLE_pawn_retrieve(const hashtable_t *h, const uint32_t hash, short *score);

static inline void HASHTABLE_transition_prefetch(const hashtable_t *h, const bitboard_t hash)
{
    int index = (int)(hash & h->key_mask);
#if __GNUC__
    __builtin_prefetch(&h->entries[index]);
#elif _MSC_VER
	_mm_prefetch((const char*)&h->entries[index], _MM_HINT_T0);
#endif
}

#endif

