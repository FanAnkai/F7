#include "module_timer.h"
#include "protocol.h"
#include "module_if.h"

extern common_module_handle_s g_common_module_handle;

void initial_timer_fun(void* arg)
{
    ql_log_info("%s\r\n", __FUNCTION__);
    module_s_init();
}

void start_init_request(void)
{
    iot_timer_start(g_common_module_handle.Initial_timer, initial_timer_fun, (timer_arg)0, 1000, 1);
}

void stop_init_request(void)
{
    iot_timer_stop(g_common_module_handle.Initial_timer);
}

void module_status_notify(void* arg)
{
    module_s_module_stauts();
}

void start_status_notify(iot_u32 MilliSeconds)
{
    iot_timer_start(g_common_module_handle.status_handle.Status_timer, module_status_notify, (timer_arg)0, MilliSeconds, 1);
}

void stop_status_notify()
{
    iot_timer_stop(g_common_module_handle.status_handle.Status_timer);
}

void timer_init(void)
{
    g_common_module_handle.Initial_timer = iot_timer_create();
    g_common_module_handle.status_handle.Status_timer = iot_timer_create();

    start_init_request();
}