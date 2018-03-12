#include <stdio.h>
#include <string.h>
#include "cecp_features.h"
#include "engine.h"
#include "thread.h"

#define COMMAND_BUFFER_SIZE 512

typedef struct {
    engine_state_t *engine;
    mutex_t mtx_engine;
    cond_t cv;
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
    state->engine = NULL;
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

void search_start(state_t *state)
{
    MUTEX_lock(&state->mtx_engine);
    state->flag_searching = 1;
    MUTEX_cond_signal(&state->cv);
    MUTEX_unlock(&state->mtx_engine);
}

void search_stop(state_t *state)
{
    ENGINE_search_stop(state->engine);
    MUTEX_lock(&state->mtx_engine);
    MUTEX_cond_signal(&state->cv);
    MUTEX_unlock(&state->mtx_engine);
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

/* Handle a "usermove" command */
void cmd_usermove(state_t *state, const char *move_str, int respond_to_move)
{
    int result;
    int pos_from, pos_to, promotion_type;

    /* Stop ongoing pondering */
    search_stop(state);

    /* Parse the user move */
    parse_move(move_str, &pos_from, &pos_to, &promotion_type);

    /* Move piece */
    result = ENGINE_apply_move(state->engine, pos_from, pos_to, promotion_type);
    if(result == ENGINE_RESULT_NONE) {
        state->num_half_moves++;
        if(respond_to_move) {
            search_start(state);
        }
    } else if(result == ENGINE_RESULT_ILLEGAL_MOVE) {
        /* Illegal move */
        fprintf(stdout, "Illegal move: %s", move_str);
        
    } else {
        send_result(result);
    }
}

/* Process command from GUI */
static void process_command(char *command, state_t *state)
{
    /* Commands that do reqire action from the engine */
    
    /* protover */
    if(strncmp(command, "protover ", 9) == 0) {
        send_features();
    }
    
    /* new */
    else if(strcmp(command, "new\n") == 0) {
        search_stop(state);

        /* Reset board */
        ENGINE_reset(state->engine);
        
        /* Reset time control */
        state_clear_time(state);

        /* Leave force mode */
        state->flag_forced = 0;
    }
    
    /* usermove */
    else if(strncmp(command, "usermove ", 9) == 0) {
        cmd_usermove(state, command + 9, !state->flag_forced);
    }
    
    /* go */
    else if(strcmp(command, "go\n") == 0) {
        /* Leave force mode */
        state->flag_forced = 0;

        /* Start searching */
        search_stop(state);
        search_start(state);
    }
    
    /*  quit */
    else if(strcmp(command, "quit\n") == 0) {
        state->flag_quit = 1;
        search_stop(state);
    }
    
    /* force */
    else if(strcmp(command, "force\n") == 0) {
        /* Enter force mode */
        state->flag_forced = 1;
        search_stop(state);
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
        search_stop(state);
        ENGINE_resize_hashtable(state->engine, hash_size_mb);
    }

    /* setboard */
    else if(strncmp(command, "setboard ", 9) == 0) {
        search_stop(state);
        if(ENGINE_set_board(state->engine, command + 9) != 0) {
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
        search_stop(state);
    }

    /* ? (move now) */
    else if(strncmp(command, "?\n", 2) == 0) {
        /* Abort the search and send the move now */
        search_stop(state);
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

void *search_thread(void *arg)
{
    state_t *state = (state_t*)arg;

    MUTEX_lock(&state->mtx_engine);
    while(1) {
        while(!state->flag_searching && !state->flag_quit)
            MUTEX_cond_wait(&state->mtx_engine, &state->cv);

        if(state->flag_quit) break;

        int time_left_ms = state->time_left_centiseconds * 10;
        int time_incremental_ms = state->time_incremental_seconds * 1000;
        int moves_left_in_period = 0;

        /* Calculate time available for search */
        if(state->time_period) {
            int num_moves = state->num_half_moves / 2;
            moves_left_in_period = state->time_period - (num_moves % state->time_period);
        }

        /* Start searching */
        int pos_from, pos_to, promotion_type;
        ENGINE_search(state->engine, moves_left_in_period, time_left_ms, time_incremental_ms, 100, &pos_from, &pos_to, &promotion_type);

        /* Handle result */
        int result = ENGINE_apply_move(state->engine, pos_from, pos_to, promotion_type);
        send_move(pos_from, pos_to, promotion_type);
        state->num_half_moves++;
        send_result(result);
        state->flag_searching = 0;
    }
    MUTEX_unlock(&state->mtx_engine);
    return NULL;
}

int main(int argc, char **argv)
{
    char command_buffer[COMMAND_BUFFER_SIZE];
    int len = 0;
    thread_t thread_search;
    state_t state;
    state_clear(&state);

    /* Disable buffering for stdout */
    setbuf(stdout, NULL);
    
    /* Create engine instance */
    ENGINE_create(&state.engine);
    ENGINE_register_search_output_cb(state.engine, &send_search_output);

    /* Create mutex and condition variable */
    MUTEX_create(&state.mtx_engine);
    MUTEX_cond_create(&state.cv);

    /* Create search thread */
    THREAD_create(&thread_search, search_thread, &state);

    /* Welcome */
    fprintf(stdout, "# Welcome to Drosophila " _VERSION "\n");
    fprintf(stdout, "# This program supports the Chess Engine Communication Protocol\n");
    fprintf(stdout, "# and should be run from XBoard or similar\n");
    
    /* Main loop */
    while(1) {
        int c = getchar();
        command_buffer[len++] = c;

        /* Full command received. Take proper action */
        if(c == '\n') {
            command_buffer[len] = '\0';
            process_command(command_buffer, &state);
            len = 0;
        }

        if(state.flag_quit || c == EOF || len == COMMAND_BUFFER_SIZE-1) {
            /* Shutdown if we get the 'quit' command or reach EOF */
            break;
        }
    }

    /* Free thread mutex and condition variable */
    search_stop(&state);
    THREAD_join(thread_search);
    MUTEX_cond_destroy(&state.cv);
    MUTEX_destroy(&state.mtx_engine);

    /* Free engine instance */
    ENGINE_destroy(state.engine);

    return 0;
}
