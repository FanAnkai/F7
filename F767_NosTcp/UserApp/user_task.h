
#ifndef __USER_TASK_H
#define __USER_TASK_H

#include "main.h"


#define Task_1000ms	1000
#define Task_500ms	500
#define Task_200ms	200
#define Task_100ms	100
#define Task_50ms	50
#define Task_20ms	20
#define Task_10ms	10
#define Task_5ms	5
#define Task_2ms	2


typedef struct
{
    uint16_t PowerUpCnt;

    uint16_t	t_1000ms;
    uint16_t	t_500ms;
    uint8_t		t_200ms;
    uint8_t		t_100ms;
    uint8_t		t_50ms;
    uint8_t		t_20ms;
    uint8_t		t_10ms;
	uint8_t		t_5ms;
	uint8_t		t_2ms;

} USER_TASK_TYPE;

extern USER_TASK_TYPE gTask;

void app_run(void);

void app_timer_inc(void);

void task_led_entry(void);
void task_test_c_entry(void);
void print_sys_log(void);


#endif /* __DRV_LED_H */

/*******************************  END OF FILE  ********************************/


