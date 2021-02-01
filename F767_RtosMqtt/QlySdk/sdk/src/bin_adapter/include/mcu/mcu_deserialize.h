#ifndef __MCU_DESERIALIZE_H__
#define __MCU_DESERIALIZE_H__

#include "iot_interface.h"
#include "protocol.h"

iot_u32 mcu_d_call_result_cb(serial_msg_s *msg);
iot_s32 mcu_d_init(serial_msg_s *msg);
iot_s32 mcu_d_sdk_status(serial_msg_s *msg);
iot_s32 mcu_d_get_net_time(serial_msg_s *msg);
iot_s32 mcu_d_data_cb(serial_msg_s *msg);
iot_s32 mcu_d_upload_dps(serial_msg_s *msg);
iot_s32 mcu_d_download_dps(serial_msg_s *msg);
iot_s32 mcu_d_tx_data(serial_msg_s *msg);
iot_s32 mcu_d_rx_data(serial_msg_s *msg);
iot_s32 mcu_d_ota_set(serial_msg_s *msg);
iot_s32 mcu_d_ota_info(serial_msg_s *msg);
iot_s32 mcu_d_ota_upgrade(serial_msg_s *msg);
iot_s32 mcu_d_patch_list(serial_msg_s *msg);
iot_s32 mcu_d_patch_req(serial_msg_s *msg);
iot_s32 mcu_d_patch_end(serial_msg_s *msg);
iot_s32 mcu_d_data_chunk(serial_msg_s *msg);
iot_s32 mcu_d_get_info(serial_msg_s *msg);
iot_s32 mcu_d_info_cb(serial_msg_s *msg);
iot_s32 mcu_d_set_config(serial_msg_s *msg);
iot_s32 mcu_d_sub_dev_act(serial_msg_s *msg);
iot_s32 mcu_d_sub_dev_inact(serial_msg_s *msg);
iot_s32 mcu_d_sub_status_cb(serial_msg_s *msg);

#endif