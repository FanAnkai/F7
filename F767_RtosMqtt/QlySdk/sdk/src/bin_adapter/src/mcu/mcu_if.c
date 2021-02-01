#include "mcu_if.h"
#include "mcu_deserialize.h"
#include "mcu_serialize.h"
#include "spec_opcode.h"
#include "stdio.h"
#include "protocol_if.h"
#include "protocol.h"

serial_protocol_s mcu_protocol_define[SERIAL_OPCODE_COUNT];

//call result
opcode_define_s opcode_recv_call_result = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_call_result_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_d_call_result_cb,
};

//0xF1
opcode_define_s opcode_recv_status = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_module_status_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_d_module_status,
};

//0xF2
opcode_define_s opcode_recv_set_mode = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_set_mode_s),
    .len_max    = sizeof(mcu_send_set_mode_s) + MODULE_SEND_SET_MODULE_EXTRA_LEN_MAX,
    .function   = mcu_d_set_mode,
};

opcode_define_s opcode_send_set_mode = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_set_mode_s),
    .len_max    = sizeof(mcu_send_set_mode_s) + MCU_SEND_SET_MODULE_EXTRA_LEN_MAX,
    .function   = mcu_send_cb,
};

//0xF3
opcode_define_s opcode_recv_module_info_cb = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = MODULE_S_MODULE_INFO_CB_MIN,
    .len_max    = MODULE_S_MODULE_INFO_CB_MAX,
    .function   = mcu_d_module_info_cb,
};

opcode_define_s opcode_send_module_info_cb = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_module_info_cb_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_send_cb,
};

//0x01
opcode_define_s opcode_recv_init = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_init_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_d_init,
};

opcode_define_s opcode_send_init = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_init_s) + 1,
    .len_max    = sizeof(mcu_send_init_s) + SERVER_ADDR_LEN_MAX,
    .function   = mcu_send_cb,
};

//0x02
opcode_define_s opcode_recv_sdk_status = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(opcode_sdk_status_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_d_sdk_status,
};

opcode_define_s opcode_send_sdk_status = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(opcode_sdk_status_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_send_cb,
};

//0x03
opcode_define_s opcode_recv_get_online_time = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_get_online_time_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_d_get_net_time,
};

opcode_define_s opcode_send_get_online_time = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = 0,
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_send_cb,
};

//0x04
opcode_define_s opcode_recv_data_cb = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_data_cb_s),
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_d_data_cb,
};

//0x11
opcode_define_s opcode_send_upload_dps = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_upload_dps_s) + 5,
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_send_cb,
};

//0X12
opcode_define_s opcode_recv_download_dps = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_download_dps_s) + 5,
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_d_download_dps,
};

//0X13
opcode_define_s opcode_send_tx_data = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_tx_data_s) + 1,
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_send_cb,
};

//0X14
opcode_define_s opcode_recv_rx_data = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_rx_data_s) + 1,
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_d_rx_data,
};

//0X21
opcode_define_s opcode_send_ota_set = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_ota_set_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_send_cb,
};

//0X22
opcode_define_s opcode_recv_ota_info = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(module_send_ota_info_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_d_ota_info,
};

//0X23
opcode_define_s opcode_recv_ota_upgrade = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = 0,
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_d_ota_upgrade,
};

//0X24
opcode_define_s opcode_recv_patch_list = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_patch_list_s) + PATCH_NUMBER_LEN,
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_d_patch_list,
};

//0X25
opcode_define_s opcode_send_patch_req = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_patch_req_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_send_cb,
};

//0X26
opcode_define_s opcode_send_patch_end = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_patch_end_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_send_cb,
};

//0x27
opcode_define_s opcode_recv_data_chunk = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_data_chunk_s) + 1,
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_d_data_chunk,
};

opcode_define_s opcode_send_data_chunk = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_data_chunk_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_send_cb,
};

//0X31
opcode_define_s opcode_send_get_info = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_get_info_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_send_cb,
};

//0x32
opcode_define_s opcode_recv_info_cb = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_info_cb_s),
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_d_info_cb,
};

//0X33
opcode_define_s opcode_send_set_config = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_set_config_s) + 1,
    .len_max    = sizeof(mcu_send_set_config_s) + OPCODE_SET_CONFIG_LEN_MAX,
    .function   = mcu_send_cb,
};

//0X41
opcode_define_s opcode_send_sub_dev_act = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(mcu_send_sub_dev_act_s) + sizeof(mcu_send_sub_dev_add),
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_send_cb,
};

//0X42
opcode_define_s opcode_send_sub_dev_inact = {
    .len_type   = FRAME_LEN_FIX,
    .len_val    = sizeof(mcu_send_sub_dev_inact_s),
    .len_min    = 0,
    .len_max    = 0,
    .function   = mcu_send_cb,
};

//0X43
opcode_define_s opcode_recv_sub_status_cb = {
    .len_type   = FRAME_LEN_FLEXIBLE,
    .len_val    = 0,
    .len_min    = sizeof(module_send_sub_status_cb_s),
    .len_max    = FRAME_DATA_LMT,
    .function   = mcu_d_sub_status_cb,
};

void mcu_opcode_set(serial_opcode_e opcode_id, iot_u8 opcode_val, opcode_define_s *recv_define, opcode_define_s *sedn_define)
{
    mcu_protocol_define[opcode_id].opcode_id = opcode_id;
    mcu_protocol_define[opcode_id].opcode_val = opcode_val;
    mcu_protocol_define[opcode_id].recv = recv_define;
    mcu_protocol_define[opcode_id].send = sedn_define;
}

void peripheral_init(void)
{
    // gpio_reset_key_init();
    // gpio_status_led_init();
    // timer_init();
}

void mcu_protocol_init(void)
{
    IOT_INFO("%s\r\n", __FUNCTION__);
    memset(mcu_protocol_define, 0, sizeof(mcu_protocol_define));

    mcu_opcode_set(SERIAL_OPCODE_MODULE_STATUS,         0xF1, &opcode_recv_status,          NULL);
    mcu_opcode_set(SERIAL_OPCODE_SET_MODE,              0xF2, &opcode_recv_set_mode,        &opcode_send_set_mode);
    mcu_opcode_set(SERIAL_OPCODE_MODULE_INFO_CB,        0xF3, &opcode_recv_module_info_cb,  &opcode_send_module_info_cb);
    mcu_opcode_set(SERIAL_OPCODE_INIT,                  0x01, &opcode_recv_init,            &opcode_send_init);
    mcu_opcode_set(SERIAL_OPCODE_SDK_STATUS,            0x02, &opcode_recv_sdk_status,      &opcode_send_sdk_status);
    mcu_opcode_set(SERIAL_OPCODE_GET_ONLINE_TIME,       0x03, &opcode_recv_get_online_time, &opcode_send_get_online_time);
    mcu_opcode_set(SERIAL_OPCODE_DATA_CB,               0x04, &opcode_recv_data_cb,         NULL);
    mcu_opcode_set(SERIAL_OPCODE_UPLOADE_DPS,           0x11, &opcode_recv_call_result,     &opcode_send_upload_dps);
    mcu_opcode_set(SERIAL_OPCODE_DOWNLOAD_DPS,          0x12, &opcode_recv_download_dps,    NULL);
    mcu_opcode_set(SERIAL_OPCODE_TX_DATA,               0x13, &opcode_recv_call_result,     &opcode_send_tx_data);
    mcu_opcode_set(SERIAL_OPCODE_RX_DATA,               0x14, &opcode_recv_rx_data,         NULL);
    mcu_opcode_set(SERIAL_OPCODE_OTA_SET,               0x21, &opcode_recv_call_result,     &opcode_send_ota_set);
    mcu_opcode_set(SERIAL_OPCODE_OTA_INFO,              0x22, &opcode_recv_ota_info,        NULL);
    mcu_opcode_set(SERIAL_OPCODE_OTA_UPGRADE,           0x23, &opcode_recv_ota_upgrade,     NULL);
    mcu_opcode_set(SERIAL_OPCODE_PATCH_LIST,            0x24, &opcode_recv_patch_list,      NULL);
    mcu_opcode_set(SERIAL_OPCODE_PATCH_REQ,             0x25, &opcode_recv_call_result,     &opcode_send_patch_req);
    mcu_opcode_set(SERIAL_OPCODE_PATCH_END,             0x26, &opcode_recv_call_result,     &opcode_send_patch_end);
    mcu_opcode_set(SERIAL_OPCODE_DATA_CHUNK,            0x27, &opcode_recv_data_chunk,      &opcode_send_data_chunk);
    mcu_opcode_set(SERIAL_OPCODE_GET_INFO,              0x31, &opcode_recv_call_result,     &opcode_send_get_info);
    mcu_opcode_set(SERIAL_OPCODE_INFO_CB,               0x32, &opcode_recv_info_cb,         NULL);
    mcu_opcode_set(SERIAL_OPCODE_SET_CONFIG,            0x33, &opcode_recv_call_result,     &opcode_send_set_config);
    mcu_opcode_set(SERIAL_OPCODE_SUB_DEV_ACT,           0x41, &opcode_recv_call_result,     &opcode_send_sub_dev_act);
    mcu_opcode_set(SERIAL_OPCODE_SUB_DEV_INACT,         0x42, &opcode_recv_call_result,     &opcode_send_sub_dev_inact);
    mcu_opcode_set(SERIAL_OPCODE_SUB_STATUS_CB,         0x43, &opcode_recv_sub_status_cb,   NULL);

    protocol_define_set(mcu_protocol_define);

    protocol_init();

    peripheral_init();

    // module_config_init();
}

void iot_status_set( DEV_STATUS_T dev_status, iot_u32 timeout )
{
    mcu_s_sdk_status(dev_status, timeout);
}

iot_u32 iot_get_onlinetime( void )
{
    mcu_s_online_time();
}

#define DEV_FRAME_SEQ_LEN 4
#define G_TXBUF_LEN (FRAME_DATA_LMT - sizeof(mcu_send_upload_dps_s))
iot_u8 * g_sub_id = NULL;
iot_u8  sub_id_set_flag = 0;
iot_u8  sub_id_is_null = 0;
iot_u16 updata_buf_len = 0;
typedef struct
{
    iot_u8 seq[DEV_FRAME_SEQ_LEN];
    iot_u8 data_act;
    iot_u8 subid[SUB_ID_LEN_MAX];
    iot_u8 buf[G_TXBUF_LEN];
}TX_BUF;
TX_BUF g_stTxBuf;

iot_s32 dp_up_check_sub_id(const char sub_id[32])
{
    if(sub_id_set_flag == 0) {
        sub_id_set_flag = 1;
        if(sub_id == NULL) {
            sub_id_is_null = 1;
        }
        else
        {
            memset(g_stTxBuf.subid, 0, SUB_ID_LEN_MAX);
            memcpy(g_stTxBuf.subid, sub_id, strlen(sub_id));
        }
        IOT_INFO("%s,%d, new sub_id:%s\r\n", __func__, __LINE__, g_stTxBuf.subid);
    }
    else {
        if(sub_id == NULL) {
            if(sub_id_is_null != 1) {
                IOT_ERR("%s,%d, exist sub_id:%s, new sub_id:%s can't add\r\n", __func__, __LINE__, g_stTxBuf.subid, sub_id);
                return -1;
            }
        }
        else {
            if(strcmp(g_stTxBuf.subid, sub_id) != 0) {
                IOT_ERR("%s,%d, exist sub_id:%s, new sub_id:%s can't add\r\n", __func__, __LINE__, g_stTxBuf.subid, sub_id);
                return -1;
            }

        }
        // IOT_INFO("%s,%d, old sub_id:%s\r\n", __func__, __LINE__, g_stTxBuf.subid);
    }

    return 0;
}

iot_s32 dp_up_add_int(const char sub_id[32], iot_u8 dpid, iot_s32 value )
{
    iot_u16 Length = 0x0004;

    if(dp_up_check_sub_id(sub_id) != 0) {
        return -1;
    }

    if((updata_buf_len + Length + 4) > G_TXBUF_LEN)
    {
        return -1;
    }

    g_stTxBuf.buf[updata_buf_len ++] = dpid;
    
    g_stTxBuf.buf[updata_buf_len ++] = DP_TYPE_INT;
    
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8)(Length >> 8);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8)Length;

    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8)(value >> 24);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8)(value >> 16);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8)(value >> 8);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8)value;
    
    return 0;
}

iot_s32 dp_up_add_bool(const char sub_id[32], iot_u8 dpid, iot_u8 value )
{
    iot_u16 Length = 0x0001;

    if(dp_up_check_sub_id(sub_id) != 0) {
        return -1;
    }

    if((updata_buf_len + Length + 4) > G_TXBUF_LEN)
    {
        return -1;
    }

    g_stTxBuf.buf[updata_buf_len ++] = dpid;
    
    g_stTxBuf.buf[updata_buf_len ++] = DP_TYPE_BOOL;
    
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )(Length >> 8);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )Length;

    g_stTxBuf.buf[updata_buf_len ++] = value;
    
    return 0;
}

iot_s32 dp_up_add_enum(const char sub_id[32], iot_u8 dpid, iot_u8 value )
{
    iot_u16 Length = 0x0001;

    if(dp_up_check_sub_id(sub_id) != 0) {
        return -1;
    }

    if((updata_buf_len + Length + 4) > G_TXBUF_LEN)
    {
        return -1;
    }

    g_stTxBuf.buf[updata_buf_len ++] = dpid;
    
    g_stTxBuf.buf[updata_buf_len ++] = DP_TYPE_ENUM;
    
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )(Length >> 8);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )Length;

    g_stTxBuf.buf[updata_buf_len ++] = value;
    
    return 0;
}

iot_s32 dp_up_add_string(const char sub_id[32],  iot_u8 dpid, const char* str, iot_u32 str_len )
{
    iot_u16 Length = str_len;

    if(dp_up_check_sub_id(sub_id) != 0) {
        return -1;
    }

    if((updata_buf_len + Length + 4) > G_TXBUF_LEN)
    {
        return -1;
    }

    g_stTxBuf.buf[updata_buf_len ++] = dpid;
    
    g_stTxBuf.buf[updata_buf_len ++] = DP_TYPE_STRING;
    
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )(Length >> 8);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )Length;

    memcpy(&g_stTxBuf.buf[updata_buf_len], str, Length);
    updata_buf_len += Length;
    
    return 0;
}

iot_s32 dp_up_add_float(const char sub_id[32],  iot_u8 dpid, iot_f32 value )
{
    iot_u16 Length = 0x0004;

    if(dp_up_check_sub_id(sub_id) != 0) {
        return -1;
    }

    if((updata_buf_len + Length + 4) > G_TXBUF_LEN)
    {
        return -1;
    }

    g_stTxBuf.buf[updata_buf_len ++] = dpid;
    
    g_stTxBuf.buf[updata_buf_len ++] = DP_TYPE_FLOAT;
    
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )(Length >> 8);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )Length;

    FLOAT32_TO_STREAM_f(&g_stTxBuf.buf[updata_buf_len], value);
    updata_buf_len += Length;
    
    return 0;
}

iot_s32  dp_up_add_fault(const char sub_id[32],  iot_u8 dpid, const char* fault, iot_u32 fault_len )
{
    iot_u16 Length = fault_len;

    if(dp_up_check_sub_id(sub_id) != 0) {
        return -1;
    }

    if((updata_buf_len + Length + 4) > G_TXBUF_LEN)
    {
        return -1;
    }

    g_stTxBuf.buf[updata_buf_len ++] = dpid;
    
    g_stTxBuf.buf[updata_buf_len ++] = DP_TYPE_FAULT;
    
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )(Length >> 8);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )Length;

    memcpy(&g_stTxBuf.buf[updata_buf_len], fault, Length);
    updata_buf_len += Length;

    return 0;
}

iot_s32  dp_up_add_binary(const char sub_id[32],  iot_u8 dpid, const iot_u8* bin, iot_u32 bin_len )
{
    iot_u16 Length = bin_len;

    if(dp_up_check_sub_id(sub_id) != 0) {
        return -1;
    }

    if((updata_buf_len + Length + 4) > G_TXBUF_LEN)
    {
        return -1;
    }

    g_stTxBuf.buf[updata_buf_len ++] = dpid;
    
    g_stTxBuf.buf[updata_buf_len ++] = DP_TYPE_BIN;
    
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )(Length >> 8);
    g_stTxBuf.buf[updata_buf_len ++] = (iot_u8 )Length;

    memcpy(&g_stTxBuf.buf[updata_buf_len], bin, Length);
    updata_buf_len += Length;

    return 0;
}

iot_u32 g_seq = 0;
iot_u32 get_cur_seq_num()
{
	return g_seq++;
}

void g_txbuf_clean(void)
{
    // memset(g_stTxBuf, 0, sizeof(g_stTxBuf));
    updata_buf_len = 0;
    sub_id_set_flag = 0;
    sub_id_is_null = 0;
}

iot_s32 iot_upload_dps ( DATA_ACT_T data_act, iot_u32* data_seq )
{
    iot_u32 Seq = get_cur_seq_num();

    *data_seq = Seq;

    g_stTxBuf.data_act = data_act;
    UINT32_TO_STREAM_f(g_stTxBuf.seq, Seq);
    
    mcu_s_upload_dps((iot_u8*)&g_stTxBuf, sizeof(mcu_send_upload_dps_s) + updata_buf_len);

    g_txbuf_clean();

    return 0;
}

extern iot_download_dps_t iot_down_dps[];
extern iot_u32 DOWN_DPS_CNT;
iot_s32 iot_dp_handle(iot_u8* pData, iot_u16 Length)
{
    iot_u8* pData_t = pData;
    iot_u8 dp_id, dp_type;
    iot_u16 dp_len;
    iot_u32 i, cnt;
    char Subid[SUB_ID_LEN_MAX + 1];

    // IOT_INFO("%s,%d\r\n", __func__, __LINE__);

    memcpy(Subid, (char*)pData_t, SUB_ID_LEN_MAX);
    pData_t += SUB_ID_LEN_MAX;

    cnt = DOWN_DPS_CNT;
    while(pData_t < pData+Length)
    {
        dp_id = *pData_t++;
        dp_type = *pData_t++;
        dp_len = (*pData_t++ << 8) ;
		dp_len += *pData_t++;

        // IOT_INFO("DP download id:%d type:%d, len:%d\r\n", dp_id, dp_type, dp_len);

        for(i = 0; i < cnt; i++)
        {
            if(iot_down_dps[i].dpid == dp_id)
            {
                if(Subid[0] == 0)
                    iot_down_dps[i].dp_down_handle(NULL, pData_t, dp_len);
                else
                    iot_down_dps[i].dp_down_handle(Subid, pData_t, dp_len);
                break;
            }
        }

        pData_t += dp_len;
    }
    
    return 0;
}

iot_s32 iot_tx_data ( const char sub_id[32], iot_u32* data_seq, const iot_u8* data, iot_u32 data_len )
{
    iot_u32 Seq = get_cur_seq_num();

    if(data_len > G_TXBUF_LEN) {
        IOT_ERR("%s,%d\r\n", __func__, __LINE__);
        return -1;
    }

    *data_seq = Seq;

    UINT32_TO_STREAM_f(g_stTxBuf.seq, Seq);

    if(sub_id != NULL)
    {
        memcpy(g_stTxBuf.subid, sub_id, SUB_ID_LEN_MAX);
    }
    else
    {
        memset(g_stTxBuf.subid, 0, SUB_ID_LEN_MAX);
    }

    memcpy(g_stTxBuf.buf, data, data_len);

    mcu_s_tx_data((iot_u8*)&g_stTxBuf, sizeof(mcu_send_tx_data_s) + data_len);

    g_txbuf_clean();

    return 0;
}

iot_s32 iot_ota_option_set ( iot_u32 expect_time, iot_u32 chunk_size ,OTA_TYPE type)
{
    mcu_s_ota_set(expect_time, chunk_size);
}

iot_s32 iot_patch_req( const char name[16], const char version[16] )
{
    return mcu_s_patch_req(name, version);
}

iot_s32 iot_patch_end( const char name[16], const char version[16], DEV_STATUS_T patch_state )
{
    return mcu_s_patch_end(name, version, patch_state);
}

iot_s32 iot_get_info( INFO_TYPE_T info_type )
{
    mcu_s_get_info(info_type);
    return 0;
}

iot_s32 iot_config_info( INFO_TYPE_T info_type, void* info )
{
    return mcu_s_set_config(info_type, info);
}

iot_u16 g_sub_buf_len = 0;
iot_u16 g_sub_count = 0;
typedef struct
{
    iot_u8      data_seq[4];
    iot_u8      opt_type;
    iot_u8      sub_count[2];
    iot_u8      buf[FRAME_DATA_LMT - sizeof(mcu_send_sub_dev_add)];
}TX_SUB_BUF;

TX_SUB_BUF g_stSubActiveBuf;

iot_s32 sub_dev_add(const char sub_id[32], const char sub_name[32], const char sub_version[5], iot_u16 sub_type)
{
    mcu_send_sub_dev_add sub_info;
    iot_u8 sub_id_len = strlen(sub_id);
    iot_u8 sub_name_len = strlen(sub_name);
    iot_u8 sub_version_len = strlen(sub_version);
    iot_u8 sub_number_len = sizeof(sub_info);

    if(sub_id_len > SUB_ID_LEN_MAX || sub_name_len > SUB_ID_LEN_MAX || sub_version_len > MCU_VER_LEN)
    {
        IOT_ERR("%s,%d\r\n", __func__, __LINE__);
        return -1;
    }

    if(g_sub_buf_len + sub_number_len > FRAME_DATA_LMT - sizeof(mcu_send_sub_dev_act_s)) {
        IOT_ERR("%s,%d\r\n", __func__, __LINE__);
        return -1;
    }

    memset(&sub_info, 0, sub_number_len);
    
    memcpy(sub_info.sub_id, sub_id, sub_id_len);
    memcpy(sub_info.sub_name, sub_name, sub_name_len);
    memcpy(sub_info.sub_version, sub_version, sub_version_len);
    UINT16_TO_STREAM_f(sub_info.sub_type, sub_type);

    memcpy(g_stSubActiveBuf.buf + g_sub_buf_len, &sub_info, sub_number_len);
    g_sub_buf_len += sub_number_len;
    g_sub_count++;

    return 0;
}

iot_s32 iot_sub_dev_active (SUB_OPT_TYPE_T opt_type, iot_u32* data_seq )
{
    iot_u32 Seq = get_cur_seq_num();

    *data_seq = Seq;

    IOT_INFO("opt_type:%d, seq:%d, count:%d\r\n", opt_type, Seq, g_sub_count);

    g_stSubActiveBuf.opt_type = opt_type;
    UINT32_TO_STREAM_f(g_stSubActiveBuf.data_seq, Seq);
    UINT16_TO_STREAM_f(g_stSubActiveBuf.sub_count, g_sub_count);

    g_sub_buf_len += sizeof(mcu_send_sub_dev_act_s);

    // hex_dump("g_stSubActiveBuf", (iot_u8 *)&g_stSubActiveBuf, g_sub_buf_len);

    mcu_s_sub_dev_act((iot_u8 *)&g_stSubActiveBuf, g_sub_buf_len);

    g_sub_buf_len = 0;
    g_sub_count = 0;

    return 0;
}

iot_s32 iot_sub_dev_inactive ( const char sub_id[32], SUB_OPT_TYPE_T 	opt_type, iot_u32* data_seq )
{
    return mcu_s_sub_dev_inact(sub_id, opt_type, data_seq);
}
