
#ifndef CONFIG_ONE_LINE_H_
#define CONFIG_ONE_LINE_H_



#include "main.h"
#include "tim.h"
#include "gpio.h"


#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"


#define TL_CNT  4   //8 //16        //设置一个（位）周期的定时器计数个数，必须为4的倍数
#define ONE_LINE_BUF_SIZE   64      //配置接收buff大小


//配置GPIO
#define ONE_LINE_PORT   GPIOB
#define ONE_LINE_PIN    GPIO_PIN_8
#define ONE_LINE_IRQ    EXTI9_5_IRQn

//写IO
#define TX_PIN_H    HAL_GPIO_WritePin(ONE_LINE_PORT, ONE_LINE_PIN, GPIO_PIN_SET)
#define TX_PIN_L    HAL_GPIO_WritePin(ONE_LINE_PORT, ONE_LINE_PIN, GPIO_PIN_RESET)

//读IO
#define RX_PIN  HAL_GPIO_ReadPin(ONE_LINE_PORT, ONE_LINE_PIN)

//以下函数需要用户根据不同的芯片自己实现
//设置IO为输出模式
void set_pin_tx(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
//设置IO为外部中断输入模式
void set_pin_rx(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif


