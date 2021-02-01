#include "module_serialize.h"
#include "ql_include.h"

//call cb
void module_s_call_result_cb(iot_u8 opcode_id, iot_u8 result)
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    module_send_call_result_s module_send_call_result;

    module_send_call_result.result = result;

    msg.opcode_id = opcode_id;
    msg.msg_len = sizeof(module_send_call_result);
    msg.arg = &module_send_call_result;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x01
void module_s_init(void)
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    iot_u8 module_mac_hex[6];

    module_send_init_s send_data;
    iot_u32 send_len = sizeof(module_send_init_s);

    qlcloud_get_mac_arr(module_mac_hex);
    // hex_dump("module_mac_hex", module_mac_hex, sizeof(module_mac_hex));

    ql_memcpy(send_data.mac, module_mac_hex, sizeof(send_data.mac));
    ql_memcpy(send_data.ver, BIN_SDK_VERSION, sizeof(send_data.ver));
    
    msg.opcode_id = SERIAL_OPCODE_INIT;
    msg.msg_len = send_len;
    msg.arg = &send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x02
void module_s_sdk_status( DEV_STATUS_T dev_status, iot_u32 timestamp )
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    opcode_sdk_status_s sdk_status_data;

    sdk_status_data.dev_status = dev_status;
    UINT32_TO_STREAM_f(sdk_status_data.timeout, timestamp);

    msg.opcode_id = SERIAL_OPCODE_SDK_STATUS;
    msg.msg_len = sizeof(sdk_status_data);
    msg.arg = &sdk_status_data;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x03
void module_s_online_time()
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    module_send_get_online_time_s module_send_get_online_time;
    iot_u32 timestamp = 0;
    s_time_t tm;
    
    timestamp = iot_get_onlinetime();
    iot_parse_timestamp(timestamp, &tm);

    UINT32_TO_STREAM_f(module_send_get_online_time.TimeStamp, timestamp);
    module_send_get_online_time.year = tm.year - 2000;
    module_send_get_online_time.month = tm.mon;
    module_send_get_online_time.day = tm.day;
    module_send_get_online_time.hour = tm.hour;
    module_send_get_online_time.minute = tm.min;
    module_send_get_online_time.second = tm.sec;
    module_send_get_online_time.week = tm.week;

    msg.opcode_id = SERIAL_OPCODE_GET_ONLINE_TIME;
    msg.msg_len = sizeof(module_send_get_online_time);
    msg.arg = &module_send_get_online_time;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x04
void module_s_data_cb( iot_u32 data_seq, DATA_LIST_T * data_list )
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    module_send_data_cb_s * send_data = NULL;
    iot_u32 send_len = 0;
    iot_u32 i = 0;

    send_len += sizeof(module_send_data_cb_s);  //head len
    for(i = 0; i < data_list->count; i++) {
        send_len += sizeof(module_send_data_list_s);
    }

    send_data = ql_zalloc(send_len);
    if(send_data == NULL) {
        ql_log_err(ERR_EVENT_NULL, "%s is NULL\r\n", __FUNCTION__);
        return;
    }

    UINT32_TO_STREAM_f(send_data->seq, data_seq);
    UINT16_TO_STREAM_f(send_data->count, data_list->count);
    for(i = 0; i < data_list->count; i++) {
        ql_memcpy(send_data->data_list[i].dev_id, data_list->info[i].dev_id, strlen(data_list->info[i].dev_id));
        send_data->data_list[i].info_type = data_list->info[i].info_type;
    }

    msg.opcode_id = SERIAL_OPCODE_DATA_CB;
    msg.msg_len = send_len;
    msg.arg = send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);

    ql_free(send_data);
}

//0x12
void module_s_download_dps(iot_u8* pData, iot_u16 Length)
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;

    msg.opcode_id = SERIAL_OPCODE_DOWNLOAD_DPS;
    msg.msg_len = Length;
    msg.arg = pData;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x14
void module_s_rx_data( char sub_id[32], iot_u64 data_timestamp, iot_u8* data, iot_u32 data_len )
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    iot_u8 * send_data = NULL;
    iot_u32 send_len = 0;
    module_send_rx_data_s module_send_rx_data_head;
    iot_u32 head_len = sizeof(module_send_rx_data_head);

    UINT64_TO_STREAM_f(module_send_rx_data_head.TimeStamp, data_timestamp);
    if(sub_id == NULL)
        ql_memset(module_send_rx_data_head.sub_id, 0, sizeof(module_send_rx_data_head.sub_id));
    else
        ql_memcpy(module_send_rx_data_head.sub_id, sub_id, sizeof(module_send_rx_data_head.sub_id));

    send_len = head_len + data_len;
    send_data = ql_zalloc(send_len);
    if(send_data == NULL) {
        ql_log_err(ERR_EVENT_NULL, "%s is NULL\r\n", __FUNCTION__);
        return;
    }

    ql_memcpy(send_data, &module_send_rx_data_head, head_len);
    ql_memcpy(send_data + head_len, data, data_len);

    msg.opcode_id = SERIAL_OPCODE_RX_DATA;
    msg.msg_len = send_len;
    msg.arg = send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);

    ql_free(send_data);
}

//0x22
void module_s_ota_info( char name[16], char version[6], iot_u32 total_len, iot_u16 file_crc )
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    module_send_ota_info_s module_send_ota_info;

    ql_memset(&module_send_ota_info, 0, sizeof(module_send_ota_info));

    ql_memcpy(module_send_ota_info.name, name, strlen(name));
    ql_memcpy(module_send_ota_info.version, version, strlen(version));
    UINT32_TO_STREAM_f(module_send_ota_info.total_len, total_len);
    UINT16_TO_STREAM_f(module_send_ota_info.file_crc, file_crc);

    msg.opcode_id = SERIAL_OPCODE_OTA_INFO;
    msg.msg_len = sizeof(module_send_ota_info);
    msg.arg = &module_send_ota_info;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x23
void module_s_ota_upgrade(void)
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;

    msg.opcode_id = SERIAL_OPCODE_OTA_UPGRADE;
    msg.msg_len = 0;
    msg.arg = NULL;

    send_opcode_fun_call(msg.opcode_id, &msg);
}

//0x24
void module_s_patch_list( iot_s32 count, patchs_list_t patchs[] )
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    module_send_patch_list_s * send_data = NULL;
    iot_u32 send_len = 0;
    iot_u32 head_len = 0;
    iot_u32 i = 0;

    //calc the send_data len
    head_len = sizeof(send_data);
    send_len += head_len;
    for(i = 0; i < count; i++) {
        send_len += PATCH_NUMBER_LEN;
    }

    send_data = (module_send_patch_list_s *)ql_zalloc(send_len);
    if(send_data == NULL) {
        ql_log_err(ERR_EVENT_NULL, "%s is NULL\r\n", __FUNCTION__);
        return;
    }

    //copy data
    UINT32_TO_STREAM_f(send_data->count, count);
    for(i = 0; i < count; i ++) {
        ql_memcpy(send_data->patch_number[i].name, patchs[i].name, ql_strlen(patchs[i].name));
        ql_memcpy(send_data->patch_number[i].ver, patchs[i].version, ql_strlen(patchs[i].version));
        UINT32_TO_STREAM_f(send_data->patch_number[i].total_len, patchs[i].total_len);
    }

    msg.opcode_id = SERIAL_OPCODE_PATCH_LIST;
    msg.msg_len = send_len;
    msg.arg = send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);

    ql_free(send_data);
}

//0x27
void module_s_data_chunk( iot_u8 chunk_stat, iot_u32 chunk_offset, iot_u32 chunk_size, const iot_s8* chunk )
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    module_send_data_chunk_s module_send_data_chunk_head;
    iot_u32 head_len = sizeof(module_send_data_chunk_head);
    iot_u32 send_len = head_len + chunk_size;
    iot_u8 *send_data = ql_zalloc(send_len);
    if(send_data == NULL) {
        ql_log_err(ERR_EVENT_NULL, "%s is NULL\r\n", __FUNCTION__);
        return;
    }

    module_send_data_chunk_head.chunk_stat = chunk_stat;
    UINT32_TO_STREAM_f(module_send_data_chunk_head.chunk_offset, chunk_offset);
    UINT32_TO_STREAM_f(module_send_data_chunk_head.chunk_size, chunk_size);

    ql_memcpy(send_data, &module_send_data_chunk_head, head_len);
    ql_memcpy(send_data + head_len, chunk, chunk_size);

    msg.opcode_id = SERIAL_OPCODE_DATA_CHUNK;
    msg.msg_len = send_len;
    msg.arg = send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);

    ql_free(send_data);
}

//0x32
#define GET_INFO_TYPEANDLEN_LEN sizeof(module_send_info_cb_s)   //data_type+data_len
#define SHARE_INFO_COUNT_LEN 4
#define INTERVAL_NUM_GROUP 5    //数据中len的个数
void module_s_info_cb( INFO_TYPE_T info_type, void* info )
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    int i = 0;
    user_info_t * master_info = NULL;
    share_info_t * share_info = NULL;
    dev_info_t * dev_info = NULL;
	iot_u8 *buff = NULL;
    iot_u16 buff_len = 0;
	int count = 0;
    ql_log_info("\r\ninfo_type(TX):%d\r\n", info_type);
	
    switch(info_type)
    {
        case INFO_TYPE_USER_MASTER:
            if(!info)
            {                
                /*发包*/
                buff_len = GET_INFO_TYPEANDLEN_LEN;
				buff = (iot_u8*)ql_zalloc(buff_len);	
				buff[0] = info_type;
				UINT16_TO_STREAM_f(buff+1,0x00);
            }
            else
            {
                master_info = (user_info_t *)info;

    			/*发包*/
                buff_len = GET_INFO_TYPEANDLEN_LEN;
    			buff_len += INTERVAL_NUM_GROUP*2+strlen(master_info->id)+strlen(master_info->email)+strlen(master_info->country_code)+strlen(master_info->phone)+strlen(master_info->name);
    			buff = (iot_u8*)ql_zalloc(buff_len);
    			
    			buff[0]= INFO_TYPE_USER_MASTER;
    			count++;
    			UINT16_TO_STREAM_f(buff+count, buff_len);
    			count+=2;
    			/*id*/
    			buff[count++]=strlen(master_info->id) + 1;
    			memcpy(buff+count,master_info->id,strlen(master_info->id));
    			count+=strlen(master_info->id) + 1;
    			
    			/*mail*/	
    			buff[count++]=strlen(master_info->email) + 1;
    			memcpy(buff+count,master_info->email,strlen(master_info->email));
    			count+=strlen(master_info->email) + 1;
    			
    			/*country_code*/			
    			buff[count++]=strlen(master_info->country_code) + 1;
    			memcpy(buff+count,master_info->country_code,strlen(master_info->country_code));
    			count+=strlen(master_info->country_code) + 1;

    			/*phone*/			
    			buff[count++]=strlen(master_info->phone) + 1;
    			memcpy(buff+count,master_info->phone,strlen(master_info->phone));
    			count+=strlen(master_info->phone) + 1;

    			/*name*/		
    			buff[count++]=strlen(master_info->name) + 1;
    			memcpy(buff+count,master_info->name,strlen(master_info->name));
    			count+=strlen(master_info->name) + 1;
            }
            
			break;
        case INFO_TYPE_USER_SHARE:
            if(!info)
            {                
                /*发包*/
                buff_len = GET_INFO_TYPEANDLEN_LEN;
				buff = (iot_u8*)ql_zalloc(buff_len);	
				buff[0] = info_type;
				UINT16_TO_STREAM_f(buff+1,0x00);
            }
            else
            {
                share_info = (share_info_t *)info;
                
                buff_len = GET_INFO_TYPEANDLEN_LEN;
                buff_len += SHARE_INFO_COUNT_LEN; //use for save share_user_count
                for(i = 0; i < share_info->count; i++)
                {
                    buff_len += INTERVAL_NUM_GROUP*2+strlen(share_info->user[i].id)+strlen(share_info->user[i].email)+strlen(share_info->user[i].country_code)+strlen(share_info->user[i].phone)+strlen(share_info->user[i].name);
                }
                
                buff = (iot_u8*)ql_zalloc(buff_len);
                
                //head
                buff[0]= info_type;
                count++;
                UINT16_TO_STREAM_f(buff+count, buff_len);
                count += 2;
                
                //share info count
                UINT32_TO_STREAM_f(buff+count, share_info->count);
                count += 4;
                
                //share info number detail
                for(i = 0; i < share_info->count; i++)
                {
                    /*id*/
                    buff[count++]=strlen(share_info->user[i].id) + 1;
                    memcpy(buff+count,share_info->user[i].id,strlen(share_info->user[i].id));
                    count+=strlen(share_info->user[i].id) + 1;
                    
                    /*mail*/    
                    buff[count++]=strlen(share_info->user[i].email) + 1;
                    memcpy(buff+count,share_info->user[i].email,strlen(share_info->user[i].email));
                    count+=strlen(share_info->user[i].email) + 1;
                    
                    /*country_code*/            
                    buff[count++]=strlen(share_info->user[i].country_code) + 1;
                    memcpy(buff+count,share_info->user[i].country_code,strlen(share_info->user[i].country_code));
                    count+=strlen(share_info->user[i].country_code) + 1;
                    
                    /*phone*/           
                    buff[count++]=strlen(share_info->user[i].phone) + 1;
                    memcpy(buff+count,share_info->user[i].phone,strlen(share_info->user[i].phone));
                    count+=strlen(share_info->user[i].phone) + 1;
                    
                    /*name*/        
                    buff[count++]=strlen(share_info->user[i].name) + 1;
                    memcpy(buff+count,share_info->user[i].name,strlen(share_info->user[i].name));
                    count+=strlen(share_info->user[i].name) + 1;
                }
            }
            break;
        case INFO_TYPE_DEVICE:
            dev_info = (dev_info_t *)info;
            ql_log_info("is_bind:%d,sdk_ver=%s\r\n", dev_info->is_bind,dev_info->sdk_ver);

			/*发包*/
            buff_len = GET_INFO_TYPEANDLEN_LEN + sizeof(dev_info_t);
			buff = (iot_u8*)ql_zalloc(buff_len);			
			buff[0]= INFO_TYPE_DEVICE;
			UINT16_TO_STREAM_f(buff+1, sizeof(dev_info_t));
	
			memcpy(buff+3,dev_info,sizeof(dev_info_t));
            
            break;
        default:
            ql_log_info("info_type is error\r\n");
    }

    msg.opcode_id = SERIAL_OPCODE_INFO_CB;
    msg.msg_len = buff_len;
    msg.arg = buff;

    send_opcode_fun_call(msg.opcode_id, &msg);

    ql_free(buff);
}

//0x43
void module_s_sub_status_cb( SUB_STATUS_T sub_status, iot_u32 timestamp, DATA_LIST_T * list , iot_u32 seq)
{
    ql_log_info("%s\r\n", __FUNCTION__);
    serial_msg_s msg;
    module_send_sub_status_cb_s * send_data = NULL;
    iot_u32 send_len = 0;
    iot_u32 i = 0;

    send_len += sizeof(module_send_sub_status_cb_s);    //head len
    for(i = 0; i < list->count; i++) {
        send_len += sizeof(module_send_sub_info);
    }

    send_data = ql_zalloc(send_len);
    if(send_data == NULL) {
        ql_log_err(ERR_EVENT_NULL, "%s is NULL\r\n", __FUNCTION__);
        return;
    }

    UINT32_TO_STREAM_f(send_data->data_seq, seq);
    send_data->sub_status = sub_status;
    UINT32_TO_STREAM_f(send_data->timestamp, timestamp);
    UINT16_TO_STREAM_f(send_data->sub_count, list->count);
    for(i = 0; i < list->count; i++) {
        ql_memcpy(send_data->sub_info[i].sub_id, list->info[i].dev_id, strlen(list->info[i].dev_id));
        send_data->sub_info[i].info_type = list->info[i].info_type;
    }

    msg.opcode_id = SERIAL_OPCODE_SUB_STATUS_CB;
    msg.msg_len = send_len;
    msg.arg = send_data;

    send_opcode_fun_call(msg.opcode_id, &msg);

    ql_free(send_data);
}

iot_s32 module_send_cb(serial_msg_s *msg)
{
    tx_device_frame(FRAME_VER, get_val_with_opcodeID(msg->opcode_id), msg->msg_len, (iot_u8 *)msg->arg);
}
