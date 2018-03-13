#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32
#include <windows.h>
typedef HANDLE thread_t;
typedef CRITICAL_SECTION mutex_t;
typedef CONDITION_VARIABLE cond_t;
#else
#include <pthread.h>
typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;
#endif

void THREAD_create(thread_t *thread, void *(*thread_function)(void*), void *arg);
void THREAD_join(thread_t thread);
void MUTEX_create(mutex_t *mutex);
void MUTEX_destroy(mutex_t *mutex);
void MUTEX_lock(mutex_t *mutex);
void MUTEX_unlock(mutex_t *mutex);
void MUTEX_cond_create(cond_t *cv);
void MUTEX_cond_destroy(cond_t *cv);
void MUTEX_cond_wait(mutex_t *mutex, cond_t *cv);
void MUTEX_cond_signal(cond_t *cv);

#endif
