#ifndef __UART_ADAPTER_H__
#define __UART_ADAPTER_H__

#include "iot_interface.h"
#include "protocol_if.h"

extern void uart_tx_buffer_for_cmd(const iot_u8 *buf, iot_u16 len);
extern void uart_intr_handler_for_cmd(void * param);
extern void uart_intr_handler_for_log(void * param);
extern void uart_intr_disable_for_cmd(void);
extern void uart_intr_enable_for_cmd(void);
extern void uart_init_for_cmd(iot_u32 baudrate);
extern void uart_init_for_log(iot_u32 baudrate);

#endif
