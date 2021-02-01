#include "ql_include.h"
#include "ql_queue.h"
#include "ql_dispatch_access.h"
#include "MQTTClient.h"
#include "ql_sm4.h"
#include "ql_patch.h"

Network         mqttn;
Client mqttc =  DefaultClient;
struct client_handle g_client_handle = {0};
MQTTPacket_connectData conndata = MQTTPacket_connectData_initializer;

type_u32 os_tick = 0;                   //Unit s
static type_u32 g_time_tick = 0;        //Unit s
ql_queue *g_queue = NULL;
extern ota_upgrade_ctx g_ota;
type_u32 set_otaupgrade_checktime = 0;
static type_u16 g_ota_cmd_msg_id = 0;
static type_s32 needclose_old = 0;
#ifdef LOCAL_SAVE
ql_mutex_t* g_local_config_mutex = NULL;
//local config must keep the same with persistence data
local_config_t g_local_config;

#endif
type_u8  g_ota_chunk_retran_time   = 0;
type_u32 g_ota_chunk_retran_seq    = 0;
type_u32 g_ota_chunk_retran_offset = 0;

void mqttRecv_handle(MessageData* md);
void cloud_disconnect_cb(CLI_STATU_T);

void free_event(struct cloud_event *e)
{
    if (e && e->ev_data) {
        ql_free(e->ev_data);
        e->ev_data = NULL;
    }
}

int direct_send_cloud(struct cloud_event* e)
{
    int rc = 0;
    MQTTMessage message;

    message.dup = 0;
    message.retained = 1;
    message.qos = (enum QoS)e->ev_qos;
    message.payload = e->ev_data;
    message.payloadlen = e->ev_len;
    rc = MQTTPublish(&mqttc, (const char*)g_client_handle.pub_topic, &message);

    return rc;
}

int send_cloud(int event_qos, int seq, char *data, int data_len)
{
    int ret = 0;
    struct cloud_event e = {0};

    e.ev_len  = data_len;
    e.ev_data = data;
    e.ev_qos  = event_qos;
    e.ev_seq  = seq;

    ret = en_ql_queue(g_queue, &e, sizeof(struct cloud_event));

    if (ret != 0) {
        free_event(&e);
    }

    return ret;
}

type_u8 DEVICE_VERSION[6]; /* for bin-firmware, version of lib is not fixed */

#if (__SDK_TYPE__== QLY_SDK_BIN)
static type_s8 data_point_map[256] = {0};
type_u32 bin_mcu_chunk_seq = 0, bin_mcu_chunk_offset = 0;
type_u32 bin_patch_chunk_seq = 0, bin_patch_chunk_offset = 0;
void ql_set_module_version(const type_u8  version[5])
{
    ql_memcpy(DEVICE_VERSION, version, 5);
    DEVICE_VERSION[5] = '\0';
}
extern void  binfile_chunk_response(type_u32 seq, type_s32 error, type_u32 offset, type_s8 owner);

void ql_reset_link(type_u32 product_id, const type_u8  product_key[16], const type_u8  mcu_version[5])
{
    g_client_handle.product_id = product_id;   
    #ifdef USE_SMPU
    ql_sec_memcpy(g_client_handle.product_aes_key, product_key, AESKEY_LEN);
    #else
    ql_memcpy(g_client_handle.product_aes_key, product_key, AESKEY_LEN);
    #endif
    ql_memcpy(g_client_handle.mcu_version, mcu_version, 5);

    if(g_client_handle.dev_status != 1)
    {
        iot_status_cb(DEV_STA_DISCONN_CLOUD, get_current_time());
        g_client_handle.dev_status = 1;
    }
    needclose_old = 1;
    ql_client_set_status( NET_MQTT_DISCONNECT, CLI_DISCONN_TYPE_RST_LINK);//reconnect from dispatch
}

void ql_chunk_response(type_u8 chunk_stat, type_s32 error, type_u32 offset)
{
    if(IOT_CHUNK_TYPE(chunk_stat) == IOT_CHUNK_OTA) {
        if(offset != bin_mcu_chunk_offset)
        {
            ql_log_err(ERR_EVENT_NULL, "%s,%d\r\n", __func__, __LINE__);
            error = -1;
        }
        binfile_chunk_response(bin_mcu_chunk_seq, error, offset, OTA_OWNER_MCU);
    }
    else {
        if(offset != bin_patch_chunk_offset)
        {
            ql_log_err(ERR_EVENT_NULL, "%s,%d\r\n", __func__, __LINE__);
            error = -1;
        }
        patch_chunk_response(bin_patch_chunk_seq, error, offset);
    }
    
}
type_s8 ql_get_dp_type(type_u8 dpid)
{
    if(dpid<0 || dpid>255)
    {
        return -1;
    }
    return data_point_map[dpid];
}
void ql_set_dp_type(type_s8 dp_map[256])
{
    ql_memset(data_point_map, 0, sizeof(data_point_map));
    ql_memcpy(data_point_map, dp_map, sizeof(data_point_map));
}

void set_ota_wifi_status(type_s32 type)
{
    g_client_handle.ota_wifi_status = type;
    g_client_handle.ota_wifi_stat_chg_tm = os_tick;
    set_download_status( (type == OTA_INIT)?DEV_DOWNLOAD_FREE:DEV_DOWNLOAD_OTA );
}

type_s32 qlcloud_ota_wifi_chunk_cb ( iot_u8  chunk_is_last, iot_u32 chunk_offset, iot_u32 chunk_size, const iot_s8*  chunk )
{
    #ifndef DEBUG_WIFI_OTA_OPEN
        return fota_chunk_recv(chunk_is_last, chunk_offset, chunk_size, chunk);
    #else
        ql_log_info("get module chunk offset = %d, size = %d\r\n", chunk_offset, chunk_size);

        if (chunk_is_last) {
            ql_log_info("get module ota chunks over, wait to reboot...\r\n");
        }
        return 0;
    #endif
}

void qlcloud_ota_wifi_upgrade_cb ( void )
{
    #ifndef DEBUG_WIFI_OTA_OPEN
        fota_upgrade();
    #else
        ql_log_info("get module ota cmd success,reboot right now\r\n");
    #endif
}

void iot_ota_wifi_info_cb( char name[16], char version[6], iot_u32 total_len, iot_u16 file_crc )
{
    printf("firmware [%s], version [%s], len [%d] crc [%d]will start upgrade\r\n", name, version, total_len, file_crc);
}

#endif


void cloud_rst_state_set(RST_STATE_t rst_state, type_u32 rst_time, type_u32 rst_timeout)
{
    g_client_handle.dev_rst_status = rst_state;
    g_client_handle.dev_rst_timestamp = rst_time;
    g_client_handle.dev_rst_timeout = rst_timeout;
}
RST_STATE_t cloud_rst_state_get(void)
{
    return (RST_STATE_t)g_client_handle.dev_rst_status;
}

type_u32 cloud_rst_time_get(void)
{
    return g_client_handle.dev_rst_timestamp;
}
type_u32 cloud_rst_timeout_get(void)
{
    return g_client_handle.dev_rst_timeout;
}



/*bind state*/
void cloud_bind_state_set(BIND_STATE_t bind_state, type_u32 bind_time,type_u32 bind_timeout)
{
    g_client_handle.dev_bind_status = bind_state;
    g_client_handle.dev_bind_timestamp = bind_time;
    g_client_handle.dev_bind_timeout = bind_timeout;
}
BIND_STATE_t cloud_bind_state_get(void)
{
    return (BIND_STATE_t)g_client_handle.dev_bind_status;
}
type_u32 cloud_bind_time_get(void)
{
    return g_client_handle.dev_bind_timestamp;
}
type_u32 cloud_bind_timeout_get(void)
{
    return g_client_handle.dev_bind_timeout;
}

/*unbound*/
void cloud_unbound_state_set(UNBOUND_STATE_t unb_state, type_u32 unb_time,type_u32 unb_timeout)
{
    g_client_handle.dev_unb_status = unb_state;
    g_client_handle.dev_unb_timestamp = unb_time;
    g_client_handle.dev_unb_timeout = unb_timeout;
}
UNBOUND_STATE_t cloud_unbound_state_get(void)
{
    return (UNBOUND_STATE_t)g_client_handle.dev_unb_status;
}

type_u32 cloud_unbound_time_get(void)
{
    return g_client_handle.dev_unb_timestamp;
}
type_u32 cloud_unbound_timeout_get(void)
{
    return g_client_handle.dev_unb_timeout;
}



void set_current_time(type_u32 tick)
{
    g_time_tick = tick;
    ql_srand(g_time_tick);
}

type_u32 get_current_time(void)
{
    return g_time_tick ;
}

int ql_client_get_status(void)
{
    return g_client_handle.client_status;
}
type_s32 ql_client_get_encrypt_type(void)
{
    return g_client_handle.encrypt_type;
}
extern void ql_local_log_to_cloud(uint8_t type, uint8_t event, void * data, uint32_t ts);

void ql_client_set_status(int client_status, CLI_STATU_T statu_type)
{
    g_client_handle.client_status = client_status;
#ifdef LOCAL_SAVE
    if(client_status == NET_MQTT_DISCONNECT)        
        ql_local_log_to_cloud(LOG_TYPE_WARN, WARN_EVENT_DISCONN, &statu_type, get_current_time());
#endif
}

type_s32 is_application_authed(void)
{
    return (g_client_handle.client_status >= NET_AUTHED) ? 1 : 0;
}

void set_ota_mcu_status(type_s32 type)
{
    g_client_handle.ota_mcu_status = type;
    g_client_handle.ota_mcu_stat_chg_tm = os_tick;
    set_download_status( (type == OTA_INIT)?DEV_DOWNLOAD_FREE:DEV_DOWNLOAD_OTA );
}

type_s32 get_ota_mcu_status()
{
    return g_client_handle.ota_mcu_status;
}

void set_ota_cur_owner(type_u8 owner)
{
    g_client_handle.ota_cur_owner = owner;
}

type_u8 get_ota_cur_owner( void )
{
    return g_client_handle.ota_cur_owner;
}

int cloud_request_send_pkt(int pkt_id, type_u32 seq, void* arg)
{
    int data_len = 0, pub_qos = QOS0;
    int ret = -1;
    //new memory and send to queue,delete after sent
    cloud_packet_t* pkt = (cloud_packet_t*)create_lib_request(pkt_id, seq, arg);

    if(pkt_id == PKT_ID_RES_OTAUPGRADE_CMD)
    {
        if(*(int*)arg == 0)
        {
            pub_qos = QOS1;
        }
    }

    if(pkt)
    {
        data_len = pkt->pkt_len;
        pkt->pkt_len = ql_htonl(pkt->pkt_len);
        ret = send_cloud(pub_qos, seq, (char *)pkt, data_len);
        if(ret == 0)
            ql_log_info("send pkt op:%x, seq:%d, len:%d\r\n", PKT_OP_CODE(pkt_id), seq, data_len);
    }
    else
    {
        ret =  -1;
    }
    return ret;
}
void  cloud_request_ota_check(type_s32 ota_owner)
{                
    if(cloud_request_send_pkt(PKT_ID_REQ_OTATRIGGER, get_cur_seq_num(), (void*)&ota_owner) == 0)
    {
        if(ota_owner == OTA_OWNER_MCU)
        {
                set_ota_mcu_status(OTA_VERSION_CHECK);
        }
        #if (__SDK_TYPE__== QLY_SDK_BIN)
        else
        {
            set_ota_wifi_status(OTA_VERSION_CHECK);
        }
        #endif
    }
}
void  passive_start_response(type_s32 seq, type_s32 error)
{
    cloud_request_send_pkt(PKT_ID_RES_OTAPASSIVE, seq, (void*)&error);
}
void  compels_upgrade_response(type_s32 seq, type_s32 error)
{
    cloud_request_send_pkt(PKT_ID_RES_OTA_COMPELS, seq, (void*)&error);
}
void  binfile_info_response(type_s32 seq, type_s32 error, type_u32 owner)
{
    if(cloud_request_send_pkt(PKT_ID_RES_OTAFILE_INFO, seq, (void*)&error) == 0)
    {
        if (error != -2) {
            if(error == 2) {//最新版本已经下载完成
                if(owner == OTA_OWNER_MCU)
                {
                    set_ota_mcu_status(OTA_FILECHUNK_END_BACK);
                }
                #if (__SDK_TYPE__== QLY_SDK_BIN)
                else
                {
                    set_ota_wifi_status(OTA_FILECHUNK_END_BACK);
                }
                #endif//END SDK BIN
            } else if (error == 1) {//当前运行版本是最新的，不需要更新
                if(owner == OTA_OWNER_MCU)
                {
                    set_ota_mcu_status(OTA_INIT);
                }
                #if (__SDK_TYPE__== QLY_SDK_BIN)
                else
                {
                    set_ota_wifi_status(OTA_INIT);
                }
                #endif//END SDK BIN
            } else if (error == -1) {//云端出错
                if(owner == OTA_OWNER_MCU)
                {
                    set_ota_mcu_status(OTA_INIT);
                }
                #if (__SDK_TYPE__== QLY_SDK_BIN)
                else
                {
                    set_ota_wifi_status(OTA_INIT);
                }
                #endif//END SDK BIN 
                ql_log_err( ERR_EVENT_NULL, "cloud err");
            }else {//error==0
                if(owner == OTA_OWNER_MCU)
                {
                    set_ota_mcu_status(OTA_FILEINFO_BACK);
                }
                #if (__SDK_TYPE__== QLY_SDK_BIN)
                else
                {
                    set_ota_wifi_status(OTA_FILEINFO_BACK);
                }
                #endif//END SDK BIN
            }
        }
    }
}

void  binfile_chunk_response(type_u32 seq, type_s32 error, type_u32 offset, type_s8 owner)
{
    ota_res_arg arg;
    arg.error  = error;
    arg.offset = offset;
    arg.owner  = owner;


    if(cloud_request_send_pkt(PKT_ID_RES_OTAFILE_CHUNK, seq, (void*)&arg) == 0)
    {
        if (error != -2) {
            if(OTA_OWNER_MCU == owner)
            {
                switch (g_client_handle.ota_mcu_status) {
                    case OTA_FILECHUNK_BEGIN:
                    set_ota_mcu_status(OTA_FILECHUNK_BEGIN_BACK);
                    break;
                case OTA_FILECHUNK_STREAM:
                    set_ota_mcu_status(OTA_FILECHUNK_STREAM_BACK);
                    break;
                case OTA_FILECHUNK_END:
                    set_ota_mcu_status(OTA_FILECHUNK_END_BACK);
                    #ifdef LOCAL_SAVE
                    ql_local_ota_ver_set(owner, g_ota.otafileinfo.version[OTA_OWNER_MCU], OTA_FILECHUNK_END);
                    #endif
                    break;
                default:
                    break;
                }
            }
            #if (__SDK_TYPE__== QLY_SDK_BIN)
            else
            {
                switch (g_client_handle.ota_wifi_status) {
                    case OTA_FILECHUNK_BEGIN:
                    set_ota_wifi_status(OTA_FILECHUNK_BEGIN_BACK);
                    break;
                case OTA_FILECHUNK_STREAM:
                    set_ota_wifi_status(OTA_FILECHUNK_STREAM_BACK);
                    break;
                case OTA_FILECHUNK_END:
                    set_ota_wifi_status(OTA_FILECHUNK_END_BACK);
                    ql_local_ota_ver_set(owner, g_ota.otafileinfo.version[OTA_OWNER_WIFI],OTA_FILECHUNK_END);
                    break;
                default:
                    break;
                }
            }
            #endif
        }
}
}
void  binfile_upgrade_response(type_s32 seq, type_s32 error)
{
    cloud_request_send_pkt(PKT_ID_RES_OTAUPGRADE_CMD, seq, (void*)&error);
}
void  appdown_start_response(type_s32 seq, type_s32 error)
{
    cloud_request_send_pkt(PKT_ID_RES_OTA_APPDOWN, seq, (void*)&error);
}

void  cloud_ota_cmd_msg_id_cmp(type_u16 mq_msg_id)
{
    if(mq_msg_id == g_ota_cmd_msg_id){
        if(get_ota_cur_owner() == OTA_OWNER_MCU)
        {
            set_ota_mcu_status(OTA_UPGRADE_CMD_BACK);
            #ifdef LOCAL_SAVE
                ql_memset(&g_local_config.cfg_brk_ota_info, 0x00, sizeof(binfile_info));
                g_local_config.cfg_brk_cur_offset = 0;
                g_local_config.cfg_brk_cur_crc = 0;
                if(ql_local_break_ota_info_set( LOCAL_BREAK_OTA_SET_T_ALL ) == -1)
                {
                    ql_log_err(ERR_EVENT_INTERFACE, "ota up err");
                    return;
                }
            #endif
            iot_ota_upgrade_cb();
        }
        #if (__SDK_TYPE__== QLY_SDK_BIN)
        else if(get_ota_cur_owner() == OTA_OWNER_WIFI)
        {
            set_ota_wifi_status(OTA_UPGRADE_CMD_BACK);

            qlcloud_ota_wifi_upgrade_cb();
        }
        #endif
    }
    else
    {
        ql_log_err( ERR_EVENT_RX_DATA,"ota cmd err");
    }
}
void  cloud_ota_cmd_msg_id_set(type_u16 mq_msg_id)
{
    g_ota_cmd_msg_id = mq_msg_id;
}

#ifdef DEVID_USE_IMEI
void _set_topic_info(type_u8 imid[18])
{
    ql_snprintf((char *)g_client_handle.pub_topic, sizeof(g_client_handle.pub_topic), "iot/p/%d/%s", g_client_handle.product_id, imid);
    ql_snprintf((char *)g_client_handle.sub_topic, sizeof(g_client_handle.sub_topic), "iot/s/%d/%s", g_client_handle.product_id, imid);
    return;
}
#else
void _set_topic_info(type_u8 mac[6])
{
    int halfone = mac_up_3(&mac[0]);
    int halfsecond = mac_up_3(&mac[3]);
    ql_snprintf((char *)g_client_handle.pub_topic, sizeof(g_client_handle.pub_topic), "d/p/%d-%d", halfone, halfsecond);
    ql_snprintf((char *)g_client_handle.sub_topic, sizeof(g_client_handle.sub_topic), "d/s/%d-%d", halfone, halfsecond);
    return;
}
#endif
int qlcloud_init_client_bymac(char mac_str[18])
{
#ifdef DEVID_USE_IMEI
    int ret = 0;
    ret = qlcloud_get_imei_id(mac_str);
    if(ret < 0)
    {
        ql_log_err( ERR_EVENT_NULL, "imei err");
        return -1;
    }
    mac_str[15] = '\0';
    
    _set_topic_info(mac_str);
#else

    while( qlcloud_get_mac_arr(g_client_handle.dev_mac_hex) != 0 )
    {
        ql_sleep_s(1);
    }
    
    _set_topic_info(g_client_handle.dev_mac_hex);
    
    ql_memset(mac_str, 0, 18);
    ql_snprintf(mac_str, 18, MACSTR, MAC2STR(g_client_handle.dev_mac_hex));
    mac_str[17] = '\0';
#endif
    return 0;
}
int qlcloud_is_version_valid(const type_u8  mcu_version[5])
{
    int i = 0;

    for(i = 0; i < 5; i++)
    {
        if(i == 2)
        {
            if(mcu_version[i] != '.')
            {
                break;
            }
        }
        else
        {
            if(mcu_version[i] < '0' || mcu_version[i] > '9')
            {
                break;
            }
        }
    }
    if(i != 5) return -1;
    return 0;
}
int qlcloud_is_addr_valid(char* addr)
{
    char* p = addr;
    if(addr == NULL)
    {
        return -1;
    }
    
    while(*p != '\0')
    {
        p++;
    }
    
    if(p - addr < 5)//x.x.x
    {
        return -1;
    }
    return 0;
}
int qlcloud_is_sn_valid(char* sn)
{
    char* p = sn;
    
    if(sn == NULL)
    {
        return -1;
    }
    
    while(*p != '\0')
    {
        if((*p >= '0' && *p <= '9')||(*p >= 'a' && *p <= 'z')||(*p >= 'A' && *p <= 'Z'))
        {
            p++;
        }
        else
        {
            return -1;
        }
    }
    
    if(p-sn == 0 || p-sn > 16)
    {
        ql_log_err( ERR_EVENT_LOCAL_DATA, "sn len[%d]", (int)(p-sn));
        return -1;
    }
    
    return 0;
}
void qlcloud_init_ota_status(type_s8 ota_owner, char local_ota_ver[5], char running_ota_ver[5])
{
    int ret = 0;
    #ifdef DEBUG_DATA_OPEN
    char local[6] = {0};
    char run[6] = {0};
    ql_memcpy(local, local_ota_ver,5);
    ql_memcpy(run, running_ota_ver,5);
    ql_printf("init owner:%d,run:%s,local:%s\r\n", ota_owner, run, local);
    #endif

    ret = ql_memcmp(local_ota_ver, running_ota_ver, 5);
    if(ret > 0)//local_ota_ver > running_ota_ver
    {
        if(OTA_OWNER_MCU == ota_owner)
        {
            set_ota_mcu_status(OTA_FILECHUNK_END_BACK);
        }
        #if (__SDK_TYPE__== QLY_SDK_BIN)
        else
        {
            set_ota_wifi_status(OTA_FILECHUNK_END_BACK);
        }
        #elif (__SDK_TYPE__== QLY_SDK_LIB)
        else
        {
            //do nothing
        }
        #endif
    }
    else //local_ota_ver <= running_ota_ver
    {
        if(OTA_OWNER_MCU == ota_owner)
        {
            set_ota_mcu_status(OTA_INIT);
        }
        #if (__SDK_TYPE__== QLY_SDK_BIN)
            else//ota_owner==wifi
            {
                set_ota_wifi_status(OTA_INIT);
            }
        #elif (__SDK_TYPE__== QLY_SDK_LIB)
            else//ota_owner==wifi
            {
                //do nothing
            }
        #endif
        if(ret < 0) //local_ota_ver < running_ota_ver   first run
        {
            ql_log_err( ERR_EVENT_LOCAL_DATA, "loc<run");
        }
    }
}

#ifdef USE_SMPU
SEC_DATA type_u8 g_product_aes_key[AESKEY_LEN+1];
SEC_DATA type_u8 g_device_aes_key[AESKEY_LEN+1];
SEC_DATA type_u8 g_server_aes_key[AESKEY_LEN+1];
#endif

int qlcloud_init_client_context(struct iot_context* ctx)
{
    int ret = 0;
    if(qlcloud_init_client_bymac((char*)g_client_handle.mac_str) < 0)
    {
        ql_log_err( ERR_EVENT_NULL, "mac err!");
        return -1;
    }

    if(qlcloud_is_version_valid(ctx->mcu_version) < 0)
    {
        ql_log_err( ERR_EVENT_NULL, "mcu ver err!");
        return -1;
    }

    if(qlcloud_is_addr_valid(ctx->server_addr) < 0)
    {
        ql_log_err( ERR_EVENT_NULL,"serv addr err!");
        return -1;
    }

    #ifdef USE_SMPU
    ql_memset(g_product_aes_key, 0, sizeof(g_product_aes_key));
    ql_memset(g_device_aes_key, 0, sizeof(g_device_aes_key));
    ql_memset(g_server_aes_key, 0, sizeof(g_server_aes_key));
    
    g_client_handle.product_aes_key = g_product_aes_key;
    g_client_handle.device_aes_key = g_device_aes_key;
    g_client_handle.server_aes_key = g_server_aes_key;
    #endif

    g_client_handle.product_id = ctx->product_id;

    #ifdef USE_SMPU
    ql_sec_memcpy(g_client_handle.product_aes_key, ctx->product_key, AESKEY_LEN);
    #else
    ql_memcpy(g_client_handle.product_aes_key, ctx->product_key, AESKEY_LEN);
    #endif

    ql_memcpy((char *)g_client_handle.mcu_version, (char *)ctx->mcu_version, 5);

    #ifdef LOCAL_SAVE
        ret = ql_local_config_load();
        if (ret != 0) {
            ql_log_err( ERR_EVENT_LOCAL_DATA, "init local fail");
            return -1;
        }
        
        #if 0/*log online switch*/ 
        g_client_handle.log_online = g_local_config.log_control;
        #endif
    #endif
    
    g_client_handle.ota_mcu_type = OTA_CHECK_TYPE_TIMER;
    if(g_client_handle.ota_mcu_chunk_size > (g_client_handle.recvbuf_size - PADDING_BUFFER_SIZE) || g_client_handle.ota_mcu_chunk_size <= 0)
    {
        g_client_handle.ota_mcu_chunk_size = 1024;
    }
    #if (__SDK_TYPE__== QLY_SDK_BIN)
    g_client_handle.ota_wifi_type = OTA_CHECK_TYPE_TIMER;
    g_client_handle.ota_wifi_chunk_size = 1024;
    #endif
#ifdef LOCAL_SAVE    
    ql_local_ota_ver_init();
#endif
    g_client_handle.datatype = DATATYPE_JSON;
    g_client_handle.conn_interval = CONN_INTERVAL_NORMAL;

    g_client_handle.client_socket = ql_tcp_socket_create();
    if (g_client_handle.client_socket == NULL) {
       ql_log_err( ERR_EVENT_INTERFACE, "tcp create fail");
        return -1;
    }

    NewNetwork(&mqttn, cloud_disconnect_cb, g_client_handle.client_socket);

    if (ctx->recvbuf_size <= 0) {
        g_client_handle.recvbuf_size = DEFAULT_BUFFER_SIZE;
    } else if (ctx->recvbuf_size > MAX_BUFFER_SIZE) {
        ql_log_err( ERR_EVENT_INTERFACE, "Rbuf len %d err", MAX_BUFFER_SIZE);
        return -1;
    } else {
        g_client_handle.recvbuf_size = ctx->recvbuf_size;
    }
    g_client_handle.recvbuf_size += PADDING_BUFFER_SIZE;
    g_client_handle.recvbuf = (unsigned char *)ql_malloc(g_client_handle.recvbuf_size);
    if (NULL == g_client_handle.recvbuf) {
        ql_log_err(ERR_EVENT_INTERFACE, "malc Rbuf err");
        return -1;
    }

    if (ctx->sendbuf_size <= 0) {
        g_client_handle.sendbuf_size = DEFAULT_BUFFER_SIZE;
    } else if (ctx->sendbuf_size > MAX_BUFFER_SIZE) {
        ql_log_err( ERR_EVENT_INTERFACE, "Sbuf len %d err!", MAX_BUFFER_SIZE);
        return -1;
    } else {
        g_client_handle.sendbuf_size = ctx->sendbuf_size;
    }
    g_client_handle.sendbuf_size += PADDING_BUFFER_SIZE;
    g_client_handle.sendbuf = (unsigned char *)ql_malloc(g_client_handle.sendbuf_size);
    if (NULL == g_client_handle.sendbuf) {
        ql_log_err( ERR_EVENT_INTERFACE, "malc Sbuf err");
        return -1;
    }

    g_client_handle.dispatch_ip   = ctx->server_addr;
    g_client_handle.dispatch_port = ctx->server_port;

    g_client_handle.dp_mutex = ql_mutex_create();
    if(g_client_handle.dp_mutex == NULL)
    {      
        return -1;
    }
    
    g_client_handle.sub_mutex = ql_mutex_create();
    if(g_client_handle.sub_mutex == NULL)
    {      
        return -1;
    }

    g_client_handle.log_mutex = ql_mutex_create();
    if(g_client_handle.log_mutex == NULL)
    {      
        return -1;
    }


    cloud_rst_state_set(RST_STATE_INIT, 0, 0);
    /*unbound*/
    cloud_unbound_state_set(UNB_STATE_INIT, 0, 0);
    /*bind state init*/
    cloud_bind_state_set(BIND_STATE_INIT, 0, 0);

    ql_client_set_status(NET_INIT, CLI_DISCONN_TYPE_NULL);

    return 0;
}
//call this function when tcp read/write fail
void cloud_disconnect_cb( CLI_STATU_T disconn_type )
{
    ql_log_warn("cloud_disconnect_cb\r\n");
    if(g_client_handle.dev_status != 1)
    {
        iot_status_cb(DEV_STA_DISCONN_CLOUD, get_current_time());
        g_client_handle.dev_status = 1;
    }
    ql_client_set_status(NET_MQTT_DISCONNECT, disconn_type);    
}

static int cloud_mqtt_connect(ql_socket_t *socket)
{
    int ret = -1;
    dispatch_ipinfo ipinfo;

    ret = qlcloud_get_dispatch_ipinfo(&ipinfo, &g_client_handle, socket);
    if (ret == -1) {
        return -1;
    } else {
        if (strncmp(ipinfo.ip, "0.0.0.0", 7) == 0 || ipinfo.port == 0) {
            ql_log_err( ERR_EVENT_INTERFACE, "get ip 0.0.0.0");
            return -1;
        }
    }
    ql_log_info("get addr:%s:%d\r\n", ipinfo.ip, ipinfo.port);

    ret = ConnectNetwork(&mqttn, ipinfo.ip, ipinfo.port);

    if (ret == -1) {
        ql_log_err( ERR_EVENT_INTERFACE, "tcp conn err:%d", ret);
        return -1;
    }

    //init mqttclient ctx.
    unsigned char *recvbuf = g_client_handle.recvbuf;
    int recvbuf_size = g_client_handle.recvbuf_size;
    if (NULL == recvbuf || recvbuf_size <= 0) {
        ql_log_err( ERR_EVENT_LOCAL_DATA, "Rbuf null(%d).", recvbuf_size);
        return -1;
    }
    ql_memset(recvbuf, 0x00, recvbuf_size);

    unsigned char *sendbuf = g_client_handle.sendbuf;
    int sendbuf_size = g_client_handle.sendbuf_size;
    if (NULL == sendbuf || sendbuf_size <= 0) {
        ql_log_err( ERR_EVENT_LOCAL_DATA, "Sbuf null(%d).", sendbuf_size);
        return -1;
    }
    ql_memset(sendbuf, 0x00, sendbuf_size);

    MQTTClient(&mqttc, &mqttn, COMMAND_TIMEOUT, sendbuf, sendbuf_size, recvbuf, recvbuf_size);

    //init conn opt
    conndata.willFlag = 0;
    conndata.MQTTVersion = 4;
    conndata.clientID.cstring = (char *)g_client_handle.mac_str;
    conndata.username.cstring = "**";
    conndata.password.cstring = "**";
    conndata.keepAliveInterval = 5;
    conndata.cleansession = 1;

    ret = MQTTConnect(&mqttc, &conndata);
    if (ret == FAILURE) {
        ql_log_err(ERR_EVENT_INTERFACE,"mqconn fail,try...");
        mqttc.ipstack->disconnect(mqttc.ipstack);
        return -1;
    }
    
    ret = MQTTSubscribe(&mqttc, (const char *)(g_client_handle.sub_topic), 0, mqttRecv_handle, QOS0);
    if (ret < 0) {
        ql_log_err( ERR_EVENT_INTERFACE, "mqsubs fail,try...");
        MQTTDisconnect(&mqttc);
        mqttc.ipstack->disconnect(mqttc.ipstack);
    } else {
        ret = 0;
    }

    return ret;
}

void ql_logic_endless_task()
{
    int ret = 0;
    struct cloud_event e;
    char info ;

    if (ql_client_get_status() == NET_DISPATCHING)
    {
        if (0 == cloud_mqtt_connect(g_client_handle.client_socket))
        {

        #ifdef LOCAL_SAVE
            qlcloud_init_ota_status(OTA_OWNER_MCU, g_local_config.cfg_mcu_ota_ver, g_client_handle.mcu_version);
            qlcloud_init_ota_status(OTA_OWNER_WIFI, g_local_config.cfg_wifi_ota_ver, DEVICE_VERSION);
        #endif
            ql_client_set_status(NET_MQTT_CONNECTED, CLI_DISCONN_TYPE_NULL);
        }
        else
        {
            ql_client_set_status(NET_DISPATCHFAIL, CLI_DISCONN_TYPE_NULL);
        }
    }
    //receive or send mqtt data after mqtt connected
    if (ql_client_get_status() >= NET_MQTT_CONNECTED)
    {
        ret = get_ql_queue(g_queue, &e, sizeof(struct cloud_event));

        if (ret == 0)
        {
            /*
            cloud_packet_t * pkt = (cloud_packet_t *)(e.ev_data);
            ql_log_info("direct send pkt op:%x, seq:%d, len:%d\r\n", ql_ntohs(pkt->pkt_opcode), ql_ntohl(pkt->pkt_seq), ql_ntohl(pkt->pkt_len));
            */
            ret = direct_send_cloud(&e);
            if(ret == SEND_TIMOUT) {
                //send timeout don't delect queue
                ql_log_warn("direct send timeout, seq:%d\r\n", e.ev_seq);
            }
            else {
                if(ret < 0)
                {
                    ql_log_err(ERR_EVENT_INTERFACE, "dirct send err %d.", ret);
                }
                // ql_log_info("direct send is ok seq:%d\r\n", e.ev_seq);
                de_ql_queue(g_queue, &e, sizeof(struct cloud_event));
                free_event(&e);
            }
        }
        else { /*do nothing*/ }
        //if no data on the tcp stream,this function will block 30 milli second.
        MQTTYield(&mqttc, mqttc.command_timeout_ms);
        //if no data on the tcp stream,mqtt ping request will be sent and recv ping response every 5 seconds.
        if (mqttc.lastreceive_tp > 0 && mqttc.lastreceive_tp < os_tick && os_tick - mqttc.lastreceive_tp >=  APPLICTION_TIMEOUT)
        {
            CLI_STATU_T statu_type = CLI_DISCONN_TYPE_NETWK_POOR;
            ql_log_warn("network environment is poor,try to reconnect server...[%d-%d]\r\n", mqttc.lastreceive_tp, os_tick);
            needclose_old = 1;
            if(g_client_handle.dev_status != 1)
            {
                iot_status_cb(DEV_STA_DISCONN_CLOUD, get_current_time());
                g_client_handle.dev_status = 1;
            }
            #ifdef LOCAL_SAVE
            g_local_config.log_conn_exc.conn_exc_ts = os_tick;
            g_local_config.log_conn_exc.last_rv_ts = mqttc.lastreceive_tp;
            ql_local_log_to_cloud(LOG_TYPE_WARN, WARN_EVENT_CONN_EXC, &statu_type, get_current_time());
            #endif
            ql_client_set_status( NET_MQTT_DISCONNECT, statu_type);//reconnect from dispatch
        }

        /*SUPPORT_PATCH*/
        if( ql_patch_main )
        {   
            ql_patch_main();
        }
    }
    else
    {
        ql_thread_schedule();
    }
    
    if( cloud_rst_state_get() == RST_STATE_RST_ED )
    {
        cloud_rst_state_set(RST_STATE_INIT, 0, 0);
        #ifdef LOCAL_SAVE
        ql_local_config_erase();
        #endif
        iot_status_cb(DEV_STA_FACTORY_RESET, get_current_time());
    }
    else if( cloud_rst_state_get() == RST_STATE_RST_ING )
    {
        if( cloud_rst_timeout_get() && (cloud_rst_time_get()+ cloud_rst_timeout_get() < g_time_tick) )
        {
            cloud_rst_state_set(RST_STATE_TIMEOUT, get_current_time(), 0);
        }
    }
    else{/* do nothing */}
    
    if( cloud_rst_state_get() == RST_STATE_RST_FAIL || cloud_rst_state_get() == RST_STATE_TIMEOUT )
    {
    #ifdef LOCAL_SAVE
        //if device offline or send 0b01 timeout,save state to local. reboot to send "rst=1" in auth packet.
        ql_local_dev_state_set(DEV_STATE_RST);
    #endif
    
        cloud_rst_state_set(RST_STATE_INIT, 0, 0);
    #ifdef LOCAL_SAVE
        ql_local_config_erase();
        if(cloud_rst_state_get() == RST_STATE_RST_FAIL)
            info = 'F';
        else
            info = 'T';
        ql_local_log_to_cloud(LOG_TYPE_WARN, WARN_EVENT_RST_EXC, (void *)&info, get_current_time());
    #endif
        iot_status_cb(DEV_STA_FACTORY_RESET, get_current_time());
    }
    else{/* do nothing */}

    /*bind state*/ 
    if( cloud_bind_state_get() == BIND_STATE_ED )
    {
        cloud_bind_state_set(BIND_STATE_INIT, 0, 0);
        udp_control(0);
        iot_status_cb(DEV_STA_BIND_SUCCESS, get_current_time());
    }
    else if( cloud_bind_state_get() == BIND_STATE_ING )
    {
        if(cloud_bind_timeout_get() && ( cloud_bind_time_get() + cloud_bind_timeout_get() < g_time_tick ))
        {
            cloud_bind_state_set(BIND_STATE_TIMEOUT, get_current_time(), 0);
        }
    }
    else{/* do nothing */}
    
    if( cloud_bind_state_get() == BIND_STATE_FAIL || cloud_bind_state_get() == BIND_STATE_TIMEOUT )
    {
        cloud_bind_state_set(BIND_STATE_INIT, 0, 0);    
        udp_control(0);  
        #ifdef LOCAL_SAVE             
        if(cloud_bind_state_get() == BIND_STATE_FAIL)
            info = 'F';
        else
            info = 'T';
        ql_local_log_to_cloud(LOG_TYPE_WARN, WARN_EVENT_P_BIND_EXC, (void *)&info, get_current_time());
        #endif
        iot_status_cb(DEV_STA_BIND_FAILED, get_current_time());        
    }
    else{/* do nothing */}

    /*unbound*/    
    if( cloud_unbound_state_get() == UNB_STATE_UNB_ED )
    {
        cloud_unbound_state_set(UNB_STATE_INIT, 0, 0);

        if(mqttn.my_socket != NULL)
        {
            ql_tcp_disconnect(mqttn.my_socket);
        }
        iot_status_cb(DEV_STA_UNBIND, get_current_time());
        if (mqttn.disconn_cb) 
        {
             mqttn.disconn_cb(CLI_DISCONN_TYPE_UNB_END);
        }
    }
    else if( cloud_unbound_state_get() == UNB_STATE_UNB_ING )
    {
        if(cloud_unbound_timeout_get() && ( cloud_unbound_time_get() + cloud_unbound_timeout_get() < g_time_tick ))
        {
            cloud_unbound_state_set(UNB_STATE_TIMEOUT, get_current_time(),0);
        }
    }
    else{/* do nothing */}
    
    if( cloud_unbound_state_get() == UNB_STATE_UNB_FAIL || cloud_unbound_state_get() == UNB_STATE_TIMEOUT )
    {    
#ifdef LOCAL_SAVE
        //if device offline or send 0b03 timeout,save state to local. reboot to send "unb=1" in auth packet.
        if(ql_local_dev_state_set(DEV_STATE_UNB) < 0)
        {
            ql_log_err(ERR_EVENT_INTERFACE, "set unb fail err");
        }
#endif
        ql_log_warn("unbound state[%d] is UNB_STATE_UNB_FAIL or UNB_STATE_TIMEOUT\r\n", cloud_unbound_state_get());
        cloud_unbound_state_set(UNB_STATE_INIT, 0, 0);
        if(mqttn.my_socket != NULL)
        {
            ql_tcp_disconnect(mqttn.my_socket);
        }
        iot_status_cb(DEV_STA_UNBIND, get_current_time());
        #ifdef LOCAL_SAVE
        if(cloud_unbound_state_get() == UNB_STATE_UNB_FAIL)
            info = 'F';
        else
            info = 'T';
        ql_local_log_to_cloud(LOG_TYPE_WARN, WARN_EVENT_UNB_EXC, (void *)&info, get_current_time());
        #endif
        if (mqttn.disconn_cb) 
        {
             mqttn.disconn_cb(CLI_DISCONN_TYPE_UNB_FAIL);
        }
    }
    else{/* do nothing */}    
}

void ql_logic_authed_handle()
{
    type_u32 random_num = 0;

    if (!(os_tick % SECONDS_OF_HEART_INTERVAL)) {//3mins
        cloud_request_send_pkt(PKT_ID_REQ_HEART, get_cur_seq_num(), NULL);
    }
    //initialization ota check random time,if user do not call set_ota_option()
    if(g_client_handle.ota_mcu_check_tm == 0 && g_time_tick > 1483200000){//2017/1/1 00:00:00
        g_client_handle.ota_mcu_check_tm = ql_get_current_sysclock() + ((ql_random() % 86400));

        if(ql_patch_init)
        {
            ql_patch_init( g_client_handle.ota_mcu_chunk_size, get_cur_seq_num, ql_get_current_sysclock);
        }

        #if (__SDK_TYPE__== QLY_SDK_BIN)
        g_client_handle.ota_wifi_check_tm = g_client_handle.ota_mcu_check_tm + OTA_WIFI_CHECK_DELAY_SEC;
        #endif
    }
    
    if (set_otaupgrade_checktime <= 3600 && set_otaupgrade_checktime >= 120 && g_time_tick > 1483200000) {
        random_num = 60 + (ql_random() % (set_otaupgrade_checktime - 60));
        // random_num = 10;
        set_otaupgrade_checktime = 0;
        ql_log_info("Device will do OTA upgrade check after %d seconds \r\n",random_num);

        g_client_handle.ota_mcu_check_tm = ql_get_current_sysclock() + random_num;
        #if (__SDK_TYPE__== QLY_SDK_BIN)
        g_client_handle.ota_wifi_check_tm = g_client_handle.ota_mcu_check_tm + OTA_WIFI_CHECK_DELAY_SEC;
        // g_client_handle.ota_wifi_check_tm = g_client_handle.ota_mcu_check_tm + 10;
        #endif

        #ifdef LOCAL_SAVE
        ql_local_log_to_cloud(LOG_TYPE_INFO, INFO_EVENT_OTA_TS, (void *)&random_num, get_current_time());
        #endif

        if(ql_patch_init)
        {
            ql_patch_init( g_client_handle.ota_mcu_chunk_size, get_cur_seq_num, ql_get_current_sysclock);
        }
    }

    //uninclude OTA_INIT , OTA_FILECHUNK_END_BACK
    if ( (OTA_VERSION_CHECK <= g_client_handle.ota_mcu_status && g_client_handle.ota_mcu_status < OTA_FILECHUNK_END_BACK )
        || OTA_UPGRADE_CMD_GET == g_client_handle.ota_mcu_status || OTA_UPGRADE_CMD_BACK == g_client_handle.ota_mcu_status) {
        //download binfile timeout, reset status to OTA_INIT.2mins.
        if (os_tick - g_client_handle.ota_mcu_stat_chg_tm > OTA_STATUS_TIMEOUT_SEC) {
            if(g_client_handle.ota_mcu_type == OTA_CHECK_TYPE_PASSIVE || g_client_handle.ota_mcu_type == OTA_CHECK_TYPE_COMPELS  || g_client_handle.ota_mcu_type == OTA_CHECK_TYPE_APPDOWN)
            {
                g_client_handle.ota_mcu_type = OTA_CHECK_TYPE_TIMER;
            }
            ql_log_warn("ota mcu tmout:%d\r\n", g_client_handle.ota_mcu_status);
            set_ota_mcu_status(OTA_INIT);
        }

        if ( g_client_handle.ota_mcu_status == OTA_FILECHUNK_BEGIN_BACK || g_client_handle.ota_mcu_status == OTA_FILECHUNK_STREAM_BACK )
        {
            if ( os_tick - g_client_handle.ota_mcu_stat_chg_tm > OTA_CHUNK_RETRAN_SEC && g_ota_chunk_retran_time < OTA_CHUNK_RETRAN_TIMES ) 
            {
                g_ota_chunk_retran_time++;
                g_client_handle.ota_mcu_stat_chg_tm  = os_tick;
                binfile_chunk_response(g_ota_chunk_retran_seq, 0, g_ota_chunk_retran_offset, OTA_OWNER_MCU);
                ql_log_warn("ota mcu retransmit %d time,offset:%d\r\n", g_ota_chunk_retran_time, g_ota_chunk_retran_offset);
            }
        }
    }
    
    //if type is MASS or COMPELS,the ota_status must be OTA_INIT
    //if os_tick > g_client_handle.ota_mcu_check_tm,the ota_staus may be OTA_INIT or OTA_FILECHUNK_END_BACK
    if (os_tick > g_client_handle.ota_mcu_check_tm || (OTA_CHECK_TYPE_NEED_PASSIVE == g_client_handle.ota_mcu_type) ||
        (OTA_CHECK_TYPE_NEED_COMPELS == g_client_handle.ota_mcu_type) || (OTA_CHECK_TYPE_NEED_APPDOWN == g_client_handle.ota_mcu_type))
    {
        if(OTA_CHECK_TYPE_NEED_PASSIVE == g_client_handle.ota_mcu_type)
        {
            g_client_handle.ota_mcu_type = OTA_CHECK_TYPE_PASSIVE;
        }
        else if(OTA_CHECK_TYPE_NEED_COMPELS == g_client_handle.ota_mcu_type)
        {
            g_client_handle.ota_mcu_type = OTA_CHECK_TYPE_COMPELS;
        }
        else if(OTA_CHECK_TYPE_NEED_APPDOWN == g_client_handle.ota_mcu_type)
        {
            g_client_handle.ota_mcu_type = OTA_CHECK_TYPE_APPDOWN;
        }
        
        if (g_client_handle.ota_mcu_status == OTA_INIT || g_client_handle.ota_mcu_status == OTA_FILECHUNK_END_BACK)
        {
            #if (__SDK_TYPE__== QLY_SDK_BIN)
            // if wifi is downloading firmware,put off mcu upgrade
            if(g_client_handle.ota_wifi_status > OTA_INIT && g_client_handle.ota_wifi_status < OTA_FILECHUNK_END_BACK)
            {
                ql_log_err(ERR_EVENT_RX_DATA, "wifi:downloading");
            }
            else
            #endif
            {
                /*download free ,ota can check*/
                if(get_download_status() == DEV_DOWNLOAD_FREE || get_download_status() == DEV_DOWNLOAD_OTA)
                {
                    cloud_request_ota_check(OTA_OWNER_MCU);
                }
            }
        }
            g_client_handle.ota_mcu_check_tm += OTA_CHECK_RANDOM_SEC;
    }

#if (__SDK_TYPE__== QLY_SDK_BIN)
    //uninclude OTA_INIT and OTA_FILECHUNK_END_BACK
    if ( (OTA_VERSION_CHECK <= g_client_handle.ota_wifi_status && g_client_handle.ota_wifi_status < OTA_FILECHUNK_END_BACK )
        || OTA_UPGRADE_CMD_GET == g_client_handle.ota_wifi_status  || OTA_UPGRADE_CMD_BACK == g_client_handle.ota_wifi_status) {
        //download binfile timeout, reset status to OTA_INIT.2mins.
        if (os_tick - g_client_handle.ota_wifi_stat_chg_tm > OTA_STATUS_TIMEOUT_SEC) {
            if(g_client_handle.ota_wifi_type == OTA_CHECK_TYPE_PASSIVE || g_client_handle.ota_wifi_type == OTA_CHECK_TYPE_APPDOWN)
            {
                g_client_handle.ota_wifi_type = OTA_CHECK_TYPE_TIMER;
            }
            ql_log_warn("ota module tmout:%d\r\n", g_client_handle.ota_wifi_status);
            set_ota_wifi_status(OTA_INIT);
        }
    }
    
    if ( g_client_handle.ota_wifi_status == OTA_FILECHUNK_BEGIN_BACK || g_client_handle.ota_wifi_status == OTA_FILECHUNK_STREAM_BACK )
    {
        if ( os_tick - g_client_handle.ota_wifi_stat_chg_tm > OTA_CHUNK_RETRAN_SEC && g_ota_chunk_retran_time < OTA_CHUNK_RETRAN_TIMES ) 
        {
            g_ota_chunk_retran_time++;
            g_client_handle.ota_wifi_stat_chg_tm  = os_tick;
            binfile_chunk_response(g_ota_chunk_retran_seq, 0, g_ota_chunk_retran_offset, OTA_OWNER_WIFI);
            ql_log_warn("ota wifi retransmit %d time,offset:%d\r\n", g_ota_chunk_retran_time, g_ota_chunk_retran_offset);
        }
    }
    //if type is MASS or COMPELS,the ota_status must be OTA_INIT
    //if os_tick > g_client_handle.ota_mcu_check_tm,the ota_staus may be OTA_INIT or OTA_FILECHUNK_END_BACK
    if ( os_tick > g_client_handle.ota_wifi_check_tm || (OTA_CHECK_TYPE_NEED_PASSIVE == g_client_handle.ota_wifi_type) || (OTA_CHECK_TYPE_NEED_APPDOWN == g_client_handle.ota_wifi_type))
    {
    
        if(OTA_CHECK_TYPE_NEED_PASSIVE == g_client_handle.ota_wifi_type)
        {
            g_client_handle.ota_wifi_type = OTA_CHECK_TYPE_PASSIVE;
        }
        else if(OTA_CHECK_TYPE_NEED_APPDOWN == g_client_handle.ota_wifi_type)
        {
            g_client_handle.ota_wifi_type = OTA_CHECK_TYPE_APPDOWN;
        }
        
        if (g_client_handle.ota_wifi_status == OTA_INIT || g_client_handle.ota_wifi_status == OTA_FILECHUNK_END_BACK)
        {
            // if mcu is downloading firmware,put off module upgrade
            if(g_client_handle.ota_mcu_status > OTA_INIT && g_client_handle.ota_mcu_status < OTA_FILECHUNK_END_BACK)
            {
                ql_log_err(ERR_EVENT_RX_DATA, "mcu:downloading");
            }
            else
            {
                cloud_request_ota_check(OTA_OWNER_WIFI);
            }
        }
        g_client_handle.ota_wifi_check_tm += OTA_CHECK_RANDOM_SEC;
    }
#endif
}

void ql_logic_period_task()
{
    static int ing_tmout = 0;
    struct cloud_event e;
    g_time_tick++;
    os_tick = ql_get_current_sysclock();

    switch (ql_client_get_status()) {
    case NET_INIT:
    case NET_DISPATCHFAIL:
        if(g_time_tick % g_client_handle.conn_interval == 0)
        {
            ql_client_set_status(NET_DISPATCHING, CLI_DISCONN_TYPE_NULL);
        }
        break;
    case NET_MQTT_DISCONNECT:
        while(!is_empty_ql_queue(g_queue))
        {
            de_ql_queue(g_queue, &e, sizeof(struct cloud_event));
            free_event(&e);
        }
        //network environment is poor or receive err no 11/12,need to close the tcp connection
        if(needclose_old == 1 || g_client_handle.conn_interval != CONN_INTERVAL_NORMAL)
        {
            ql_log_warn("close abnormal connection [%d-%d]\r\n", needclose_old, g_client_handle.conn_interval);
            mqttc.ipstack->disconnect(mqttc.ipstack);
            needclose_old = 0;
        }
        ql_client_set_status(NET_DISPATCHFAIL, CLI_DISCONN_TYPE_NULL);
        break;
    case NET_REGINFING:
        break;
    case NET_MQTT_CONNECTED:
    case NET_REGINFAIL:
    case NET_AUTHFAIL:
        if(cloud_request_send_pkt(PKT_ID_REQ_REG, get_cur_seq_num(), NULL) == 0)
        {
            ql_client_set_status(NET_REGINFING, CLI_DISCONN_TYPE_NULL);
        }
        break;
    case NET_AUTHING:
        break;
    case NET_REGINFED:
        if(cloud_request_send_pkt(PKT_ID_REQ_AUTH, get_cur_seq_num(), NULL) == 0)
        {
            ql_client_set_status(NET_AUTHING, CLI_DISCONN_TYPE_NULL);
        }
        break;
    case NET_AUTHED:
#if (__SDK_TYPE__== QLY_SDK_BIN)
    case NET_DP_GETFAIL:        /* 18 */
        if(cloud_request_send_pkt(PKT_ID_REQ_DP_GET, get_cur_seq_num(), NULL)== 0)
        {
            ql_client_set_status(NET_DP_GETING, CLI_DISCONN_TYPE_NULL);
        }
        break;
    case NET_DP_GETING:         /* 17 */
        break;
    case NET_DP_GETED:          /* 19 */
#endif
        ql_logic_authed_handle();
        break;
    default:
        break;
    }

    if (NET_REGINFING == ql_client_get_status() || NET_AUTHING == ql_client_get_status()
#if (__SDK_TYPE__== QLY_SDK_BIN)
        || NET_DP_GETING == ql_client_get_status()
#endif
    )
    {
        ing_tmout++;
        if(ing_tmout >= 20)
        {
            ing_tmout = 0;
            ql_log_warn("status[%d] time out\r\n", ql_client_get_status());
            ql_client_set_status(NET_REGINFAIL, CLI_DISCONN_TYPE_NULL);//register again without dissconnect tcp and mqtt link
        }
    }
    else
    {
        ing_tmout = 0;
    }
}

void cloud_errno_handle( type_s32 pkt_errno, type_u16 pkt_opcode )
{
    ql_log_err( ERR_EVENT_NULL, "--ERR MSG NO:%d--", pkt_errno);

    if (pkt_errno < 0 || pkt_errno > ERRNO_MAX)
    {
        return;
    }

    switch( pkt_errno )
    {
    case ERRNO_GETREGISTER_INFO_FAILD:
        ql_log_err( ERR_EVENT_NULL, "register fail, please check the Serial Number!");
        break;
    case ERRNO_VERIFY_PRODUCTSIGN_FAILD:
        ql_log_err( ERR_EVENT_NULL, "PRODUCT ID or PRODUCT KEY wrong!");
        break;
    case ERRNO_LicenceLimit:
        ql_log_err( ERR_EVENT_NULL, "reach the upper licence limit!");
        break;
    case ERRNO_MacBindOtherID:
        ql_log_err( ERR_EVENT_NULL, "mac address has bound other product!");
        break;
    }

    if (pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_REG)) {
        if(pkt_errno == ERRNO_LicenceLimit || pkt_errno == ERRNO_MacBindOtherID)
        {
            iot_status_cb((DEV_STATUS_T)pkt_errno, get_current_time());
            g_client_handle.dev_status = pkt_errno;
            g_client_handle.conn_interval = CONN_INTERVAL_LICENCE_LIMIT;
            ql_client_set_status( NET_MQTT_DISCONNECT, CLI_DISCONN_TYPE_REG_ERR);
        }
        else
        {
            ql_client_set_status(NET_REGINFAIL, CLI_DISCONN_TYPE_NULL);
        }
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_AUTH)) {
        ql_client_set_status(NET_AUTHFAIL, CLI_DISCONN_TYPE_NULL);
    }
    else{}

    //convert to re-auth when token expired in net authed or data trans status.
    if (is_application_authed())
    {
        if (pkt_errno == ERRNO_TOKEN_OUTOFDATE)
        {
            ql_client_set_status( NET_AUTHFAIL, CLI_DISCONN_TYPE_NULL);
        }
        else if( pkt_errno == ERRNO_RstFail )
        {
            if( cloud_rst_state_get() == RST_STATE_RST_ING )
            {
                cloud_rst_state_set(RST_STATE_RST_FAIL, get_current_time(), 0);
            }
        }
        else if( pkt_errno == ERROR_UnboundFail )
        {
            if( cloud_unbound_state_get() == UNB_STATE_UNB_ING )
            {
                cloud_unbound_state_set(UNB_STATE_UNB_FAIL, get_current_time(), 0);
            }
        }        
    }
}

void mqttRecv_handle(MessageData* md)
{
    type_u8* in = NULL;
    type_u32 inlen = 0;

    type_s8   pkt_errno = 0;
    type_u16 pkt_opcode = 0;
    type_u32 pkt_seq = 0, pkt_len = 0;
    char* op_str = NULL;

    type_u8* p_in_data = NULL;
    type_u32 in_data_len = 0;

    type_u8* p_encrypt_data = NULL;
    type_u32 encrypt_len = 0;

    in = md->message->payload;
    inlen = md->message->payloadlen;

    if(in == NULL  || inlen < 16 )
    {
        return;
    }

    in[inlen] = '\0';
    //len  | hlen | version|  dir | type |errno| opcode |seq   |
    //0-3 | 4-5  | 6-7     |8(1) | 8(7) |  9   | 10-11  |12-15 |
    pkt_len     = STREAM_TO_UINT32_f(in, 0);
    pkt_opcode  = STREAM_TO_UINT16_f(in, 10);
    pkt_seq     = STREAM_TO_UINT32_f(in, 12);
    pkt_errno   = in[9];

    ql_log_info("recv pkt op:%x, seq:%d, len:%d, errno:%d\r\n", pkt_opcode, pkt_seq, pkt_len, pkt_errno);

    if(!is_packet_opcode_valid(pkt_opcode, &op_str))
    {
        ql_log_err( ERR_EVENT_RX_DATA, "opcode invalid");
        return;
    }

    if( ERRNO_OK == pkt_errno )
    {

    }
    else
    {
        cloud_errno_handle(pkt_errno, pkt_opcode);
        return;
    }

    if (g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL || pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_REG))
    {
        in_data_len = inlen - 16;
        p_in_data   = in + 16;
    }
    else if (g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_AES||g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SM4)
    {
        //AES decrypt.
        in_data_len = inlen - 16;
        encrypt_len = in_data_len;
        p_encrypt_data = (type_u8*)ql_malloc(in_data_len+1);
        if (NULL == p_encrypt_data)
        {
            return;
        }
        else
        {
            ql_memset(p_encrypt_data, 0x00, in_data_len+1);
        }

        //iot_aes_cbc128_decrypt((type_u8*)(in + 16), in_data_len, p_encrypt_data, (type_u32*)&encrypt_len, g_client_handle.server_aes_key);
        if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_AES)
        {
            iot_aes_cbc128_decrypt((type_u8*)(in + 16), in_data_len, p_encrypt_data, (type_u32*)&encrypt_len, g_client_handle.server_aes_key);
        }
        else if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SM4)
        {
            /*add sm4 decrypt*/
            ql_sm4_decrypt_cbc(g_client_handle.server_aes_key,(type_u8*)(in + 16),in_data_len,p_encrypt_data,encrypt_len);
        }
        p_encrypt_data[encrypt_len] = '\0';

        in_data_len = encrypt_len;
        p_in_data = p_encrypt_data;
    }
    else {    }

#ifdef DEBUG_DATA_OPEN
    if(pkt_opcode!=PKT_OP_CODE(PKT_ID_RES_OTAFILE_CHUNK) && pkt_opcode != 0x0302)//0302 is binary
    {
        ql_printf("data[%d]:%s\r\n", in_data_len, p_in_data);
    }
#endif

    if( pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_REG))
    {
        if(ql_client_get_status() == NET_REGINFING)
        {
            cloud_reginfo_store((const char *)p_in_data, (type_size_t)in_data_len);
        }
        else
        {
            ql_log_err( ERR_EVENT_STATU, "stat need be reging");
        }
    }
    else if( pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_AUTH))
    {
        if(ql_client_get_status() == NET_AUTHING)
        {
            cloud_auth_store((const char *)p_in_data, (type_size_t)in_data_len);
        }
        else
        {
            ql_log_err( ERR_EVENT_STATU, "stat need be authing");
        }
    }
    #if (__SDK_TYPE__== QLY_SDK_BIN)
    else if( pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_DP_GET))
    {
        cloud_dp_store(p_in_data, in_data_len);
    }
    #endif
    else {
        if(ql_client_get_status() < NET_AUTHED)
        {
            ql_log_warn("receive data before authed\r\n");
            goto exit ;
        }
    }

    if (pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_UPLOAD_DPS))
    {
        qlcloud_parse_data(pkt_opcode, pkt_seq, (const char*)p_in_data, (type_size_t)in_data_len);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_TX_DATA) || pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_TX_DATA_V2))
    {
        qlcloud_parse_data(pkt_opcode, pkt_seq, (const char*)p_in_data, (type_size_t)in_data_len);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_RCV_RX_DATA))
    {
        qlcloud_process_rx_data((const char*)p_in_data, (type_size_t)in_data_len, pkt_seq);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_RCV_RX_DATA_V2))
    {
        qlcloud_process_rx_data_v2((const char*)p_in_data, (type_size_t)in_data_len, pkt_seq);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_RCV_DOWNLOAD_DPS))
    {
        #if(__SDK_TYPE__== QLY_SDK_BIN)
        if(ql_client_get_status() < NET_DP_GETED) 
        {
            ql_log_err( ERR_EVENT_STATU, "Rdp before get dp");
            return ;
        }
        #endif
        qlcloud_process_downdps((const char*)p_in_data, (type_size_t)in_data_len, pkt_seq);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_SUB_ACT))
    { 
        qlcloud_parse_data(pkt_opcode, pkt_seq, (const char*)p_in_data, (type_size_t)in_data_len);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_SUB_INACT))
    {
        qlcloud_parse_data(pkt_opcode, pkt_seq, (const char*)p_in_data, (type_size_t)in_data_len);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_HEART))
    {
        qlcloud_process_heart((const char*)p_in_data, (type_size_t)in_data_len, pkt_seq);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_RES_OTAPASSIVE)) //0A00
    {
        qlcloud_process_otapassive((const char*) p_in_data, in_data_len, pkt_seq);
    }
    else if(pkt_opcode == PKT_OP_CODE(PKT_ID_RES_OTA_COMPELS)) //0109
    {
        qlcloud_process_ota_compels((const char *)p_in_data, in_data_len, pkt_seq);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_RES_OTAFILE_INFO))//0A02
    {
        qlcloud_process_otafileinfo((const char*) p_in_data, in_data_len, pkt_seq);
    }
    else if(pkt_opcode == PKT_OP_CODE(PKT_ID_RES_OTAFILE_CHUNK))//0A03
    {
        qlcloud_process_otafilechunk((const char*)p_in_data, in_data_len, pkt_seq);
    }
    else if(pkt_opcode == PKT_OP_CODE(PKT_ID_RES_OTAUPGRADE_CMD)) //0A04
    {
        qlcloud_process_otacmd((const char*)p_in_data, in_data_len, pkt_seq);
    }
    else if (pkt_opcode == PKT_OP_CODE(PKT_ID_RES_OTA_APPDOWN)) //0A05
    {
        qlcloud_process_ota_appdown((const char*) p_in_data, in_data_len, pkt_seq);
    }
    else if ( pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_RST) )
    {
        if( cloud_rst_state_get() == RST_STATE_RST_ING )
        {
            cloud_rst_state_set(RST_STATE_RST_ED, get_current_time(), 0);
        }
    }
    else if(pkt_opcode == PKT_OP_CODE(PKT_ID_GET_INFO)) //0303
    {
        qlcloud_process_info((const char*)p_in_data, in_data_len, pkt_seq);
    }
    else if ( pkt_opcode == PKT_OP_CODE(PKT_ID_RCV_BIND_INFO) ) //b02
    { 
        qlcloud_process_bindinfo(PKT_ID_RCV_BIND_INFO, (const char*)p_in_data);
    }
    else if ( pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_UNB) ) //b03
    {
        qlcloud_process_bindinfo(PKT_ID_REQ_UNB, (const char*)p_in_data);
    }
    else if ( pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_SHAREINFO_ADD) ) //b04
    {
        iot_status_cb(DEV_STA_USER_SHARE_INC, get_current_time());
    }
    else if ( pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_SHAREINFO_DELETE) ) //b05
    {
        iot_status_cb(DEV_STA_USER_SHARE_DEC, get_current_time());            
    }
    else if ( pkt_opcode == PKT_OP_CODE(PKT_ID_RES_PATCH_PASSIVE)       || 
              pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_PATCHS_CHECK_LIST)   || 
              pkt_opcode == PKT_OP_CODE(PKT_ID_RES_PATCH_CHUNK)         || 
              pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_PATCH_END)  )        //patch packet
    {
        if( ql_patch_handle )
        {
            ql_patch_handle(pkt_opcode, (const char*)p_in_data, (type_size_t)in_data_len, pkt_seq);
        }
        else
        {
            ql_log_err( ERR_EVENT_INTERFACE,"pach hdl undef!");
        }
    }
    else if(pkt_opcode == PKT_OP_CODE(PKT_ID_RES_SUB_FORCED_UNB))   //104
    {
        qlcloud_sub_forced_unb_data(pkt_seq, (const char*)p_in_data, (type_size_t)in_data_len);
    }
    
    #if 0/*log online switch*/
     else if ( pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_LOG) ) //b06
    {
        #ifdef LOCAL_SAVE
         qlcloud_process_log_sw((const char*) p_in_data, in_data_len);         
        #endif
    }
    #endif
exit:
    if ((g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_AES||g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SM4) && pkt_opcode != PKT_OP_CODE(PKT_ID_REQ_REG))
    {
        ql_free(p_in_data);
        p_in_data = NULL;
    }
}

#if (__SDK_PLATFORM__ == 0x1C)

#pragma comment(lib, "pthreadVC2.lib")
#include <pthread.h>

volatile int ql_logic_main_running = 0;
volatile int ql_logic_time_running = 0;

#define QL_THREAD_LOGIC_MAIN    1
#define QL_THREAD_LOGIC_TIME    2

static void thread_cancel(void *para)
{
    printf("thread %d: cancel called\n", (int)para);

    if ((int)para == QL_THREAD_LOGIC_MAIN)
    {
        ql_logic_main_running = 0;
    }
    else if ((int)para == QL_THREAD_LOGIC_TIME)
    {
        ql_logic_time_running = 0;
    }
}

#endif

void *ql_logic_main(void *para)
{
#if (__SDK_PLATFORM__ == 0x1C)
    ql_logic_main_running = 1;

    pthread_cleanup_push(thread_cancel, (void *)QL_THREAD_LOGIC_MAIN);
#endif

    for(;;)
    {
#if (__SDK_PLATFORM__ == 0x1C)
        pthread_testcancel();

//        printf("ql_logic_main() is running\r\n");
#endif

        ql_logic_endless_task();
    }

#if (__SDK_PLATFORM__ == 0x1C)
    pthread_cleanup_pop(0);
    printf("thread %s: finished\n", "ql_logic_main");

    ql_logic_main_running = 0;
#endif
}

void *ql_logic_time(void *para)
{
#if (__SDK_PLATFORM__ == 0x1C)
    ql_logic_time_running = 1;

    pthread_cleanup_push(thread_cancel, (void *)QL_THREAD_LOGIC_TIME);
#endif

    for(;;)
    {
#if (__SDK_PLATFORM__ == 0x1C)
        pthread_testcancel();

//        printf("ql_logic_time() is running\r\n");
#endif

        ql_logic_period_task();
        ql_sleep_s(1);
    }

#if (__SDK_PLATFORM__ == 0x1C)
    pthread_cleanup_pop(0);
    printf("thread %s: finished\n", "ql_logic_time");

    ql_logic_time_running = 0;
#endif
}

#ifdef LOCAL_SAVE
int ql_local_data_read( void* data, type_u32 data_len )
{
    int ret = 0;    
    ret = ql_mutex_lock(g_local_config_mutex);
    if(ret)
    {
        ret = ql_persistence_data_load(data, data_len);
        ql_mutex_unlock(g_local_config_mutex);
    }
    else
    {
        ret = -1;
    }
    return ret;
}
int ql_local_data_write( void* data, type_u32 data_len )
{
    int ret = 0;
    ret = ql_mutex_lock(g_local_config_mutex);
    if(ret)
    {        
        ret = ql_persistence_data_save(data, data_len);
        ql_mutex_unlock(g_local_config_mutex);
    }
    else
    {
        ret = -1;
    }
    return ret;
}

int ql_local_config_get_all(local_config_t** all_cfg, int* all_cfg_len)
{
    int cfg_len = 0;
    local_config_t* p = NULL;

    cfg_len = MEM_ALIGN_SIZE(g_local_config.cfg_user_data_len);
    cfg_len = sizeof(local_config_t) + cfg_len;
    
    p = ql_zalloc(cfg_len);
    if(p == NULL)
    {
        return -1;
    }
    
    if(ql_local_data_read((void*)p, cfg_len) < 0)
    {
        ql_free(p);
        return -1;
    }
    
    *all_cfg = p;
    *all_cfg_len = cfg_len;

    return 0;
}
//set the bit of dev_state to 1
int ql_local_dev_state_set(int dev_state)
{
    int cfg_len = 0, ret = -1;
    local_config_t* p = NULL;
    //the bit need to reset already is 1
    if(g_local_config.cfg_dev_local_state & dev_state)
    {
        #ifdef DEBUG_DATA_OPEN
            ql_printf("the bit need to set already is 1\r\n");
        #endif
        return 0;
    }
    ret = ql_local_config_get_all(&p, &cfg_len);
    if( 0 == ret && p->cfg_magic == LOCAL_CONFIG_MAGIC)
    {
        g_local_config.cfg_dev_local_state |= dev_state;
        p->cfg_dev_local_state = g_local_config.cfg_dev_local_state;
        ret = ql_local_data_write((void*)p, cfg_len);
    }
    else
    {
        ret = -1;
    }
    
    if(p)
    {
        ql_free(p);
    }

    return ret;
}
//set the bit of dev_state to 0
int ql_local_dev_state_reset(int dev_state)
{
    int cfg_len = 0, ret = -1;
    local_config_t* p = NULL;
    //the bit need to reset already is 0
    if((g_local_config.cfg_dev_local_state & dev_state) == 0)
    {
        #ifdef DEBUG_DATA_OPEN
            ql_printf("the bit need to reset already is 0\r\n");
        #endif
        return 0;
    }

    ret = ql_local_config_get_all(&p, &cfg_len);
    if( 0 == ret && p->cfg_magic == LOCAL_CONFIG_MAGIC)
    {
        g_local_config.cfg_dev_local_state &= ~dev_state;
        p->cfg_dev_local_state = g_local_config.cfg_dev_local_state;
        ret = ql_local_data_write((void*)p, cfg_len);
    }
    else
    {
        ret = -1;
    }
    
    if(p)
    {
        ql_free(p);
    }

    return ret;
}

int ql_local_dev_state_get(int dev_state)
{
    return ((g_local_config.cfg_dev_local_state & dev_state) != 0);
}

int ql_local_config_load( void )
{
    int ret = -1;

    if(g_local_config_mutex == NULL)
    {
        g_local_config_mutex = ql_mutex_create();
        if(g_local_config_mutex == NULL)
        {      
            return -1;
        }
    }
    //local config has been load to memory
    if(g_local_config.cfg_magic == LOCAL_CONFIG_MAGIC)
    {
        return 0;
    }
    ql_memset(&g_local_config, 0, sizeof(local_config_t));

    ret = ql_local_data_read(&g_local_config, sizeof(local_config_t) );
    if(0 != ret)
    {
        return -1;
    }

    if(g_local_config.cfg_magic == LOCAL_CONFIG_MAGIC)
    {
        //do nothing
        ql_log_info("load local data [%d] bytes\r\n", g_local_config.cfg_user_data_len);
    
        if(g_client_handle.ota_type == OTA_TYPE_BASIC || qlcloud_is_version_valid((const type_u8*)g_local_config.cfg_brk_ota_info.version[g_local_config.cfg_brk_ota_info.owner]) < 0)
        {
            if(ql_strlen(g_local_config.cfg_brk_ota_info.version[g_local_config.cfg_brk_ota_info.owner]) == 0 && g_local_config.cfg_brk_cur_offset == 0 && g_local_config.cfg_brk_cur_crc == 0)
            {
            }
            else
            {                
                /*local break ota*/        
                ql_memset(&g_local_config.cfg_brk_ota_info, 0x00, sizeof(binfile_info));
                g_local_config.cfg_brk_cur_offset = 0;
                g_local_config.cfg_brk_cur_crc = 0;
                
                ret = ql_local_data_write(&g_local_config, sizeof(local_config_t));
            }
        }
        
        #ifdef DEBUG_DATA_OPEN            
            char mcu_ver[6] = {0};
            char wifi_ver[6] = {0};
            ql_memcpy(mcu_ver, g_local_config.cfg_mcu_ota_ver, 5);
            ql_memcpy(wifi_ver, g_local_config.cfg_wifi_ota_ver, 5);
            ql_printf("local rst state:%d, unbound:%d, mcu ver:%s, wifi ver:%s, brk_v: %s, brk_c: %d, brk_s: %d, brk_cur_crc: %d\r\n", ql_local_dev_state_get(DEV_STATE_RST),ql_local_dev_state_get(DEV_STATE_UNB), mcu_ver, wifi_ver,
                                                             g_local_config.cfg_brk_ota_info.version[g_local_config.cfg_brk_ota_info.owner], g_local_config.cfg_brk_ota_info.fcrc, g_local_config.cfg_brk_cur_offset, g_local_config.cfg_brk_cur_crc);
        #endif
    }
    else//config is null,load config first time
    {
        #ifdef DEBUG_DATA_OPEN
            ql_printf("config is null,load config first time\r\n");
        #endif
        /*local config data init*/
        g_local_config.cfg_magic = LOCAL_CONFIG_MAGIC;
        g_local_config.cfg_user_data_len = 0;
        g_local_config.cfg_dev_local_state = 0;
        ql_memcpy(g_local_config.cfg_mcu_ota_ver, g_client_handle.mcu_version, 5);
        ql_memcpy(g_local_config.cfg_wifi_ota_ver, DEVICE_VERSION, 5);

        /*local log data init*/

        ql_memset(&g_local_config.log_disconn, 0x00, sizeof(LOG_DISCONN_INTO_T));        
        ql_memset(&g_local_config.log_conn_exc, 0x00, sizeof(LOG_CONN_EXC_INTO_T));        
        ql_memset(g_local_config.log_unb_exc_ts, 0x00, 5);        
        ql_memset(g_local_config.log_rst_exc_ts, 0x00, 5);
        ql_memset(g_local_config.log_err, 0x00, LOG_ERR_MAX * LOG_ERR_LEN_MAX );

        /*local break ota*/        
        ql_memset(&g_local_config.cfg_brk_ota_info, 0x00, sizeof(binfile_info));
        g_local_config.cfg_brk_cur_offset = 0;
        g_local_config.cfg_brk_cur_crc = 0;

        ret = ql_local_data_write(&g_local_config, sizeof(local_config_t));
    }
    return ret;
}

int ql_local_config_erase( void )
{
    /*local config*/
    g_local_config.cfg_magic = LOCAL_CONFIG_MAGIC;
    g_local_config.cfg_user_data_len = 0;
    ql_memcpy(g_local_config.cfg_mcu_ota_ver, "00.00", 5);
    ql_memcpy(g_local_config.cfg_wifi_ota_ver, "00.00", 5);
    /*local log*/
    ql_memset(&g_local_config.log_disconn, 0x00, sizeof(LOG_DISCONN_INTO_T));        
    ql_memset(&g_local_config.log_conn_exc, 0x00, sizeof(LOG_CONN_EXC_INTO_T));        
    ql_memset(g_local_config.log_unb_exc_ts, 0x00, 5);        
    ql_memset(g_local_config.log_rst_exc_ts, 0x00, 5);
    ql_memset(g_local_config.log_err, 0x00, LOG_ERR_MAX * LOG_ERR_LEN_MAX );

    return ql_local_data_write(&g_local_config, sizeof(local_config_t));
}

int ql_local_ota_ver_set(type_s8 ota_owner, const type_u8 ver[5], type_u8 ota_status)
{
#ifdef DEBUG_DATA_OPEN
    char local[6] = {0};
    ql_memcpy(local, ver,5);
#endif
   
    int cfg_len = 0, ret = -1;
    local_config_t* p = NULL;
    if(NULL == ver)
    {
       return -1;
    }

    ret = ql_local_config_get_all(&p, &cfg_len);
    
    if( 0 == ret && p->cfg_magic == LOCAL_CONFIG_MAGIC)
    {
        if(ota_owner == OTA_OWNER_MCU)
        {
            ql_log_info("g_client_handle.ota_mcu_type:%d\r\n", g_client_handle.ota_mcu_type);
            if((OTA_CHECK_TYPE_TIMER == g_client_handle.ota_mcu_type) || (OTA_CHECK_TYPE_APPDOWN == g_client_handle.ota_mcu_type) || 
                 (OTA_FILEINFO_BACK == ota_status))
            {
                ql_memcpy(g_local_config.cfg_mcu_ota_ver, ver, 5);
                ql_memcpy(p->cfg_mcu_ota_ver, ver, 5);
                #ifdef DEBUG_DATA_OPEN
                    ql_printf("mcu ver set:%s\r\n",local);
                #endif
                ret = ql_local_data_write((void*)p, cfg_len);
                g_client_handle.ota_mcu_type = (OTA_CHECK_TYPE_APPDOWN == g_client_handle.ota_mcu_type) ? OTA_CHECK_TYPE_TIMER : g_client_handle.ota_mcu_type;
            }
            else//passive or compels type need not save the version to local storage area
            {//only OTA_FILECHUNK_END status can change ota_type
                g_client_handle.ota_mcu_type = OTA_CHECK_TYPE_TIMER;
                ret = 0;
            }
        }
        #if (__SDK_TYPE__== QLY_SDK_BIN)
            else if(ota_owner == OTA_OWNER_WIFI)
            {
                if((OTA_CHECK_TYPE_TIMER == g_client_handle.ota_wifi_type) || (OTA_CHECK_TYPE_APPDOWN == g_client_handle.ota_wifi_type) || 
                  (OTA_FILEINFO_BACK == ota_status))
                {
                    ql_memcpy(g_local_config.cfg_wifi_ota_ver, ver, 5);
                    ql_memcpy(p->cfg_wifi_ota_ver, ver, 5);
                    #ifdef DEBUG_DATA_OPEN
                        ql_printf("moudle ver set:%s\r\n",local);
                    #endif
                    ret = ql_local_data_write((void*)p, cfg_len);
                    g_client_handle.ota_wifi_type = (OTA_CHECK_TYPE_APPDOWN == g_client_handle.ota_wifi_type) ? OTA_CHECK_TYPE_TIMER : g_client_handle.ota_wifi_type;
                }
                else//passive or compels type need not save the version to local storage area
                {//only OTA_FILECHUNK_END status can change ota_type
                    g_client_handle.ota_wifi_type = OTA_CHECK_TYPE_TIMER;
                    ret = 0;
                }
            }
        #endif
        else{ret = -1;}
    }
    else
    {
        ret = -1;
    }

    if(p)
    {
        ql_free(p);
    }

    return ret;
}

int ql_local_ota_ver_init( void )
{
    int cfg_len = 0, ret = -1;
    local_config_t* p = NULL;
    int is_need_save = 0;

    ret = ql_local_config_get_all(&p, &cfg_len);
    if( 0 == ret && p->cfg_magic == LOCAL_CONFIG_MAGIC)
    {        
        if(ql_memcmp(g_client_handle.mcu_version, g_local_config.cfg_mcu_ota_ver, 5) == 0 && 
            ql_memcmp(DEVICE_VERSION, g_local_config.cfg_wifi_ota_ver, 5) == 0)
        {
            #ifdef DEBUG_DATA_OPEN
                ql_printf("ver same\r\n");
            #endif
            ret = 0;
        }
        else
        {
            #ifdef SUPPORT_REBOOT_OTA
                if(ql_memcmp(g_client_handle.mcu_version, g_local_config.cfg_mcu_ota_ver, 5) > 0)
            #else
                //moudle reboot after downloaded chunks
                if(ql_memcmp(g_client_handle.mcu_version, g_local_config.cfg_mcu_ota_ver, 5) != 0)
            #endif
                {
                    ql_memcpy(g_local_config.cfg_mcu_ota_ver, g_client_handle.mcu_version, 5);
                    ql_memcpy(p->cfg_mcu_ota_ver, g_client_handle.mcu_version, 5);
                    is_need_save = 1;
                }

            #ifdef SUPPORT_REBOOT_OTA
                if(ql_memcmp(DEVICE_VERSION, g_local_config.cfg_wifi_ota_ver, 5) > 0)
            #else
                if(ql_memcmp(DEVICE_VERSION, g_local_config.cfg_wifi_ota_ver, 5) != 0)
            #endif
                {
                    ql_memcpy(g_local_config.cfg_wifi_ota_ver, DEVICE_VERSION, 5);
                    ql_memcpy(p->cfg_wifi_ota_ver, DEVICE_VERSION, 5);
                    is_need_save = 1;
                }
            if(is_need_save)
            {
                #ifdef DEBUG_DATA_OPEN                
                    char local[6] = {0};    
                    ql_memcpy(local, p->cfg_mcu_ota_ver, 5);
                    ql_printf("mcu init save:%s\r\n", local);
                    ql_memcpy(local, p->cfg_wifi_ota_ver, 5);
                    ql_printf("moudle init save:%s\r\n", local);
                #endif
                ret = ql_local_data_write((void*)p, cfg_len);
            }
            else
            {
                ret = 0;
            }
        }
        ql_memcpy(g_ota.otafileinfo.version[OTA_OWNER_MCU], g_local_config.cfg_mcu_ota_ver, 5);
        ql_memcpy(g_ota.otafileinfo.version[OTA_OWNER_WIFI], g_local_config.cfg_wifi_ota_ver, 5);
    }
    else
    {
        ret = -1;
    }
    
    if(p)
    {
        ql_free(p);
    }
    return ret;
}

int32_t ql_local_log_save(uint8_t type, uint8_t event, void * data, uint32_t ts)
{
    int i = 0;
    int ret = 0, cfg_len = 0;
    local_config_t *p = NULL;

    ret = ql_local_config_get_all(&p, &cfg_len);
    
    if( 0 == ret && p->cfg_magic == LOCAL_CONFIG_MAGIC)
    {       
        if(type == LOG_TYPE_WARN)
        {
            switch(event)
            {
            case WARN_EVENT_DISCONN:    
                g_local_config.log_disconn.timestamp = get_current_time(); 
                g_local_config.log_disconn.statu_type = *(int *)data;
                p->log_disconn.timestamp = g_local_config.log_disconn.timestamp;
                p->log_disconn.statu_type = g_local_config.log_disconn.statu_type;
                break;
            case WARN_EVENT_CONN_EXC:             
                g_local_config.log_conn_exc.timestamp = get_current_time();                   
                p->log_conn_exc.timestamp = g_local_config.log_conn_exc.timestamp;
                p->log_conn_exc.conn_exc_ts = g_local_config.log_conn_exc.conn_exc_ts;
                p->log_conn_exc.last_rv_ts= g_local_config.log_conn_exc.last_rv_ts;
                break;
            case WARN_EVENT_UNB_EXC:
                UINT32_TO_STREAM_f(g_local_config.log_unb_exc_ts, ts);
                g_local_config.log_unb_exc_ts[4] = *(char *)data;     
                ql_memcpy(p->log_unb_exc_ts, g_local_config.log_unb_exc_ts, 5);
                break;
            case WARN_EVENT_RST_EXC:               
                UINT32_TO_STREAM_f(g_local_config.log_rst_exc_ts, ts);
                g_local_config.log_rst_exc_ts[4] = *(char *)data;
                ql_memcpy(p->log_unb_exc_ts, g_local_config.log_unb_exc_ts, 5);
                break;
            default:
                break;
            }
            
            ret = ql_local_data_write((void*)p, cfg_len);
        } 
        else if(type == LOG_TYPE_ERR && data != NULL)
        {
            for(i = 0; i < LOG_ERR_MAX; i++)
            {
                if(0 != g_local_config.log_err[i][4])
                    continue;
                
                UINT32_TO_STREAM_f(g_local_config.log_err[i], ts);
                g_local_config.log_err[i][4] = event;     
                
                if(ql_strlen(data) > LOG_ERR_LEN_MAX - 5 )
                {
                    ql_log_err(ERR_EVENT_LOCAL_DATA, "log_data len err");                    
                    if(p)
                    {
                        ql_free(p);
                        p = NULL;
                    }
                    return -1;
                }                
                ql_memcpy(g_local_config.log_err[i]+5, data, ql_strlen(data) + 1);
                
                ql_memcpy(p->log_err[i], g_local_config.log_err[i], ql_strlen(g_local_config.log_err[i]) + 1);                          
                ret = ql_local_data_write((void*)p, cfg_len);
                break;
            }
            if(i == LOG_ERR_MAX)
                ql_log_err(ERR_EVENT_NULL, "log get max");               
        }
        
    }
    else
    {
        ret = -1;
    }
     
    if(p)
    {
        ql_free(p);
        p = NULL;
    }
    return ret;
}


void ql_local_log_to_cloud(uint8_t type, uint8_t event, void * data, uint32_t ts)
{
    int ret = 0, seq = 0;

    if((type == LOG_TYPE_INFO || (type == LOG_TYPE_WARN && event == WARN_EVENT_P_BIND_EXC) || (type == LOG_TYPE_ERR && NULL != data) ) && is_application_authed())
    {
        ret = log_up_add( type, event, data, get_current_time());
        if(ret == 0)
            log_upload_dps(&seq);     
    }        
    else
    {
        ql_local_log_save(type, event, data, ts);
    }
}

void ql_local_uplog_to_cloud()
{   
    int ret = 0, cfg_len = 0;
    local_config_t *p = NULL;
    
#if 0/*log online switch*/
    if(get_log_online_switch())
#endif
    {
        int ret = 0, seq = 0;
        uint8_t log_flag = 0;

        char log_unb_info, log_rst_info;
        if((ret = ql_local_config_load()) < 0)
        {
            return ;
        }
#if 0/*log online switch*/
        if(get_log_online_switch() & 1)
#endif
        {               
            if(g_local_config.log_disconn.timestamp)
            {            
                log_flag = 1;
                log_up_add( LOG_TYPE_WARN, WARN_EVENT_DISCONN, &g_local_config.log_disconn.statu_type, g_local_config.log_disconn.timestamp);
            }
            
            if(g_local_config.log_conn_exc.timestamp)
            {   
                CLI_STATU_T statu_type = 2;
                log_flag = 1;
                log_up_add( LOG_TYPE_WARN, WARN_EVENT_CONN_EXC, &statu_type, g_local_config.log_conn_exc.timestamp);
            }
            if( g_local_config.log_unb_exc_ts[4])
            {       
                log_flag = 1;
                log_unb_info = g_local_config.log_unb_exc_ts[4];
                log_up_add( LOG_TYPE_WARN, WARN_EVENT_UNB_EXC, (void *)&log_unb_info, STREAM_TO_UINT32_f(g_local_config.log_unb_exc_ts,0));     
            }
          
            if(g_local_config.log_rst_exc_ts[4])
            {  
                log_flag = 1;
                log_rst_info = g_local_config.log_rst_exc_ts[4];
                log_up_add( LOG_TYPE_WARN, WARN_EVENT_RST_EXC, (void *)&log_rst_info, STREAM_TO_UINT32_f(g_local_config.log_rst_exc_ts,0));
            }
        }
 #if 0       
        if(get_log_online_switch() & 2)
 #endif
        {
            int i ;
            uint32_t log_err_ts = 0;
            uint8_t log_err_event = 0;
            for(i = 0; i < LOG_ERR_MAX; i++)
            {
                log_err_ts = 0;
                log_err_event = 0;
                if(g_local_config.log_err[i][4])
                {    
                    log_flag = 1;
                    log_err_event = g_local_config.log_err[i][4];
                    log_up_add( LOG_TYPE_ERR, log_err_event, (void *)(g_local_config.log_err[i] + 5), STREAM_TO_UINT32_f(g_local_config.log_err[i],0));
                }
            }
        }

        if(1 == log_flag)
        {            
            /*1. send log to cloud*/
            log_upload_dps(&seq);

            /*2. clean log in flash */
            ql_memset(&g_local_config.log_disconn, 0x00, sizeof(LOG_DISCONN_INTO_T));        
            ql_memset(&g_local_config.log_conn_exc, 0x00, sizeof(LOG_CONN_EXC_INTO_T));        
            ql_memset(g_local_config.log_unb_exc_ts,0x00,sizeof(g_local_config.log_unb_exc_ts));     
            ql_memset(g_local_config.log_rst_exc_ts,0x00,sizeof(g_local_config.log_rst_exc_ts));     
            ql_memset(g_local_config.log_err,0x00,sizeof(g_local_config.log_err));     

            #if 0/*log online switch*/
            g_local_config.log_control = g_client_handle.log_online;
            #endif
            
            ret = ql_local_config_get_all(&p, &cfg_len);
            if( 0 == ret && p->cfg_magic == LOCAL_CONFIG_MAGIC)
            {
                ql_memcpy(p, &g_local_config, sizeof(g_local_config));
                ret = ql_local_data_write((void*)p, cfg_len);
            }
            else
            {
                ret = -1;
            }
        }
    }

    if(p)
    {
        ql_free(p);
        p = NULL;
    }
}

type_u32 ql_local_break_ota_info_set( type_u32 set_type)
{
    int cfg_len = 0, ret = -1;
    local_config_t * p = NULL;

    
    ret = ql_local_config_get_all(&p, &cfg_len);
    if(ret == 0 && p != NULL && p->cfg_magic == LOCAL_CONFIG_MAGIC)
    {
        
        if(set_type == LOCAL_BREAK_OTA_SET_T_ALL)
        {
            ql_memcpy(&p->cfg_brk_ota_info, &g_local_config.cfg_brk_ota_info, sizeof(binfile_info));
        }

        p->cfg_brk_cur_offset = g_local_config.cfg_brk_cur_offset;        
        p->cfg_brk_cur_crc = g_local_config.cfg_brk_cur_crc;
        if(ql_local_data_write((void*)p, cfg_len) == -1)
        {
            ql_log_err(ERR_EVENT_INTERFACE,"ota chunk w err");
            ret = -1;
        }
    }
    else
    {
        ql_log_err(ERR_EVENT_INTERFACE,"brk_o set err");
        ret = -1;
    }

    if(p)
    {
        ql_free(p);
        p = NULL;
    }
    return ret;
}

type_u32 ql_local_break_ota_info_get(char brk_v[5], type_u16* brk_c, type_u32* brk_s)
{
    int cfg_len = 0, ret = -1;
    local_config_t * p = NULL;

    ret = ql_local_config_get_all(&p, &cfg_len);
    if(ret == 0 && p != NULL && p->cfg_magic == LOCAL_CONFIG_MAGIC)
    {
        ql_memcpy(brk_v, p->cfg_brk_ota_info.version[g_local_config.cfg_brk_ota_info.owner], 5);
        *brk_c = p->cfg_brk_ota_info.fcrc;
        *brk_s = p->cfg_brk_cur_offset;        
    }
    else
    {
        ql_log_err(ERR_EVENT_INTERFACE,"brk_o get err");
        ret = -1;
    }

    if(p)
    {
        ql_free(p);
        p = NULL;
    }
    return ret;
}

#endif

#if (__SDK_TYPE__ == QLY_SDK_BIN)
type_u32 serial_mutex_create()
{
    g_client_handle.serial_mutex = ql_mutex_create();
    if(g_client_handle.serial_mutex == NULL)
    {
        return -1;
    }
    return 0;
}

int32_t set_wifi_config(uint8_t mode,char * ap_ssid, char * ap_pwd)
{
     wifi_info_t info;    
     int ret = 0;
     
    /*1.get wifi info*/
     ql_memset(&info, 0x00, sizeof(info));
     if(ap_ssid == NULL || ap_pwd == NULL )
     {
        ql_log_err(ERR_EVENT_RX_DATA, "ap config NULL");
        return -1;
     }
     
     if( ql_strlen(ap_ssid) > 0 )   
        ql_memcpy(info.ssid, ap_ssid, 32);
     else
     {
        ql_log_err(ERR_EVENT_RX_DATA, "ssid err");
        return -1;
     }
     
     if( ql_strlen(ap_pwd) > 0 )
        ql_memcpy(info.password, ap_pwd, 64);
     
     ql_log_info("[Reset mode:%d]:ssid:%s,password:%s \r\n", mode, info.ssid, info.password);

     /*2.set wifi*/
     if( (ret = ql_set_wifi_info(mode,&info)) < 0)
     {          
         ql_log_err(ERR_EVENT_INTERFACE,"set wifi err");            
         return -1;
     } 
    return 0;
}


#endif

void set_download_status(DEV_DOWNLOAD_STATUS_T statu)
{
    g_client_handle.dev_downloading = statu;
}

int get_download_status()
{
    return g_client_handle.dev_downloading;
}
