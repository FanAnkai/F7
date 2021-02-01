#ifndef __MODULE_IF_H__
#define __MODULE_IF_H__

#include "iot_interface.h"
#include "timer_adapter.h"

typedef struct common_module_handle{
    iot_timer_t *Initial_timer;
    struct module_spec_status_param {
        iot_timer_t *Status_timer;
        iot_u8 g_ModuleStatus;
    } status_handle;
}common_module_handle_s;

void module_protocol_init(void);

#endif