#include "ql_include.h"
#include "ql_queue.h"
#include "ql_dispatch_access.h"

#ifdef LAN_DETECT
#include "ql_udp.h"
#endif

extern ql_queue *g_queue;
extern type_u32 set_otaupgrade_checktime;

#define OSI_STACK_SIZE_ 2048
static ql_thread_t *g_qlcloud_back_logic = NULL;
static ql_thread_t *g_qlcloud_back_time = NULL;
int g_sdk_is_inited = 0;

type_s32 iot_start(struct iot_context* ctx)
{
    int ret = 0;
   
#ifdef LAN_DETECT
    static ql_thread_t *g_qlcloud_udp = NULL;
#endif

    if(g_sdk_is_inited == 1)
    {
        return -1;
    }
#if (__SDK_TYPE__== QLY_SDK_LIB)
    ql_memcpy(DEVICE_VERSION, "03.11", 5);
    DEVICE_VERSION[5] = '\0';
#endif
#ifdef DEBUG_WIFI_OTA_OPEN
    ql_memcpy(DEVICE_VERSION, "03.11", 5);
    DEVICE_VERSION[5] = '\0';
#endif
    char log_str[64] = {0};
    ql_snprintf(log_str, 64, "============================================================\r\n");
    iot_print(log_str);
    ql_snprintf(log_str, 64, "===sdk[%02x:%02x] version:%s, compiled time:%s===\r\n", __SDK_TYPE__, __SDK_PLATFORM__, DEVICE_VERSION, __DATE__);
    iot_print(log_str);
    ql_snprintf(log_str, 64, "============================================================\r\n");
    iot_print(log_str);
    
    ret = qlcloud_init_client_context(ctx);
    if (ret != 0) {
        return -1;
    }
    
    if (g_queue) {
        destroy_ql_queue(&g_queue);
    }

    g_queue = create_ql_queue(sizeof(struct cloud_event), QUEUESIZE);
    if (NULL == g_queue) {
        ql_log_err( ERR_EVENT_INTERFACE, "crt queue err.");
        return -1;
    }

    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL) /* when openssl is used, 2048 bytes are not enough */
    {
        g_qlcloud_back_logic = ql_thread_create(1, OSI_STACK_SIZE_ * 3, (ql_thread_fn)ql_logic_main, NULL);
    }
    else
    {
        g_qlcloud_back_logic = ql_thread_create(1, OSI_STACK_SIZE_, (ql_thread_fn)ql_logic_main, NULL);
    }
    
    if (NULL == g_qlcloud_back_logic) {
        ql_log_err( ERR_EVENT_INTERFACE, "thread_main err");
        return -1;
    }

    g_qlcloud_back_time = ql_thread_create(1, OSI_STACK_SIZE_, (ql_thread_fn)ql_logic_time, NULL);
    if (NULL == g_qlcloud_back_time) {
        ql_log_err( ERR_EVENT_INTERFACE, "thread_time err");
        return -1;
    }
#ifdef LAN_DETECT
    g_qlcloud_udp = ql_thread_create(1, 1024, (ql_thread_fn)qlcloud_udp_server, NULL);
    if (NULL == g_qlcloud_udp) {
        ql_log_err( ERR_EVENT_INTERFACE, "thread_udp err");
        return -1;
    }
#endif

    g_sdk_is_inited = 1;   
    return 0;
}

iot_s32 iot_config_info( INFO_TYPE_T info_type, void* info )
{
    if(g_sdk_is_inited == 1)
    {
        ql_log_err( ERR_EVENT_STATU, "server no init!");
        return -1;
    }

    if(info_type == INFO_TYPE_ENCRYPT)
    {
        if(ql_memcmp((char*)info, "AES", 3) == 0)
        {
            g_client_handle.encrypt_type = 0;
        }
        else if(ql_memcmp((char*)info, "SSL", 3) == 0)
        {
            g_client_handle.encrypt_type = 1;
        }
        else if(ql_memcmp((char*)info, "SM4", 3) == 0)
        {
            g_client_handle.encrypt_type = 2;
        }
        else
        {
            g_client_handle.encrypt_type = 0;
            ql_log_err(ERR_EVENT_LOCAL_DATA, "enc_t[%s] err", (char*)info);
            return -1;
        }
    }
    else if(info_type == INFO_TYPE_SN)
    {
        if(qlcloud_is_sn_valid((char*)info) < 0)
        {
            ql_log_err( ERR_EVENT_LOCAL_DATA, "sn err");
            return -1;
        }
        else
        {
            ql_memcpy(g_client_handle.sn, (char*)info, 16);
        }
    }
    else
    {
        ql_log_err( ERR_EVENT_LOCAL_DATA, "config[%d] err", info_type);
        return -1;
    }
    return 0;
}

type_u32 iot_get_onlinetime(void)
{
    type_u32 t = get_current_time();
    if(t < 1483200000)
    {
        t = 0;
    }
    return t;
}

type_s32 iot_get_info(INFO_TYPE_T info_type)
{   
    int info_type_tmp = 0;
    if (!is_application_authed()) {
        return -1;
    }
    
    if(info_type == INFO_TYPE_USER_MASTER || 
        info_type == INFO_TYPE_USER_SHARE ||
        info_type == INFO_TYPE_DEVICE)
    {
        info_type_tmp = info_type;
        return cloud_request_send_pkt(PKT_ID_GET_INFO, get_cur_seq_num(), (void*)&info_type_tmp);
    }
    
    ql_log_err( ERR_EVENT_TX_DATA, "info_t[%d] err", info_type);
    return -1;
}
/*

use <dp_key_sub_id> as json`s key , insert <dp_value_json_obj> into json`s value array.

json string like this : {"subid":[{"i":2,"v":"3","t":0},{"i":8,"v":"6","t":0}]}

*/
static int dp_up_add_to_array(const char dp_key_sub_id[32], cJSON* dp_value_json_obj)
{
    cJSON* dp_value_json_array = NULL;
    char* tmp_sub_id = NULL;
    char* blank = "";

    if(dp_key_sub_id != NULL && ql_strlen(dp_key_sub_id) > SUB_DEV_ID_MAX)
    {
        ql_log_err(ERR_EVENT_TX_DATA, "subid len[%d] err",(int)ql_strlen(dp_key_sub_id));
        return -1;
    }

    tmp_sub_id = (dp_key_sub_id == NULL) ? blank : (char*)dp_key_sub_id;
   
    if(ql_mutex_lock(g_client_handle.dp_mutex))
    {
        if(g_client_handle.dp_total_json_obj == NULL)
        {
            g_client_handle.dp_total_json_obj = cJSON_CreateObject();
            if(NULL == g_client_handle.dp_total_json_obj)
            {
                ql_mutex_unlock(g_client_handle.dp_mutex);
                ql_log_err( ERR_EVENT_INTERFACE, "json obj err");
                return -1;
            }
        }

        dp_value_json_array = cJSON_GetObjectItem(g_client_handle.dp_total_json_obj, tmp_sub_id);
        
        if(dp_value_json_array == NULL)
        {
            dp_value_json_array = cJSON_CreateArray();
            if(NULL == dp_value_json_array)
            {        
                cJSON_Delete(g_client_handle.dp_total_json_obj);
                g_client_handle.dp_total_json_obj = NULL;
                ql_mutex_unlock(g_client_handle.dp_mutex);
                ql_log_err( ERR_EVENT_INTERFACE, "json val err");
                return -1;
            }
            cJSON_AddItemToObject(g_client_handle.dp_total_json_obj, tmp_sub_id, dp_value_json_array);
        }

        cJSON_AddItemToArray(dp_value_json_array, dp_value_json_obj);

        ql_mutex_unlock(g_client_handle.dp_mutex);
    }

    return 0;
}

static cJSON* dp_up_create_json_obj(type_u8 dpid, const char* value, type_u8 type)
{
    cJSON* json_dp_obj = NULL;

    if (!is_application_authed())
    {
        return NULL;
    }

    json_dp_obj = cJSON_CreateObject();
    if(NULL == json_dp_obj)
    {
        ql_log_err( ERR_EVENT_INTERFACE, "json obj err");
        return NULL;
    }

    cJSON_AddNumberToObject(json_dp_obj, "i", dpid);
    cJSON_AddStringToObject(json_dp_obj, "v", value);
    cJSON_AddNumberToObject(json_dp_obj, "t", type);

    return json_dp_obj;
}

static int dp_up_add_str(const char sub_id[32],     type_u8    dpid,  const char*    value, type_u8 type, type_u32 len)
{
    cJSON* json_dp_obj = NULL;

    if(value == NULL || ql_strlen(value) != len)
    {
        ql_log_err(ERR_EVENT_TX_DATA, "string len err");
        return -1;
    }
    
    json_dp_obj = dp_up_create_json_obj(dpid, value, type);
    if (json_dp_obj == NULL)
    {
        return -1;
    }

    return dp_up_add_to_array(sub_id, json_dp_obj);
}

static int dp_up_add_num(const char sub_id[32], type_u8 dpid, type_s32 value, type_u8 type)
{
    char i_str[12] = {0};
    int str_len = 0;
    cJSON* json_dp_obj = NULL;
    
    str_len = ql_snprintf(i_str, sizeof(i_str), "%d", value);

    return dp_up_add_str(sub_id, dpid, i_str, type, str_len);
}

int dp_up_add_int(const char sub_id[32],    type_u8    dpid,  type_s32    value)
{
    return dp_up_add_num(sub_id, dpid, value, DP_TYPE_INT);
}

int dp_up_add_bool (const char sub_id[32],     type_u8    dpid,  type_u8    value)
{
    if(value != 0 && value != 1)
    {
        ql_log_err( ERR_EVENT_TX_DATA, "bool invalid");
        return -1;
    }
    
    return dp_up_add_num(sub_id, dpid, (type_s32)value, DP_TYPE_BOOL);
}
int dp_up_add_enum(const char sub_id[32],      type_u8    dpid,  type_u8    value)
{
    return dp_up_add_num(sub_id, dpid, (type_s32)value, DP_TYPE_ENUM);
}

int dp_up_add_float(const char sub_id[32],      type_u8    dpid,  type_f32    value)
{
    int ret = 0;
    cJSON* json_sub = NULL, *json_dev = NULL;
    char i_str[16] = {0};

    ret = float2str(value, i_str, sizeof(i_str), 6);

    if(ret < 0)
    {
        return -1;
    }

    return dp_up_add_str(sub_id, dpid, i_str, DP_TYPE_FLOAT, ret);
}

int dp_up_add_string(const char sub_id[32],      type_u8    dpid, const char*    str,        type_u32  str_len)
{
    return dp_up_add_str(sub_id, dpid, str, DP_TYPE_STRING, str_len);
}

int dp_up_add_fault(const char sub_id[32],     type_u8    dpid, const char*    fault,     type_u32  fault_len)
{
    return dp_up_add_str(sub_id, dpid, fault, DP_TYPE_FAULT, fault_len);
}

int dp_up_add_binary(const char sub_id[32],  type_u8    dpid, const type_u8*    bin,      type_u32  bin_len)
{
    type_u32 str_len_need = bin_len * 4 / 3;
    char* p = NULL;
    type_u32 str_len_after_base = 0;

    if (!is_application_authed()) {
        return -1;
    }

    str_len_need = str_len_need + 8 - (str_len_need % 4);

    p = (char*)ql_malloc(str_len_need);

    if(p == NULL)
    {
        ql_log_err( ERR_EVENT_INTERFACE, "malc bin[%d] err", str_len_need);
        return -1;
    }
    else
    {
        ql_memset(p, 0, str_len_need);
    }
    
    str_len_after_base = iot_encode_base64( (char *)p, (type_size_t)str_len_need, (const char *)bin, bin_len);
    if ( str_len_after_base < 0 )
    {
        ql_free(p);
        return -1;
    }
    
    if (dp_up_add_str(sub_id, dpid, p, DP_TYPE_BIN, str_len_after_base) < 0)
    {
        ql_free(p);
        return -1;
    }
    
    ql_free(p);
    return 0;
}

/*##########################################################################*/

type_s32 iot_upload_dps (DATA_ACT_T data_act, type_u32* data_seq )
{
    int ret = 0;
    char * p = NULL;

    if (!is_application_authed()) {
        return -1;
    }

    #if (__SDK_TYPE__== QLY_SDK_LIB)
    *data_seq = get_cur_seq_num();
    #endif

    if(ql_mutex_lock(g_client_handle.dp_mutex))
    {
        if(NULL == g_client_handle.dp_total_json_obj)
        {
            ql_mutex_unlock(g_client_handle.dp_mutex);
            ql_log_err(ERR_EVENT_TX_DATA, "dp empty");
            return -1;
        }

        p = cJSON_PrintUnformatted(g_client_handle.dp_total_json_obj);

        cJSON_Delete(g_client_handle.dp_total_json_obj);
        g_client_handle.dp_total_json_obj = NULL;
        
        ql_mutex_unlock(g_client_handle.dp_mutex);
    }

    if(NULL == p)
    {
       return -1;
    }
    g_client_handle.data_act = data_act;

    ret = cloud_request_send_pkt(PKT_ID_REQ_UPLOAD_DPS, *data_seq, (void*)p);
    ql_free(p);

    return ret;
}

type_s32 iot_tx_data ( const char sub_id[32], type_u32* data_seq, const type_u8* data, type_u32 data_len )
{
    push_tx_pkt_t *    push_tx_pkt_data = NULL;
    type_u32           sub_id_len = 0;
    type_u8 *          sub_id_data = NULL;
    int ret = 0;
    if (!is_application_authed()) 
    {
        return -1;
    }
    push_tx_pkt_data=(push_tx_pkt_t *)ql_malloc(sizeof(push_tx_pkt_t));
    if(push_tx_pkt_data == NULL)
    {
        ql_log_err(ERR_EVENT_INTERFACE, "malc push_d err!");
        return -1;
    }

    if(sub_id == NULL)
    {
        ql_memset(g_client_handle.sub_dev_id, 0, SUB_DEV_ID_MAX); 
    }
    else
    {
        if(ql_strlen(sub_id) <= 0 || ql_strlen(sub_id) > SUB_DEV_ID_MAX)
        {
            ql_log_err(ERR_EVENT_TX_DATA, "subid err: %d", (int)ql_strlen(sub_id));
            ql_free(push_tx_pkt_data);
            return -1;
        }        
        ql_memcpy(g_client_handle.sub_dev_id, sub_id, SUB_DEV_ID_MAX); 

        sub_id_len = ql_strlen(sub_id);
        sub_id_data = (type_u8 *)sub_id;
    }
    
    #if (__SDK_TYPE__== QLY_SDK_LIB)
        *data_seq = get_cur_seq_num();
    #endif

    push_tx_pkt_data->token_len = ql_strlen(g_client_handle.iottoken);
    push_tx_pkt_data->token = g_client_handle.iottoken;
    push_tx_pkt_data->sub_id_len =sub_id_len;
    push_tx_pkt_data->sub_id = sub_id_data;
    push_tx_pkt_data->data_len = data_len;
    push_tx_pkt_data->data = data;
    push_tx_pkt_data->pkt_len = push_tx_pkt_data->token_len + sub_id_len + data_len + 6; //6 = 1B token + 1B sub_len + 4B data_len

    ret = cloud_request_send_pkt(PKT_ID_REQ_TX_DATA_V2, *data_seq, (void*)push_tx_pkt_data);
    ql_free(push_tx_pkt_data);

    return ret;
}

type_s32 iot_ota_option_set ( type_u32 expect_time, type_u32 chunk_size, OTA_TYPE type )
{
    static type_s32 is_option_seted = 0;
    int random_num = 0;

    if(is_option_seted == 1 || chunk_size > (g_client_handle.recvbuf_size - PADDING_BUFFER_SIZE) || chunk_size <= 0)
    {
        ql_log_err( ERR_EVENT_NULL, "ota opt:%d,ch_s:%d", is_option_seted, chunk_size);
        return -1;
    }

    if(type >= OTA_TYPE_MAX)
    {
        ql_log_err( ERR_EVENT_NULL, "ota type err:%d", type);
        g_client_handle.ota_type = OTA_TYPE_BASIC;
        return -1;
    }
    else
    {
        g_client_handle.ota_type = type;
    }

    //Verify whether it is a power of 2
    if((chunk_size > 0) && ((chunk_size & (chunk_size -1)) ==0) && (expect_time >= 120) && (expect_time <= 3600))
    {
        set_otaupgrade_checktime = expect_time;
        g_client_handle.ota_mcu_chunk_size = chunk_size;
        is_option_seted = 1;
    }
    else
    {
        ql_log_err( ERR_EVENT_NULL, "ota opt err,e:%d,c:%d", expect_time, chunk_size);
        return -1;
    }

    return random_num;
}

type_s32 bytes_to_int ( const type_u8 bytes[4] )
{
    type_s32 number = 0;
    ql_memcpy(&number, bytes, 4);
    return number;
}
type_u8 bytes_to_bool ( const type_u8 bytes[1] )
{
    return ( bytes[0] );
}
type_u8 bytes_to_enum ( const type_u8 bytes[1] )
{
    return ( bytes[0] );
}
type_f32 bytes_to_float ( const type_u8 bytes[4] )
{
    type_f32 number = 0;

    ql_memcpy(&number, bytes, 4);

    return number;
}

/*##########################################################################*/
void iot_status_set ( DEV_STATUS_T dev_status, type_u32 timeout )
{
    if( DEV_STA_FACTORY_RESET == dev_status )
    {
        if(!g_sdk_is_inited)
        {
            iot_status_cb(DEV_STA_FACTORY_RESET, 0);
        }
        
        if( cloud_rst_state_get() == RST_STATE_INIT )
        {
            if ( is_application_authed() )
            {
                cloud_request_send_pkt(PKT_ID_REQ_RST, get_cur_seq_num(), NULL);
                cloud_rst_state_set(RST_STATE_RST_ING, get_current_time(), timeout);
            }
            else
            {
                cloud_rst_state_set(RST_STATE_RST_FAIL, get_current_time(), 0);
            }
        }
        
    }
    else if(DEV_STA_BIND_PERMIT == dev_status)
    {   
        ql_log_info("permit_bind: %d\r\n",timeout);
        if( cloud_bind_state_get() == BIND_STATE_INIT )/*bind state*/
        {
            if ( is_application_authed() )
            {      
                udp_control(1);
                cloud_bind_state_set(BIND_STATE_ING,get_current_time(),timeout);
                #ifdef LOCAL_SAVE
                ql_local_log_to_cloud( LOG_TYPE_INFO, INFO_EVENT_PERMIT_BIND, NULL, get_current_time());
                #endif
            }
            else
            {
                cloud_bind_state_set(BIND_STATE_FAIL, get_current_time(),0);
            }
        }
    }
    else if( DEV_STA_UNBIND == dev_status )
    {    
        if(!g_sdk_is_inited)
        {
            iot_status_cb(DEV_STA_UNBIND, 0);
        }
        if( cloud_unbound_state_get() == UNB_STATE_INIT )/*unbound*/
        {
            if ( is_application_authed() )
            {
                cloud_request_send_pkt(PKT_ID_REQ_UNB, get_cur_seq_num(), NULL);
                cloud_unbound_state_set(UNB_STATE_UNB_ING, get_current_time(),timeout);
            }
            else
            {
                cloud_unbound_state_set(UNB_STATE_UNB_FAIL, get_current_time(), 0);
            }
        }
    }
}

#ifdef LOCAL_SAVE
extern local_config_t g_local_config;
/*##########################################################################*/
type_s32 iot_local_save( type_u32 data_len, const void* data )
{
    int cfg_len = 0, ret = -1;
    char* p = NULL;
    
    cfg_len = MEM_ALIGN_SIZE(data_len);// 4 bytes align
    cfg_len = sizeof(local_config_t) + cfg_len; 

    if(cfg_len > LOCAL_CONFIG_PAGE_SIZE)
    {
        return -1;
    }

    ret = ql_local_config_load();
    if (ret != 0) {
        ql_log_err( ERR_EVENT_LOCAL_DATA, "save local err");
        return -1;
    }

    p = ql_zalloc(cfg_len);
    if(p == NULL)
    {
        return -1;
    }
    g_local_config.cfg_user_data_len = data_len;

    ql_memcpy(p, &g_local_config, sizeof(local_config_t));
    ql_memcpy(p + sizeof(local_config_t), data, data_len);

    ret = ql_local_data_write(p, cfg_len);

    ql_free(p);

    return ret;
}

type_s32 iot_local_load( type_u32 data_len, void* data )
{
    int cfg_len = 0, ret = -1;
    char* p = NULL;
    
    if( data == NULL )
    {
        return -1;
    }

    ret = ql_local_config_load();
    if (ret != 0) {
        ql_log_err(ERR_EVENT_LOCAL_DATA, "load local err");
        return -1;
    }

    if( g_local_config.cfg_user_data_len < data_len )
    {        
        return -1;
    }
    
    cfg_len = MEM_ALIGN_SIZE(data_len);
    cfg_len = sizeof(local_config_t) + cfg_len; 
    
    p = ql_zalloc(cfg_len);
    if(p == NULL)
    {        
        return -1;
    }

    ret = ql_local_data_read(p, cfg_len);
    if( 0 == ret )
    {
        ql_memcpy(data, p + sizeof(local_config_t), data_len);
    }
    
    ql_free(p);   

    return ret;
}

type_s32 iot_local_reset( void )
{
    int cfg_len = 0, ret = -1;
    char* p = NULL;
    
    cfg_len = sizeof(local_config_t); 

    p = ql_zalloc(cfg_len);
    if(p == NULL)
    {
        return -1;
    }

    g_local_config.cfg_user_data_len = 0;
    ql_memcpy(p, &g_local_config, sizeof(local_config_t));
    ret = ql_local_data_write(p, cfg_len);

    ql_free(p);
    
    return ret;
}
#endif

type_s32 sub_dev_add(const char sub_id[32], const char sub_name[32], const char sub_version[5], type_u16 sub_type)
{
    cJSON* sub_value_json_obj = NULL;

    if (!is_application_authed()) 
    {
        return -1;
    }

    if( sub_id == NULL || sub_name == NULL || sub_version == NULL )
    {
        ql_log_err(ERR_EVENT_LOCAL_DATA, "para NULL");
        return -1;
    }

    if( ql_strlen(sub_id) <= 0 || ql_strlen(sub_id) > SUB_DEV_ID_MAX)
    {
        ql_log_err(ERR_EVENT_LOCAL_DATA, "subid len err:%d", (int)ql_strlen(sub_id));
        return -1;
    }
    if( ql_strlen(sub_name) > 32 )
    {
         ql_log_err(ERR_EVENT_LOCAL_DATA,"subname len err");
        return -1;
    }
    if( ql_strlen(sub_version) > 5 )
    {
        ql_log_err(ERR_EVENT_LOCAL_DATA,"subver len err");
        return -1;
    }

    if(ql_mutex_lock(g_client_handle.sub_mutex))
    {
        if(g_client_handle.sub_total_json_arr == NULL) 
        {
            g_client_handle.sub_total_json_arr = cJSON_CreateArray();
            if(NULL == g_client_handle.sub_total_json_arr )
             {
                ql_mutex_unlock(g_client_handle.sub_mutex);
                ql_log_err(ERR_EVENT_INTERFACE, "sub_js arr err");
                return -1;
             }
        }

        sub_value_json_obj = cJSON_CreateObject();
        if(NULL == sub_value_json_obj)
        {
            cJSON_Delete(g_client_handle.sub_total_json_arr);
            g_client_handle.sub_total_json_arr = NULL;
            ql_mutex_unlock(g_client_handle.sub_mutex);
            ql_log_err(ERR_EVENT_INTERFACE,"sub_js obj err");
            return -1;
        }

        cJSON_AddStringToObject(sub_value_json_obj, "subid", sub_id);
        cJSON_AddStringToObject(sub_value_json_obj, "name", sub_name);
        cJSON_AddStringToObject(sub_value_json_obj, "v", sub_version);
        cJSON_AddNumberToObject(sub_value_json_obj, "type", sub_type);

        cJSON_AddItemToArray(g_client_handle.sub_total_json_arr, sub_value_json_obj);        
        ql_mutex_unlock(g_client_handle.sub_mutex);
    }
    return 0;
}

type_s32 iot_sub_dev_active(SUB_OPT_TYPE_T opt_type, type_u32* data_seq )
{
    int ret = 0;
    char * p = NULL;

    if (!is_application_authed()) 
    {
        return -1;
    }

    if(opt_type < 0 || opt_type > 1)
    {
        ql_log_err(ERR_EVENT_LOCAL_DATA,"act_type err");
        return -1;
    }
#if (__SDK_TYPE__== QLY_SDK_LIB)
    *data_seq = get_cur_seq_num();
#endif

    if(ql_mutex_lock(g_client_handle.sub_mutex))
    {
        if(NULL == g_client_handle.sub_total_json_arr)
        {
            ql_mutex_unlock(g_client_handle.sub_mutex);
            ql_log_err(ERR_EVENT_TX_DATA, "sub empty");
            return -1;
        }
        
        g_client_handle.sub_opt_type = opt_type;
        p = cJSON_PrintUnformatted(g_client_handle.sub_total_json_arr);

        cJSON_Delete(g_client_handle.sub_total_json_arr);
        g_client_handle.sub_total_json_arr = NULL;
        
        ql_mutex_unlock(g_client_handle.sub_mutex);
    }

    if(NULL == p)
    {
       return -1;
    }

    ret = cloud_request_send_pkt(PKT_ID_REQ_SUB_ACT, *data_seq, (void*)p);
    ql_free(p);
    p = NULL;
    return ret;
}

type_s32 iot_sub_dev_inactive(const char sub_id[32], SUB_OPT_TYPE_T opt_type, type_u32* data_seq )
{
    int ret = 0;
    char* p = NULL;
    cJSON* json_sub = NULL;
    cJSON* in_json_arr = NULL;

    if (!is_application_authed()) 
    {
        return -1;
    }

    if( sub_id == NULL )
    {
        return -1;
    }

    if( ql_strlen(sub_id) <= 0 || ql_strlen(sub_id) > SUB_DEV_ID_MAX)
    {
        ql_log_err(ERR_EVENT_RX_DATA, "subid len err");
        return -1;
    }

    if(opt_type < 0 || opt_type > 1)
    {
        ql_log_err(ERR_EVENT_LOCAL_DATA,"act_type err");
        return -1;
    }

    if(in_json_arr  == NULL)
    {
        in_json_arr = cJSON_CreateArray();
        if(NULL == in_json_arr )
         {
             ql_log_err(ERR_EVENT_INTERFACE, "inact arr err");
             return -1;
         }
    }

    json_sub = cJSON_CreateObject();
    if(NULL == json_sub)
    {
        cJSON_Delete(in_json_arr);
        ql_log_err(ERR_EVENT_INTERFACE, "inact sub err");
        return -1;
    }

    #if (__SDK_TYPE__== QLY_SDK_LIB)
        *data_seq = get_cur_seq_num();
    #endif    
    json_sub = cJSON_CreateString(sub_id);
    cJSON_AddItemToArray(in_json_arr, json_sub);

    p = cJSON_PrintUnformatted(in_json_arr);
    cJSON_Delete(in_json_arr);
    in_json_arr = NULL;

    if(NULL == p)
    {
       return -1;
    }
    
    g_client_handle.sub_opt_type = opt_type;
    ret = cloud_request_send_pkt(PKT_ID_REQ_SUB_INACT, *data_seq, (void*)p);
    ql_free(p);
    return ret;
}

#define LOCAL_LOG_EVENT_DEVICE      "%d-%d-%s-%s"
#define LOCAL_LOG_EVENT_TIME        "%d"
#define LOCAL_LOG_EVENT_STATU       "%c"
#define LOCAL_LOG_EVENT_INFO        "%s"
#define LOCAL_LOG_EVENT_DISCONN     LOCAL_LOG_EVENT_INFO
#define LOCAL_LOG_EVENT_CONN_EXC    "%d-%d"
#ifdef LOCAL_SAVE
type_s32 log_up_add(uint8_t type, uint32_t event, void * data, uint32_t ts)
{
    cJSON* log_value_json_obj = NULL;
    char info[30] ;
    ql_memset(info,0x00,sizeof(info));
        
    if (!is_application_authed()) 
    {
        return -1;
    }
    
    if(type < LOG_TYPE_INFO || type > LOG_TYPE_ERR)
        return -1;

    if((type == LOG_TYPE_ERR && data == NULL)||(type == LOG_TYPE_INFO && event == INFO_EVENT_OTA_TS && data == NULL)||
        (type == LOG_TYPE_WARN && data == NULL))
        return -1;

    if(ql_mutex_lock(g_client_handle.log_mutex))
    {
        if(g_client_handle.log_total_json_arr == NULL)
        {
            g_client_handle.log_total_json_arr = cJSON_CreateArray();
            if(NULL == g_client_handle.log_total_json_arr)
            {
                ql_mutex_unlock(g_client_handle.log_mutex);
                ql_log_err( ERR_EVENT_INTERFACE, "log_js arr err");                
                /*本地资源不足时，将原因直接存入本地*/
                ql_local_log_save( LOG_TYPE_ERR, ERR_EVENT_LOG_IF, "log_js arr err", get_current_time());
                
                /*非本地化存储的数据不存*/
                if(type == LOG_TYPE_ERR || (type == LOG_TYPE_WARN && event != WARN_EVENT_P_BIND_EXC))
                   ql_local_log_save( type, event, data, ts);
                return -1;
            }
        }

        log_value_json_obj = cJSON_CreateObject();
        if(NULL == log_value_json_obj)
        {                    
            ql_log_err(ERR_EVENT_NULL,"log obj err");        
            /*本地资源不足时，将原因直接存入本地*/
            ql_local_log_save( LOG_TYPE_ERR, ERR_EVENT_LOG_IF, "log obj err", get_current_time());
            
            /*非本地化存储的数据不存*/
            if(type == LOG_TYPE_ERR || (type == LOG_TYPE_WARN && event != WARN_EVENT_P_BIND_EXC))
                ql_local_log_save( type, event, data, ts);
            
            cJSON_Delete(g_client_handle.log_total_json_arr);
            g_client_handle.log_total_json_arr = NULL;
            ql_mutex_unlock(g_client_handle.log_mutex);
            return -1;
        }
    
    
        if(LOG_TYPE_INFO == type)
        {
            switch(event)
            {
            case INFO_EVENT_DEV:
                ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_DEVICE, __SDK_TYPE__, __SDK_PLATFORM__, DEVICE_VERSION, __DATE__);
                break;
            case INFO_EVENT_PERMIT_BIND:
                ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_TIME, cloud_bind_timeout_get());
                break;
            case INFO_EVENT_OTA_TS:   
                ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_TIME, *(int *)data);
                break;
            default:
                break;
            }
        }
        else if(LOG_TYPE_WARN == type)
        {
            switch(event)
            {
            case WARN_EVENT_DISCONN:  
                if(*(CLI_STATU_T *)data == CLI_DISCONN_TYPE_REG_ERR)
                    ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_DISCONN, "reg err");
                else if(*(CLI_STATU_T *)data == CLI_DISCONN_TYPE_NETWK_POOR)
                    ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_DISCONN, "net poor");
                else if(*(CLI_STATU_T *)data == CLI_DISCONN_TYPE_RD_CLS)
                    ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_DISCONN, "sock read err");
                else if(*(CLI_STATU_T *)data == CLI_DISCONN_TYPE_WRT_CLS)
                    ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_DISCONN, "sock write err");
                else if(*(CLI_STATU_T *)data == CLI_DISCONN_TYPE_UNB_END)
                    ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_DISCONN, "unb sucess");
                else if(*(CLI_STATU_T *)data == CLI_DISCONN_TYPE_UNB_FAIL)
                    ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_DISCONN, "unb F/T");
                else if(*(CLI_STATU_T *)data == CLI_DISCONN_TYPE_RST_LINK)
                    ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_DISCONN, "rst link");
                break;
            case WARN_EVENT_CONN_EXC:
                ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_CONN_EXC, g_local_config.log_conn_exc.conn_exc_ts, g_local_config.log_conn_exc.last_rv_ts);
                break;
            case WARN_EVENT_UNB_EXC:            
                ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_STATU, *(char *)data);
                break;
            case WARN_EVENT_P_BIND_EXC:        
                ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_STATU, *(char *)data);
                break;
            case WARN_EVENT_RST_EXC:            
                ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_STATU, *(char *)data);
                break;
            default:
                break;
            }
        }
        else if(LOG_TYPE_ERR == type)
        {       
            if(ql_strlen(data) > LOG_ERR_LEN_MAX)
            {
                ql_log_err(ERR_EVENT_LOCAL_DATA,"log info err");
                cJSON_Delete(log_value_json_obj);
                log_value_json_obj = NULL;
                
                cJSON_Delete(g_client_handle.log_total_json_arr);
                g_client_handle.log_total_json_arr = NULL;                
                ql_mutex_unlock(g_client_handle.log_mutex);
                return -1;
            }
            ql_snprintf(info,sizeof(info), LOCAL_LOG_EVENT_INFO, (char *)data);
        }
        
        
        cJSON_AddNumberToObject(log_value_json_obj, "t", type);
        cJSON_AddNumberToObject(log_value_json_obj, "e", event);
        if(ql_strlen(info))
        {
            cJSON_AddStringToObject(log_value_json_obj, "i", info);
        }   
        cJSON_AddNumberToObject(log_value_json_obj, "ts", ts);
        cJSON_AddNumberToObject(log_value_json_obj, "fm", 0);
        
        cJSON_AddItemToArray(g_client_handle.log_total_json_arr, log_value_json_obj);
    
        ql_mutex_unlock(g_client_handle.log_mutex);
    }

    return 0;
}
#endif
type_s32 log_upload_dps( type_u32* data_seq )
{
    int ret = 0;
    char * p = NULL;

    if (!is_application_authed()) 
    {
        return -1;
    }

#if (__SDK_TYPE__== QLY_SDK_LIB)
    *data_seq = get_cur_seq_num();
#endif

    if(ql_mutex_lock(g_client_handle.log_mutex))
    {
        if(NULL == g_client_handle.log_total_json_arr)
        {
            ql_mutex_unlock(g_client_handle.log_mutex);
            ql_log_err(ERR_EVENT_TX_DATA, "log empty");
            return -1;
        }

        p = cJSON_PrintUnformatted(g_client_handle.log_total_json_arr);

        cJSON_Delete(g_client_handle.log_total_json_arr);
        g_client_handle.log_total_json_arr = NULL;
        
        ql_mutex_unlock(g_client_handle.log_mutex);
    }

    if(NULL == p)
    {
      return -1;
    }

    ret = cloud_request_send_pkt(PKT_ID_REQ_LOG, *data_seq, (void*)p);
    ql_free(p);
    return ret;
}

#if (__SDK_TYPE__ == QLY_SDK_BIN)
/*1 : ap配网模式     ; 0 : 非ap配网模式*/
type_u8 g_ap_smart_status;
void set_ap_smart_status(type_u8 status)
{
    g_ap_smart_status = status;
}
type_u8 get_ap_smart_status()
{
    return g_ap_smart_status;
}

type_s32 ap_smart(struct iot_context* ctx)
{
    static ql_thread_t *g_qlcloud_udp = NULL;
    udp_control(1);
    set_ap_smart_status(1);
    g_client_handle.product_id = ctx->product_id;
    
    if( qlcloud_get_mac_arr(g_client_handle.dev_mac_hex) != 0 )
    {
        ql_log_err(ERR_EVENT_INTERFACE, "[AP]mac err!");
        return -1;
    }
    g_qlcloud_udp = ql_thread_create(1, 1024 * 2, (ql_thread_fn)qlcloud_udp_server, NULL);
    if (NULL == g_qlcloud_udp) {
        ql_log_err(ERR_EVENT_INTERFACE, "[AP]thread_udp err");
        return -1;
    }
    return 0;
}
#endif

#if (__SDK_PLATFORM__ == 0x1C)

void iot_destroy_back_threads() {
    printf("iot_destroy_back_threads()\r\n");

    if (g_qlcloud_back_logic != NULL) {
        ql_thread_destroy(&g_qlcloud_back_logic);
    }

    if (g_qlcloud_back_time != NULL) {
        ql_thread_destroy(&g_qlcloud_back_time);
    }

#ifdef LAN_DETECT
    if (g_qlcloud_udp != NULL) {
        ql_thread_destroy(&g_qlcloud_udp);
    }
#endif
}

#endif

type_u16 iot_crc16_calc(type_u8* data, type_u32 data_len, type_u16 pre_crc)
{
    return crc16( data, data_len, pre_crc);
}
