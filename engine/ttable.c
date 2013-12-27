#include <stdlib.h>
#include "ttable.h"
#include "search.h"

ttable_t *TTABLE_create(int log2_num_entries)
{
    int num_entries = 1 << log2_num_entries;
    
    ttable_t *t = malloc(sizeof(ttable_t));
    t->entries = calloc(num_entries, sizeof(ttable_entry_t));
    t->key_mask = num_entries - 1;
    
    return t;
}

void TTABLE_destroy(ttable_t *t)
{
    free(t->entries);
    free(t);
}

void TTABLE_store(ttable_t *t, bitboard_t hash, short depth, short type, int score, move_t best_move)
{
    int index = (int)(hash & t->key_mask);
    ttable_entry_t *entry = &t->entries[index];

    if(entry->hash == hash && entry->depth == depth) {
        if(type == TTABLE_TYPE_EXACT) {
            entry->score[0] = entry->score[1] = score;
        } else if(type == TTABLE_TYPE_LOWER_BOUND) {
            entry->score[0] = score;
        } else {
            entry->score[1] = score;
        }
        entry->best_move = best_move;
    } else {
        entry->hash = hash;
        entry->depth = depth;

        if(type == TTABLE_TYPE_EXACT) {
            entry->score[0] = entry->score[1] = score;
        } else if(type == TTABLE_TYPE_LOWER_BOUND) {
            entry->score[0] = score;
            entry->score[1] = SEARCH_MAX_RESULT(depth);
        } else {
            entry->score[0] = SEARCH_MIN_RESULT(depth);
            entry->score[1] = score;
        }
        entry->best_move = best_move;
    }
}

ttable_entry_t *TTABLE_retrieve(ttable_t *t, bitboard_t hash)
{
    int index = (int)(hash & t->key_mask);
    if(t->entries[index].hash == hash) {
        return &t->entries[index];
    }

    return NULL;
}

