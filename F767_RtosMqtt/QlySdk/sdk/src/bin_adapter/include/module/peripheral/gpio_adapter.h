#ifndef __GPIO_ADAPTER_H__
#define __GPIO_ADAPTER_H__

#include "iot_interface.h"
#include "protocol_if.h"

#define KEY_PUSH  0
#define KEY_NOPUSH  1
#define LED_ON  0
#define LED_OFF  1

extern void gpio_status_led_init();
extern void gpio_status_led_output(iot_u8 Value);
extern void gpio_reset_key_init();
extern iot_u8 gpio_reset_key_input_get();

#endif
