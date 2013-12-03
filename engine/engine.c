#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include "engine.h"
#include "state.h"
#include "search.h"
#include "defines.h"

#define MOVE_STACK_SIZE 5000

struct engine_state {
    chess_state_t   *chess_state;
    move_t          *move_stack;
};

static void ENGINE_init()
{
    static int first_run = 1;
    if(first_run) {
        bitboard_init();
        first_run = 0;
    }
}
void ENGINE_create(engine_state_t **state)
{
    ENGINE_init();
    *state = malloc(sizeof(engine_state_t));
    (*state)->chess_state = malloc(sizeof(chess_state_t));
    (*state)->move_stack = malloc(sizeof(int) * MOVE_STACK_SIZE);
    ENGINE_reset(*state);
}

void ENGINE_destroy(engine_state_t *state)
{
    free(state->move_stack);
    free(state->chess_state);
    free(state);
}

void ENGINE_reset(engine_state_t *state)
{
    STATE_reset(state->chess_state);
}

int ENGINE_apply_move(engine_state_t *state, int pos_from, int pos_to, int promotion_type)
{
    int num_moves;
    int i;
    chess_state_t temporary_state;
    
    /* Generate all possible moves */
    num_moves = STATE_generate_moves(state->chess_state, state->move_stack);
    
    /* Loop through all generated moves to find the right one */
    for(i = 0; i < num_moves; i++) {
        int move = state->move_stack[i];
        if(MOVE_GET_POS_FROM(move) != pos_from) continue;
        if(MOVE_GET_POS_TO(move) != pos_to) continue;
        if((MOVE_GET_SPECIAL_FLAGS(move) & promotion_type) != promotion_type) continue;
        
        /* Pseudo legal move found: Apply to state */
        STATE_clone(&temporary_state, state->chess_state);
        STATE_apply_move(&temporary_state, move);
        
        /* Check if the move is legal */
        if(SEARCH_is_check(&temporary_state, state->chess_state->player)) {
            /* Not legal */
            break;
        }
        
        /* Legal */
        STATE_clone(state->chess_state, &temporary_state);
        return ENGINE_result(state);
    }
    
    /* No valid move found: Illegal move */
    return ENGINE_RESULT_ILLEGAL_MOVE;
}

int ENGINE_think_and_move(engine_state_t *state, int *pos_from, int *pos_to, int *promotion_type)
{
    int move;
    int special;
    int score;

    move = SEARCH_perform_search(state->chess_state, state->move_stack, 4, &score);

    *pos_from = MOVE_GET_POS_FROM(move);
    *pos_to = MOVE_GET_POS_TO(move);
    special = MOVE_GET_SPECIAL_FLAGS(move);
    
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
        
    STATE_apply_move(state->chess_state, move);
    
    return ENGINE_result(state);
}

int ENGINE_result(engine_state_t *state)
{
    if(SEARCH_is_mate(state->chess_state, state->move_stack)) {
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
