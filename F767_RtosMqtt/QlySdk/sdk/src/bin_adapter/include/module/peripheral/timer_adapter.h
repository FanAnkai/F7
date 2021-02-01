#ifndef __FOTA_ADAPTER_H__
#define __FOTA_ADAPTER_H__

typedef struct iot_timer_t iot_timer_t;
typedef void* timer_arg;
typedef void (*timer_fun)(timer_arg arg);
typedef unsigned int timer_tout;
typedef unsigned char timer_repeat;

iot_timer_t *iot_timer_create(void);
void iot_timer_start(iot_timer_t *iot_timer, timer_fun fun, timer_arg arg, timer_tout tout_ms, timer_repeat repeat);
void iot_timer_stop(iot_timer_t *iot_timer);

#endif
