#pragma comment(lib, "pthreadVC2.lib")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <pthread.h>

#include "ql_device_thread_impl.h"
#include "ql_device_basic_impl.h"

struct ql_thread_t {
    pthread_t thread;
};

struct ql_mutex_t {
        pthread_mutex_t mutex;
};

ql_thread_t *ql_thread_create(int priority, int stack_size, ql_thread_fn fn, void* arg)
{
    int ret = 0;

    ql_thread_t *ql_thread = (ql_thread_t *)malloc(sizeof(ql_thread_t));
    if (NULL == ql_thread) {
        return NULL;
    }

    ret = pthread_create(&(ql_thread->thread), NULL, fn, arg);
    if (ret != 0) {
        ql_free(ql_thread);
        return NULL;
    }

    return ql_thread;
}

void ql_thread_destroy(ql_thread_t **thread)
{
    if (thread && *thread) {
        pthread_cancel((*thread)->thread);
        ql_free(*thread);
        *thread = NULL;
    }
}

void ql_thread_schedule(void)
{
    return;
}

ql_mutex_t *ql_mutex_create()
{
    int ret = 0;
    ql_mutex_t *ql_mutex = NULL;

    ql_mutex = (ql_mutex_t *)malloc(sizeof(ql_mutex_t));
    if (NULL == ql_mutex) {
        return NULL;
    }

    ret = pthread_mutex_init(&(ql_mutex->mutex), NULL);
    if (ret != 0) {
        ql_free(ql_mutex);
        return NULL;
    }

    return ql_mutex;
}

void ql_mutex_destroy(ql_mutex_t **mutex)
{
    if (mutex && *mutex) {
        pthread_mutex_destroy(&((*mutex)->mutex));
        ql_free(*mutex);
        *mutex = NULL;
    }
}

int ql_mutex_lock(ql_mutex_t *mutex)
{
    int ret = pthread_mutex_lock(&(mutex->mutex));
	if (ret == 0) {
		return 1;
	} else {
		return 0;
	}
}

int ql_mutex_unlock(ql_mutex_t *mutex)
{
    return pthread_mutex_unlock(&(mutex->mutex));
}
