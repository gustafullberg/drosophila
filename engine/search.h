#ifndef _SEARCH_H
#define _SEARCH_H

#include "defines.h"
#include "state.h"
#include "ttable.h"
#include "history.h"

#define SEARCH_MIN_RESULT(depth) (-1000-(depth))
#define SEARCH_MAX_RESULT(depth) (1000+(depth))

#define SEARCH_ITERATIONS_BETWEEN_CLOCK_CHECK 10000

typedef struct {
    ttable_t        *ttable;
    history_t       *history;
    int             abort_search;
    int             next_clock_check;
    int64_t         start_time_ms;
    int64_t         time_for_move_ms;
} search_state_t;

int SEARCH_perform_search(const chess_state_t *s, ttable_t *ttable, history_t *history, int time_for_move_ms, int *score);
int SEARCH_is_check(const chess_state_t *s, int color);
int SEARCH_is_mate(const chess_state_t *state);

static inline int SEARCH_clamp_score_to_valid_range(int score, int depth)
{
    int min = SEARCH_MIN_RESULT(depth);
    int max = SEARCH_MAX_RESULT(depth);
    if(score < min) {
        score = min;
    } else if(score > max) {
        score = max;
    }
    return score;
}

#endif

