#ifndef __MODULE_TIMER_H__
#define __MODULE_TIMER_H__

#include "ql_include.h"

void start_init_request(void);
void stop_init_request(void);
void module_status_notify(void* arg);
void start_status_notify(iot_u32 MilliSeconds);
void stop_status_notify();
void timer_init(void);

#endif