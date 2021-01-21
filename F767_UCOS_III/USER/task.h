#ifndef TASK_H_
#define TASK_H_

#include "sys.h"
#include "includes.h"

extern u32 i;
extern OS_SEM	MY_SEM;	

//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		128
//任务控制块
extern OS_TCB StartTaskTCB;
//任务堆栈	
extern CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define TASK1_TASK_PRIO		9
//任务堆栈大小	
#define TASK1_STK_SIZE 		128
//任务控制块
extern OS_TCB Task1_TCB;
//任务堆栈	
extern CPU_STK TASK1_STK[TASK1_STK_SIZE];
void task1_entry(void *p_arg);

//任务优先级
#define TASK2_TASK_PRIO		4
//任务堆栈大小	
#define TASK2_STK_SIZE 		128
//任务控制块
extern OS_TCB Task2_TCB;
//任务堆栈	
extern CPU_STK TASK2_STK[TASK2_STK_SIZE];
void task2_entry(void *p_arg);

//任务优先级
#define TASK3_TASK_PRIO		8
//任务堆栈大小	
#define TASK3_STK_SIZE 		128
//任务控制块
extern OS_TCB Task3_TCB;
//任务堆栈	
extern CPU_STK TASK3_STK[TASK3_STK_SIZE];
void task3_entry(void *p_arg);


//任务优先级
#define TASK4_TASK_PRIO		5
//任务堆栈大小	
#define TASK4_STK_SIZE 		128
//任务控制块
extern OS_TCB Task4_TCB;
//任务堆栈	
extern CPU_STK TASK4_STK[TASK3_STK_SIZE];
void task4_entry(void *p_arg);


#endif
