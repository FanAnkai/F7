#include <stdio.h>
#include <stdlib.h>
#include "ql_device_thread_impl.h"
#include "ql_device_basic_impl.h"

#include "ql_include.h"
#include "hsf.h"
#include "hf_tcp_impl.h"
#include "drv_timer.h"

struct ql_thread_t {
    // pthread_t thread;
    int thread;
};

struct ql_mutex_t {
    // pthread_mutex_t mutex;
    int mutex;
};

int ql_logic_main_process_flag = 0;

PROCESS(ql_logic_main_process, "ql_logic_main_process");
PROCESS(ql_logic_time_process, "ql_logic_time_process");
PROCESS(qlcloud_udp_server_process, "qlcloud_udp_server_process");

extern void *ql_logic_main(void *para);
extern void *ql_logic_time(void *para);
extern void *qlcloud_udp_server(void *para);
extern void ql_logic_endless_task();
extern void ql_logic_period_task();
extern void ql_udp_server_task(void);

static void test_timer_start(void);

ql_thread_t *ql_thread_create(int priority, int stack_size, ql_thread_fn fn, void* arg)
{
    ql_thread_t *ql_thread = (ql_thread_t *)ql_malloc(sizeof(ql_thread_t));
    if (NULL == ql_thread) {
        return NULL;
    }

    if(ql_logic_main == fn)
    {
        process_start(&ql_logic_main_process, NULL);
        tcp_process_start();
        test_timer_start();
    }
    else if(ql_logic_time == fn)
    {
        process_start(&ql_logic_time_process, NULL);
    }
    else if (qlcloud_udp_server == fn)
    {
        process_start(&qlcloud_udp_server_process, NULL);
    }
    else
    {
        return 0;
    }

    ql_thread->thread = (void *)fn;
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

ql_mutex_t *ql_mutex_create()
{
    // int ret = 0;
    // ql_mutex_t *ql_mutex = NULL;

    // ql_mutex = (ql_mutex_t *)malloc(sizeof(ql_mutex_t));
    // if (NULL == ql_mutex) {
    //     return NULL;
    // }

    // ret = pthread_mutex_init(&(ql_mutex->mutex), NULL);
    // if (ret != 0) {
    //     ql_free(ql_mutex);
    //     return NULL;
    // }

    // return ql_mutex;

    return (ql_mutex_t *)ql_malloc(sizeof(ql_mutex_t));
}

void ql_mutex_destroy(ql_mutex_t **mutex)
{
    // if (mutex && *mutex) {
    //     pthread_mutex_destroy(&((*mutex)->mutex));
    //     ql_free(*mutex);
    //     *mutex = NULL;
    // }
}

int ql_mutex_lock(ql_mutex_t *mutex)
{
    // int ret = pthread_mutex_lock(&(mutex->mutex));
	// if (ret == 0) {
	// 	return 1;
	// } else {
	// 	return 0;
	// }

    return 1;
}

int ql_mutex_unlock(ql_mutex_t *mutex)
{
    // return pthread_mutex_unlock(&(mutex->mutex));
    return 1;
}

void post_event_logic_main_process(void)
{
	process_post(&ql_logic_main_process, PROCESS_EVENT_CONTINUE, NULL);
}

void post_event_logic_time_process(void)
{
	process_post(&ql_logic_time_process, PROCESS_EVENT_CONTINUE, NULL);
}

void post_event_udp_server_process(void)
{
	process_post(&qlcloud_udp_server_process, PROCESS_EVENT_CONTINUE, NULL);
}

PROCESS_THREAD(ql_logic_main_process, ev, data)
{
	PROCESS_BEGIN();

	ql_log_info("ql_logic_main_process is begin\r\n");
	while(1) 
	{
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER || ev == PROCESS_EVENT_CONTINUE);
        ql_logic_main_process_flag = 2;
        ql_logic_endless_task();
        ql_logic_main_process_flag = 0;
    }
	PROCESS_END();
}

PROCESS_THREAD(ql_logic_time_process, ev, data)
{
	PROCESS_BEGIN();

	ql_log_info("ql_logic_time_process is begin\r\n");
	while(1) 
	{
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER || ev == PROCESS_EVENT_CONTINUE);
        ql_logic_period_task();
    }
	PROCESS_END();
}

PROCESS_THREAD(qlcloud_udp_server_process, ev, data)
{
	PROCESS_BEGIN();

	ql_log_info("qlcloud_udp_server_process is begin\r\n");
	while(1) 
	{
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER || ev == PROCESS_EVENT_CONTINUE);
        ql_udp_server_task();
    }
	PROCESS_END();
}

ATTRIBUTE_SECTION_KEEP_IN_SRAM static void ms_timer_callback( void *arg )
{
	static unsigned int count = 0;

    if(ql_logic_main_process_flag != 2)
    {
        post_event_logic_main_process();
        ql_logic_main_process_flag = 1;
    }

	if(count % 10 == 0)
	{
		// post_event_tcp_process();
        post_event_logic_time_process();
#ifdef LAN_DETECT
        post_event_udp_server_process();
#endif
	}
	count++;
}

static void test_timer_start(void)
{
	ql_log_info("To init timer...\r\n");
	// MS_TIMER2 定时 100 ms，循环定时
	hwtmr_start(MS_TIMER2,100,ms_timer_callback,NULL, HTMR_PERIODIC);
}
