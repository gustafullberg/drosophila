#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "io.h"

#define INPUT_BUFFER_SIZE 1024

static char input_buffer[INPUT_BUFFER_SIZE];

void IO_init(void)
{
    /* Disable buffering for stdout */
    setbuf(stdout, NULL);
    
    /* Terminate input buffer */
    input_buffer[0] = '\0';
}

int IO_read_input(char *command_buffer, int max_size)
{
    char *p;
    fd_set set;
    struct timeval timeout = { 0, 10 }; /* 10 usec timeout */
    
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);

    /* Poll for input on STDIN */
    if(0 < select(STDIN_FILENO+1, &set, NULL, NULL, &timeout)) {
        /* Append new input at the end of the input buffer */
        int len = strlen(input_buffer);
        len += read(STDIN_FILENO, input_buffer + len, INPUT_BUFFER_SIZE - len - 1);
        input_buffer[len] = '\0';
    }
    
    /* Check if there is a complete command in the buffer */
    p = strchr(input_buffer, '\n');
    if(p) {
        /* Move it to the command ufer */
        int len = (int)(p - input_buffer) + 1;
        if(len >= max_size) {
            len = max_size - 1;
        }
        strncpy(command_buffer, input_buffer, len);
        command_buffer[len] = '\0';
        memmove(input_buffer, &input_buffer[len], INPUT_BUFFER_SIZE - len);
        
        return len;
    }
    
    return 0;
}
