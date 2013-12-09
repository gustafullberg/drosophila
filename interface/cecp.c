#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cecp_features.h"
#include "io.h"
#include "engine.h"

#define INPUT_BUFFER_SIZE 1024

enum state_t { STATE_NORMAL, STATE_FORCE, STATE_QUIT };

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

void make_move(engine_state_t *engine)
{
    int pos_from, pos_to, promotion_type, result;
    result = ENGINE_think_and_move(engine, &pos_from, &pos_to, &promotion_type);
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
    
    send_result(result);
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

void user_move(engine_state_t *engine, const char *move_str, int respond_to_move)
{
    int result;
    int pos_from, pos_to, promotion_type;
    parse_move(move_str, &pos_from, &pos_to, &promotion_type);

    /* Move piece */
    result = ENGINE_apply_move(engine, pos_from, pos_to, promotion_type);
    if(result == ENGINE_RESULT_NONE) {
        if(respond_to_move) {
            make_move(engine);
        }
    } else if(result == ENGINE_RESULT_ILLEGAL_MOVE) {
        /* Illegal move */
        fprintf(stdout, "Illegal move: %s", move_str);
        
    } else {
        send_result(result);
    }
}

static int process_command(engine_state_t *engine, char *command, enum state_t state)
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
    }
    
    /* usermove */
    else if(strncmp(command, "usermove ", 9) == 0) {
        user_move(engine, command + 9, state == STATE_NORMAL);
    }
    
    /* go */
    else if(strcmp(command, "go\n") == 0) {
        state = STATE_NORMAL;
        make_move(engine);
    }
    
    /*  quit */
    else if(strcmp(command, "quit\n") == 0) {
        return STATE_QUIT;
    }
    
    /* force */
    else if(strcmp(command, "force\n") == 0) {
        state = STATE_FORCE;
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
    
    /* hard */
    else if(strcmp(command, "hard\n") == 0) {}
    
    /* cores */
    else if(strncmp(command, "cores ", 6) == 0) {}
    
    /* level */
    else if(strncmp(command, "level ", 6) == 0) {}
    
    /* result */
    else if(strncmp(command, "result ", 7) == 0) {}
    
    /* computer */
    else if(strcmp(command, "computer\n") == 0) {}

    
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
    
    return state;
}


int main(int argc, char **argv)
{
    enum state_t state = STATE_NORMAL;
    engine_state_t *engine;
    char input_buffer[INPUT_BUFFER_SIZE];
    
    IO_init();
    
    /* Create engine instance */
    ENGINE_create(&engine);

    /* Welcome */
    fprintf(stdout, "# Welcome to Pawned\n");
    fprintf(stdout, "# This program supports the Chess Engine Communication Protocol\n");
    fprintf(stdout, "# and should be run from XBoard or similar\n");
    
    /* Main loop */
    while(1) {
        /* Check if a command is sent from Xboard */
        if(IO_read_input(input_buffer, INPUT_BUFFER_SIZE)) {
            /* Take proper action */
            state = process_command(engine, input_buffer, state);
            if(state == STATE_QUIT) {
                /* Shutdown if we get the 'quit' command etc */
                break;
            }
        }
        
        /* Sleep a little */
        usleep(1);
    }

    /* Free engine instance */
    ENGINE_destroy(engine);
    
    return 0;
}
