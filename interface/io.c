#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "io.h"

#define STDIN 0

void IO_init()
{
    /* Disable buffering for stdout */
    setbuf(stdout, NULL);
    
    /* Non-blocking stdin */
    fcntl(STDIN, F_SETFL, fcntl(STDIN, F_GETFL, 0) | O_NONBLOCK);
}

int IO_read_input(char *input_buffer, int max_size)
{
    char *p;
    
    /* Something to read? */
    p = fgets(input_buffer, max_size, stdin); 
    if(p) {
        /* Make sure to continue reading until newline is found */
        while(!strchr(input_buffer, '\n')) {
            int bytes_read = strlen(input_buffer);
            p = fgets(input_buffer + bytes_read, max_size - bytes_read, stdin);
        }
        return 1;
    } else {
        return 0;
    }
}
