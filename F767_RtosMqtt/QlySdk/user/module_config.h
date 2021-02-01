#ifndef __MODULE_CONFIG_H__
#define __MODULE_CONFIG_H__

#include "iot_interface.h"
#include "protocol_if.h"
#include "usart.h"

#define _LOCATION_ 

#define __PROTOCOL_TYPE__                   PROTOCOL_TYPE_MCU
#define __SDK_BIN_MODULE_TYPE__             MODULE_TYPE_WIFI
#define __SDK_BIN_MODULE_DATA_MAX_LEN__     1024

iot_u8* UINT16_TO_STREAM_f(iot_u8 *p, iot_u16 u16);
iot_u8* UINT32_TO_STREAM_f(iot_u8 *p, iot_u32 u32);
iot_u8* UINT64_TO_STREAM_f(iot_u8 *p, iot_u64 u64);
iot_u16 STREAM_TO_UINT16_f(iot_u8* p, iot_u16 offset);
iot_u32 STREAM_TO_UINT32_f(iot_u8* p, iot_u16 offset);
iot_u64 STREAM_TO_UINT64_f(iot_u8* p, iot_u16 offset);
iot_f32 STREAM_TO_FLOAT32_f(iot_u8* p, iot_u16 offset);
iot_u8* FLOAT32_TO_STREAM_f(iot_u8* p, iot_f32 f32);

void hex_dump(char* name, unsigned char* buf, iot_u32 len);

#endif
