#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32
#include <windows.h>
typedef HANDLE thread_t;
#else
#include <pthread.h>
typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
#endif

void THREAD_create(thread_t *thread, void *(*thread_function)(void*), void *arg);
void THREAD_join(thread_t thread);
void MUTEX_create(mutex_t *mutex);
void MUTEX_destroy(mutex_t *mutex);
void MUTEX_lock(mutex_t *mutex);
void MUTEX_unlock(mutex_t *mutex);

#endif
