#include <stdlib.h>
#include "hashtable.h"
#include "search.h"

static int log2i(int n)
{
    int l = 0;
    while(n >>= 1) l++;
    return l;
}

hashtable_t *HASHTABLE_create(const int size_mb)
{
    int num_entries = 1 << log2i(size_mb * 1024 * 1024 / sizeof(transposition_entry_t));

    hashtable_t *h = (hashtable_t*)malloc(sizeof(hashtable_t));
    
    h->entries = calloc(num_entries, sizeof(transposition_entry_t));
    h->key_mask = num_entries - 1;

    return h;
}

void HASHTABLE_destroy(hashtable_t *h)
{
    free(h->entries);
    free(h);
}

void HASHTABLE_transition_store(hashtable_t *h, const bitboard_t hash, const unsigned char depth, const unsigned char type, const short score, const move_t best_move)
{
    int index = (int)(hash & h->key_mask);
    transposition_entry_t *entry = &h->entries[index];

    if(entry->hash == (hash >> 32)) {
        /* Update existing entry */
        entry->best_move = best_move;
        if(type == TTABLE_TYPE_LOWER_BOUND) {
            entry->score_low = score;
            entry->depth_low = depth;
        } else {
            entry->score_high = score;
            entry->depth_high = depth;
        }
    } else { 
        /* New entry */
        entry->hash = hash >> 32;
        entry->best_move = best_move;
        if(type == TTABLE_TYPE_LOWER_BOUND) {
            entry->score_low = score;
            entry->score_high = SEARCH_MAX_RESULT(MAX_SEARCH_DEPTH);
        } else {
            entry->score_low = SEARCH_MIN_RESULT(MAX_SEARCH_DEPTH);
            entry->score_high = score;
        }
        entry->depth_low = depth;
        entry->depth_high = depth;
    }
}

transposition_entry_t *HASHTABLE_transition_retrieve(const hashtable_t *h, const bitboard_t hash)
{
    int index = (int)(hash & h->key_mask);
    if(h->entries[index].hash == hash >> 32) {
        return &h->entries[index];
    }

    return NULL;
}
