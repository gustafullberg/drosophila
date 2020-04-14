#include <stdlib.h>
#include <stdio.h>
#include "engine.h"
#include "state.h"
#include "hashtable.h"
#include "history.h"
#include "openingbook.h"
#include "search.h"
#include "san.h"
#include "fen.h"
#include "clock.h"
#include "eval.h"
#include "defines.h"

struct engine_state {
    chess_state_t       *chess_state;
    hashtable_t         *hashtable;
    history_t           *history;
    openingbook_t       *obook;
    thinking_output_cb  think_cb;
    search_state_t      search_state;
};

static void ENGINE_init()
{
    static int first_run = 1;
    if(first_run) {
        BITBOARD_init();
        first_run = 0;
    }
}
void ENGINE_create(engine_state_t **state)
{
    ENGINE_init();
    *state = (engine_state_t*)calloc(1, sizeof(engine_state_t));
    (*state)->chess_state = (chess_state_t*)malloc(sizeof(chess_state_t));
    (*state)->hashtable = HASHTABLE_create(64);
    (*state)->history = HISTORY_create();
    (*state)->obook = OPENINGBOOK_create("book.bin");
    (*state)->think_cb = NULL;
    (*state)->search_state.hashtable = (*state)->hashtable;
    (*state)->search_state.history = (*state)->history;
    ENGINE_reset(*state);
}

void ENGINE_destroy(engine_state_t *state)
{
    HASHTABLE_destroy(state->hashtable);
    HISTORY_destroy(state->history);
    OPENINGBOOK_destroy(state->obook);
    free(state->chess_state);
    free(state);
}

void ENGINE_reset(engine_state_t *state)
{
    STATE_reset(state->chess_state);
    HISTORY_reset(state->history);
}

int ENGINE_apply_move(engine_state_t *state, const int pos_from, const int pos_to, const int promotion_type)
{
    int num_moves;
    int i;
    move_t moves[256];
    
    /* Generate all possible moves */
    num_moves = STATE_generate_moves_simple(state->chess_state, moves);
    
    /* Loop through all generated moves to find the right one */
    for(i = 0; i < num_moves; i++) {
        move_t move = moves[i];
        if(MOVE_GET_POS_FROM(move) != pos_from) continue;
        if(MOVE_GET_POS_TO(move) != pos_to) continue;
        if(MOVE_PROMOTION_TYPE(move) != promotion_type) continue;
        
        /* Move found: Apply to state */
        STATE_apply_move(state->chess_state, move);
        HISTORY_push(state->history, state->chess_state->hash);
        return ENGINE_RESULT_NONE;
    }
    
    /* No valid move found: Illegal move */
    return ENGINE_RESULT_ILLEGAL_MOVE;
}

int ENGINE_apply_move_san(engine_state_t *state, const char *san)
{
    move_t move;
    move = SAN_parse_move(state->chess_state, san);
    
    if(move) {
        /* Legal move found: Apply to state */
        STATE_apply_move(state->chess_state, move);
        HISTORY_push(state->history, state->chess_state->hash);
        return ENGINE_RESULT_NONE;
    }

    /* No valid move found: Illegal move */
    return ENGINE_RESULT_ILLEGAL_MOVE;
}

int ENGINE_search(engine_state_t *state, const int moves_left_in_period, const int time_left_ms, const int time_incremental_ms, const unsigned char max_depth, int *pos_from, int *pos_to, int *promotion_type)
{
    int64_t time_for_move_ms;

    /* Calculate time for this move */
    if(moves_left_in_period) {
        time_for_move_ms = time_left_ms / moves_left_in_period * 3 / 2;
    } else {
        time_for_move_ms = time_left_ms * 4 / 100;
    }

    /* Add incremental time */
    time_for_move_ms += time_incremental_ms;

    /* Make sure we have 100 ms margin */
    if(time_for_move_ms > time_left_ms - 100) {
        time_for_move_ms = time_left_ms - 100;
    }

    if(time_for_move_ms < 0) time_for_move_ms = 1;

    /* Setup search state */
    state->search_state.abort_search = 0;
    state->search_state.next_clock_check = SEARCH_ITERATIONS_BETWEEN_CLOCK_CHECK;
    state->search_state.start_time_ms = CLOCK_now();
    state->search_state.time_for_move_ms = time_for_move_ms;
    state->search_state.max_depth = max_depth;
    state->search_state.num_nodes_searched = 0;
    state->search_state.think_cb = state->think_cb;

    /* Look for a move in the opening book */
    short score = 0;
    move_t move = OPENINGBOOK_get_move(state->obook, state->chess_state);
    if(!move) {
        /* No move in the opening book. Search! */
        move = SEARCH_perform_search(state->chess_state, &state->search_state, &score);
    }

    /* Translate move to: pos_from, pos_to, promotion_type */
    *pos_from = MOVE_GET_POS_FROM(move);
    *pos_to = MOVE_GET_POS_TO(move);
    switch(MOVE_GET_SPECIAL_FLAGS(move))
    {
        case MOVE_KNIGHT_PROMOTION:
        case MOVE_KNIGHT_PROMOTION_CAPTURE:
        *promotion_type = ENGINE_PROMOTION_KNIGHT;
        break;

        case MOVE_BISHOP_PROMOTION:
        case MOVE_BISHOP_PROMOTION_CAPTURE:
        *promotion_type = ENGINE_PROMOTION_BISHOP;
        break;

        case MOVE_ROOK_PROMOTION:
        case MOVE_ROOK_PROMOTION_CAPTURE:
        *promotion_type = ENGINE_PROMOTION_ROOK;
        break;

        case MOVE_QUEEN_PROMOTION:
        case MOVE_QUEEN_PROMOTION_CAPTURE:
        *promotion_type = ENGINE_PROMOTION_QUEEN;
        break;

        default:
        *promotion_type = ENGINE_PROMOTION_NONE;
        break;
    }

    return score;
}

void ENGINE_search_stop(engine_state_t *state)
{
    state->search_state.abort_search = 1;
}

void ENGINE_register_search_output_cb(engine_state_t *state, thinking_output_cb think_cb)
{
    state->think_cb = think_cb;
}

void ENGINE_resize_hashtable(engine_state_t *state, const int size_mb)
{
    HASHTABLE_destroy(state->hashtable);
    state->hashtable = HASHTABLE_create(size_mb);
}

int ENGINE_set_board(engine_state_t *state, const char *fen)
{
    chess_state_t s;
    if(FEN_read(&s, fen)) {
        *state->chess_state = s;
        HISTORY_reset_after_load(state->history, state->chess_state);
        return 0;
    }
    return 1;
}

int ENGINE_playing_side(engine_state_t *state)
{
    return state->chess_state->player;
}
