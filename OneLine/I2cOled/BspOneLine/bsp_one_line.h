
#ifndef BSP_ONE_LINE_H_
#define BSP_ONE_LINE_H_



#include "main.h"
#include "tim.h"
#include "gpio.h"


#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"


#define MAX_BUFF_SIZE   64

typedef struct
{

    uint8_t         rx_data[MAX_BUFF_SIZE];
    uint16_t        rx_len;    //接收到的长度
    __IO uint8_t    rx_ok;     //接收完成

} ONE_LINE_TYPE;


extern ONE_LINE_TYPE gOneLine;

uint8_t one_line_send(uint8_t * data, uint16_t len);


void send_timer_cb(void);
void rev_timer_cb(void);

void rev_exit_cb(void);
#endif


