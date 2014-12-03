#include <stddef.h>
#include "thread.h"

void THREAD_create(thread_t *thread, void *(*thread_function)(void*), void *arg)
{
    pthread_create(thread, NULL, thread_function, arg);
}

void THREAD_join(thread_t thread)
{
    pthread_join(thread, NULL);
}
