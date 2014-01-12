#ifndef _SEARCH_H
#define _SEARCH_H

#include <time.h>
#include "defines.h"
#include "state.h"
#include "ttable.h"

#define SEARCH_MIN_RESULT(depth) (-1000-(depth))
#define SEARCH_MAX_RESULT(depth) (1000+(depth))

#define SEARCH_ITERATIONS_BETWEEN_CLOCK_CHECK 10000

typedef struct {
    ttable_t        *ttable;
    int             abort_search;
    int             next_clock_check;
    struct timespec start_time;
    int64_t         time_for_move_ms;
} search_state_t;

int SEARCH_perform_search(const chess_state_t *s, ttable_t *ttable, int *score);
int SEARCH_is_check(const chess_state_t *s, int color);
int SEARCH_is_mate(const chess_state_t *state);
void SEARCH_time_now(struct timespec *time);
int64_t SEARCH_time_left_ms(search_state_t *search_state);

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

