

#include "led.h"
#include "global.h"

void Led_Drv_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    LED_PORT_EN();

    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;


    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
}

void Led_App_Init(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);//µãÁÁ
}

unsigned int led_cnt = 0;
unsigned char led_flag = 1;
//50msµ÷ÓÃ
void Task_Led(void)
{
    if(gTask_Rrg.PowerUpCnt > 0)
        return;

    led_cnt++;

	if(led_cnt%10 == 0)
	{
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, led_flag & (1<<0));
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, led_flag & (1<<1));
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, led_flag & (1<<2));

		led_flag <<= 1;
		if(led_flag >= 0x10)
		{
			led_flag = 0x01;
		}


	}
}

/*******************************  END OF FILE  ********************************/
