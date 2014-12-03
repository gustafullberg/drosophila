#ifndef _THREAD_H
#define _THREAD_H

#include <pthread.h>

typedef pthread_t thread_t;

void THREAD_create(thread_t *thread, void *(*thread_function)(void*), void *arg);
void THREAD_join(thread_t thread);

#endif
