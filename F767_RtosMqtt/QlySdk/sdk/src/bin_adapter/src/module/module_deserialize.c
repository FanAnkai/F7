#include "module_deserialize.h"
#include "module_store.h"
#include "module_serialize.h"
#include "ql_udp.h"

struct iot_context iot_ctx;
char MCU_VER[MCU_VER_LEN] = {0};
char SERVER_ADDR[SERVER_ADDR_LEN_MAX] = {0};

//0x01
iot_s32 module_d_init(serial_msg_s *msg)
{
    ql_log_info("%s, opcode_id:%d, msg_len:%d\r\n", __FUNCTION__, msg->opcode_id, msg->msg_len);
    ql_log_info("RX init cb\r\n");
    iot_u8 server_addr_len = 0;
    PRODUCT_CFG ProductCfg;
    mcu_send_init_s * mcu_send_init_data = NULL;

    mcu_send_init_data = ql_zalloc(msg->msg_len);
    if(mcu_send_init_data == NULL) {
        ql_log_err(ERR_EVENT_NULL, "%s is NULL\r\n", __FUNCTION__);
        return PROTOCOL_RET_ERR;
    }

    ql_memcpy(mcu_send_init_data, msg->arg, msg->msg_len);
    // hex_dump("mcu_send_init_data", (iot_u8 *)&mcu_send_init_data, sizeof(mcu_send_init_data));
    
    ql_memcpy(MCU_VER, mcu_send_init_data->mcu_ver, sizeof(MCU_VER));
    /* mcu_ver is valid */
    if(!IS_VALID_VER(MCU_VER))
    {
        ql_log_err(ERR_EVENT_NULL, "mcu ver err\r\n");
        goto out;
    }
    
    server_addr_len = mcu_send_init_data->addr_len;
    if(server_addr_len > SERVER_ADDR_LEN_MAX) {
        ql_log_err(ERR_EVENT_NULL, "server_addr_len:%d > SERVER_ADDR_LEN_MAX\r\n", server_addr_len);
        goto out;
    }
    ql_memset(SERVER_ADDR, 0, sizeof(SERVER_ADDR));
    ql_memcpy(SERVER_ADDR, mcu_send_init_data->server_addr, server_addr_len);

    iot_ctx.product_id = STREAM_TO_UINT32_f(mcu_send_init_data->product_id, 0);
    ql_memcpy(iot_ctx.product_key, mcu_send_init_data->product_key, sizeof(iot_ctx.product_key));
    iot_ctx.mcu_version = MCU_VER;
    iot_ctx.recvbuf_size = STREAM_TO_UINT32_f(mcu_send_init_data->recvbuf_size, 0);
    iot_ctx.sendbuf_size = STREAM_TO_UINT32_f(mcu_send_init_data->sendbuf_size, 0);
    iot_ctx.server_port = STREAM_TO_UINT16_f(mcu_send_init_data->server_port, 0);
    iot_ctx.server_addr = SERVER_ADDR;

    // ql_log_info("product_id:%d, mcu_ver:%s, recv:%d, send:%d, port:%d\r\n", iot_ctx.product_id, iot_ctx.mcu_version, iot_ctx.recvbuf_size, iot_ctx.sendbuf_size, iot_ctx.server_port);
    // ql_log_info("server_addr:%s\r\n", iot_ctx.server_addr);
    // hex_dump("product_key", iot_ctx.product_key, sizeof(iot_ctx.product_key));

    stop_init_request(); 
    
    memset((void *)&ProductCfg, 0x00, sizeof(ProductCfg));
    ProductCfg.product_id = iot_ctx.product_id;
    memcpy(ProductCfg.product_key, iot_ctx.product_key, PRODUCT_KEY_LEN);
    memcpy(ProductCfg.mcu_ver, iot_ctx.mcu_version, MCU_VER_LEN);

    if(0 == product_init_config(&ProductCfg))
    {        
        start_status_notify(2 * 1000);
        module_config_recv_init_cmd((void *)&ProductCfg);
    }
    else
    {
        ql_log_err(ERR_EVENT_NULL, "init cfg err\r\n");
        start_init_request();
    }

out:
    ql_free(mcu_send_init_data);
    return PROTOCOL_RET_OK;
}

//0x02
iot_s32 module_d_sdk_status(serial_msg_s *msg)
{
    opcode_sdk_status_s sdk_status_data;
    iot_u8 dev_status;
    iot_u32 timeout;

    ql_memcpy(&sdk_status_data, msg->arg, sizeof(sdk_status_data));

    dev_status = sdk_status_data.dev_status;
    timeout = STREAM_TO_UINT32_f(sdk_status_data.timeout, 0);

    iot_status_set(dev_status, timeout);

    return PROTOCOL_RET_OK;
}

//0x03
iot_s32 module_d_get_net_time(serial_msg_s *msg)
{
    module_s_online_time();
    return PROTOCOL_RET_OK;
}

//0x11
iot_s32 module_d_upload_dps(serial_msg_s *msg)
{
    mcu_send_upload_dps_s upload_dps;
    iot_u8* pData_t = NULL;
    iot_u32 seq = 0;
    iot_u8  act = 0;
    iot_u8  sub_id[SUB_ID_LEN_MAX] = {0};
    iot_u8  dp_id, dp_type;
    iot_u16 dp_len;
    iot_u8  result = 0;

    ql_memcpy(&upload_dps, msg->arg, sizeof(upload_dps));

    seq = STREAM_TO_UINT32_f(upload_dps.seq, 0);
    act = upload_dps.act;
    ql_memcpy(sub_id, upload_dps.sub_id, sizeof(sub_id));

    pData_t = (iot_u8 *)msg->arg + sizeof(upload_dps);
    while(pData_t < (iot_u8 *)msg->arg + msg->msg_len)
    {
        iot_u32 pd=0;      

        dp_id = *pData_t++;
        dp_type = *pData_t++;
        dp_len = (*pData_t++ << 8) ;
		dp_len += *pData_t++;

        ql_log_info("DP up add id:%d type:%d, len:%d\r\n", dp_id, dp_type, dp_len);

        switch(dp_type)
        {
            case 0:				
				pd = *pData_t++ << 24;
				pd += *pData_t++ << 16;
				pd += *pData_t++ << 8;
				pd += *pData_t++;				
				dp_up_add_int(sub_id, dp_id, pd);
		
                break;
            case 1:
                dp_up_add_bool(sub_id, dp_id, *pData_t++);
                break;
            case 2:
                dp_up_add_enum(sub_id, dp_id, *pData_t++);
                break;
            case 3:
            {
                char add_string[dp_len + 1];
                strncpy(add_string, pData_t, dp_len);
                add_string[dp_len] = 0;
                dp_up_add_string(sub_id, dp_id, add_string, dp_len);
                pData_t += dp_len;
                break;
            }
            case 4:
                {
                iot_f32 fValue = STREAM_TO_FLOAT32_f(pData_t, 0);
                dp_up_add_float(sub_id, dp_id, fValue);
                pData_t += dp_len;
                }
                break;
            case 5:
            {
                char add_string[dp_len + 1];
                strncpy(add_string, pData_t, dp_len);
                add_string[dp_len] = 0;
                dp_up_add_fault(sub_id, dp_id, add_string, dp_len);
                pData_t += dp_len;
                break;
            }
            case 6:
                dp_up_add_binary(sub_id, dp_id, pData_t, dp_len);
                pData_t += dp_len;
                break;
            default:
                ql_log_err(ERR_EVENT_NULL, "dp type err\r\n ");
                break;
        }
    }
    
    if(iot_upload_dps(act, &seq) != 0) {
        ql_log_err(ERR_EVENT_NULL, "%s iot_upload_dps err\r\n", __FUNCTION__);
        result = 1;
    }
    else {
        ql_log_info("seq:%d\r\n", seq);
        result = 0;
    }

    module_s_call_result_cb(msg->opcode_id, result);

    return PROTOCOL_RET_OK;
}

//0x13
iot_s32 module_d_tx_data(serial_msg_s *msg)
{
    mcu_send_tx_data_s mcu_send_tx_data;

    iot_u8* pData_t = msg->arg;
    iot_u32 DataSeq = 0;
    iot_u8  sub_id[SUB_ID_LEN_MAX]={0};
    iot_u8  ret = 0, result = 0;

    ql_memcpy(&mcu_send_tx_data, msg->arg, sizeof(mcu_send_tx_data));
    DataSeq = STREAM_TO_UINT32_f(mcu_send_tx_data.seq, 0);
    ql_memcpy(sub_id, mcu_send_tx_data.sub_id, sizeof(sub_id));
   
	pData_t+=sizeof(mcu_send_tx_data);

    if(0 == strlen(sub_id))
    {
        #if (__SDK_TYPE__== QLY_SDK_BIN)
        if(SOFTAP_MODE == ql_get_wifi_opmode())
        {
            if(0x06 == module_status_get())
                ret = udp_conn_send(pData_t, (msg->msg_len - sizeof(mcu_send_tx_data)), DataSeq, UDP_PKT_TYPE_AP_UPLINK_REQ);
            else
                ret = -1;
        }
        else
        #endif
        {
            ret = iot_tx_data(NULL,   &DataSeq, pData_t, (msg->msg_len - sizeof(mcu_send_tx_data)));
        }
    }
    else
    {
        #if (__SDK_TYPE__== QLY_SDK_BIN)
        if(SOFTAP_MODE == ql_get_wifi_opmode())
        {      
            /*sub_id not suppose*/
            ret = -1;
        }
        else
        #endif
        {
            ret = iot_tx_data(sub_id, &DataSeq, pData_t, (msg->msg_len - sizeof(mcu_send_tx_data)));
        }
    }
    
    if(ret != 0) {
        result = 1;
    }

    module_s_call_result_cb(msg->opcode_id, result);

    return PROTOCOL_RET_OK;
}

//0x21
iot_s32 module_d_ota_set(serial_msg_s *msg)
{
    mcu_send_ota_set_s mcu_send_ota_set;
    iot_u32 expect_time = 0;
    iot_u32 chunk_size = 0;
    iot_u8 result = 0;
    iot_u8 ret = 0;

    ql_memcpy(&mcu_send_ota_set, msg->arg, sizeof(mcu_send_ota_set));

    expect_time = STREAM_TO_UINT32_f(mcu_send_ota_set.expect_time, 0);
    chunk_size = STREAM_TO_UINT32_f(mcu_send_ota_set.chunk_size, 0);

    ret = iot_ota_option_set(expect_time, chunk_size, OTA_TYPE_BASIC);
    if(ret >= 0)
        result = 0;
    else
        result = 1;

    module_s_call_result_cb(msg->opcode_id, result);

    return PROTOCOL_RET_OK;
}

//0x25
iot_s32 module_d_patch_req(serial_msg_s *msg)
{
    iot_s32 ret = 0;
    iot_u32 result = 0;
    mcu_send_patch_req_s mcu_send_patch_req;

    ql_memcpy(&mcu_send_patch_req, msg->arg, sizeof(mcu_send_patch_req));

    ret = iot_patch_req(mcu_send_patch_req.name, mcu_send_patch_req.ver);
    if(ret != 0) {
        result = 1;
    }

    module_s_call_result_cb(msg->opcode_id, result);

    return PROTOCOL_RET_OK;
}

//0x26
iot_s32 module_d_patch_end(serial_msg_s *msg)
{
    iot_s32 ret = 0;
    iot_u32 result = 0;
    mcu_send_patch_end_s mcu_send_patch_end;

    ql_memcpy(&mcu_send_patch_end, msg->arg, sizeof(mcu_send_patch_end));

    ret = iot_patch_end(mcu_send_patch_end.name, mcu_send_patch_end.ver, mcu_send_patch_end.patch_state);
    if(ret != 0) {
        result = 1;
    }

    module_s_call_result_cb(msg->opcode_id, result);

    return PROTOCOL_RET_OK;
}

//0x27
iot_s32 module_d_data_chunk(serial_msg_s *msg)
{
    mcu_send_data_chunk_s mcu_send_data_chunk;
    iot_u8  chunk_stat = 0;
    iot_u8  result = 0;
    iot_u32 offset = 0;
    iot_s32 error = 0;

    ql_memcpy(&mcu_send_data_chunk, msg->arg, sizeof(mcu_send_data_chunk));

    chunk_stat = mcu_send_data_chunk.chunk_stat;
    result = mcu_send_data_chunk.result;
    offset = STREAM_TO_UINT32_f(mcu_send_data_chunk.offset, 0);

    if(result != 0)
        error = -1;

    ql_chunk_response(chunk_stat, error, offset);

    return PROTOCOL_RET_OK;
}

//0x31
iot_s32 module_d_get_info(serial_msg_s *msg)
{
    iot_s32 ret = 0;
    iot_u32 result = 0;
    mcu_send_get_info_s mcu_send_get_info;

    ql_memcpy(&mcu_send_get_info, msg->arg, sizeof(mcu_send_get_info));

    ret = iot_get_info(mcu_send_get_info.info_type);
    if(ret != 0) {
        result = 1;
    }

    module_s_call_result_cb(msg->opcode_id, result);

    return PROTOCOL_RET_OK;
}

//0x33
iot_s32 module_d_set_config(serial_msg_s *msg)
{
    mcu_send_set_config_s mcu_send_set_config_head;
    iot_u32 head_len = sizeof(mcu_send_set_config_head);
    iot_u8  set_info[OPCODE_SET_CONFIG_LEN_MAX + 1] = {0};
    iot_s32 ret = 0;
    iot_u8  result = 0;

    ql_memcpy(&mcu_send_set_config_head, msg->arg, head_len);
    ql_memcpy(set_info, (iot_u8 *)msg->arg + head_len, mcu_send_set_config_head.info_len);

    ql_log_info("info_type:%d, info:%s\r\n", mcu_send_set_config_head.info_type, set_info);

    ret = iot_config_info(mcu_send_set_config_head.info_type, set_info);
    if(ret != 0) {
        result = 1;
    }

    module_s_call_result_cb(msg->opcode_id, result);

    return PROTOCOL_RET_OK;
}

//0x41
iot_s32 module_d_sub_dev_act(serial_msg_s *msg)
{
    mcu_send_sub_dev_act_s *payload = NULL;
    iot_u32 seq = 0;
    iot_u16 count = 0;
    char sub_id[SUB_ID_LEN_MAX + 1] = {0};
    char sub_name[SUB_ID_LEN_MAX + 1] = {0};
    char sub_version[MCU_VER_LEN + 1] = {0};
    iot_u16 sub_type = 0;
    iot_s32 ret = 0;
    iot_u8  result = 0;
    iot_u32 i = 0;

    payload = ql_zalloc(msg->msg_len);
    if(payload == NULL) {
        ql_log_err(ERR_EVENT_NULL, "%s is NULL\r\n", __FUNCTION__);
        ret = -1;
        goto out;
    }

    ql_memcpy(payload, (iot_u8 *)msg->arg, msg->msg_len);
    seq = STREAM_TO_UINT32_f(payload->data_seq, 0);
    count = STREAM_TO_UINT16_f(payload->sub_count, 0);
    ql_log_info("opt_type:%d, seq:%d, count:%d\r\n", payload->opt_type, seq, count);

    for(i = 0; i < count; i++) {
        ql_memcpy(sub_id, payload->sub_info[i].sub_id, SUB_ID_LEN_MAX);
        ql_memcpy(sub_name, payload->sub_info[i].sub_name, SUB_ID_LEN_MAX);
        ql_memcpy(sub_version, payload->sub_info[i].sub_version, MCU_VER_LEN);
        sub_type = STREAM_TO_UINT16_f(payload->sub_info[i].sub_type, 0);
        ql_log_info("i:%d, sub_id:%s, name:%s, version:%s, sub_type:%d\r\n",i, sub_id, sub_name, sub_version, sub_type);

        ret = sub_dev_add(sub_id, sub_name, sub_version, sub_type);
        if(ret != 0) {
            ql_log_err(ERR_EVENT_NULL, "%s sub_dev_add err\r\n", __FUNCTION__);
            goto out;
        }
    }

    ret = iot_sub_dev_active(payload->opt_type, &seq);
out:
    if(ret != 0) {
        result = 1;
    }

    module_s_call_result_cb(msg->opcode_id, result);

    ql_free(payload);

    return PROTOCOL_RET_OK;
}

//0x42
iot_s32 module_d_sub_dev_inact(serial_msg_s *msg)
{
    mcu_send_sub_dev_inact_s mcu_send_sub_dev_inact;
    iot_u32 seq = 0;
    char sub_id[SUB_ID_LEN_MAX + 1] = {0};
    iot_s32 ret = 0;
    iot_u8  result = 0;

    ql_memcpy(&mcu_send_sub_dev_inact, msg->arg, sizeof(mcu_send_sub_dev_inact));

    ql_memcpy(sub_id, mcu_send_sub_dev_inact.sub_id, sizeof(mcu_send_sub_dev_inact.sub_id));
    seq = STREAM_TO_UINT32_f(mcu_send_sub_dev_inact.data_seq, 0);

    ql_log_info("sub_id:%s, opt_type:%d, seq:%d\r\n", sub_id, mcu_send_sub_dev_inact.opt_type, seq);

    ret = iot_sub_dev_inactive(sub_id, mcu_send_sub_dev_inact.opt_type, &seq);
    if(ret != 0) {
        result = 1;
    }

    module_s_call_result_cb(msg->opcode_id, result);

    return PROTOCOL_RET_OK;
}
