#include <stdio.h>
#include <string.h>
#include "cecp_features.h"
#include "io.h"
#include "engine.h"

#define COMMAND_BUFFER_SIZE 512

typedef struct {
    int flag_forced;
    int flag_quit;
    int flag_pondering;             /* Ponder between moves                             */
    int time_period;                /* Time control period (number of turns)            */
    int time_seconds;               /* Seconds per control period                       */
    int time_incremental_seconds;   /* Seconds added per turn                           */
    int time_left_centiseconds;     /* Time left in current control period (10^-2 sec)  */
    int num_half_moves;             /* Number of half moves                             */
} state_t;

void state_clear(state_t *state)
{
    state->flag_forced = 0;
    state->flag_quit = 0;
    state->flag_pondering = 0;
    state->time_period = 0;
    state->time_seconds = 0;
    state->time_incremental_seconds = 0;
    state->time_left_centiseconds = 0;
    state->num_half_moves = 0;
}

void state_clear_time(state_t *state)
{
    state->time_period = 0;
    state->time_seconds = 0;
    state->time_incremental_seconds = 0;
    state->time_left_centiseconds = 0;
    state->num_half_moves = 0;
}

void send_features()
{
    const char **feature;
    
    /* Loop over all features */
    for(feature = cecp_features; *feature != 0; feature++) {
        /* Send feature */
        fprintf(stdout, "feature %s\n", *feature);
    }
}

void send_result(int result)
{
    switch(result) {
    case ENGINE_RESULT_WHITE_MATES:
        fprintf(stdout, "1-0 {White mates}\n");
        break;
    case ENGINE_RESULT_BLACK_MATES:
        fprintf(stdout, "0-1 {Black mates}\n");
        break;
    case ENGINE_RESULT_STALE_MATE:
        fprintf(stdout, "1/2-1/2 {Stalemate}\n");
        break;
    default:
        break;
    }
}

void pondering_start(state_t *state, engine_state_t *engine)
{
    if(state->flag_pondering) {
        ENGINE_think_start(engine, 1, 3600*1000, 0, 100);
    }
}

void pondering_stop(state_t *state, engine_state_t *engine)
{
    if(ENGINE_think_get_status(engine) != ENGINE_SEARCH_NONE) {
        int pos_from, pos_to, promotion_type;
        ENGINE_think_stop(engine);
        ENGINE_think_get_result(engine, &pos_from, &pos_to, &promotion_type);
    }
}

void make_move(state_t *state, engine_state_t *engine)
{
    int pos_from, pos_to, promotion_type, result;
    int time_left_ms = state->time_left_centiseconds * 10;
    int time_incremental_ms = state->time_incremental_seconds * 1000;
    int moves_left_in_period = 0;
    
    if(state->time_period) {
        int num_moves = state->num_half_moves / 2;
        moves_left_in_period = state->time_period - (num_moves % state->time_period);
    }
    
    ENGINE_think_start(engine, moves_left_in_period, time_left_ms, time_incremental_ms, 100);
    ENGINE_think_get_result(engine, &pos_from, &pos_to, &promotion_type);
    result = ENGINE_apply_move(engine, pos_from, pos_to, promotion_type);
    if(promotion_type) {
        char pt = 0;
        if(promotion_type == 1) pt = 'n';
        else if(promotion_type == 2) pt = 'b';
        else if(promotion_type == 3) pt = 'r';
        else if(promotion_type == 4) pt = 'q';
        fprintf(stdout, "move %c%c%c%c%c\n", (pos_from%8)+'a', (pos_from/8)+'1', (pos_to%8)+'a', (pos_to/8)+'1', pt);
    } else {
        fprintf(stdout, "move %c%c%c%c\n", (pos_from%8)+'a', (pos_from/8)+'1', (pos_to%8)+'a', (pos_to/8)+'1');
    }
    
    state->num_half_moves++;
    send_result(result);

    if(result == ENGINE_RESULT_NONE) {
        pondering_start(state, engine);
    }
}

void str_remove_newline(char *p)
{
    /* Find newline */
    p = strchr(p, '\n');
    
    /* Replace with null-zero */
    if(p) {
        *p = '\0';
    }
}

void parse_move(const char *move_str, int *pos_from, int *pos_to, int *promotion_type)
{
    *pos_from = move_str[0]-'a' + 8 * (move_str[1]-'1');
    *pos_to = move_str[2]-'a' + 8 * (move_str[3]-'1');
    switch(move_str[4])
    {
        case 'n':
            *promotion_type = ENGINE_PROMOTION_KNIGHT;
            break;
        case 'b':
            *promotion_type = ENGINE_PROMOTION_BISHOP;
            break;
        case 'r':
            *promotion_type = ENGINE_PROMOTION_ROOK;
            break;
        case 'q':
            *promotion_type = ENGINE_PROMOTION_QUEEN;
            break;
        default:
            *promotion_type = 0;
            break;
    }
}

void user_move(state_t *state, engine_state_t *engine, const char *move_str, int respond_to_move)
{
    int result;
    int pos_from, pos_to, promotion_type;

    pondering_stop(state, engine);

    /* Parse the user move */
    parse_move(move_str, &pos_from, &pos_to, &promotion_type);

    /* Move piece */
    result = ENGINE_apply_move(engine, pos_from, pos_to, promotion_type);
    if(result == ENGINE_RESULT_NONE) {
        state->num_half_moves++;
        if(respond_to_move) {
            make_move(state, engine);
        }
    } else if(result == ENGINE_RESULT_ILLEGAL_MOVE) {
        /* Illegal move */
        fprintf(stdout, "Illegal move: %s", move_str);
        
    } else {
        send_result(result);
    }
}

void parse_time_control(state_t *state, const char *level)
{
    int period = 0;
    int minutes = 0;
    int seconds = 0;
    int inc_seconds = 0;
    int ret = 0;

    /* Try to parse "period minutes:seconds inc" */
    ret = sscanf(level, "%d %d:%d %d\n", &period, &minutes, &seconds, &inc_seconds);
    if(ret != 4) {
        /* Try to parse "period minutes inc" */
        seconds = 0;
        ret = sscanf(level, "%d %d %d\n", &period, &minutes, &inc_seconds);
        if(ret != 3) {
            /* Try fixed number of seconds per move */
            period = 0;
            minutes = 0;
            seconds = 0;
            ret = sscanf(level, "%d\n", &inc_seconds);
            if(ret != 1) {
                /* Failed to parse */
                return;
            }
        }
    }
    
    state->time_period = period;
    state->time_left_centiseconds = 100 * (60 * minutes + seconds);
    state->time_incremental_seconds = inc_seconds;
}

void thinking_output(int ply, int score, int time_ms, unsigned int nodes, int pv_length, int *pos_from, int *pos_to, int *promotion_type)
{
    int i;
    fprintf(stdout, "%d %d %d %d", ply, score, time_ms / 10, nodes);
    for(i = 0; i < pv_length; i++) {
        int from = pos_from[i];
        int to = pos_to[i];
        int promotion = promotion_type[i];
        
        if(promotion) {
            char pt = 0;
            if(promotion == 1) pt = 'n';
            else if(promotion == 2) pt = 'b';
            else if(promotion == 3) pt = 'r';
            else if(promotion == 4) pt = 'q';
            fprintf(stdout, " %c%c%c%c%c", (from%8)+'a', (from/8)+'1', (to%8)+'a', (to/8)+'1', pt);
        } else {
            fprintf(stdout, " %c%c%c%c", (from%8)+'a', (from/8)+'1', (to%8)+'a', (to/8)+'1');
        }
    }
    fprintf(stdout, "\n");
}

static void process_command(engine_state_t *engine, char *command, state_t *state)
{
    /* Commands that do reqire action from the engine */
    
    /* protover */
    if(strncmp(command, "protover ", 9) == 0) {
        send_features();
    }
    
    /* new */
    else if(strcmp(command, "new\n") == 0) {
        /* Reset board */
        ENGINE_reset(engine);
        
        /* Reset time control */
        state_clear_time(state);

        /* Leave force mode */
        state->flag_forced = 0;
    }
    
    /* usermove */
    else if(strncmp(command, "usermove ", 9) == 0) {
        user_move(state, engine, command + 9, !state->flag_forced);
    }
    
    /* go */
    else if(strcmp(command, "go\n") == 0) {
        /* Leave force mode */
        state->flag_forced = 0;

        /* Move */
        make_move(state, engine);
    }
    
    /*  quit */
    else if(strcmp(command, "quit\n") == 0) {
        state->flag_quit = 1;
        pondering_stop(state, engine);
    }
    
    /* force */
    else if(strcmp(command, "force\n") == 0) {
        /* Enter force mode */
        state->flag_forced = 1;
        pondering_stop(state, engine);
    }

    /* level */
    else if(strncmp(command, "level ", 6) == 0) {
        parse_time_control(state, command + 6);
    }

    /* st */
    else if(strncmp(command, "st ", 3) == 0) {
        parse_time_control(state, command + 3);
    }

    /* time */
    else if(strncmp(command, "time ", 5) == 0) {
        int centiseconds = 0;
        sscanf(command + 5, "%d\n", &centiseconds);
        state->time_left_centiseconds = centiseconds;
    }
    
    /* memory */
    else if(strncmp(command, "memory ", 7) == 0) {
        int hash_size_mb = 0;
        sscanf(command + 7, "%d\n", &hash_size_mb);
        ENGINE_resize_hashtable(engine, hash_size_mb);
    }

    /* setboard */
    else if(strncmp(command, "setboard ", 9) == 0) {
        if(ENGINE_set_board(engine, command + 9) != 0) {
            fprintf(stdout, "tellusererror Illegal position\n");
        }
    }

    /* easy */
    else if(strcmp(command, "easy\n") == 0) {
        state->flag_pondering = 0;
    }

    /* hard */
    else if(strcmp(command, "hard\n") == 0) {
        state->flag_pondering = 1;
    }

    /* result */
    else if(strncmp(command, "result ", 7) == 0) {
        pondering_stop(state, engine);
    }

    /* Commands that do not reqire action from the engine (or not implemented) */
    
    /* xboard */
    else if(strcmp(command, "xboard\n") == 0) {}
    
    /* accepted */
    else if(strncmp(command, "accepted ", 9) == 0) {}
    
    /* random */
    else if(strcmp(command, "random\n") == 0) {}
    
    /* post */
    else if(strcmp(command, "post\n") == 0) {}
    
    /* cores */
    else if(strncmp(command, "cores ", 6) == 0) {}
    
    /* computer */
    else if(strcmp(command, "computer\n") == 0) {}

    /* otim */
    else if(strncmp(command, "otim ", 5) == 0) {}
	    
    /* Errors */
    
    /* rejected */
    else if(strncmp(command, "rejected ", 9) == 0) {
        str_remove_newline(command);
        fprintf(stdout, "# Feature rejected \"%s\"\n", command+9);
    }
    
    /* unknown command */
    else {
        fprintf(stdout, "Error (unknown command): %s", command);
    }
}


int main(int argc, char **argv)
{
    engine_state_t *engine;
    char command_buffer[COMMAND_BUFFER_SIZE];
    state_t state;
    state_clear(&state);
    
    IO_init();
    
    /* Create engine instance */
    ENGINE_create(&engine);
    ENGINE_register_thinking_output_cb(engine, &thinking_output);

    /* Welcome */
    fprintf(stdout, "# Welcome to Pawned " _VERSION "\n");
    fprintf(stdout, "# This program supports the Chess Engine Communication Protocol\n");
    fprintf(stdout, "# and should be run from XBoard or similar\n");
    
    /* Main loop */
    while(1) {
        /* Check if a command is sent from Xboard */
        if(IO_read_input(command_buffer, COMMAND_BUFFER_SIZE)) {
            /* Take proper action */
            process_command(engine, command_buffer, &state);
            if(state.flag_quit) {
                /* Shutdown if we get the 'quit' command etc */
                break;
            }
        }
    }

    /* Free engine instance */
    ENGINE_destroy(engine);
    
    return 0;
}
