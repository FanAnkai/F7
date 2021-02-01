#include "spec_opcode.h"

//0xF1
iot_s32 mcu_d_module_status(serial_msg_s *msg)
{
    module_send_module_status_s  module_send_module_status;
    iot_u32 timestamp = 0;

    memcpy(&module_send_module_status, msg->arg, msg->msg_len);

    timestamp = STREAM_TO_UINT32_f(module_send_module_status.TimeStamp, 0);

    iot_module_status_cb(module_send_module_status.module_type, module_send_module_status.module_status, timestamp);

    return 0;
}

//0xF2
void mcu_s_set_mode(iot_u8 mode, iot_u8 timeout, iot_u32 extra_len, void * extra)
{
    serial_msg_s msg;
    mcu_send_set_mode_s * send_data = NULL;
    iot_u32 send_len = sizeof(mcu_send_set_mode_s) + extra_len;

    send_data = (mcu_send_set_mode_s *)malloc(send_len);

    send_data->module_type = MODULE_TYPE_WIFI;
    send_data->mode = mode;
    send_data->timeout = timeout;
    UINT32_TO_STREAM_f(send_data->extra_len, extra_len);
    if(extra_len != 0 && extra != NULL)
        memcpy(send_data->extra, extra, extra_len);

    msg.opcode_id = SERIAL_OPCODE_SET_MODE;
    msg.msg_len = send_len;
    msg.arg = send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);

    free(send_data);
}

iot_s32 mcu_d_set_mode(serial_msg_s *msg)
{
    module_send_set_mode_s * recv_data = NULL;
    iot_u32 extra_len = 0;
    module_send_factory_s module_send_factory;

    recv_data = (module_send_set_mode_s *)malloc(msg->msg_len);
    memcpy(recv_data, msg->arg, msg->msg_len);

    extra_len = STREAM_TO_UINT32_f(recv_data->extra_len, 0);
    if(extra_len == 0) {
        iot_module_set_mode_cb(recv_data->mode, recv_data->timeout, recv_data->result);
    }
    else if(recv_data->mode == RESET_TYPE_STA_FACTORYMODE) {
        memcpy(&module_send_factory, recv_data->extra, sizeof(module_send_factory));
        iot_module_set_factory_cb(module_send_factory.ret, module_send_factory.rssi);
    }

    free(recv_data);
    return 0;
}

void iot_module_set_reset(iot_u8 timeout)
{
    IOT_INFO("%s,%d\r\n", __func__, __LINE__);
    mcu_s_set_mode(RESET_TYPE_MODULE_RESET, timeout, 0, NULL);
}

void iot_module_set_smartconfig(iot_u8 timeout)
{
    IOT_INFO("%s,%d\r\n", __func__, __LINE__);
    mcu_s_set_mode(RESET_TYPE_SMARTCONFIG, timeout, 0, NULL);
}

void iot_module_set_ap(iot_u8 timeout, char * ssid, char * passwd)
{
    iot_u8 extra[MCU_SEND_SET_MODULE_EXTRA_LEN_MAX] = {0};
    iot_u32 index = 0;

    IOT_INFO("%s,%d\r\n", __func__, __LINE__);

    if(ssid == NULL) {
        IOT_ERR("%s,%d, NULL\r\n", __func__, __LINE__);
        return;
    }

    if(strlen(ssid) > AP_CONFIG_MAX || strlen(passwd) > AP_CONFIG_MAX) {
        IOT_ERR("%s,%d, string is too long\r\n", __func__, __LINE__);
        return;
    }

    extra[index++] = strlen(ssid);
    memcpy(extra + index, ssid, strlen(ssid));
    index +=  strlen(ssid);

    if(passwd == NULL) {
        extra[index++] = 0;
    }
    else {
        extra[index++] = strlen(passwd);
        if(strlen(passwd) != 0) {
            memcpy(extra + index, passwd, strlen(passwd));
            index +=  strlen(passwd);
        }
    }

    IOT_INFO("%s,%d, index:%d\r\n", __func__, __LINE__, index);

    mcu_s_set_mode(RESET_TYPE_AP_SMART, timeout, index, extra);
}

void iot_module_set_factroy(iot_u8 timeout)
{
    IOT_INFO("%s,%d\r\n", __func__, __LINE__);
    mcu_s_set_mode(RESET_TYPE_STA_FACTORYMODE, timeout, 0, NULL);
}

//0xF3
void mcu_s_module_info_cb(iot_u8 info_type)
{
    serial_msg_s msg;
    mcu_send_module_info_cb_s send_data;

    memset(&send_data, 0, sizeof(send_data));

    send_data.module_type   = MODULE_TYPE_WIFI;
    send_data.info_type     = info_type;

    msg.opcode_id = SERIAL_OPCODE_MODULE_INFO_CB;
    msg.msg_len = sizeof(send_data);
    msg.arg = &send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

iot_s32 mcu_d_module_info_cb(serial_msg_s *msg)
{
    module_send_module_info_cb_s * recv_data = NULL;
    iot_u32 info_len = 0;

    recv_data = (module_send_module_info_cb_s *)malloc(msg->msg_len);
    memcpy(recv_data, msg->arg, msg->msg_len);

    if(recv_data->module_type != MODULE_TYPE_WIFI) {
        IOT_ERR("%s, %d err!\r\n", __FUNCTION__, __LINE__);
        return PROTOCOL_RET_ERR;
    }

    info_len = STREAM_TO_UINT32_f(recv_data->info_len, 0);
    if(info_len == 0) {
        iot_module_info_cb(recv_data->info_type, info_len, NULL);
    }
    else {
        iot_module_info_cb(recv_data->info_type, info_len, recv_data->info);
    }

    free(recv_data);
    return 0;
}

void iot_module_info_get(iot_u8 info_type)
{
    mcu_s_module_info_cb(info_type);
}
