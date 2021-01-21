

#include "config_one_line.h"


//以下函数需要用户根据不同的芯片自己实现

//设置IO为输出模式
void set_pin_tx(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    HAL_NVIC_DisableIRQ(ONE_LINE_IRQ);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
    
    TX_PIN_L;
}


//设置IO为外部中断输入模式
void set_pin_rx(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
    
    TX_PIN_L;
    
    HAL_NVIC_SetPriority(ONE_LINE_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(ONE_LINE_IRQ);
    
}




