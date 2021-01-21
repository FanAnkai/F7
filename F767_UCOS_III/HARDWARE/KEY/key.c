#include "key.h"
#include "delay.h"
#include "delay.h"
#include "led.h"

//支持F767
//按键处理函数
void user_key_init(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOC_CLK_ENABLE();           //开启GPIOC时钟

    GPIO_Initure.Pin = GPIO_PIN_13;          //PA0
    GPIO_Initure.Mode = GPIO_MODE_INPUT;    //输入
    GPIO_Initure.Pull = GPIO_PULLDOWN;      //下拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;   //高速
    HAL_GPIO_Init(GPIOC, &GPIO_Initure);

}


void sys_into_stop(void)
{
    HAL_Delay(100);
    GPIO_InitTypeDef GPIO_InitStruct;
    
    __HAL_RCC_PWR_CLK_ENABLE();
//    HAL_PWREx_EnableUltraLowPower();
//    HAL_PWREx_EnableFastWakeUp();
//    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
    
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    __HAL_RCC_GPIOC_CLK_ENABLE();
//    __HAL_RCC_GPIOD_CLK_ENABLE();	
//    GPIO_InitStructure.Pin = GPIO_PIN_All;
//    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
//    GPIO_InitStructure.Pull = GPIO_NOPULL;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
//    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
//    HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
//    __HAL_RCC_GPIOA_CLK_DISABLE();
//    __HAL_RCC_GPIOB_CLK_DISABLE();
//    __HAL_RCC_GPIOC_CLK_DISABLE();
//    __HAL_RCC_GPIOD_CLK_DISABLE();
    

   
    
    __HAL_RCC_GPIOC_CLK_ENABLE();           //开启GPIOC时钟

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn,3,0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
   
    //进入stop
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_UNDERDRIVE_ON, PWR_STOPENTRY_WFI|PWR_STOPENTRY_WFE);


}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    user_key_init();
}

//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;


uint8_t user_key_scan(uint8_t mode)
{
    static uint8_t key_up = 1;  //按键松开标志
    if(mode == 1)
    {
        key_up = 1;    //支持连按
    }

    if(key_up && (READ_WKUP_PIN == 1))
    {
        delay_ms(10);
        key_up = 0;
        return KEY_WKUP;
    }
    else if(READ_WKUP_PIN == 0)
    {
        key_up = 1;
    }
    return 0;   //无按键按下
}



