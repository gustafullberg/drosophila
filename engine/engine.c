#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include "engine.h"
#include "state.h"
#include "move.h"
#include "search.h"
#include "defines.h"

#define MOVE_STACK_SIZE 500

struct engine_state {
    chess_state_t   *chess_state;
    move_t          *move_stack;
};

static void engine_init()
{
    static int first_run = 1;
    if(first_run) {
        bitboard_init();
        first_run = 0;
    }
}
void engine_create(engine_state_t **state)
{
    engine_init();
    *state = malloc(sizeof(engine_state_t));
    (*state)->chess_state = malloc(sizeof(chess_state_t));
    (*state)->move_stack = malloc(sizeof(int) * MOVE_STACK_SIZE);
    engine_reset(*state);
}

void engine_destroy(engine_state_t *state)
{
    free(state->move_stack);
    free(state->chess_state);
    free(state);
}

void engine_reset(engine_state_t *state)
{
    state_reset(state->chess_state);
}

int engine_opponent_move(engine_state_t *state, int pos_from, int pos_to, int promotion_type)
{
    int num_moves;
    int i;
    int checkmate;
    
    /* Generate all possible moves */
    num_moves = state_generate_moves(state->chess_state, state->move_stack, &checkmate);
    
    /* Loop through all generated moves to find the right one */
    for(i = 0; i < num_moves; i++) {
        int move = state->move_stack[i];
        if(MOVE_POS_FROM(move) != pos_from) continue;
        if(MOVE_POS_TO(move) != pos_to) continue;
        if((MOVE_SPECIAL(move) & promotion_type) != promotion_type) continue;
        
        /* Valid move found: Apply to state */
        state_apply_move(state->chess_state, move);
        return 0;
    }
    
    /* No valid move found: Illegal move */
    return -1;
}

int engine_ai_move(engine_state_t *state, int *pos_from, int *pos_to, int *promotion_type)
{
    int move;
    int special;
    int score;

    move = minimax_search(state->chess_state, state->move_stack, 4, &score);
    *pos_from = MOVE_POS_FROM(move);
    *pos_to = MOVE_POS_TO(move);
    special = MOVE_SPECIAL(move);
    
    switch(special)
    {
        case MOVE_KNIGHT_PROMOTION:
        case MOVE_KNIGHT_PROMOTION_CAPTURE:
        *promotion_type = 1;
        break;
        
        case MOVE_BISHOP_PROMOTION:
        case MOVE_BISHOP_PROMOTION_CAPTURE:
        *promotion_type = 2;
        break;
        
        case MOVE_ROOK_PROMOTION:
        case MOVE_ROOK_PROMOTION_CAPTURE:
        *promotion_type = 3;
        break;
        
        case MOVE_QUEEN_PROMOTION:
        case MOVE_QUEEN_PROMOTION_CAPTURE:
        *promotion_type = 4;
        break;
        
        default:
        *promotion_type = 0;
        break;
    }
    
    state_apply_move(state->chess_state, move);
    
    return 0;
}

int engine_result(engine_state_t *state)
{
    /*
    int check, mate;
    
    check = search_is_check(state->chess_state, state->move_stack);
    mate  = search_is_mate(state->chess_state, state->move_stack);
    
    if(mate) {
        if(!check) {
            if(state->chess_state->player == WHITE) {
                return ENGINE_RESULT_WHITE_MATES;
            } else {
                return ENGINE_RESULT_BLACK_MATES;
            }
        } else {
            return ENGINE_RESULT_STALE_MATE;
        }
    }
    */
    return ENGINE_RESULT_NONE;
}
