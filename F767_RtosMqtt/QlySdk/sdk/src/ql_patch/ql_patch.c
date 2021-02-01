#include "ql_patch.h"

ql_patch_t patch;
patch_download_ctx g_patch_ctx;

void patch_load()
{
    ql_log_info("%s\r\n", __func__);
}

static type_u8 create_patch_list()
{    
    patch.patchs_list = ( patchs_list_t * )ql_zalloc( sizeof(patchs_list_t) * patch.patchs_count );
    if( patch.patchs_list == NULL )
        return -1;
    patch.patchs_crc = ( type_u32 * )ql_zalloc( sizeof(type_u32) * patch.patchs_count );
    if( patch.patchs_crc == NULL )
        return -1;
    return 0;
}

static void delete_patch_list()
{
    if( patch.patch_json )
    {                
        cJSON_Delete(patch.patch_json);
        
        if(patch.patchs_list)
        {
            ql_free(patch.patchs_list);
            patch.patchs_list = NULL;
        }
        if(patch.patchs_crc)
        {
            ql_free(patch.patchs_crc);
            patch.patchs_crc = NULL;
        }      
        
        patch.patchs_count = 0;
    }
}

static type_u8 clear_patch_data()
{
    ql_memset(&g_patch_ctx, 0x00, sizeof(g_patch_ctx));
    g_patch_ctx.patching_index = -1;
    return 0;
}

void ql_patch_init( type_u32 chunk_size, get_seq_t get_seq, get_time_t get_time )
{
    patch.patch_status = PATCH_STATE_INIT;
    patch.patch_chunk_size = chunk_size;
    
    patch.patch_get_seq = get_seq;
    patch.patch_get_time = get_time;    

}

/*------------------------------------------tx interface--------------------------------------*/

/*0x0a10 验证升级的回复报文*/
type_s32 ql_patch_passive_res(type_s32 seq, type_s32 error)
{
    if(patch.patch_get_seq)
    {
        cloud_request_send_pkt(PKT_ID_RES_PATCH_PASSIVE, patch.patch_get_seq(), (void*)&error);
    }   
    else
        ql_log_warn("patch not init\r\n");
    return 0;
}

/*0x0a11:设备获取补丁列表*/
static type_s32 ql_patchs_check_list_pkt( void )
{
    cJSON* json_sub = NULL;
    char * p = NULL;
    int ret = -1;

    json_sub = cJSON_CreateObject();
    if(NULL == json_sub)
    {
        return -1;
    }

    cJSON_AddNumberToObject(json_sub, "upgradetype", patch.patch_upgrade_type);
    cJSON_AddNumberToObject(json_sub, "chunksize", patch.patch_chunk_size);
    
    p = cJSON_PrintUnformatted(json_sub);

    cJSON_Delete(json_sub);

    if(NULL == p)
    {
       return -1;
    }
    cloud_request_send_pkt(PKT_ID_REQ_PATCHS_CHECK_LIST, patch.patch_get_seq(), (void*)p);
    ql_free(p);
    
    return ret;    
}

void ql_patch_main( void )
{    
    if(patch.patch_get_time == NULL)
    {
        return;
    }
    
    if( patch.patch_upgrade_type == PATCH_CHECK_TYPE_PASSIVE ||  patch.patch_check_tm == patch.patch_get_time() )
    {   
        if(get_download_status() == 0)
        {
            ql_patchs_check_list_pkt();
        }

        if( patch.patch_upgrade_type == PATCH_CHECK_TYPE_PASSIVE && patch.patch_check_tm == 0 )
        {        
            ql_patch_check_tm_set(patch.patch_get_time() + PATCH_CHECK_DELAY);            
        }
        else
        {        
            if(patch.patch_check_tm < patch.patch_get_time() + PATCH_CHECK_DELAY)
                ql_patch_check_tm_set(patch.patch_check_tm + PATCH_CHECK_DELAY);
        }

        /*如果ota不在init状态，此时patch 验证过来了，应该返回给云端0 ,-1还是-2?*/
        if(patch.patch_upgrade_type == PATCH_CHECK_TYPE_PASSIVE)
        {
            patch.patch_upgrade_type = PATCH_CHECK_TYPE_TIMER;
        }
    }

    if( patch.patch_status == PATCH_STATE_ING &&  (patch.patch_get_time() - patch.patch_stat_chg_tm > QL_PATCHING_MAXTIME))
    {       
        ql_log_warn("patching timeout! \r\n");
        patch.patch_status = PATCH_STATE_INIT;

        /*设置当前全局下载状态空闲*/
        set_download_status(0);
    }
    
}

/*------------------------------------- rx interface ------------------------------------*/
/*0x0a10，补丁升级*/
static void ql_patch_handle_passive(const char *in, int inlen, type_u32 pkt_seq)
{
    int ret = 0;

    switch(get_download_status())
    {
        case 0:
            /* download status free*/
            break;
        case 1:            
            /* download status ota*/
            if(patch.patch_check_tm == 0)
                ql_patch_check_tm_set(patch.patch_get_time() + PATCH_CHECK_DELAY);
            else if(patch.patch_check_tm < patch.patch_get_time() + PATCH_CHECK_DELAY)
                ql_patch_check_tm_set(patch.patch_check_tm + PATCH_CHECK_DELAY);
            ret = -2;
            ql_log_err(ERR_EVENT_STATU,"ptch_p:otaing[%d]",get_ota_mcu_status());
            break;
        case 2:            
            /* download status patch*/        
            ret = -2;
            ql_log_err(ERR_EVENT_STATU,"ptch_p:ptching");
            break;
        default:
            ql_log_err(ERR_EVENT_LOCAL_DATA,"ptc pass statu err");
            break;
    }

    patch.patch_upgrade_type = PATCH_CHECK_TYPE_PASSIVE;
    ql_patch_passive_res(pkt_seq, ret);
}

/*0x0a11 补丁列表*/
static void ql_patch_handle_list(const char *in, int inlen, type_u32 pkt_seq)
{
    type_u8 i = 0;
    cJSON *root = NULL, *res_arr = NULL, *value_arr = NULL, *arr_item = NULL, *sub_item = NULL;

    /*更新列表时，先清空缓存中的json*/
    delete_patch_list();       

    root = cJSON_Parse((const char *)in);
    if (root == NULL)
        return ;

    patch.patch_json = root;
    res_arr = cJSON_GetObjectItem(root, "res");
    if (res_arr == NULL)
        return;

    value_arr = cJSON_GetObjectItem(root, "value");
    if (value_arr == NULL)
        return;
    patch.patchs_count = cJSON_GetArraySize(value_arr);
    if( patch.patchs_count == 0)
    {
        ql_log_info("get new patch num :0\r\n");
        return;
    }

    if( create_patch_list() == -1 )
    {    
        ql_log_err(ERR_EVENT_INTERFACE,"malc ptc_list err");
        return;
    }    
        
    for(i = 0; i <  patch.patchs_count; i++)
    {
        arr_item = cJSON_GetArrayItem(value_arr, i);
        if(arr_item)
        {
            /*name*/
            sub_item = cJSON_GetObjectItem(arr_item, "n");
            if(sub_item == NULL)
                return;
            patch.patchs_list[i].name = sub_item->valuestring;
            
            /*version*/
            sub_item = cJSON_GetObjectItem(arr_item, "v");
            if(sub_item == NULL)
                return;
            patch.patchs_list[i].version = sub_item->valuestring;

            /*size*/
            sub_item = cJSON_GetObjectItem(arr_item, "tl");
            if(sub_item == NULL)
                return;
            patch.patchs_list[i].total_len = sub_item->valueint;
            
            /*crc*/
            sub_item = cJSON_GetObjectItem(arr_item, "c");
            if(sub_item == NULL)
                return;
            patch.patchs_crc[i] = sub_item->valueint;
        }
    }
    /*向用户发送补丁列表*/
    iot_patchs_list_cb(  patch.patchs_count, patch.patchs_list);    
}

static int _check_patchfile_chunk(type_u8  isend, type_u32 offset, type_u32 datalen, type_u8*  data)
{
	unsigned short precrc = 0;
	int len = 0;

	if (data == NULL || datalen <= 0) {
		return -1;
	}

	if (g_patch_ctx.patching_index == -1) {
		return -1;
	}

	if (g_patch_ctx.exptect_offset != offset) {
		return -1;
	}

	len = g_patch_ctx.cur_len + datalen;

	if (len > patch.patchs_list[g_patch_ctx.patching_index].total_len){        
		return -1;
	}

	if (offset == 0) {
		precrc = 0xFFFF;
		g_patch_ctx.cur_crc = crc16(data, datalen, precrc);
	} else {
		g_patch_ctx.cur_crc = crc16(data, datalen, g_patch_ctx.cur_crc);
	}

	g_patch_ctx.cur_len = len;
	g_patch_ctx.exptect_offset += datalen;

	/* last chunk packet, check the length and crc. */
	if (isend) {
		if (len != patch.patchs_list[g_patch_ctx.patching_index].total_len || 
            g_patch_ctx.cur_crc != patch.patchs_crc[g_patch_ctx.patching_index]) {

			return -1;
		}
            
	}


	return 0;
}

void  patch_chunk_response(type_u32 seq, type_s32 error, type_u32 offset)
{
    patch_res_arg arg;
    arg.error  = error;
    arg.offset = offset;

    if(cloud_request_send_pkt(PKT_ID_RES_PATCH_CHUNK, seq, (void*)&arg) == 0)
    {
        if (error != -1) 
        {
            patch.patch_stat_chg_tm = patch.patch_get_time();
        }
    }
}

/*0x0a13 块传输报文*/
static void ql_patch_handle_chunk(const char *in, int inlen, type_u32 pkt_seq)
{
    type_s8     is_end = 0;
    type_u32    offset = 0, data_len = 0;
    type_u8*    data = NULL;
    int ret = -1;
    type_u8 chunk_stat;

    if (inlen <= 5) 
    {
        goto fatal_err;
    }

    if(patch.patch_status != PATCH_STATE_ING)
    {
        ret = -2;        
        goto fatal_err;
    }

    is_end   = in[0];
    offset   = STREAM_TO_UINT32_f((type_u8*)in, 1);
    data_len = inlen - 5;
    data     = (type_u8*)(in + 5);

    /*写入文件  */
    chunk_stat = 0x10 + is_end;
    ret = iot_chunk_cb( chunk_stat, offset, data_len,(const type_s8*)data);
    if(ret == 0)
    {
        ret = _check_patchfile_chunk(is_end, offset, data_len, data);        
        if (ret == 0)
        {
            if (is_end == 1)
            {
                /*下载成功*/
                patch.patch_status = PATCH_STATE_DOWNLOADED;
                /*patch置为初始化状态*/
                patch.patch_status = PATCH_STATE_INIT;     

                /*设置全局下载状态：空闲*/
                set_download_status(0);
            }
        }
        else
        {
            ql_log_err(ERR_EVENT_RX_DATA, "p_chunk fail");            
            ret = -1;
            goto fatal_err;
        }

        #if (__SDK_TYPE__== QLY_SDK_BIN)
            extern type_u32 bin_patch_chunk_seq, bin_patch_chunk_offset;
            bin_patch_chunk_offset = offset;
            bin_patch_chunk_seq    = pkt_seq;
            #ifndef DEBUG_WIFI_OTA_OPEN
                //bin version, send to uart without response to cloud
                return;
            #endif
        #endif//END SDK BIN
    }
    else
    {
        //the value user return err
        ret = -1;
        goto fatal_err;
    }  
           
fatal_err: 
    patch_chunk_response(pkt_seq, ret, offset);
}

/*0x0a14 升级状态的回复报文*/
static void ql_patch_handle_end(const char *in, int inlen, type_u32 pkt_seq)
{
    printf("ql_patch_handle_end \r\n");    
}

void ql_patch_handle(type_u16 op_code, const char *in, int inlen, type_u32 pkt_seq)
{
    switch(op_code)
    {
    case QL_OP_PATCH_PASSIVE://0x0a10
        ql_patch_handle_passive(in, inlen, pkt_seq);
        break;
    case QL_OP_PATCHS_CHECK_LIST://0x0a11
        ql_patch_handle_list(in, inlen, pkt_seq);
        break;
    case QL_OP_PATCH_CHUNK://0x0a13
        ql_patch_handle_chunk(in, inlen, pkt_seq);
        break;
    case QL_OP_PATCH_END://0x0a14
        ql_patch_handle_end(in, inlen, pkt_seq);
        break;       
    }
}

type_s32 ql_patch_statu_get()
{
    return patch.patch_status;
}

void ql_patch_check_tm_set(type_u32 check_tm)
{
    patch.patch_check_tm = check_tm;
}

/*-------------------------------------user interface-------------------------------------*/

int iot_patch_req( const char name[16], const char version[16] )
{
    int info_len = 0, ret = 0, i = 0;
    char * info  = NULL;

    if (!is_application_authed()) 
    {
        return -1;
    }

    if( name == NULL || version == NULL )
    {
        ql_log_err( ERR_EVENT_LOCAL_DATA, "ptch_req data err");
        return -1;
    }

    if( ql_strlen(name) > 16 ||  ql_strlen(version) > 16 )
    {
        ql_log_err( ERR_EVENT_LOCAL_DATA, "ptch_req data len err");
        return -1;
    }

    clear_patch_data();

        for( i = 0; i < patch.patchs_count; i++)
        {        
            if(strcmp(patch.patchs_list[i].name, name) == 0 && strcmp(patch.patchs_list[i].version, version) == 0)
            {
                g_patch_ctx.patching_index = i;
                break;
            }
        }
        if(i == patch.patchs_count)
        {
            ql_log_err(ERR_EVENT_RX_DATA,"ptch_req invalid");
            return -1;
        }
    
    #define PATCH_REQ_DATA "{\"n\":\"%s\",\"v\":\"%s\"}"
    info_len = sizeof(PATCH_REQ_DATA) + 36;//36 > 17+17

    info = ql_malloc(info_len);
    if(info == NULL)
    {   
        ql_log_err( ERR_EVENT_INTERFACE, "ptch_req malc err"); 
        return -1;
    }
    
    ql_snprintf(info, info_len, PATCH_REQ_DATA, name, version);
    
    if(get_download_status() > 0)
    {
        ql_log_err(ERR_EVENT_STATU,"ptch_req down:%d",get_ota_mcu_status());
        ret = -1;
    }
    else
    {
        ret = cloud_request_send_pkt(PKT_ID_REQ_PATCH_REQUEST, patch.patch_get_seq(), (void*)info);
        if(ret == 0)
        {
            patch.patch_status = PATCH_STATE_ING;  
            set_download_status(2);
        }    
    }
    
    if(info)
    {
        ql_free(info);
        info = NULL;
    }

    if(patch.patch_get_time())
        patch.patch_stat_chg_tm = patch.patch_get_time();
    return ret;
}

/*state: 0:成功 ，-1:失败*/
int iot_patch_end( const char name[16], const char version[16], DEV_STATUS_T patch_state )
{
    char * info = NULL;
    int info_len = 0, state = 0;

    printf("iot_patch_end n:%s, v:%s, s:%d\r\n", name, version, patch_state);

    if( name == NULL || version == NULL )
    {
        ql_log_err( ERR_EVENT_LOCAL_DATA, "ptch_cmp data err");
        return -1;
    }

    if( ql_strlen(name) > 16 || ql_strlen(version) > 16 || 
        (patch_state != DEV_STA_PATCH_FAILED && patch_state != DEV_STA_PATCH_SUCCESS) )
    {
        ql_log_err( ERR_EVENT_LOCAL_DATA, "ptch_end data err");
        return -1;
    }

    state = (patch_state == DEV_STA_PATCH_FAILED) ? -1 : 0;

    #define PATCH_END_DATA "{\"n\":\"%s\",\"v\":\"%s\",\"s\":%d}"
    info_len = sizeof(PATCH_END_DATA) + 48;//48 > 17 + 17 + 3

    info = ql_malloc(info_len);
    
    if(info == NULL)
    {   
        ql_log_err( ERR_EVENT_INTERFACE, "ptch_end malc err"); 
        return -1;
    }
    ql_snprintf(info, info_len, PATCH_END_DATA, name, version, state);
    
    cloud_request_send_pkt(PKT_ID_REQ_PATCH_END, patch.patch_get_seq(), (void*)info);

    if(info)
    {
        ql_free(info);
        info = NULL;
    }
    return 0;
}
/*------------------------------------- user interface end -------------------------------------*/

