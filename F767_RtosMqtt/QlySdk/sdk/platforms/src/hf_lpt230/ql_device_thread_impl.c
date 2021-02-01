#include "hsf.h"
#include "ql_device_thread_impl.h"
#include "ql_device_basic_impl.h"

struct ql_thread_t {
    hfthread_hande_t thread;
};

ql_thread_t *ql_thread_create(int priority, int stack_size, ql_thread_fn fn, void* arg)
{
    int ret = 0;
    char name[32] = {0};
    static int thread_no = 0;

    ql_snprintf(name, 32, "thread %d", thread_no);
    thread_no++;

    ql_thread_t *ql_thread = (ql_thread_t *)ql_malloc(sizeof(ql_thread_t));
    if (NULL == ql_thread) {
        return NULL;
    }

    ret = hfthread_create((PHFTHREAD_START_ROUTINE)fn, name, stack_size, NULL, priority, &(ql_thread->thread), NULL);
    if (ret != HF_SUCCESS)
    {
        ql_free(ql_thread);
        return NULL;
    }

    return ql_thread;
}

void ql_thread_destroy(ql_thread_t **thread)
{
    if (thread && *thread) {
        ql_free(*thread);
        *thread = NULL;
    }
}

void ql_thread_schedule(void)
{
    return;
}

struct ql_mutex_t {
    hfthread_mutex_t mutex;
};

ql_mutex_t *ql_mutex_create()
{
    int ret = 0;
    ql_mutex_t *ql_mutex = NULL;

    ql_mutex = (ql_mutex_t *)ql_malloc(sizeof(ql_mutex_t));
    if (NULL == ql_mutex) {
        return NULL;
    }

    ret = hfthread_mutext_new(&(ql_mutex->mutex));
    if (ret != HF_SUCCESS)
    {
        ql_free(ql_mutex);
        return NULL;
    }

    return ql_mutex;


}

void ql_mutex_destroy(ql_mutex_t **mutex)
{
    if (mutex && *mutex) {
        hfthread_mutext_free((*mutex)->mutex);
        ql_free(*mutex);
        *mutex = NULL;
    }
}

int ql_mutex_lock(ql_mutex_t *mutex)
{
    int ret;
    ret = hfthread_mutext_lock(mutex->mutex);
    if(ret == HF_SUCCESS)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int ql_mutex_unlock(ql_mutex_t *mutex)
{

    hfthread_mutext_unlock(mutex->mutex);

    return 0;
}
