#include "task.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "my_test.h"

//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];

//任务1控制块
OS_TCB Task1_TCB;
//任务1堆栈	
CPU_STK TASK1_STK[TASK3_STK_SIZE];

//任务2控制块
OS_TCB Task2_TCB;
//任务2堆栈	
CPU_STK TASK2_STK[TASK3_STK_SIZE];

//任务3控制块
OS_TCB Task3_TCB;
//任务3堆栈	
CPU_STK TASK3_STK[TASK3_STK_SIZE];


//任务4控制块
OS_TCB Task4_TCB;
//任务4堆栈	
CPU_STK TASK4_STK[TASK3_STK_SIZE];

/**************************变量区*******************************/
//***************************************************************
OS_SEM	MY_SEM;		    //定义一个信号量，用于访问共享资源
uint8_t key = 0;
uint32_t task_cnt = 0;

//****************************************************************
//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	//使能时间片轮转调度功能,设置默认的时间片长度
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
		
	OS_CRITICAL_ENTER();	//进入临界区
	//创建一个信号量
	OSSemCreate ((OS_SEM*	)&MY_SEM,
                 (CPU_CHAR*	)"MY__SEM",
                 (OS_SEM_CTR)0,		
                 (OS_ERR*	)&err);
				 
	//创建TASK1任务
	OSTaskCreate((OS_TCB 	* )&Task1_TCB,		
				 (CPU_CHAR	* )"Task1 task", 		
                 (OS_TASK_PTR )task1_entry, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK1_TASK_PRIO,     
                 (CPU_STK   * )&TASK1_STK[0],	
                 (CPU_STK_SIZE)TASK1_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP,
                 (OS_ERR 	* )&err);
				 
	//创建TASK2任务
	OSTaskCreate((OS_TCB 	* )&Task2_TCB,		
				 (CPU_CHAR	* )"Task2 task", 		
                 (OS_TASK_PTR )task2_entry, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK2_TASK_PRIO,     
                 (CPU_STK   * )&TASK2_STK[0],	
                 (CPU_STK_SIZE)TASK2_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK2_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP,
                 (OS_ERR 	* )&err);
				 
	//创建TASK3任务
	OSTaskCreate((OS_TCB 	* )&Task3_TCB,		
				 (CPU_CHAR	* )"Task3 task", 		
                 (OS_TASK_PTR )task3_entry, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK3_TASK_PRIO,     
                 (CPU_STK   * )&TASK3_STK[0],	
                 (CPU_STK_SIZE)TASK3_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK3_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP,
                 (OS_ERR 	* )&err);
				 
	//创建TASK4任务
	OSTaskCreate((OS_TCB 	* )&Task4_TCB,		
				 (CPU_CHAR	* )"Task4 task", 		
                 (OS_TASK_PTR )task4_entry, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK4_TASK_PRIO,     
                 (CPU_STK   * )&TASK4_STK[0],	
                 (CPU_STK_SIZE)TASK4_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK4_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR|OS_OPT_TASK_SAVE_FP,
                 (OS_ERR 	* )&err);
				 
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}


//任务1实验板流水灯任务
void task1_entry(void *p_arg)
{
	OS_ERR err;
    err = err;
	while(1)
	{
		led_disp();
	}
}

//任务2按键扫描任务
void task2_entry(void *p_arg)
{
	OS_ERR err;
    uint8_t key_val = 0;
	while(1)
	{
        key_val = user_key_scan(KEY_CLICK);
        if(key_val == KEY_WKUP)
        {
            //sys_into_stop();
        }
        
        led_ctrl_freq();
		OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时1s
		
	}
}

void task3_entry(void *p_arg)
{
	OS_ERR err;
	
	while(1)
    {
		OSTaskSemPend(0,OS_OPT_PEND_BLOCKING,0,&err);		//请求任务内建的信号量
		
		app_test();
		OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);   //延时
	}
}


//按键流水灯任务
void task4_entry(void *p_arg)
{
	OS_ERR err;
	while(1)
    {
		OSTaskSemPost(&Task3_TCB, OS_OPT_POST_FIFO, &err);
        
        printf("ADS1256是TI TexasInstruments公司推出的一款低噪声高分辨率的24位Sigma-Delta模数转换器\r\n");
        printf("\r\n%s %d\r\n", __func__, task_cnt);
        task_cnt++;
		
		OSTimeDlyHMSM(0,0,0,1000,OS_OPT_TIME_PERIODIC,&err);
	}
}




