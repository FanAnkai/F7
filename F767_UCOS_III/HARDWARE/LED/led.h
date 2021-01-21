#ifndef _LED_H
#define _LED_H
#include "sys.h"

//LED¶Ë¿Ú¶¨Òå
#define LED2(n)		(n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET))
#define LED2_Toggle (HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14))
#define LED1(n)		(n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET))
#define LED1_Toggle (HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7))
#define LED0(n)		(n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET))
#define LED0_Toggle (HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0))

void LED_Init(void);


void led_disp(void);
void led_ctrl_freq(void);

#endif
