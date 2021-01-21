
#ifndef BSP_ONE_LINE_H_
#define BSP_ONE_LINE_H_


#include "main.h"


#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"


#define MAX_BUFF_SIZE   64

typedef struct
{
    
    uint8_t  rx_data[MAX_BUFF_SIZE];
    uint16_t rx_len;      //要发送到长度
    uint8_t  rx_ok;
    
}ONE_LINE_TYPE;



uint8_t one_line_send(uint8_t * data, uint16_t len);


void send_timer_cb(void);
void rev_timer_cb(void);

void rev_exit_cb(void);
#endif


