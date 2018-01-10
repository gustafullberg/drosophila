#include <stdlib.h>
#include "history.h"

struct _history_t
{
    bitboard_t hash[1024];
    int idx;
};

history_t *HISTORY_create()
{
    history_t *h = (history_t*)malloc(sizeof(history_t));
    HISTORY_reset(h);
    return h;
}

void HISTORY_destroy(history_t *h)
{
    free(h);
}

void HISTORY_reset(history_t *h)
{
    chess_state_t s;
    STATE_reset(&s);
    h->idx = -1;
    HISTORY_push(h, s.hash);
}

void HISTORY_reset_after_load(history_t *h, const chess_state_t *s)
{
    int i;
    h->idx = -1;
    for(i = 0; i < s->halfmove_clock - 1; i++) {
        HISTORY_push(h, 0);
    }
    HISTORY_push(h, s->hash);
}

void HISTORY_push(history_t *h, const bitboard_t hash)
{
    h->hash[++(h->idx)] = hash;
}

void HISTORY_pop(history_t *h)
{
    h->idx--;
}

int HISTORY_is_repetition(const history_t *h, const int halfmove_clock)
{
    int i;
    const bitboard_t hash = h->hash[h->idx];
    const int first = h->idx - halfmove_clock;
    const int last = h->idx - 4;
    
    for(i = last; i >= first; i -= 2) {
        if(h->hash[i] == hash) {
            return 1;
        }
    }

    if(halfmove_clock >= 100) {
        return 1;
    }

    return 0;
}

int HISTORY_is_threefold_repetition(const history_t *h, const int halfmove_clock)
{
    int i;
    int repetitions = 1;
    const bitboard_t hash = h->hash[h->idx];
    const int first = h->idx - halfmove_clock;
    const int last = h->idx - 4;
    
    for(i = last; i >= first; i -= 2) {
        if(h->hash[i] == hash) {
            repetitions++;
        }
    }

    return (repetitions >= 3);
}
