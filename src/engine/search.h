#ifndef _SEARCH_H
#define _SEARCH_H

#include "defines.h"
#include "state.h"
#include "hashtable.h"
#include "history.h"
#include "engine.h"

#define SEARCH_MIN_RESULT(depth) (-1000-((short)depth))
#define SEARCH_MAX_RESULT(depth) (1000+((short)depth))

#define SEARCH_ITERATIONS_BETWEEN_CLOCK_CHECK 10000

typedef struct {
    hashtable_t         *hashtable;
    history_t           *history;
    int                 abort_search;
    int                 next_clock_check;
    int64_t             start_time_ms;
    int64_t             time_for_move_ms;
    unsigned char       max_depth;
    thinking_output_cb  think_cb;
} search_state_t;

int SEARCH_perform_search(const chess_state_t *s, hashtable_t *hashtable, history_t *history, const int time_for_move_ms, const unsigned char max_depth, short *score, thinking_output_cb think_cb);
int SEARCH_is_check(const chess_state_t *s, const int color);
int SEARCH_is_mate(const chess_state_t *state);

#endif

