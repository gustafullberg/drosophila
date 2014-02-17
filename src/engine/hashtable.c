#include <stdlib.h>
#include "hashtable.h"
#include "search.h"

hashtable_t *HASHTABLE_create(const int log2_num_entries)
{
    int num_entries = 1 << log2_num_entries;
    int num_pawn_entries = 1 << 17;
    
    hashtable_t *h = (hashtable_t*)malloc(sizeof(hashtable_t));
    
    h->entries = calloc(num_entries, sizeof(transposition_entry_t));
    h->key_mask = num_entries - 1;
    
    h->pawn_entries = calloc(num_pawn_entries, sizeof(pawn_entry_t));
    h->pawn_key_mask = num_pawn_entries - 1;
    
    return h;
}

void HASHTABLE_destroy(hashtable_t *h)
{
    free(h->pawn_entries);
    free(h->entries);
    free(h);
}

void HASHTABLE_transition_store(hashtable_t *h, const bitboard_t hash, const unsigned char depth, const unsigned char type, const short score, const move_t best_move)
{
    int index = (int)(hash & h->key_mask);
    transposition_entry_t *entry = &h->entries[index];

    entry->hash = hash;
    entry->best_move = best_move;
    entry->score = score;
    entry->depth = depth;
    entry->type = type;
}

transposition_entry_t *HASHTABLE_transition_retrieve(const hashtable_t *h, const bitboard_t hash)
{
    int index = (int)(hash & h->key_mask);
    if(h->entries[index].hash == hash) {
        return &h->entries[index];
    }

    return NULL;
}

void HASHTABLE_pawn_store(hashtable_t *h, const uint32_t hash, const short score)
{
    int index = (int)(hash & h->pawn_key_mask);
    pawn_entry_t *p = &h->pawn_entries[index];
    
    p->hash = hash;
    p->score = score;
}

int HASHTABLE_pawn_retrieve(const hashtable_t *h, const uint32_t hash, short *score)
{
    int index = (int)(hash & h->pawn_key_mask);
    if(h->pawn_entries[index].hash == hash) {
        *score = h->pawn_entries[index].score;
        return 1;
    }
    return 0;
}
