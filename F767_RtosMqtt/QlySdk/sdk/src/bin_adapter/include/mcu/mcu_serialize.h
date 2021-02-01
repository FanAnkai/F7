#ifndef __MCU_SERIALIZE_H__
#define __MCU_SERIALIZE_H__

#include "iot_interface.h"
#include "protocol.h"

void mcu_s_init(void);
void mcu_s_sdk_status( DEV_STATUS_T dev_status, iot_u32 timestamp );
void mcu_s_online_time();
void mcu_s_data_cb( iot_u32 data_seq, DATA_LIST_T * data_list );
void mcu_s_upload_dps(iot_u8 * data, iot_u16 data_len);
void mcu_s_download_dps(iot_u8* pData, iot_u16 Length);
void mcu_s_tx_data(iot_u8 * data, iot_u16 data_len);
void mcu_s_rx_data( char sub_id[32], iot_u64 data_timestamp, iot_u8* data, iot_u32 data_len );
void mcu_s_ota_set(iot_u32 expect_time, iot_u32 chunk_size);
void mcu_s_ota_info( char name[16], char version[6], iot_u32 total_len, iot_u16 file_crc );
void mcu_s_ota_upgrade(void);
void mcu_s_patch_list( iot_s32 count, patchs_list_t patchs[] );
iot_s32 mcu_s_patch_req(const char name[16], const char version[16]);
iot_s32 mcu_s_patch_end(const char name[16], const char version[16], DEV_STATUS_T patch_state);
void msc_s_data_chunk(iot_u8 chunk_stat, iot_u32 chunk_offset, iot_u8 result);
void mcu_s_get_info( INFO_TYPE_T info_type );
void mcu_s_info_cb( INFO_TYPE_T info_type, void* info );
iot_s32 mcu_s_set_config(INFO_TYPE_T info_type, void* info);
void mcu_s_sub_dev_act(iot_u8 * data, iot_u16 data_len);
iot_s32 mcu_s_sub_dev_inact(const char sub_id[32], SUB_OPT_TYPE_T opt_type, iot_u32* data_seq);
void mcu_s_sub_status_cb( SUB_STATUS_T sub_status, iot_u32 timestamp, DATA_LIST_T * list , iot_u32 seq);

iot_s32 mcu_send_cb(serial_msg_s *msg);

#endif