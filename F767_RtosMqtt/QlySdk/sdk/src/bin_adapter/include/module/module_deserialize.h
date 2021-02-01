#ifndef __MODULE_DESERIALIZE_H__
#define __MODULE_DESERIALIZE_H__

#include "ql_include.h"
#include "protocol.h"

iot_s32 module_d_init(serial_msg_s *msg);
iot_s32 module_d_sdk_status(serial_msg_s *msg);
iot_s32 module_d_get_net_time(serial_msg_s *msg);
iot_s32 module_d_upload_dps(serial_msg_s *msg);
iot_s32 module_d_tx_data(serial_msg_s *msg);
iot_s32 module_d_ota_set(serial_msg_s *msg);
iot_s32 module_d_patch_req(serial_msg_s *msg);
iot_s32 module_d_patch_end(serial_msg_s *msg);
iot_s32 module_d_data_chunk(serial_msg_s *msg);
iot_s32 module_d_get_info(serial_msg_s *msg);
iot_s32 module_d_set_config(serial_msg_s *msg);
iot_s32 module_d_sub_dev_act(serial_msg_s *msg);
iot_s32 module_d_sub_dev_inact(serial_msg_s *msg);

#endif