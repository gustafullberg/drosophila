#include <stddef.h>
#include "thread.h"

void THREAD_create(thread_t *thread, void *(*thread_function)(void*), void *arg)
{
#ifdef _WIN32
    *thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)*thread_function, arg, 0, NULL);
#else
    pthread_create(thread, NULL, thread_function, arg);
#endif
}

void THREAD_join(thread_t thread)
{
#ifdef _WIN32
    WaitForSingleObject(thread, INFINITE);
#else
    pthread_join(thread, NULL);
#endif
}
