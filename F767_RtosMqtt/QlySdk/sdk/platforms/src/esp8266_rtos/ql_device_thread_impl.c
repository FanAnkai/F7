#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "ql_device_thread_impl.h"
#include "ql_device_basic_impl.h"
#include "ql_log.h"
struct ql_thread_t {
    xTaskHandle thread;
};

ql_thread_t *ql_thread_create(int priority, int stack_size, ql_thread_fn fn, void* arg)
{
    int ret = 0;
    static int taskno = 1;
    char taskname[15];
    
    ql_thread_t *ql_thread = (ql_thread_t *)ql_malloc(sizeof(ql_thread_t));
    if (NULL == ql_thread) {
        return NULL;
    }
#ifdef DEBUG_DATA_OPEN
    ql_snprintf(taskname,sizeof(taskname), "Task_%d", taskno++);
#endif
    ret = xTaskCreate(fn, taskname, stack_size/4, NULL, 1, &(ql_thread->thread));
    if(pdPASS != ret)
    {
    
#ifdef DEBUG_DATA_OPEN
        ql_printf("task create failed, ret:%d\r\n", ret);
#endif
        ql_free(ql_thread);
        return NULL;
    }
#ifdef DEBUG_DATA_OPEN
    ql_printf("new [%s]\r\n", taskname);
#endif
    return ql_thread;
}

void ql_thread_destroy(ql_thread_t **thread)
{
    if (thread && *thread) 
    {
        vTaskDelete((*thread)->thread);
        ql_free(*thread);
        *thread = NULL;
    }
}

void ql_thread_schedule(void)
{
    return;
}

struct ql_mutex_t {
    xSemaphoreHandle mutex;
};


ql_mutex_t *ql_mutex_create()
{
    ql_mutex_t *ql_mutex = NULL;

    ql_mutex = (ql_mutex_t *)ql_malloc(sizeof(ql_mutex_t));
    if (NULL == ql_mutex) 
    {
        return NULL;
    }

    ql_mutex->mutex = xSemaphoreCreateMutex();
    if (ql_mutex->mutex == NULL)
    {
    
#ifdef DEBUG_DATA_OPEN
        ql_printf("mutex create failed\r\n");
#endif
        ql_free(ql_mutex);
        return NULL;
    }

    return ql_mutex;
}

void ql_mutex_destroy(ql_mutex_t **mutex)
{
    if (mutex && *mutex) 
    {
        ql_free(*mutex);
        *mutex = NULL;
    }
}

int ql_mutex_lock(ql_mutex_t *mutex)
{
    if(xSemaphoreTake(mutex->mutex, 10) == pdTRUE)
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
    return xSemaphoreGive(mutex->mutex);
}
