#ifndef _KEY_H
#define _KEY_H
#include "sys.h"



#define READ_WKUP_PIN       HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)  //WKUP PC13

#define KEY_WKUP   4

#define KEY_CLICK   0   //短按、点击
#define KEY_PRESS   1   //长按、按压
extern uint8_t * str;

void user_key_init(void);
uint8_t user_key_scan(uint8_t mode);


void sys_into_stop(void);


#endif
