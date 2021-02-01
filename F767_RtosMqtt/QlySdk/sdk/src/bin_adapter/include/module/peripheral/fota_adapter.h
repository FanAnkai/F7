#ifndef __FOTA_ADAPTER_H__
#define __FOTA_ADAPTER_H__

#include "iot_interface.h"

iot_s32 fota_chunk_recv(iot_u8 chunk_is_last, iot_u32 chunk_offset, iot_u32 chunk_size, const iot_s8* chunk);
iot_s32 fota_upgrade();

#endif
