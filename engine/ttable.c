#include <stdlib.h>
#include "ttable.h"
/*
int tt_total=0;
int tt_hit=0;
#include <stdio.h>
*/

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

    entry->hash = hash;
    entry->depth = depth;
    entry->type = type;
    entry->score = score;
    entry->best_move = best_move;
}

ttable_entry_t *TTABLE_retrieve(ttable_t *t, bitboard_t hash)
{
    int index = (int)(hash & t->key_mask);
    /*tt_total++;*/
    if((t->entries[index].hash == hash)) {
        /*
        tt_hit++;
        printf("#TTABLE GETS %d, HITS: %d, HITRATE %f\n", tt_total, tt_hit, ((float)tt_hit) / ((float)(tt_total)));
        */
        return &t->entries[index];
    }

    return NULL;
}

