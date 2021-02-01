#include "mcu_deserialize.h"
#include "mcu_serialize.h"
#include "string.h"
#include "protocol_if.h"

struct iot_context iot_ctx;
char MCU_VER[MCU_VER_LEN] = {0};
char SERVER_ADDR[SERVER_ADDR_LEN_MAX] = {0};

//call cb result
iot_u32 mcu_d_call_result_cb(serial_msg_s *msg)
{
    module_send_call_result_s recv_data;

    memcpy(&recv_data, msg->arg, msg->msg_len);

    iot_call_cb(get_val_with_opcodeID(msg->opcode_id), recv_data.result);

    return PROTOCOL_RET_OK;
}

//0x01
extern iot_u8  iot_start_flag;
iot_s32 mcu_d_init(serial_msg_s *msg)
{
    module_send_init_s module_send_init;

    memcpy(&module_send_init, msg->arg, msg->msg_len);

    // hex_dump("module_send_init.mac", module_send_init.mac, sizeof(module_send_init.mac));
    // IOT_INFO("ver:%s\r\n", module_send_init.ver);
    iot_init_cb( module_send_init.mac, module_send_init.ver);

    if(iot_start_flag == 1) {
        mcu_s_init();
    }

    return PROTOCOL_RET_OK;
}

//0x02
iot_s32 mcu_d_sdk_status(serial_msg_s *msg)
{
    opcode_sdk_status_s sdk_status_data;
    iot_u8 dev_status;
    iot_u32 time;

    memcpy(&sdk_status_data, msg->arg, sizeof(sdk_status_data));

    dev_status = sdk_status_data.dev_status;
    time = STREAM_TO_UINT32_f(sdk_status_data.timeout, 0);

    iot_status_cb(dev_status, time);

    return PROTOCOL_RET_OK;
}

//0x03
iot_s32 mcu_d_get_net_time(serial_msg_s *msg)
{
    module_send_get_online_time_s recv_data;
    iot_u32 timestamp;
    s_time_t st;

    memcpy(&recv_data, msg->arg, msg->msg_len);

    timestamp = STREAM_TO_UINT32_f(recv_data.TimeStamp, 0);
    st.sec = recv_data.second;
    st.min = recv_data.minute;
    st.hour = recv_data.hour;
    st.day = recv_data.day;
    st.mon = recv_data.month;
    st.year = recv_data.year;
    st.week = recv_data.week;

    iot_parse_timestamp(timestamp, &st);
    return PROTOCOL_RET_OK;
}

//0x04
DATA_LIST_T  g_data_list = {0, NULL};
iot_s32 mcu_d_data_cb(serial_msg_s *msg)
{
    module_send_data_cb_s * recv_data = NULL;
    iot_u32 seq = 0;
    iot_u32 i = 0;

    recv_data = (module_send_data_cb_s *)malloc(msg->msg_len);
    if(recv_data == NULL) {
        IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
        return PROTOCOL_RET_ERR;
    }

    memcpy(recv_data, msg->arg, msg->msg_len);
    seq = STREAM_TO_UINT32_f(recv_data->seq, 0);
    g_data_list.count = STREAM_TO_UINT16_f(recv_data->count, 0);

    if(g_data_list.count > 0) {
        g_data_list.info = (DATA_INFO_T*)malloc( sizeof(DATA_INFO_T) * g_data_list.count );
        if(g_data_list.info == NULL) {
            IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
            goto out;
        }

        for(i = 0; i < g_data_list.count; i++)
        {
            if(strcmp(recv_data->data_list[i].dev_id, "") == 0)
                g_data_list.info[i].dev_id = NULL;
            else
                g_data_list.info[i].dev_id = recv_data->data_list[i].dev_id;

            g_data_list.info[i].info_type = recv_data->data_list[i].info_type;
        }
    }

    iot_data_cb(seq, &g_data_list);

out:
    if(g_data_list.info) {
        free(g_data_list.info);
        g_data_list.info = NULL;
    }
    free(recv_data);

    return PROTOCOL_RET_OK;
}

//0x12
iot_s32 mcu_d_download_dps(serial_msg_s *msg)
{
    iot_dp_handle((iot_u8 *)msg->arg, msg->msg_len);

    return PROTOCOL_RET_OK;
}

//0x14
iot_s32 mcu_d_rx_data(serial_msg_s *msg)
{
    module_send_rx_data_s module_send_rx_data;
    iot_u32 head_len = sizeof(module_send_rx_data);
    iot_u64 timestamp = 0;
    char sub_id[SUB_ID_LEN_MAX+1];

    memset(sub_id, 0, sizeof(sub_id));

    memcpy(&module_send_rx_data, msg->arg, head_len);

    timestamp = STREAM_TO_UINT64_f(module_send_rx_data.TimeStamp, 0);
    memcpy(sub_id, module_send_rx_data.sub_id, SUB_ID_LEN_MAX);

    if(sub_id[0] == 0)
        iot_rx_data_cb (NULL, timestamp, (iot_u8 *)msg->arg + head_len, msg->msg_len - head_len);
    else
        iot_rx_data_cb (sub_id, timestamp, (iot_u8 *)msg->arg + head_len, msg->msg_len - head_len);

    return PROTOCOL_RET_OK;
}

//0x22
iot_s32 mcu_d_ota_info(serial_msg_s *msg)
{
    module_send_ota_info_s recv_data;
    char name[OTA_INFO_NAME_LEN + 1];
    char ver[OTA_INFO_VER_LEN + 1];
    iot_u32 total_len = 0;
    iot_u16 file_crc = 0;

    memset(name, 0, sizeof(name));
    memset(ver, 0, sizeof(ver));

    memcpy(&recv_data, msg->arg, sizeof(recv_data));
    memcpy(name, recv_data.name, OTA_INFO_NAME_LEN);
    memcpy(ver, recv_data.version, OTA_INFO_VER_LEN);
    total_len = STREAM_TO_UINT32_f(recv_data.total_len, 0);
    file_crc = STREAM_TO_UINT16_f(recv_data.file_crc, 0);

    iot_ota_info_cb(name, ver, total_len, file_crc);

    return PROTOCOL_RET_OK;
}

//0x23
iot_s32 mcu_d_ota_upgrade(serial_msg_s *msg)
{
    iot_ota_upgrade_cb();

    return PROTOCOL_RET_OK;
}

//0x24
patchs_list_t * patchs = NULL;
module_send_patch_list_s * recv_data = NULL;
iot_s32 mcu_d_patch_list(serial_msg_s *msg)
{
    iot_u32 count = 0;
    iot_u32 i = 0;

    if(recv_data != NULL) {
        free(recv_data);
        recv_data = NULL;
    }
    recv_data = (module_send_patch_list_s *)malloc(msg->msg_len);
    if(recv_data == NULL) {
        IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
        return PROTOCOL_RET_ERR;
    }

    memcpy(recv_data, msg->arg, msg->msg_len);
    count = STREAM_TO_UINT32_f(recv_data->count, 0);

    if(patchs != NULL) {
        free(patchs);
        patchs = NULL;
    }
    patchs = (patchs_list_t *)malloc(count * sizeof(patchs_list_t));
    if(patchs == NULL) {
        IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
        return PROTOCOL_RET_ERR;
    }

    for(i = 0; i < count; i++) {
        patchs[i].name = recv_data->patch_number[i].name;
        patchs[i].version = recv_data->patch_number[i].ver;
        patchs[i].total_len = STREAM_TO_UINT32_f(recv_data->patch_number[i].total_len, 0);
    }

    iot_patchs_list_cb(count, patchs);

    return PROTOCOL_RET_OK;
}

//0x27
iot_s32 mcu_d_data_chunk(serial_msg_s *msg)
{
    module_send_data_chunk_s module_send_data_chunk_head;
    iot_u32 head_len = sizeof(module_send_data_chunk_head);
    iot_u32 chunk_offset = 0;
    iot_u32 chunk_size = 0;
    iot_s32 ret = 0;
    iot_u8  result = 0;

    memcpy(&module_send_data_chunk_head, msg->arg, head_len);
    chunk_offset = STREAM_TO_UINT32_f(module_send_data_chunk_head.chunk_offset, 0);
    chunk_size = STREAM_TO_UINT32_f(module_send_data_chunk_head.chunk_size, 0);

    ret = iot_chunk_cb (module_send_data_chunk_head.chunk_stat, chunk_offset, chunk_size, (iot_u8 *)msg->arg + head_len);
    if(ret != 0)
        result = 1;
    msc_s_data_chunk(module_send_data_chunk_head.chunk_stat, chunk_offset, result);

    return PROTOCOL_RET_OK;
}

//0x32
iot_s32 mcu_d_info_cb(serial_msg_s *msg)
{
    module_send_info_cb_s module_send_info_cb_head;
    iot_u32 head_len = sizeof(module_send_info_cb_head);
    iot_u8 * pData = (iot_u8 *)msg->arg;
    iot_u8 info_type = 0;
    iot_u16 data_len = 0;
    iot_u16 data_index = 0;
    iot_u8 str_len = 0;
    iot_u32 i = 0;

    user_info_t * master_info = NULL;
    share_info_t * share_info = NULL;
    dev_info_t * dev_info = NULL;

    memcpy(&module_send_info_cb_head, msg->arg, head_len);
    info_type = module_send_info_cb_head.info_type;
    data_len = STREAM_TO_UINT16_f(module_send_info_cb_head.data_len, 0);
    data_index += head_len;

    switch (info_type)
    {
    case INFO_TYPE_USER_MASTER:
        if(data_len == 0) {
            iot_info_cb(info_type, NULL);
        }
        else {
            master_info = (user_info_t *)malloc(sizeof(user_info_t));
            if(master_info == NULL) {
                IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
                goto out;
            }

            str_len = pData[data_index++];
            master_info->id = (char *)pData + data_index;
            data_index += str_len;

            str_len = pData[data_index++];
            master_info->email = (char *)pData + data_index;
            data_index += str_len;

            str_len = pData[data_index++];
            master_info->country_code = (char *)pData + data_index;
            data_index += str_len;

            str_len = pData[data_index++];
            master_info->phone = (char *)pData + data_index;
            data_index += str_len;

            str_len = pData[data_index++];
            master_info->name = (char *)pData + data_index;
            data_index += str_len;

            if(data_index != data_len) {
                IOT_ERR("%s, %d is err, data_index:%d, data_len:%d\r\n", __func__, __LINE__, data_index, data_len);
                goto out;
            }

            iot_info_cb(info_type, (void *)master_info);
        }
        break;
    case INFO_TYPE_USER_SHARE:
        if(data_len == 0) {
            iot_info_cb(info_type, NULL);
        }
        else {
            share_info = (share_info_t *)malloc(sizeof(share_info_t));
            if(share_info == NULL) {
                IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
                goto out;
            }
            share_info->count = STREAM_TO_UINT32_f(pData + data_index, 0);
            data_index += 4;
            share_info->user = (user_info_t*)malloc(share_info->count * sizeof(user_info_t));
            if(share_info->user == NULL) {
                IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
                goto out;
            }

            for(i = 0; i < share_info->count; i++) {
                str_len = pData[data_index++];
                share_info->user[i].id = (char *)pData + data_index;
                data_index += str_len;

                str_len = pData[data_index++];
                share_info->user[i].email = (char *)pData + data_index;
                data_index += str_len;

                str_len = pData[data_index++];
                share_info->user[i].country_code = (char *)pData + data_index;
                data_index += str_len;

                str_len = pData[data_index++];
                share_info->user[i].phone = (char *)pData + data_index;
                data_index += str_len;

                str_len = pData[data_index++];
                share_info->user[i].name = (char *)pData + data_index;
                data_index += str_len;
            }


            if(data_index != data_len) {
                IOT_ERR("%s, %d is err, data_index:%d, data_len:%d\r\n", __func__, __LINE__, data_index, data_len);
                goto out;
            }

            iot_info_cb(info_type, (void *)share_info);
        }
        break;
    case INFO_TYPE_DEVICE:
            dev_info = (dev_info_t *)malloc(sizeof(dev_info_t));
            if(dev_info == NULL) {
                IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
                goto out;
            }

            dev_info->is_bind = pData[data_index++];
            memcpy(dev_info->sdk_ver, pData + data_index, sizeof(dev_info->sdk_ver));
            iot_info_cb(info_type, (void *)dev_info);
        break;
    default:
        IOT_ERR("%s, %d is err\r\n", __func__, __LINE__);
        break;
    }

out:
    if(master_info != NULL)
        free(master_info);
    if(share_info != NULL) {
        if(share_info->user != NULL)
            free(share_info->user);
        free(share_info);
    }
    if(dev_info != NULL)
        free(dev_info);

    return PROTOCOL_RET_OK;
}

//0x43
iot_s32 mcu_d_sub_status_cb(serial_msg_s *msg)
{
    module_send_sub_status_cb_s * recv_data = NULL;
    iot_u32 data_seq = 0;
    iot_u8  sub_status;
    iot_u32 timestamp = 0;
    iot_u32 i = 0;

    recv_data = (module_send_sub_status_cb_s *)malloc(msg->msg_len);
    if(recv_data == NULL) {
        IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
        return PROTOCOL_RET_ERR;
    }

    memset(recv_data, 0, msg->msg_len);
    memcpy(recv_data, msg->arg, msg->msg_len);

    data_seq = STREAM_TO_UINT32_f(recv_data->data_seq, 0);
    sub_status = recv_data->sub_status;
    timestamp = STREAM_TO_UINT32_f(recv_data->timestamp, 0);
    g_data_list.count = STREAM_TO_UINT16_f(recv_data->sub_count, 0);

    if(g_data_list.count > 0) {
        g_data_list.info = (DATA_INFO_T*)malloc( sizeof(DATA_INFO_T) * g_data_list.count );
        if(g_data_list.info == NULL) {
            IOT_ERR("%s, %d is NULL\r\n", __func__, __LINE__);
            goto out;
        }

        for(i = 0; i < g_data_list.count; i++)
        {
            if(strcmp(recv_data->sub_info[i].sub_id, "") == 0)
                g_data_list.info[i].dev_id = NULL;
            else
                g_data_list.info[i].dev_id = recv_data->sub_info[i].sub_id;

            g_data_list.info[i].info_type = recv_data->sub_info[i].info_type;
        }
    }

    iot_sub_status_cb( sub_status, timestamp, &g_data_list, data_seq);

out:
    if(g_data_list.info) {
        free(g_data_list.info);
        g_data_list.info = NULL;
    }
    free(recv_data);

    return PROTOCOL_RET_OK;
}
