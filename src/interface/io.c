#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <io.h>
#define STDIN_FILENO 0
#define read _read
#else
#include <unistd.h>
#include <sys/select.h>
#endif
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

int IO_ready()
{
#ifdef _WIN32
    DWORD num_bytes = 0;
    HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    PeekNamedPipe(handle, NULL, 0, NULL, &num_bytes, NULL);
    if(num_bytes == 0) Sleep(1);
    return num_bytes > 0;
#else
    fd_set set;
    struct timeval timeout = { 0, 10 }; /* 10 usec timeout */
    
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);

    /* Poll for input on STDIN */
    return select(STDIN_FILENO+1, &set, NULL, NULL, &timeout) > 0;
#endif
}

int IO_read_input(char *command_buffer, int max_size)
{
    char *p;

    /* Poll for input on STDIN */
    if(IO_ready()) {
        /* Append new input at the end of the input buffer */
        int len = (int)strlen(input_buffer);
        len += read(STDIN_FILENO, input_buffer + len, INPUT_BUFFER_SIZE - len - 1);
        input_buffer[len] = '\0';
    }
    
    /* Check if there is a complete command in the buffer */
    p = strchr(input_buffer, '\n');
    if(p) {
        /* Move it to the command buffer */
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
