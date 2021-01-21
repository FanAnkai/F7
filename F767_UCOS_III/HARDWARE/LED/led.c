
#include "led.h"

#include "includes.h"

void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();
	
    GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_14|GPIO_PIN_7;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
    GPIO_Initure.Pull=GPIO_PULLUP;          
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
}



#define LED_CNT_UP      1
#define LED_CNT_DOWN    0

uint32_t led_cnt = 300;
uint8_t led_flag = LED_CNT_DOWN;

void led_disp(void)
{
    OS_ERR err;
    
    LED0_Toggle;
    OSTimeDlyHMSM(0,0,0,led_cnt,OS_OPT_TIME_PERIODIC,&err);
    LED1_Toggle;
    OSTimeDlyHMSM(0,0,0,led_cnt,OS_OPT_TIME_PERIODIC,&err);
    LED2_Toggle;
    OSTimeDlyHMSM(0,0,0,led_cnt,OS_OPT_TIME_PERIODIC,&err);
}


void led_ctrl_freq(void)
{
    if(led_flag == LED_CNT_UP)
    {
        if(led_cnt<100)
        {
            led_cnt+=1;
        }
        else
        {
            led_cnt+=2;
        }
        
        if(led_cnt > 300)
        {
            led_flag = !led_flag;
        }
    }
    else
    {
        if(led_cnt<100)
        {
            led_cnt-=1;
        }
        else
        {
            led_cnt-=2;
        }
        
        if(led_cnt < 30)
        {
            led_flag = !led_flag;
        }
    }
}

