/*
 * Copyright (c) 2016 qinglianyun.
 * All rights reserved.
 */

#ifndef QL_DEVICE_THREAD_IMPL_H_
#define QL_DEVICE_THREAD_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************线程接口************************************/
typedef struct ql_thread_t ql_thread_t;
typedef struct ql_mutex_t ql_mutex_t;

#if (__SDK_PLATFORM__ == 0x02 || __SDK_PLATFORM__ == 0x03 ) /*esp8266 freeRtos*/
typedef void (*ql_thread_fn)(void *);
#elif (__SDK_PLATFORM__ == 0x12 )
typedef void (*ql_thread_fn)(void);
#else
typedef void * (*ql_thread_fn)(void *);
#endif

/**创建线程 - malloc from heap
 * @param priority 0-系统默认值
 * @param stack_size 线程需要使用的栈大小
 * @param fn 线程的执行函数体
 * @param arg fn函数传入参数
 *
 * @return 线程（句柄）标记
 */
extern ql_thread_t *ql_thread_create(int priority,
                                int stack_size,
								ql_thread_fn fn,
                                void* arg);

/**销毁线程句柄*/
extern void ql_thread_destroy(ql_thread_t **thread);

extern void ql_thread_schedule(void);


/******************************mutex接口**********************************/

/*创建mutex - malloc from heap*/
extern ql_mutex_t *ql_mutex_create(void);

/*销毁mutex*/
extern void ql_mutex_destroy(ql_mutex_t **mutex);

/*获取锁mutex*/
/*成功 1; 失败 0*/
extern int ql_mutex_lock(ql_mutex_t *mutex);

/*解锁mutex*/
extern int ql_mutex_unlock(ql_mutex_t *mutex);

#if (__SDK_PLATFORM__ == 0x12 )
extern void ql_start_logic(ql_thread_fn fn);
extern void ql_start_time(ql_thread_fn fn);
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* QL_DEVICE_THREAD_IMPL_H_ */
