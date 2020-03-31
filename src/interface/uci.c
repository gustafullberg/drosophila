#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "engine.h"
#include "thread.h"

#define COMMAND_BUFFER_SIZE 10240

typedef struct {
    engine_state_t *engine;
    mutex_t mtx_engine;
    cond_t cv;
    int flag_quit;              /* Quit as soon as possible                         */
    int flag_searching;         /* Is currently searching for a move                */
    int moves_left_in_period;   /* Moves to go in current time control period       */
    int time_incremental_ms;    /* Seconds added per turn                           */
    int time_left_ms;           /* Time left in current control period (10^-2 sec)  */
} state_t;

/* Reset state to known defaults */
void state_clear(state_t *state)
{
    state->engine = NULL;
    state->flag_quit = 0;
    state->flag_searching = 0;
    state->moves_left_in_period = 0;
    state->time_incremental_ms = 0;
    state->time_left_ms = 0;
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
    state->flag_searching = 0;
    ENGINE_search_stop(state->engine);
    MUTEX_lock(&state->mtx_engine);
    MUTEX_cond_signal(&state->cv);
    MUTEX_unlock(&state->mtx_engine);
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
        fprintf(stdout, "bestmove %c%c%c%c%c\n", (pos_from%8)+'a', (pos_from/8)+'1', (pos_to%8)+'a', (pos_to/8)+'1', pt);
    } else {
        fprintf(stdout, "bestmove %c%c%c%c\n", (pos_from%8)+'a', (pos_from/8)+'1', (pos_to%8)+'a', (pos_to/8)+'1');
    }
}

/* Send stats and PV to GUI while searching for move */
void send_search_output(int ply, int score, int time_ms, unsigned int nodes, int pv_length, int *pos_from, int *pos_to, int *promotion_type)
{
    int i;

    uint64_t nps = nodes;
    if(time_ms) nps = 1000 * nps / time_ms;

    fprintf(stdout, "info depth %d score cp %d time %d nodes %d nps %ld pv", ply, score, time_ms, nodes, nps);
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

/* Parse decimal integer */
int parse_int(const char *s)
{
    int v = 0;

    while(*s >= '0' && *s <= '9') {
        v *= 10;
        v += (*(s++)) - '0';
    }

    return v;
}

/* Parse and set board position */
void parse_position(state_t *state, const char *position)
{
    /* Set initial position or FEN */
    if(strncmp(position, "startpos", 8) == 0) {
        /* Reset to initial position */
        ENGINE_reset(state->engine);
    } else if(strncmp(position, "fen ", 4) == 0) {
        /* FEN position */
        ENGINE_set_board(state->engine, position+4);
    }

    /* Apply additional moves */
    if((position = strstr(position, "moves "))) {
        position += 5;
        do {
            int pos_from, pos_to, promotion_type;
            parse_move(++position, &pos_from, &pos_to, &promotion_type);
            ENGINE_apply_move(state->engine, pos_from, pos_to, promotion_type);
        } while((position = strchr(++position, ' ')));
    }
}

/* Parse search parameters and start searching */
void parse_go(state_t *state, const char *parameters)
{
    int side = ENGINE_playing_side(state->engine);

    /* Default parameters */
    state->moves_left_in_period = 0;
    state->time_left_ms = 100;
    state->time_incremental_ms = 0;

    while((parameters = strchr(parameters, ' '))) {
        parameters++;

        if(strncmp(parameters, "searchmoves ", 12) == 0) {
            parameters += 12;
            /* TODO */
        }
        if(strncmp(parameters, "wtime ", 6) == 0) {
            parameters += 6;
            if(side == 0) state->time_left_ms = parse_int(parameters);
        }
        else if(strncmp(parameters, "btime ", 6) == 0) {
            parameters += 6;
            if(side == 1) state->time_left_ms = parse_int(parameters);
        }
        else if(strncmp(parameters, "winc ", 5) == 0) {
            parameters += 5;
            if(side == 0) state->time_incremental_ms = parse_int(parameters);
        }
        else if(strncmp(parameters, "binc ", 5) == 0) {
            parameters += 5;
            if(side == 1) state->time_incremental_ms = parse_int(parameters);
        }
        else if(strncmp(parameters, "movestogo ", 10) == 0) {
            parameters += 10;
            state->moves_left_in_period = parse_int(parameters);
        }
        else if(strncmp(parameters, "depth ", 6) == 0) {
            parameters += 6;
            /* TODO */
        }
        else if(strncmp(parameters, "nodes ", 6) == 0) {
            parameters += 6;
            /* TODO */
        }
        else if(strncmp(parameters, "mate ", 5) == 0) {
            parameters += 5;
            /* TODO */
        }
        else if(strncmp(parameters, "movetime ", 9) == 0) {
            parameters += 9;
            state->moves_left_in_period = 1;
            state->time_left_ms = parse_int(parameters);
            state->time_incremental_ms = 0;
        }
        else if(strncmp(parameters, "infinite", 8) == 0) {
            parameters += 8;
            state->moves_left_in_period = 1;
            state->time_left_ms = 2000000000;
            state->time_incremental_ms = 0;
        }
    }

    search_start(state);
}

/* Parse options set by GUI */
void parse_option(state_t *state, const char *parameters)
{
    if(strncmp(parameters, "Hash value ", 11) == 0) {
        parameters += 11;
        int size_mb = parse_int(parameters);
        if(size_mb < 1) size_mb = 1;
        else if(size_mb > 1024) size_mb = 1024;
        ENGINE_resize_hashtable(state->engine, size_mb);
    }
}

/* Process command from GUI */
static void process_command(char *command, state_t *state)
{
    /* uci */
    if(strcmp(command, "uci\n") == 0) {
        fprintf(stdout, "id name Drosophila " _VERSION "\n");
        fprintf(stdout, "id author Gustaf Ullberg\n");
        fprintf(stdout, "option name Hash type spin default 64 min 1 max 1024\n");
        fprintf(stdout, "uciok\n");
    }
    
    /* debug */
    else if(strncmp(command, "debug ", 6) == 0) {
        /* TODO */
    }

    /* isready */
    else if(strcmp(command, "isready\n") == 0) {
        fprintf(stdout, "readyok\n");
    }
    
    /* setoption */
    else if(strncmp(command, "setoption name ", 15) == 0) {
        parse_option(state, command + 15);
    }

    /* register */
    else if(strncmp(command, "register ", 9) == 0) {
        /* NOP */
    }

    /* ucinewgame */
    else if(strcmp(command, "ucinewgame\n") == 0) {
        ENGINE_reset(state->engine);
    }

    /* position */
    else if(strncmp(command, "position ", 9) == 0) {
        parse_position(state, command + 9);
    }

    /* go */
    else if(strncmp(command, "go ", 3) == 0 || strcmp(command, "go\n") == 0) {
        parse_go(state, command + 2);
    }

    /* stop */
    else if(strcmp(command, "stop\n") == 0) {
        search_stop(state);
    }
    
    /* ponderhit */
    else if(strcmp(command, "ponderhit\n") == 0) {
        /* TODO */
    }
    
    /* quit */
    else if(strcmp(command, "quit\n") == 0) {
        state->flag_quit = 1;
        search_stop(state);
    }
}

void search(state_t *state)
{
    int pos_from, pos_to, promotion_type;

    /* Start searching */
    ENGINE_search(state->engine, state->moves_left_in_period, state->time_left_ms, state->time_incremental_ms, 100, &pos_from, &pos_to, &promotion_type);

    /* Handle result */
    send_move(pos_from, pos_to, promotion_type);
}

void *search_thread(void *arg)
{
    state_t *state = (state_t*)arg;

    MUTEX_lock(&state->mtx_engine);
    while(1) {
        while(!state->flag_searching && !state->flag_quit)
            MUTEX_cond_wait(&state->mtx_engine, &state->cv);

        if(state->flag_quit) break;

        search(state);

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

    /* Main loop */
    while(1) {
        int c = getchar();
        command_buffer[len++] = (char)c;

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

    /* Free thread, mutex and condition variable */
    search_stop(&state);
    THREAD_join(thread_search);
    MUTEX_cond_destroy(&state.cv);
    MUTEX_destroy(&state.mtx_engine);

    /* Free engine instance */
    ENGINE_destroy(state.engine);

    return 0;
}
