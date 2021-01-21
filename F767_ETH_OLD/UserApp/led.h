
#ifndef _LED_H
#define _LED_H



#define LED_PORT_EN()		__HAL_RCC_GPIOE_CLK_ENABLE();
#define LED_PORT GPIOE
#define LED_PIN GPIO_PIN_5

void Led_Drv_init(void);
void Task_Led(void);
void Led_App_Init(void);



#endif /* __DRV_LED_H */

/*******************************  END OF FILE  ********************************/



