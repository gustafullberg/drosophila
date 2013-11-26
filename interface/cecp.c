#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include "cecp_features.h"
#include "engine.h"
#include "state.h"

#define STDIN 0
#define INPUT_BUFFER_SIZE 40

int read_line_blocking(char *input_buffer, int max_size)
{
    /* Read from STDIN */
    if(fgets(input_buffer, max_size, stdin)) {
        return strlen(input_buffer);
    }
    
    return 0;
}

int read_line_nonblocking(char *input_buffer, int max_size)
{
    fd_set read_set;
    
    /* Clear set of file descriptors */
    FD_ZERO(&read_set);
    
    /* Add STDIN to set of file descriptors */
    FD_SET(STDIN, &read_set);
    
    /* Poll if there is a command to read */
    if(select(STDIN+1, &read_set, NULL, NULL, NULL) > 0)
    {
        /* Read from STDIN */
        if(fgets(input_buffer, max_size, stdin)) {
            return strlen(input_buffer);
        }
    }
    
    return 0;
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

void str_remove_newline(char *p)
{
    /* Find newline */
    p = strchr(p, '\n');
    
    /* Replace with null-zero */
    if(p) {
        *p = '\0';
    }
}

void process_commands(engine_state_t *engine)
{
    char input_buffer[INPUT_BUFFER_SIZE];

    while(1) {
        if(read_line_blocking(input_buffer, INPUT_BUFFER_SIZE)) {
            /* Commands that do reqire action from the engine */
            
            /* protover */
            if(strncmp(input_buffer, "protover ", 9) == 0) {
                /* Get protocol version number */
                int protocol_version = 0;
                sscanf(input_buffer+9, "%d\n", &protocol_version);
                
                if(protocol_version >= 2) {
                    send_features();
                } else {
                    fprintf(stdout, "# Chess Engine Communication Protocol version must be >= 2. Aborting.\n");
                    break;
                }
            }
            
            /* new */
            else if(strcmp(input_buffer, "new\n") == 0) {
                /* Reset board */
                engine_reset(engine);
            }
            
            /* usermove */
            else if(strncmp(input_buffer, "usermove ", 9) == 0) {
                int result;
                int pos_from, pos_to, promotion_type;
                pos_from = input_buffer[9]-'a' + 8 * (input_buffer[10]-'1');
                pos_to = input_buffer[11]-'a' + 8 * (input_buffer[12]-'1');
                switch(input_buffer[13])
                {
                    case 'n':
                        promotion_type = MOVE_KNIGHT_PROMOTION;
                        break;
                    case 'b':
                        promotion_type = MOVE_BISHOP_PROMOTION;
                        break;
                    case 'r':
                        promotion_type = MOVE_ROOK_PROMOTION;
                        break;
                    case 'q':
                        promotion_type = MOVE_QUEEN_PROMOTION;
                        break;
                    default:
                        promotion_type = 0;
                        break;
                }
                /* Move piece */
                result = engine_opponent_move(engine, pos_from, pos_to, promotion_type);
                if( result == ENGINE_RESULT_NONE) {
                    int pos_from, pos_to, promotion_type;
                    result = engine_ai_move(engine, &pos_from, &pos_to, &promotion_type);
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
                    
                } else if(result == ENGINE_RESULT_ILLEGAL_MOVE) {
                    /* Illegal move */
                    fprintf(stdout, "Illegal move: %s", input_buffer+9);
                    
                } else {
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
            }
            
            /*  quit */
            else if(strcmp(input_buffer, "quit\n") == 0) {
                break;
            }

            
            /* Commands that do not reqire action from the engine (or not implemented) */
            
            /* xboard */
            else if(strcmp(input_buffer, "xboard\n") == 0) {}
            
            /* accepted */
            else if(strncmp(input_buffer, "accepted ", 9) == 0) {}
            
            /* random */
            else if(strcmp(input_buffer, "random\n") == 0) {}
            
            /* post */
            else if(strcmp(input_buffer, "post\n") == 0) {}
            
            /* hard */
            else if(strcmp(input_buffer, "hard\n") == 0) {}
            
            /* cores */
            else if(strncmp(input_buffer, "cores ", 6) == 0) {}
            
            /* level */
            else if(strncmp(input_buffer, "level ", 6) == 0) {}
            
            /* result */
            else if(strncmp(input_buffer, "result ", 7) == 0) {}

            
            /* Errors */
            
            /* rejected */
            else if(strncmp(input_buffer, "rejected ", 9) == 0) {
                str_remove_newline(input_buffer);
                fprintf(stdout, "# Feature rejected \"%s\"\n", input_buffer+9);
            }
            
            /* unknown command */
            else {
                fprintf(stdout, "Error (unknown command): %s", input_buffer);
            }
            
        }
    }
}


int main(int argc, char **argv)
{
    engine_state_t *engine;
    
    /* Create engine instance */
    engine_create(&engine);
    
    /* Disable buffering for stdout */
    setbuf(stdout, NULL);

    /* Welcome */
    fprintf(stdout, "# Welcome to Pawned\n");
    fprintf(stdout, "# This program supports the Chess Engine Communication Protocol\n");
    fprintf(stdout, "# and should be run from XBoard or similar\n");
    
    /* Accept commands from xboard */
    process_commands(engine);

    /* Free engine instance */
    engine_destroy(engine);
    
    return 0;
}
