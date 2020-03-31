#include <stdio.h>
#include <string.h>
#include "engine.h"
#include "thread.h"

#define COMMAND_BUFFER_SIZE 512

FILE *f; // DEBUG CODE!

typedef struct {
    engine_state_t *engine;
    mutex_t mtx_engine;
    cond_t cv;
    int flag_quit;              /* Quit as soon as possible                         */
    int flag_searching;         /* Is currently searching for a move                */
    int flag_pondering;         /* Ponder between moves                             */
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
    state->flag_pondering = 0;
    state->moves_left_in_period = 0;
    state->time_incremental_ms = 0;
    state->time_left_ms = 0;
}

/* Reset only the time control of the state */
void state_clear_time(state_t *state)
{
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
    return;
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

int parse_int(const char *s)
{
    int v = 0;

    while(*s >= '0' && *s <= '9') {
        v *= 10;
        v += (*(s++)) - '0';
    }

    return v;
}

#if 0
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
            period = 1;
            minutes = 0;
            inc_seconds = 0;
            ret = sscanf(level, "%d\n", &seconds);
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
#endif

/* Parse and set board position */
void parse_position(state_t *state, const char *position)
{
    /* Set initial position or FEN */
    if(strncmp(position, "startpos", 8) == 0) {
        /* Reset to initial position */
        ENGINE_reset(state->engine);
    } else {
        /* FEN position */
        int result = ENGINE_set_board(state->engine, position);
        fprintf(f, "FEN result %d\n", result);
    }

    /* Apply additional moves */
    if(position = strstr(position, "moves ")) {
        position += 5;
        do {
            int pos_from, pos_to, promotion_type;
            parse_move(++position, &pos_from, &pos_to, &promotion_type);
            fprintf(f, "Applying move from %d to %d, promotion %d\n", pos_from, pos_to, promotion_type);
            int result = ENGINE_apply_move(state->engine, pos_from, pos_to, promotion_type);
            fprintf(f, "move result %d\n", result);
        } while(position = strchr(++position, ' '));
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

    while(parameters = strchr(parameters, ' ')) {
        if(strncmp(parameters, "searchmoves ", 12) == 0) {
            parameters += 12;
            /* TODO */
        }
        if(strncmp(parameters, "wtime ", 6) == 0) {
            parameters += 6;
            fprintf(f, "start parse wtime\n");
            if(side == 0) state->time_left_ms = parse_int(parameters+1);
            fprintf(f, "time_left_ms is %d\n", state->time_left_ms);
            fprintf(f, "end parse wtime\n");
        }
        else if(strncmp(parameters, "btime ", 6) == 0) {
            parameters += 6;
            if(side == 1) state->time_left_ms = parse_int(parameters);
            fprintf(f, "time_left_ms is %d\n", state->time_left_ms);
        }
        else if(strncmp(parameters, "winc ", 5) == 0) {
            parameters += 5;
            if(side == 0) state->time_incremental_ms = parse_int(parameters);
            fprintf(f, "time_incremental_ms is %d\n", state->time_incremental_ms);
        }
        else if(strncmp(parameters, "binc ", 5) == 0) {
            parameters += 5;
            if(side == 1) state->time_incremental_ms = parse_int(parameters);
            fprintf(f, "time_incremental_ms is %d\n", state->time_incremental_ms);
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
        else if(strncmp(parameters, "infinite ", 9) == 0) {
            parameters += 9;
            state->moves_left_in_period = 1;
            state->time_left_ms = 2000000000;
            state->time_incremental_ms = 0;
        }
        else {
            parameters++;
        }
    }

    search_start(state);
}

#if 0
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
#endif

/* Process command from GUI */
static void process_command(char *command, state_t *state)
{
    /* uci */
    if(strcmp(command, "uci\n") == 0) {
        fprintf(stdout, "id name Drosophila " _VERSION "\n");
        fprintf(stdout, "id author Gustaf Ullberg\n");
        fprintf(stdout, "uciok\n");
    }
    
    /* debug */
    else if(strncmp(command, "debug ", 6) == 0) {
    }

    /* isready */
    else if(strcmp(command, "isready\n") == 0) {
        fprintf(stdout, "readyok\n");
    }
    
    /* setoption */
    else if(strncmp(command, "setoption name ", 15) == 0) {
    }

    /* register */
    else if(strncmp(command, "register ", 9) == 0) {
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
    }
    
    /* ponderhit */
    else if(strcmp(command, "ponderhit\n") == 0) {
    }
    
    /* quit */
    else if(strcmp(command, "quit\n") == 0) {
        state->flag_quit = 1;
        search_stop(state);
    }
#if 0 
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
    
    /* exit */
    else if(strcmp(command, "exit\n") == 0) {
        search_stop(state);
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
#endif 
}

void search(state_t *state)
{
    int pos_from, pos_to, promotion_type;

    fprintf(f, "STARTING SEARCH:\n\tmove left: %d\n\ttime left: %d\n\ttime increment: %d\n", state->moves_left_in_period, state->time_left_ms, state->time_incremental_ms);

    /* Start searching */
    ENGINE_search(state->engine, state->moves_left_in_period, state->time_left_ms, state->time_incremental_ms, 100, &pos_from, &pos_to, &promotion_type);

    /* Handle result */
    send_move(pos_from, pos_to, promotion_type);
}

#if 0
void search_ponder(state_t *state)
{
    int time_left_ms = 3600000; /* 1 hour */
    int time_incremental_ms = 0;
    int moves_left_in_period = 1;

    /* Start searching */
    int pos_from, pos_to, promotion_type;
    ENGINE_search(state->engine, moves_left_in_period, time_left_ms, time_incremental_ms, 100, &pos_from, &pos_to, &promotion_type);
}
#endif

void *search_thread(void *arg)
{
    state_t *state = (state_t*)arg;

    MUTEX_lock(&state->mtx_engine);
    while(1) {
        while(!state->flag_searching && !state->flag_quit)
            MUTEX_cond_wait(&state->mtx_engine, &state->cv);

        if(state->flag_quit) break;

        search(state);

        if(state->flag_searching && state->flag_pondering) {
            //search_ponder(state);
        }

        state->flag_searching = 0;
    }
    MUTEX_unlock(&state->mtx_engine);
    return NULL;
}

int main(int argc, char **argv)
{
    f = fopen("/tmp/debug.txt", "w"); // DEBUG CODE!

    char command_buffer[COMMAND_BUFFER_SIZE];
    int len = 0;
    thread_t thread_search;
    state_t state;
    state_clear(&state);

    /* Disable buffering for stdout */
    setbuf(stdout, NULL);
    setbuf(f, NULL); // DEBUG CODE!
    
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
            fprintf(f, "%s", command_buffer); // DEBUG CODE!
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
