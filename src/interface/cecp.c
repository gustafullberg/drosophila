#include <stdio.h>
#include <string.h>
#include "cecp_features.h"
#include "io.h"
#include "engine.h"

#define COMMAND_BUFFER_SIZE 512

typedef struct {
    int flag_forced;
    int flag_quit;
    int flag_searching;             /* Is currently searching for a move                */
    int flag_pondering;             /* Ponder between moves                             */
    int time_period;                /* Time control period (number of turns)            */
    int time_seconds;               /* Seconds per control period                       */
    int time_incremental_seconds;   /* Seconds added per turn                           */
    int time_left_centiseconds;     /* Time left in current control period (10^-2 sec)  */
    int num_half_moves;             /* Number of half moves                             */
} state_t;

/* Reset state to known defaults */
void state_clear(state_t *state)
{
    state->flag_forced = 0;
    state->flag_quit = 0;
    state->flag_searching = 0;
    state->flag_pondering = 0;
    state->time_period = 0;
    state->time_seconds = 0;
    state->time_incremental_seconds = 0;
    state->time_left_centiseconds = 0;
    state->num_half_moves = 0;
}

/* Reset only the time control of the state */
void state_clear_time(state_t *state)
{
    state->time_period = 0;
    state->time_seconds = 0;
    state->time_incremental_seconds = 0;
    state->time_left_centiseconds = 0;
    state->num_half_moves = 0;
}

/* Send all the supported features of the CECP to GUI */
void send_features()
{
    const char **feature;
    
    /* Loop over all features */
    for(feature = cecp_features; *feature != 0; feature++) {
        /* Send feature */
        fprintf(stdout, "feature %s\n", *feature);
    }
}

/* Send a move to GUI */
void send_move(int pos_from, int pos_to, int promotion_type)
{
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
}

/* Send a result to GUI if game has ended */
void send_result(int result)
{
    switch(result) {
    case ENGINE_RESULT_WHITE_MATES:
        fprintf(stdout, "1-0 {White mates}\n");
        break;
    case ENGINE_RESULT_BLACK_MATES:
        fprintf(stdout, "0-1 {Black mates}\n");
        break;
    case ENGINE_RESULT_DRAW_STALE_MATE:
        fprintf(stdout, "1/2-1/2 {Stalemate}\n");
        break;
    case ENGINE_RESULT_DRAW_INSUFFICIENT_MATERIAL:
        fprintf(stdout, "1/2-1/2 {Draw by insufficient material}\n");
        break;
    case ENGINE_RESULT_DRAW_FIFTY_MOVE:
        fprintf(stdout, "1/2-1/2 {Draw by fifty-move rule}\n");
        break;
    case ENGINE_RESULT_DRAW_REPETITION:
        fprintf(stdout, "1/2-1/2 {Draw by repetition}\n");
        break;
    default:
        break;
    }
}

/* Send stats and PV to GUI while searching for move */
void send_search_output(int ply, int score, int time_ms, unsigned int nodes, int pv_length, int *pos_from, int *pos_to, int *promotion_type)
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

/* Parse a move of the form e7e8q */
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

/* Parse time control "40 4:00 0", "40 4 0" or "10" */
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

/* Start searching for the next move */
void search_move(state_t *state, engine_state_t *engine)
{
    int time_left_ms = state->time_left_centiseconds * 10;
    int time_incremental_ms = state->time_incremental_seconds * 1000;
    int moves_left_in_period = 0;

    /* Calculate time available for search */
    if(state->time_period) {
        int num_moves = state->num_half_moves / 2;
        moves_left_in_period = state->time_period - (num_moves % state->time_period);
    }

    /* Flag that a search is in progress */
    state->flag_searching = 1;

    /* Spawn a new thread and start searching */
    ENGINE_think_start(engine, moves_left_in_period, time_left_ms, time_incremental_ms, 100);
}

/* Start pondering */
void search_pondering(state_t *state, engine_state_t *engine)
{
    /* Only start ponder if pondering is enabled */
    if(state->flag_pondering) {
        /* Limit search to one hour or a depth of 100 */
        ENGINE_think_start(engine, 1, 3600*1000, 0, 100);
    }
}

/* Abort search (move or pondering) */
void search_stop(state_t *state, engine_state_t *engine, int *pos_from, int *pos_to, int *promotion_type)
{
    int _pos_from = 0, _pos_to = 0, _promotion_type = 0;

    /* Flag that no search for a move is going on */
    state->flag_searching = 0;

    /* Check if a search is going on */
    if(ENGINE_think_get_status(engine) != ENGINE_SEARCH_NONE) {
        /* Abort the search */
        ENGINE_think_stop(engine);

        /* Get the result */
        ENGINE_think_get_result(engine, &_pos_from, &_pos_to, &_promotion_type);
    }

    if(pos_from && pos_to && promotion_type) {
        *pos_from = _pos_from;
        *pos_to = _pos_to;
        *promotion_type = _promotion_type;
    }
}

/* Return non-zero if a search has been completed and result is waiting */
int search_is_completed(state_t *state, engine_state_t *engine)
{
    return ENGINE_think_get_status(engine) == ENGINE_SEARCH_COMPLETED;
}

/* Remove first newline of a string and terminate it */
void str_remove_newline(char *p)
{
    /* Find newline */
    p = strchr(p, '\n');
    
    /* Replace with null-zero */
    if(p) {
        *p = '\0';
    }
}

/* Get a move (result from search) and send it to the GUI */
void get_and_send_move(state_t *state, engine_state_t *engine)
{
    int pos_from, pos_to, promotion_type, result;

    search_stop(state, engine, &pos_from, &pos_to, &promotion_type);
    result = ENGINE_apply_move(engine, pos_from, pos_to, promotion_type);

    send_move(pos_from, pos_to, promotion_type);

    state->num_half_moves++;
    send_result(result);

    if(result == ENGINE_RESULT_NONE) {
        search_pondering(state, engine);
    }
}

/* Handle a "usermove" command */
void cmd_usermove(state_t *state, engine_state_t *engine, const char *move_str, int respond_to_move)
{
    int result;
    int pos_from, pos_to, promotion_type;

    /* Stop ongoing pondering */
    search_stop(state, engine, NULL, NULL, NULL);

    /* Parse the user move */
    parse_move(move_str, &pos_from, &pos_to, &promotion_type);

    /* Move piece */
    result = ENGINE_apply_move(engine, pos_from, pos_to, promotion_type);
    if(result == ENGINE_RESULT_NONE) {
        state->num_half_moves++;
        if(respond_to_move) {
            search_move(state, engine);
        }
    } else if(result == ENGINE_RESULT_ILLEGAL_MOVE) {
        /* Illegal move */
        fprintf(stdout, "Illegal move: %s", move_str);
        
    } else {
        send_result(result);
    }
}

/* Process command from GUI */
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
        cmd_usermove(state, engine, command + 9, !state->flag_forced);
    }
    
    /* go */
    else if(strcmp(command, "go\n") == 0) {
        /* Leave force mode */
        state->flag_forced = 0;

        /* Move */
        search_move(state, engine);
    }
    
    /*  quit */
    else if(strcmp(command, "quit\n") == 0) {
        state->flag_quit = 1;
        search_stop(state, engine, NULL, NULL, NULL);
    }
    
    /* force */
    else if(strcmp(command, "force\n") == 0) {
        /* Enter force mode */
        state->flag_forced = 1;
        search_stop(state, engine, NULL, NULL, NULL);
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
        search_stop(state, engine, NULL, NULL, NULL);
    }

    /* ? (move now) */
    else if(strncmp(command, "?\n", 2) == 0) {
        /* If a search for next move is ongoing */
        if(state->flag_searching) {
            /* Abort the search and send the move now */
            get_and_send_move(state, engine);
        }
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
    ENGINE_register_thinking_output_cb(engine, &send_search_output);

    /* Welcome */
    fprintf(stdout, "# Welcome to Drosophila " _VERSION "\n");
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

        /* Poll for found move */
        if(state.flag_searching) {
            if(search_is_completed(&state, engine)) {
                get_and_send_move(&state, engine);
            }
        }
    }

    /* Free engine instance */
    ENGINE_destroy(engine);
    
    return 0;
}
