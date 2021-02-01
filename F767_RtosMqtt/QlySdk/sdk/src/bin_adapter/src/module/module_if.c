#include "module_if.h"
#include "protocol.h"
#include "module_deserialize.h"
#include "module_serialize.h"
#include "module_timer.h"
#include "spec_opcode.h"

serial_protocol_s module_protocol_define[SERIAL_OPCODE_COUNT];
common_module_handle_s g_common_module_handle;

//call result
opcode_define_s opcode_send_call_result = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_call_result_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_send_cb,
};

//0xF1
opcode_define_s opcode_send_status = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_module_status_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_send_cb,
};

//0xF2
opcode_define_s opcode_recv_set_mode = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_set_mode_s),
    .len_max    = sizeof(mcu_send_set_mode_s) + MCU_SEND_SET_MODULE_EXTRA_LEN_MAX,
    .function   = module_d_set_mode,
};

opcode_define_s opcode_send_set_mode = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_set_mode_s),
    .len_max    = sizeof(module_send_set_mode_s) + MODULE_SEND_SET_MODULE_EXTRA_LEN_MAX,
    .function   = module_send_cb,
};

//0xF3
opcode_define_s opcode_recv_module_info_cb = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_module_info_cb_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_d_module_info_cb,
};

opcode_define_s opcode_send_module_info_cb = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = MODULE_S_MODULE_INFO_CB_MIN,
    .len_max    = MODULE_S_MODULE_INFO_CB_MAX,
    .function   = module_send_cb,
};

//0x01
opcode_define_s opcode_recv_init = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_init_s) + 1,
    .len_max    = sizeof(mcu_send_init_s) + SERVER_ADDR_LEN_MAX,
    .function   = module_d_init,
};

opcode_define_s opcode_send_init = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_init_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_send_cb,
};

//0x02
opcode_define_s opcode_recv_sdk_status = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(opcode_sdk_status_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_d_sdk_status,
};

opcode_define_s opcode_send_sdk_status = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(opcode_sdk_status_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_send_cb,
};

//0x03
opcode_define_s opcode_recv_get_online_time = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = 0,
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_d_get_net_time,
};

opcode_define_s opcode_send_get_online_time = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_get_online_time_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_send_cb,
};

//0x04
opcode_define_s opcode_send_data_cb = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_data_cb_s),
    .len_max    = FRAME_DATA_LMT,
    .function   = module_send_cb,
};

//0x11
opcode_define_s opcode_recv_upload_dps = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_upload_dps_s) + 5,
    .len_max    = FRAME_DATA_LMT,
    .function   = module_d_upload_dps,
};

//0X12
opcode_define_s opcode_send_download_dps = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_download_dps_s) + 5,
    .len_max    = FRAME_DATA_LMT,
    .function   = module_send_cb,
};

//0X13
opcode_define_s opcode_recv_tx_data = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_tx_data_s) + 1,
    .len_max    = FRAME_DATA_LMT,
    .function   = module_d_tx_data,
};

//0X14
opcode_define_s opcode_send_rx_data = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_rx_data_s) + 1,
    .len_max    = FRAME_DATA_LMT,
    .function   = module_send_cb,
};

//0X21
opcode_define_s opcode_recv_ota_set = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_ota_set_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_d_ota_set,
};

//0X22
opcode_define_s opcode_send_ota_info = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_ota_info_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_send_cb,
};

//0X23
opcode_define_s opcode_send_ota_upgrade = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = 0,
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_send_cb,
};

//0X24
opcode_define_s opcode_send_patch_list = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_patch_list_s) + PATCH_NUMBER_LEN,
    .len_max    = FRAME_DATA_LMT,
    .function   = module_send_cb,
};

//0X25
opcode_define_s opcode_recv_patch_req = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_patch_req_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_d_patch_req,
};

//0X26
opcode_define_s opcode_recv_patch_end = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_patch_end_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_d_patch_end,
};

//0x27
opcode_define_s opcode_recv_data_chunk = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_data_chunk_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_d_data_chunk,
};

opcode_define_s opcode_send_data_chunk = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_data_chunk_s) + 1,
    .len_max    = FRAME_DATA_LMT,
    .function   = module_send_cb,
};

//0X31
opcode_define_s opcode_recv_get_info = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_get_info_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_d_get_info,
};

//0x32
opcode_define_s opcode_send_info_cb = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_info_cb_s),
    .len_max    = FRAME_DATA_LMT,
    .function   = module_send_cb,
};

//0X33
opcode_define_s opcode_recv_set_config = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_set_config_s) + 1,
    .len_max    = sizeof(mcu_send_set_config_s) + OPCODE_SET_CONFIG_LEN_MAX,
    .function   = module_d_set_config,
};

//0X41
opcode_define_s opcode_recv_sub_dev_act = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_sub_dev_act_s) + sizeof(mcu_send_sub_dev_add),
    .len_max    = FRAME_DATA_LMT,
    .function   = module_d_sub_dev_act,
};

//0X42
opcode_define_s opcode_recv_sub_dev_inact = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_sub_dev_inact_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = module_d_sub_dev_inact,
};

//0X43
opcode_define_s opcode_send_sub_status_cb = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_sub_status_cb_s),
    .len_max    = FRAME_DATA_LMT,
    .function   = module_send_cb,
};

void module_opcode_set(serial_opcode_e opcode_id, iot_u8 opcode_val, opcode_define_s *recv_define, opcode_define_s *sedn_define)
{
    module_protocol_define[opcode_id].opcode_id = opcode_id;
    module_protocol_define[opcode_id].opcode_val = opcode_val;
    module_protocol_define[opcode_id].recv = recv_define;
    module_protocol_define[opcode_id].send = sedn_define;
}

void peripheral_init(void)
{
    gpio_reset_key_init();
    gpio_status_led_init();
    timer_init();
}

void module_protocol_init(void)
{
    ql_log_info("%s\r\n", __FUNCTION__);
    ql_memset(module_protocol_define, 0, sizeof(module_protocol_define));
    ql_memset(&g_common_module_handle, 0, sizeof(g_common_module_handle));

    module_opcode_set(SERIAL_OPCODE_MODULE_STATUS,      0xF1, NULL,                         &opcode_send_status);
    module_opcode_set(SERIAL_OPCODE_SET_MODE,           0xF2, &opcode_recv_set_mode,        &opcode_send_set_mode);
    module_opcode_set(SERIAL_OPCODE_MODULE_INFO_CB,     0xF3, &opcode_recv_module_info_cb,  &opcode_send_module_info_cb);
    module_opcode_set(SERIAL_OPCODE_INIT,               0x01, &opcode_recv_init,            &opcode_send_init);
    module_opcode_set(SERIAL_OPCODE_SDK_STATUS,         0x02, &opcode_recv_sdk_status,      &opcode_send_sdk_status);
    module_opcode_set(SERIAL_OPCODE_GET_ONLINE_TIME,    0x03, &opcode_recv_get_online_time, &opcode_send_get_online_time);
    module_opcode_set(SERIAL_OPCODE_DATA_CB,            0x04, NULL,                         &opcode_send_data_cb);
    module_opcode_set(SERIAL_OPCODE_UPLOADE_DPS,        0x11, &opcode_recv_upload_dps,      &opcode_send_call_result);
    module_opcode_set(SERIAL_OPCODE_DOWNLOAD_DPS,       0x12, NULL,                         &opcode_send_download_dps);
    module_opcode_set(SERIAL_OPCODE_TX_DATA,            0x13, &opcode_recv_tx_data,         &opcode_send_call_result);
    module_opcode_set(SERIAL_OPCODE_RX_DATA,            0x14, NULL,                         &opcode_send_rx_data);
    module_opcode_set(SERIAL_OPCODE_OTA_SET,            0x21, &opcode_recv_ota_set,         &opcode_send_call_result);
    module_opcode_set(SERIAL_OPCODE_OTA_INFO,           0x22, NULL,                         &opcode_send_ota_info);
    module_opcode_set(SERIAL_OPCODE_OTA_UPGRADE,        0x23, NULL,                         &opcode_send_ota_upgrade);
    module_opcode_set(SERIAL_OPCODE_PATCH_LIST,         0x24, NULL,                         &opcode_send_patch_list);
    module_opcode_set(SERIAL_OPCODE_PATCH_REQ,          0x25, &opcode_recv_patch_req,       &opcode_send_call_result);
    module_opcode_set(SERIAL_OPCODE_PATCH_END,          0x26, &opcode_recv_patch_end,       &opcode_send_call_result);
    module_opcode_set(SERIAL_OPCODE_DATA_CHUNK,         0x27, &opcode_recv_data_chunk,      &opcode_send_data_chunk);
    module_opcode_set(SERIAL_OPCODE_GET_INFO,           0x31, &opcode_recv_get_info,        &opcode_send_call_result);
    module_opcode_set(SERIAL_OPCODE_INFO_CB,            0x32, NULL,                         &opcode_send_info_cb);
    module_opcode_set(SERIAL_OPCODE_SET_CONFIG,         0x33, &opcode_recv_set_config,      &opcode_send_call_result);
    module_opcode_set(SERIAL_OPCODE_SUB_DEV_ACT,        0x41, &opcode_recv_sub_dev_act,     &opcode_send_call_result);
    module_opcode_set(SERIAL_OPCODE_SUB_DEV_INACT,      0x42, &opcode_recv_sub_dev_inact,   &opcode_send_call_result);
    module_opcode_set(SERIAL_OPCODE_SUB_STATUS_CB,      0x43, NULL,                         &opcode_send_sub_status_cb);

    protocol_define_set(module_protocol_define);

    protocol_init();

    peripheral_init();

    module_config_init();

    ql_set_module_version(BIN_SDK_VERSION);
}

void module_tx_data_downlink(iot_u8* pData, iot_u16 Length)
{
    PRINT_INFO("TX data down\r\n");

    module_s_download_dps(pData, Length);
}
