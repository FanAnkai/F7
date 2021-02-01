#ifndef __MODULE_SERIALIZE_H__
#define __MODULE_SERIALIZE_H__

#include "iot_interface.h"
#include "protocol.h"

void module_s_call_result_cb(iot_u8 opcode_id, iot_u8 result);
void module_s_init(void);
void module_s_sdk_status( DEV_STATUS_T dev_status, iot_u32 timestamp );
void module_s_online_time();
void module_s_data_cb( iot_u32 data_seq, DATA_LIST_T * data_list );
void module_s_download_dps(iot_u8* pData, iot_u16 Length);
void module_s_rx_data( char sub_id[32], iot_u64 data_timestamp, iot_u8* data, iot_u32 data_len );
void module_s_ota_info( char name[16], char version[6], iot_u32 total_len, iot_u16 file_crc );
void module_s_ota_upgrade(void);
void module_s_patch_list( iot_s32 count, patchs_list_t patchs[] );
void module_s_data_chunk( iot_u8 chunk_stat, iot_u32 chunk_offset, iot_u32 chunk_size, const iot_s8* chunk );
void module_s_info_cb( INFO_TYPE_T info_type, void* info );
void module_s_sub_status_cb( SUB_STATUS_T sub_status, iot_u32 timestamp, DATA_LIST_T * list , iot_u32 seq);

iot_s32 module_send_cb(serial_msg_s *msg);

#endif