#ifndef __PROTOCOL_IF_H__
#define __PROTOCOL_IF_H__

#include "iot_interface.h"
#include "stdio.h"

#define PROTOCOL_TYPE_MODULE    0
#define PROTOCOL_TYPE_MCU       1

#define MODULE_TYPE_WIFI    0
#define MODULE_TYPE_2G      1
#define MODULE_TYPE_4G      2

#define PRINT_INFO	printf
// #define IOT_INFO(format, ...)  printf("[IOT-INFO-%s:%d] "format, get_time_str(),system_get_free_heap_size(), ##__VA_ARGS__);
#define IOT_INFO	printf

#define PRINT_ERR	printf
// #define IOT_ERR(format, ...)  printf("[-IOT-ERR-%s] "format, get_time_str(), ##__VA_ARGS__);
#define IOT_ERR		printf

#endif

void iot_call_cb(iot_u8 opcode_value, iot_u8 result);
void iot_init_cb(iot_u8* ModuleMac, iot_u8* ModuleVersion);
