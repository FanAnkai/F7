#ifndef QL_LOGIC_CONTEXT_H_
#define QL_LOGIC_CONTEXT_H_

#include "ql_include.h"
#ifdef __cplusplus
extern "C" {
#endif

extern type_u8 DEVICE_VERSION[];

#define PROTOCOL_VERSION                0x0201

#define SECONDS_OF_10_MINUTES           600
#define SECONDS_OF_30_MINUTES           1800
#define SECONDS_OF_HEART_INTERVAL       180

#define CONN_INTERVAL_LICENCE_LIMIT     SECONDS_OF_30_MINUTES
#define CONN_INTERVAL_NORMAL            2

#define APPLICTION_TIMEOUT              20
#define COMMAND_TIMEOUT                 2000

enum ERROR_CODE {
    ERRNO_OK                            = 0,
    ERRNO_ERR                           = 1,
    ERRNO_VERIFY_PRODUCTSIGN_FAILD      = 2,// Confirm the sdk is online or offline
    ERRNO_VERIFY_DEVSIGN_FAILD          = 3,
    ERRNO_TOKEN_OUTOFDATE               = 4,
    ERRNO_GENERATE_SERVICE_KEY_FAILD    = 5,
    ERRNO_UNKNOWN_MESSAGEDATA           = 6,
    ERRNO_GETDATA_FAILD                 = 7,
    ERRNO_GETREGISTER_INFO_FAILD        = 8,
    ERRNO_GETAUTH_INFO_FAILD            = 9,
    ERRNo_PeerNotOnline                 = 10,
    ERRNO_LicenceLimit                  = 11,
    ERRNO_MacBindOtherID                = 12,
    ERRNO_Unbound                       = 13,
    ERRNO_RstFail                       = 14,
    ERROR_UnboundFail                   = 15,/*unbound*/
    ERROR_SubNotOnline                  = 16,
    ERROR_SubBindOther                  = 17,
    ERROR_SubBindNone                   = 18,
    ERRNO_MAX,
};

enum NETWORK_STATUS {
    NET_INIT = 1,   ///app initialization

    NET_DISPATCHING, /* 2  */
    NET_DISPATCHFAIL,
    NET_DISPATCHED, ///no use

    NET_MQTT_DISCONNECT,// 5
    NET_MQTT_CONNECTED,

    NET_REGINFING, /* 7 */
    NET_REGINFAIL,
    NET_REGINFED,

    NET_AUTHING, /* 10 */
    NET_AUTHFAIL,
    NET_AUTHED,

#if (__SDK_TYPE__== QLY_SDK_BIN)
    NET_DP_GETING, /* 13 */
    NET_DP_GETFAIL,
    NET_DP_GETED,
#endif
};

enum OTA_STATUS {
    OTA_INIT = 0,
    OTA_VERSION_CHECK,
    OTA_FILEINFO_GET,           /* 2 */
    OTA_FILEINFO_BACK,
    OTA_FILECHUNK_BEGIN,
    OTA_FILECHUNK_BEGIN_BACK,   /* 5 */
    OTA_FILECHUNK_STREAM,
    OTA_FILECHUNK_STREAM_BACK,
    OTA_FILECHUNK_END,          /* 8 */
    OTA_FILECHUNK_END_BACK,
    OTA_UPGRADE_CMD_GET,
    OTA_UPGRADE_CMD_BACK,       /* 11 */
};

#define OTA_CHECK_TYPE_TIMER        0   //定时
#define OTA_CHECK_TYPE_PASSIVE      1   //被动验证固件
#define OTA_CHECK_TYPE_NEED_PASSIVE 2   //需要被动验证固件
#define OTA_CHECK_TYPE_NEED_COMPELS 3   //需要强制升级，只支持MCU升级
#define OTA_CHECK_TYPE_NEED_APPDOWN 4   //需要APP触发下载升级
#define OTA_CHECK_TYPE_APPDOWN      5   //APP触发下载升级
#define OTA_CHECK_TYPE_COMPELS      9   //强制，只支持MCU升级

#define OTA_CHECK_RANDOM_SEC        86400   // 24 hours
#define OTA_STATUS_TIMEOUT_SEC      120     // 2 minutes
#define OTA_WIFI_CHECK_DELAY_SEC    600     // 10  minutes
#define OTA_CHUNK_RETRAN_SEC        30      // 30
#define OTA_CHUNK_RETRAN_TIMES      3       // 3 times

typedef struct _binfile_info {
    type_u32        flen;
    type_u16        fcrc;
    type_u8         owner;
    type_u8         version[2][5];
} binfile_info;

typedef struct _ota_upgrade_ctx {
    binfile_info    otafileinfo;
    type_u32        cur_len;
    type_u32        exptect_offset;
    type_u16        cur_crc;
} ota_upgrade_ctx;

typedef enum RST_STATE
{
    RST_STATE_INIT,      /* 0 */
    RST_STATE_RST_ING,
    RST_STATE_RST_FAIL,  /* 2 */
    RST_STATE_RST_ED,
    RST_STATE_TIMEOUT    /* 4 */
}RST_STATE_t;

/*bind state*/
typedef enum BIND_STATE
{
    BIND_STATE_INIT,      /* 0 */
    BIND_STATE_ING,
    BIND_STATE_FAIL,      /* 2 */
    BIND_STATE_ED,
    BIND_STATE_TIMEOUT    /* 4 */
}BIND_STATE_t;

/*unbound*/
typedef enum UNBOUND_STATE
{
    UNB_STATE_INIT,      /* 0 */
    UNB_STATE_UNB_ING,
    UNB_STATE_UNB_FAIL,  /* 2 */
    UNB_STATE_UNB_ED,
    UNB_STATE_TIMEOUT    /* 4 */
}UNBOUND_STATE_t;

struct cloud_event {
    int     ev_qos;
    int     ev_seq;
    int     ev_len;
    char*   ev_data;
};

#if __SDK_PLATFORM__ == 0x16 || __SDK_PLATFORM__ == 0x17 || __SDK_PLATFORM__ == 0x1A
#define QUEUESIZE                   64
#else
#define QUEUESIZE                   32
#endif

#define TOPIC_LEN                   64
#define DEV_UUID_LEN                32

#define AESKEY_LEN                  16
#define DEVICE_ID_LEN               16
#define IOTID_LEN                   16
#define TOKEN_LEN                   44

#define VERSION_LEN                 33
#define SUB_DEV_ID_MAX              32

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

typedef enum DEV_DOWNLOAD_STATUS {
    DEV_DOWNLOAD_FREE       = 0,        //当前设备下载状态为空闲
    DEV_DOWNLOAD_OTA        = 1,        //当前设备下载状态为OTA下载中
    DEV_DOWNLOAD_PATCH      = 2,        //当前设备下载状态为热补丁下载中
    DEV_DOWNLOAD_FILE       = 3,        //当前设备下载状态为文件下载中
}DEV_DOWNLOAD_STATUS_T;

#define PACKET_HEAD_LENGTH          16
#define MAX_BUFFER_SIZE             4194304
#define DEFAULT_BUFFER_SIZE         1024
#define PADDING_BUFFER_SIZE         128
//16 frame head + 74 json = 90 + 22 mqtt head + n queue sign = 112+2

struct client_handle {
    type_u8                  datatype;      //数据类型  JSON  PROTOBUF
    type_s32                 encrypt_type;
    type_s32                 recvbuf_size;
    type_s32                 sendbuf_size;
    type_u8*                 recvbuf;
    type_u8*                 sendbuf;
    type_u8                  pub_topic[TOPIC_LEN];
    type_u8                  sub_topic[TOPIC_LEN];
    type_u8                  mac_str[DEV_UUID_LEN+1];
    #ifdef USE_SMPU
    type_u8*                 product_aes_key;
    type_u8*                 device_aes_key;
    type_u8*                 server_aes_key;
    #else
    type_u8                  product_aes_key[AESKEY_LEN];
    type_u8                  device_aes_key[AESKEY_LEN+1];
    type_u8                  server_aes_key[AESKEY_LEN+1];
    #endif
    type_u32                 product_id;
    type_u8                  mcu_version[6];
    type_u8                  dev_mac_hex[6];
    type_u8                  deviceid[DEVICE_ID_LEN+1];
    type_u8                  iotid[IOTID_LEN+1];
    type_u8                  iottoken[TOKEN_LEN+1];
    type_u8                  dev_status;
    
    type_u8                  dev_rst_status;/*reset恢复出厂*/
    type_u32                 dev_rst_timestamp;
    type_u32                 dev_rst_timeout;

    type_u8                  dev_unb_status;/*unbound*/
    type_u32                 dev_unb_timestamp;    
    type_u32                 dev_unb_timeout;
    
    type_u8                  dev_bind_status;/*bind*/
    type_u32                 dev_bind_timestamp;
    type_u32                 dev_bind_timeout;
    
    struct ql_socket_t*      client_socket;

    volatile type_s32        client_status;

    type_u32                 conn_interval;

    type_u8                  ota_cur_owner;

    type_u8                  ota_mcu_status;
    type_u8                  ota_mcu_type;
    type_u32                 ota_mcu_chunk_size;
    type_u32                 ota_mcu_stat_chg_tm;
    type_u32                 ota_mcu_check_tm;
    OTA_TYPE                 ota_type;

    #if (__SDK_TYPE__== QLY_SDK_BIN)
    type_u8                  ota_wifi_status;
    type_u8                  ota_wifi_type;
    type_u32                 ota_wifi_chunk_size;
    type_u32                 ota_wifi_stat_chg_tm;
    type_u32                 ota_wifi_check_tm;
    #endif
    char                     sub_dev_id[SUB_DEV_ID_MAX+1];
    
    char*                    dispatch_ip;
    type_u16                 dispatch_port;
    char                     sn[17];
    type_u8                  sub_opt_type;
    DEV_DOWNLOAD_STATUS_T    dev_downloading;
    #if 0/*log online switch*/
    type_u8                  log_online;
    #endif
    type_u32                 data_act;

    cJSON*                   dp_total_json_obj;
    ql_mutex_t*              dp_mutex;

    cJSON*                   sub_total_json_arr;
    ql_mutex_t*              sub_mutex;
    
    cJSON*                   log_total_json_arr;
    ql_mutex_t*              log_mutex;
#if (__SDK_TYPE__== QLY_SDK_BIN)
    ql_mutex_t*              serial_mutex;
#endif
};

extern struct client_handle g_client_handle;

typedef enum CLI_STATU_TYPE {
    CLI_DISCONN_TYPE_NULL               = 0,        //无
    /*disconnect reason start*/
    CLI_DISCONN_TYPE_REG_ERR            = 1,        //注册失败
    CLI_DISCONN_TYPE_NETWK_POOR         = 2,        //网络连接超时
    CLI_DISCONN_TYPE_RD_CLS             = 3,        //socket read 失败
    CLI_DISCONN_TYPE_WRT_CLS            = 4,        //socket write失败
    CLI_DISCONN_TYPE_UNB_END            = 5,        //解绑完成后重连云端
    CLI_DISCONN_TYPE_UNB_FAIL           = 6,        //解绑失败或解绑超时主动断开重连
    CLI_DISCONN_TYPE_RST_LINK           = 7,        //重新连接
    /*disconnect reason end*/
}CLI_STATU_T;

#ifdef LOCAL_SAVE

#define DEV_STATE_PRODUCT_TEST              ((type_u16)0x0008)
#define DEV_STATE_AP                        ((type_u16)0x0004)
#define DEV_STATE_SMART_LINK                ((type_u16)0x0002)
#define DEV_STATE_RST                       ((type_u16)0x0001)
#define DEV_STATE_UNB                       ((type_u16)0x0002)

#define LOCAL_CONFIG_MAGIC      0xA5A55A5A
#define LOCAL_CONFIG_PAGE_SIZE  (4096)
#define LOG_ERR_MAX         9
#define LOG_ERR_LEN_MAX     26

#define LOCAL_BREAK_OTA_SET_T_ALL    0 
#define LOCAL_BREAK_OTA_SET_T_OFFSET 1

typedef struct LOG_DISCONN_INFO
{
    type_u32        timestamp;
    CLI_STATU_T     statu_type;
}LOG_DISCONN_INTO_T;

typedef struct LOG_CONN_EXC_INFO
{
    type_u32        timestamp;
    type_u32        last_rv_ts;
    type_u32        conn_exc_ts;
}LOG_CONN_EXC_INTO_T;

typedef struct local_config     /* sizeof(local_config_t) = 220 bytes */
{
/*local config data:size must be 32 bytes*/
    type_u32                    cfg_magic;
    type_u16                    cfg_user_data_len;
    type_u16                    cfg_dev_local_state;    
    type_u8                     cfg_mcu_ota_ver[5];
    type_u8                     cfg_wifi_ota_ver[5];
    type_u8                     cfg_pad[14];
    
/*local log data:size 30 + 26*9 + 24 = 288 bytes*/    
    LOG_DISCONN_INTO_T          log_disconn;                                            //长连接断开时间戳
    LOG_CONN_EXC_INTO_T         log_conn_exc;                                           //网络异常时间戳
    type_u8                     log_unb_exc_ts[5];                                      //解绑异常            |ts(4byte)|info(1byte)|
    type_u8                     log_rst_exc_ts[5];                                      //恢复出厂异常          |ts(4byte)|info(1byte)|
    type_u8                     log_err[LOG_ERR_MAX][LOG_ERR_LEN_MAX];                  //错误信息            |ts(4byte)|event(1byte)|info(21byte)|        
    #if 0/*log online switch*/    
    type_u8                     log_control;                                            //控制上传log的类型
    type_u8                     log_pad[9];  
    #else
    type_u8                     log_pad[24];
    #endif

    /*break ota:size 32*/    
    type_u32        cfg_brk_cur_offset;                                      //断点续传当前的ota偏移量
    type_u16        cfg_brk_cur_crc;                                         //断点续传当前crc
    binfile_info    cfg_brk_ota_info;                                        //断点续传ota信息
    type_u8         cfg_brk_pad[4]; 

}local_config_t;

extern int ql_local_dev_state_get(int dev_state);
extern int ql_local_dev_state_set(int dev_state);
extern int ql_local_dev_state_reset(int dev_state);
extern int ql_local_config_load( void );
extern int ql_ota_ver_set(type_s8 ota_owner, const type_u8 ver[5]);
extern int ql_local_ota_ver_set(type_s8 ota_owner, const type_u8 ver[5], type_u8 ota_status);
extern int32_t ql_local_log_save(uint8_t type, uint8_t event, void * data, uint32_t ts);
extern void ql_local_log_to_cloud(uint8_t type, uint8_t event, void * data, uint32_t ts);
extern void ql_local_uplog_to_cloud(void);
extern type_s32 log_up_add(uint8_t type, uint32_t event, void * data, uint32_t ts);
extern type_s32 log_upload_dps( type_u32* data_seq );

extern type_u32 ql_local_break_ota_info_get(char brk_v[5], type_u16* brk_c, type_u32* brk_s);
extern type_u32 ql_local_break_ota_info_set( type_u32 set_type);
#endif

extern void *ql_logic_time(void *para);
extern void *ql_logic_main(void *para);
extern int qlcloud_init_client_context(struct iot_context* ctx);
extern type_u32 get_current_time(void);
extern type_s32 is_application_authed(void);
extern int cloud_request_send_pkt(int pkt_id, type_u32 seq, void* arg);
extern type_s32 ql_client_get_encrypt_type(void);
extern void ql_client_set_status(int client_status, CLI_STATU_T statu_type);
extern void set_current_time(type_u32 tick);
extern void set_ota_cur_owner(type_u8 owner);
extern void set_ota_mcu_status(type_s32 type);
extern type_s32 get_ota_mcu_status();
extern void  passive_start_response(type_s32 seq, type_s32 error);
extern void  compels_upgrade_response(type_s32 seq, type_s32 error);
extern void  binfile_info_response(type_s32 seq, type_s32 error, type_u32 owner);
extern void  binfile_chunk_response(type_u32 seq, type_s32 error, type_u32 offset, type_s8 owner);
extern void  binfile_upgrade_response(type_s32 seq, type_s32 error);
extern void  appdown_start_response(type_s32 seq, type_s32 error);
extern void cloud_rst_state_set(RST_STATE_t rst_state, type_u32 rst_time, type_u32 rst_timeout);
extern RST_STATE_t cloud_rst_state_get(void);
extern type_u32 cloud_rst_time_get(void);
extern int qlcloud_is_version_valid(const type_u8  mcu_version[5]);
extern int ql_local_ota_ver_init( void );
extern int ql_local_config_erase( void );
extern type_u8 get_ota_cur_owner( void );
extern int ql_local_data_write( void* data, type_u32 data_len );
extern int ql_local_data_read( void* data, type_u32 data_len );
extern void cloud_unbound_state_set(UNBOUND_STATE_t unb_state, type_u32 unb_time, type_u32 unb_timeout);
extern UNBOUND_STATE_t cloud_unbound_state_get(void);
extern void cloud_bind_state_set(BIND_STATE_t bind_state, type_u32 bind_time, type_u32 bind_timeout);
extern BIND_STATE_t cloud_bind_state_get(void);
extern void udp_control(type_u8 control_type);
extern int qlcloud_is_sn_valid(char* sn);
extern int qlcloud_is_addr_valid(char* addr);
extern type_u32 cloud_bind_timeout_get(void);

extern int ql_set_wifi_info(unsigned char mode,wifi_info_t* info);
extern int32_t set_wifi_config(uint8_t mode,char * ap_ssid, char * ap_pwd);

extern void set_download_status(DEV_DOWNLOAD_STATUS_T statu);
extern int get_download_status();

#ifdef __cplusplus
}
#endif

#endif /* QL_LOGIC_CONTEXT_H_ */
