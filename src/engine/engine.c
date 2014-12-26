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
#include "time.h"
#include "defines.h"

struct engine_state {
    chess_state_t       *chess_state;
    hashtable_t         *hashtable;
    history_t           *history;
    openingbook_t       *obook;
    thinking_output_cb  think_cb;
    search_state_t      *search_state;
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
    (*state)->search_state = NULL;
    ENGINE_reset(*state);
}

void ENGINE_destroy(engine_state_t *state)
{
    HASHTABLE_destroy(state->hashtable);
    HISTORY_destroy(state->history);
    OPENINGBOOK_destroy(state->obook);
    if(state->search_state) free(state->search_state);
    free(state->chess_state);
    free(state);
}

void ENGINE_reset(engine_state_t *state)
{
    STATE_reset(state->chess_state);
    HISTORY_reset(state->history);
    if(state->search_state) {
        free(state->search_state);
        state->search_state = NULL;
    }
}

int ENGINE_apply_move(engine_state_t *state, const int pos_from, const int pos_to, const int promotion_type)
{
    int num_moves;
    int i;
    chess_state_t temporary_state;
    move_t moves[256];
    
    /* Generate all possible moves */
    num_moves = STATE_generate_moves(state->chess_state, moves);
    
    /* Loop through all generated moves to find the right one */
    for(i = 0; i < num_moves; i++) {
        int move = moves[i];
        if(MOVE_GET_POS_FROM(move) != pos_from) continue;
        if(MOVE_GET_POS_TO(move) != pos_to) continue;
        if(MOVE_PROMOTION_TYPE(move) != promotion_type) continue;
        
        /* Pseudo legal move found: Apply to state */
        temporary_state = *state->chess_state;
        STATE_apply_move(&temporary_state, move);
        
        /* Check if the move is legal */
        if(SEARCH_is_check(&temporary_state, state->chess_state->player)) {
            /* Not legal */
            break;
        }
        
        /* Legal */
        *state->chess_state = temporary_state;
        HISTORY_push(state->history, state->chess_state->hash);
        return ENGINE_result(state);
    }
    
    /* No valid move found: Illegal move */
    return ENGINE_RESULT_ILLEGAL_MOVE;
}

int ENGINE_apply_move_san(engine_state_t *state, const char *san)
{
    move_t move;
    chess_state_t temporary_state;
    move = SAN_parse_move(state->chess_state, san);
    
    if(move) {
        /* Pseudo legal move found: Apply to state */
        temporary_state = *state->chess_state;
        STATE_apply_move(&temporary_state, move);
        
        /* Check if the move is legal */
        if(!SEARCH_is_check(&temporary_state, state->chess_state->player)) {
            /* Legal */
            *state->chess_state = temporary_state;
            HISTORY_push(state->history, state->chess_state->hash);
            return ENGINE_result(state);
        }
    }

    /* No valid move found: Illegal move */
    return ENGINE_RESULT_ILLEGAL_MOVE;
}

static void *ENGINE_think_thread(void *arg)
{
    engine_state_t *state = (engine_state_t*)arg;

    move_t move;
    short score;
    
    /* Look for a move in the opening book */
    move = OPENINGBOOK_get_move(state->obook, state->chess_state);
    if(!move) {
        /* No move in the opening book. Search! */
        move = SEARCH_perform_search(state->chess_state, state->search_state, &score);
    }
    
    state->search_state->move = move;
    state->search_state->status = ENGINE_SEARCH_COMPLETED;

    return NULL;
}

void ENGINE_think_start(engine_state_t *state, const int moves_left_in_period, const int time_left_ms, const int time_incremental_ms, const unsigned char max_depth)
{
    int64_t time_for_move_ms;

    if(state->search_state) {
        /* Search already in progress => abort */
        return;
    }

    /* Create search state */
    state->search_state = calloc(1, sizeof(search_state_t));

    /* Calculate time for this move */
    if(moves_left_in_period) {
        time_for_move_ms = time_left_ms / moves_left_in_period;
    } else {
        time_for_move_ms = time_left_ms * 2 / 100;
    }

    /* Add incremental time */
    time_for_move_ms += time_incremental_ms;

    /* Make sure we have 100 ms margin */
    if(time_for_move_ms > time_left_ms - 100) {
        time_for_move_ms = time_left_ms - 100;
    }

    /* Set members of search_state */
    state->search_state->hashtable = state->hashtable;
    state->search_state->history = state->history;
    state->search_state->abort_search = 0;
    state->search_state->next_clock_check = SEARCH_ITERATIONS_BETWEEN_CLOCK_CHECK;
    state->search_state->start_time_ms = TIME_now();
    state->search_state->time_for_move_ms = time_for_move_ms;
    state->search_state->max_depth = max_depth;
    state->search_state->num_nodes_searched = 0;
    state->search_state->think_cb = state->think_cb;
    state->search_state->move = 0;
    state->search_state->status = ENGINE_SEARCH_RUNNING;

    /* Spawn search thread */
    THREAD_create(&state->search_state->thread, &ENGINE_think_thread, (void*)state);
}

void ENGINE_think_stop(engine_state_t *state)
{
    if(state->search_state) {
        state->search_state->abort_search = 1;
    }
}

int  ENGINE_think_get_status(engine_state_t *state)
{
    if(state->search_state) {
        return state->search_state->status;
    } else {
        return ENGINE_SEARCH_NONE;
    }
}

void ENGINE_think_get_result(engine_state_t *state, int *pos_from, int *pos_to, int *promotion_type)
{
    move_t move;
    int special;

    if(!state->search_state) {
        /* No search to get result from */
        return;
    }

    /* Join search thread */
    THREAD_join(state->search_state->thread);

    /* Get best move */
    move = state->search_state->move;

    /* Free memory in search state */
    free(state->search_state);
    state->search_state = NULL;

    *pos_from = MOVE_GET_POS_FROM(move);
    *pos_to = MOVE_GET_POS_TO(move);
    special = MOVE_GET_SPECIAL_FLAGS(move);

    /* Translate move to: pos_from, pos_to, promotion_type */

    switch(special)
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
}

int ENGINE_result(const engine_state_t *state)
{
    if(SEARCH_is_mate(state->chess_state)) {
        if(SEARCH_is_check(state->chess_state, state->chess_state->player)) {
            if(state->chess_state->player == WHITE) {
                return ENGINE_RESULT_BLACK_MATES;
            } else {
                return ENGINE_RESULT_WHITE_MATES;
            }
        } else {
            return ENGINE_RESULT_STALE_MATE;
        }
    }
    
    return ENGINE_RESULT_NONE;
}

void ENGINE_register_thinking_output_cb(engine_state_t *state, thinking_output_cb think_cb)
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
