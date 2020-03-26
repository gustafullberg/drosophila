#ifndef SEARCH_H
#define SEARCH_H

#include "defines.h"
#include "state.h"
#include "hashtable.h"
#include "history.h"
#include "engine.h"

#define SEARCH_MIN_RESULT(depth) (-1000-((short)depth))
#define SEARCH_MAX_RESULT(depth) (1000+((short)depth))

#define SEARCH_ITERATIONS_BETWEEN_CLOCK_CHECK 10000

typedef struct {
    move_t              moves[MAX_SEARCH_DEPTH];
    int                 size;
} pv_line_t;

typedef struct {
    hashtable_t         *hashtable;
    history_t           *history;
    int                 abort_search;
    int                 next_clock_check;
    int64_t             start_time_ms;
    int64_t             time_for_move_ms;
    unsigned char       max_depth;
    unsigned int        num_nodes_searched;
    thinking_output_cb  think_cb;
    move_t              killer_move[MAX_SEARCH_DEPTH+1][2];
    int                 history_heuristic[2][64][64];
    pv_line_t           pv;
    pv_line_t           pv_table[MAX_SEARCH_DEPTH];
} search_state_t;

move_t SEARCH_perform_search(const chess_state_t *s, search_state_t *search_state, short *score);
int SEARCH_is_check(const chess_state_t *s, const int color);
int SEARCH_is_mate(const chess_state_t *state);

#endif

