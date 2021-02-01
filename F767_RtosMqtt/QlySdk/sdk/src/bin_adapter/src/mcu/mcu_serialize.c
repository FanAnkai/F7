#include "mcu_serialize.h"
#include "protocol_if.h"

//0x01
struct iot_context *g_ctx;
iot_u8  iot_start_flag = 0;
iot_s32 iot_start( struct iot_context* ctx )
{
    IOT_INFO("%s\r\n", __FUNCTION__);
    g_ctx = ctx;
    iot_start_flag = 1;
}

void mcu_s_init(void)
{
    IOT_INFO("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    mcu_send_init_s * send_data;
    iot_u32 send_len = 0;

    send_len = sizeof(mcu_send_init_s) + strlen(g_ctx->server_addr);
    IOT_INFO("send_len:%d\r\n", send_len);
    send_data = (mcu_send_init_s *)malloc(send_len);
    if(send_data == NULL) {
        IOT_ERR("%s is NULL\r\n", __FUNCTION__);
        return;
    }

    UINT32_TO_STREAM_f(send_data->product_id, g_ctx->product_id);
    memcpy(send_data->product_key, g_ctx->product_key, sizeof(send_data->product_key));
    memcpy(send_data->mcu_ver, g_ctx->mcu_version, sizeof(send_data->mcu_ver));
    UINT32_TO_STREAM_f(send_data->recvbuf_size, g_ctx->recvbuf_size);
    UINT32_TO_STREAM_f(send_data->sendbuf_size, g_ctx->sendbuf_size);
    UINT16_TO_STREAM_f(send_data->server_port, g_ctx->server_port);
    send_data->addr_len = strlen(g_ctx->server_addr);
    memcpy(send_data->server_addr,g_ctx->server_addr, strlen(g_ctx->server_addr));
    
    msg.opcode_id = SERIAL_OPCODE_INIT;
    msg.msg_len = send_len;
    msg.arg = send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);

    free(send_data);
}

//0x02
void mcu_s_sdk_status( DEV_STATUS_T dev_status, iot_u32 time )
{
    IOT_INFO("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    opcode_sdk_status_s sdk_status_data;

    sdk_status_data.dev_status = dev_status;
    UINT32_TO_STREAM_f(sdk_status_data.timeout, time);

    msg.opcode_id = SERIAL_OPCODE_SDK_STATUS;
    msg.msg_len = sizeof(sdk_status_data);
    msg.arg = &sdk_status_data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x03
void mcu_s_online_time()
{
    IOT_INFO("%s\r\n", __FUNCTION__);
    serial_msg_s msg;

    msg.opcode_id = SERIAL_OPCODE_GET_ONLINE_TIME;
    msg.msg_len = 0;
    msg.arg = NULL;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0X11
void mcu_s_upload_dps(iot_u8 * data, iot_u16 data_len)
{
    IOT_INFO("%s\r\n", __FUNCTION__);
    serial_msg_s msg;

    msg.opcode_id = SERIAL_OPCODE_UPLOADE_DPS;
    msg.msg_len = data_len;
    msg.arg = data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x13
void mcu_s_tx_data(iot_u8 * data, iot_u16 data_len)
{
    IOT_INFO("%s,\r\n", __FUNCTION__);
    serial_msg_s msg;

    msg.opcode_id = SERIAL_OPCODE_TX_DATA;
    msg.msg_len = data_len;
    msg.arg = data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x21
void mcu_s_ota_set(iot_u32 expect_time, iot_u32 chunk_size)
{
    IOT_INFO("%s,\r\n", __FUNCTION__);
    serial_msg_s msg;
    mcu_send_ota_set_s send_data;
    iot_u32 send_len = sizeof(send_data);

    UINT32_TO_STREAM_f(send_data.expect_time, expect_time);
    UINT32_TO_STREAM_f(send_data.chunk_size, chunk_size);

    msg.opcode_id = SERIAL_OPCODE_OTA_SET;
    msg.msg_len = send_len;
    msg.arg = &send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x25
iot_s32 mcu_s_patch_req(const char name[16], const char version[16])
{
    IOT_INFO("%s,\r\n", __FUNCTION__);
    serial_msg_s msg;
    mcu_send_patch_req_s send_data;
    iot_u32 send_len = sizeof(send_data);

    if(name == NULL || version == NULL || strlen(name) > 16 || strlen(version) > 16) {
        IOT_ERR("%s,%d\r\n", __FUNCTION__, __LINE__);
        return -1;
    }

    memset(&send_data, 0, send_len);
    memcpy(send_data.name, name, strlen(name));
    memcpy(send_data.ver, version, strlen(version));

    msg.opcode_id = SERIAL_OPCODE_PATCH_REQ;
    msg.msg_len = send_len;
    msg.arg = &send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);
    return 0;
}

//0x26
iot_s32 mcu_s_patch_end(const char name[16], const char version[16], DEV_STATUS_T patch_state)
{
    IOT_INFO("%s,\r\n", __FUNCTION__);
    serial_msg_s msg;
    mcu_send_patch_end_s send_data;
    iot_u32 send_len = sizeof(send_data);

    if(name == NULL || version == NULL || strlen(name) > 16 || strlen(version) > 16) {
        IOT_ERR("%s,%d\r\n", __FUNCTION__, __LINE__);
        return -1;
    }

    memset(&send_data, 0, send_len);

    memcpy(send_data.name, name, strlen(name));
    memcpy(send_data.ver, version, strlen(version));
    send_data.patch_state = patch_state;

    msg.opcode_id = SERIAL_OPCODE_PATCH_END;
    msg.msg_len = send_len;
    msg.arg = &send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);
    return 0;
}

//0x27
void msc_s_data_chunk(iot_u8 chunk_stat, iot_u32 chunk_offset, iot_u8 result)
{
    IOT_INFO("%s,\r\n", __FUNCTION__);
    serial_msg_s msg;
    mcu_send_data_chunk_s send_data;
    iot_u32 send_len = sizeof(send_data);

    send_data.chunk_stat = chunk_stat;
    send_data.result = result;
    UINT32_TO_STREAM_f(send_data.offset, chunk_offset);

    msg.opcode_id = SERIAL_OPCODE_DATA_CHUNK;
    msg.msg_len = send_len;
    msg.arg = &send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x31
void mcu_s_get_info( INFO_TYPE_T info_type )
{
    IOT_INFO("%s,\r\n", __FUNCTION__);
    serial_msg_s msg;
    mcu_send_get_info_s send_data;
    iot_u32 send_len = sizeof(send_data);

    send_data.info_type = info_type;

    msg.opcode_id = SERIAL_OPCODE_GET_INFO;
    msg.msg_len = send_len;
    msg.arg = &send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x33
iot_s32 mcu_s_set_config(INFO_TYPE_T info_type, void* info)
{
    IOT_INFO("%s,\r\n", __FUNCTION__);
    serial_msg_s msg;
    mcu_send_set_config_s * send_data = NULL;
    iot_u32 send_len = 0;
    iot_u32 info_len = strlen((char *)info);

    if(info_len > OPCODE_SET_CONFIG_LEN_MAX) {
        IOT_ERR("%s,%d\r\n", __FUNCTION__, __LINE__);
        return -1;
    }

    send_len = sizeof(send_data) + info_len;
    send_data = (mcu_send_set_config_s *)malloc(send_len);
    if(send_data == NULL) {
        IOT_ERR("%s is NULL\r\n", __FUNCTION__);
        return -1;
    }

    send_data->info_type = info_type;
    send_data->info_len = info_len;
    memcpy(send_data->info, info, info_len);

    msg.opcode_id = SERIAL_OPCODE_SET_CONFIG;
    msg.msg_len = send_len;
    msg.arg = send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);

    free(send_data);

    return 0;
}

//0X41
void mcu_s_sub_dev_act(iot_u8 * data, iot_u16 data_len)
{
    IOT_INFO("%s\r\n", __FUNCTION__);
    serial_msg_s msg;

    msg.opcode_id = SERIAL_OPCODE_SUB_DEV_ACT;
    msg.msg_len = data_len;
    msg.arg = data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0X42
iot_s32 mcu_s_sub_dev_inact(const char sub_id[32], SUB_OPT_TYPE_T opt_type, iot_u32* data_seq)
{
    IOT_INFO("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    mcu_send_sub_dev_inact_s send_data;
    iot_u32 send_len = sizeof(send_data);
    iot_u32 Seq = get_cur_seq_num();

    *data_seq = Seq;

    if(sub_id == NULL || strlen(sub_id) > SUB_ID_LEN_MAX) {
        IOT_ERR("%s,%d\r\n", __func__, __LINE__);
        return -1;
    }

    memset(&send_data, 0, send_len);

    memcpy(&send_data, sub_id, strlen(sub_id));
    send_data.opt_type = opt_type;
    UINT32_TO_STREAM_f(send_data.data_seq, Seq);

    msg.opcode_id = SERIAL_OPCODE_SUB_DEV_INACT;
    msg.msg_len = send_len;
    msg.arg = &send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);

    return 0;
}

iot_s32 mcu_send_cb(serial_msg_s *msg)
{
    tx_device_frame(FRAME_VER, get_val_with_opcodeID(msg->opcode_id), msg->msg_len, (iot_u8 *)msg->arg);
}

