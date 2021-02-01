#include "mbed.h"

#include "ql_device_thread_impl.h"
#include "ql_device_basic_impl.h"

struct ql_thread_t {
	Thread* p_thread;
};

ql_thread_t *ql_thread_create(int priority, int stack_size, ql_thread_fn fn, void* arg)
{
    ql_thread_t *ql_thread = (ql_thread_t *)ql_malloc(sizeof(ql_thread_t));
    if (NULL == ql_thread) {
        return NULL;
    }
	ql_thread->p_thread = new Thread();
	
    if (!ql_thread->p_thread ) {
        ql_free(ql_thread);
        return NULL;
    }
	ql_thread->p_thread->start(fn);
    return ql_thread;
}

void ql_thread_destroy(ql_thread_t **thread)
{
    if (thread && *thread) {
		delete (*thread)->p_thread;
        ql_free(*thread);
        *thread = NULL;
    }
}

void ql_thread_schedule(void)
{
    return;
}

struct ql_mutex_t {
	Mutex* p_mutex;
};

ql_mutex_t *ql_mutex_create(void)
{
    ql_mutex_t *ql_mutex = NULL;

    ql_mutex = (ql_mutex_t *)ql_malloc(sizeof(ql_mutex_t));
    if (NULL == ql_mutex) {
        return NULL;
    }
	ql_mutex->p_mutex = new Mutex();
	
    if (!ql_mutex->p_mutex) {
        ql_free(ql_mutex);
        return NULL;
    }
    return ql_mutex;
}

void ql_mutex_destroy(ql_mutex_t **mutex)
{
    if (mutex && *mutex) {
		delete (*mutex)->p_mutex;
		ql_free(*mutex);
        *mutex = NULL;
    }
}

int ql_mutex_lock(ql_mutex_t *mutex)
{
    int ret = 0;

	ret = mutex->p_mutex->lock();

	if (ret == 0) {
		return 1;
	} else {
		return 0;
	}
}

int ql_mutex_unlock(ql_mutex_t *mutex)
{
    return mutex->p_mutex->unlock();
}
