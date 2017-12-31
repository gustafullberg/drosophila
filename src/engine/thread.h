#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32
#include <windows.h>
typedef HANDLE thread_t;
#else
#include <pthread.h>
typedef pthread_t thread_t;
#endif

void THREAD_create(thread_t *thread, void *(*thread_function)(void*), void *arg);
void THREAD_join(thread_t thread);

#endif
