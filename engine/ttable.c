#include <stdlib.h>
#include "ttable.h"
/*
int tt_total=0;
int tt_hit=0;
int tt_shallow=0;
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

void TTABLE_store(ttable_t *t, bitboard_t hash, short depth, short type, int score)
{
    int index = (int)(hash & t->key_mask);
    ttable_entry_t *entry = &t->entries[index];

    entry->hash = hash;
    entry->depth = depth;
    entry->type = type;
    entry->score = score;
}

ttable_entry_t *TTABLE_get(ttable_t *t, bitboard_t hash, short depth)
{
    int index = (int)(hash & t->key_mask);
    /*tt_total++;*/
    if((t->entries[index].hash == hash) && (t->entries[index].depth >= depth)) {
        /*
        tt_hit++;
        printf("#TTABLE GETS %d, HITS: %d, TOO SHALLOW %d, HITRATE %f, SHALLOWRATE %f\n", tt_total, tt_hit, tt_shallow, ((float)tt_hit) / ((float)(tt_total)), ((float)tt_shallow) / ((float)(tt_total)));
        */
        return &t->entries[index];
    }
    /*
    else if((t->entries[index].hash == hash)) {
        tt_shallow++;
    }
    */

    return NULL;
}

