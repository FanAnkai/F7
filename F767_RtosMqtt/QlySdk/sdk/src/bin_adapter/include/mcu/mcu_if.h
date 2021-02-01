#ifndef __MCU_IF_H__
#define __MCU_IF_H__

#include "iot_interface.h"

// typedef struct common_module_handle{
//     iot_timer_t *Initial_timer;
//     struct module_spec_status_param {
//         iot_timer_t *Status_timer;
//         iot_u8 g_ModuleStatus;
//     } status_handle;
// }common_module_handle_s;

void mcu_protocol_init(void);

#endif