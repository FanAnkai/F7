#ifndef __SPEC_OPCODE_H__
#define __SPEC_OPCODE_H__

#include "iot_interface.h"
#include "protocol.h"

#define ROUTER_SSID_MAX     32
#define ROUTER_PASSWD_MAX   32

#define AP_CONFIG_MAX 32
#define MCU_SEND_SET_MODULE_EXTRA_LEN_MAX ((AP_CONFIG_MAX) + 1 ) * 2
#define MODULE_SEND_SET_MODULE_EXTRA_LEN_MAX sizeof(module_send_factory_s)

#define IOT_AP_SSID            "IOT_AP_%02X%02X"
#define IOT_AP_PASSWORD        "12345678"

typedef enum
{      
    NETCONF_SMART,
    NETCONF_AP,
    NETCONN_NOK,
    NETCONN_OK,
    CLOUD_CONN_OK,
    FACTORYMODE,
    NETCONN_AP_OK,//AP模式下有设备接入
    STATUS_INVALID
} MODULE_STATUS;

typedef enum {
    RESET_TYPE_MODULE_RESET,
    RESET_TYPE_SMARTCONFIG,
    RESET_TYPE_AP_SMART,
    RESET_TYPE_STA_FACTORYMODE,
} RESET_TYPE_T;

//opcode detail

//opcode = 0xF1, module status
typedef struct
{
    iot_u8      module_type;
    iot_u8      module_status;
    iot_u8      TimeStamp[4];
}module_send_module_status_s;

// opcode = 0xF2, set module mode
typedef struct
{
    iot_u8      module_type;
    iot_u8      mode;
    iot_u8      timeout;
    iot_u8      extra_len[4];
    iot_u8      extra[];
}mcu_send_set_mode_s;

typedef struct module_send_factory
{
    iot_u8 ret;
    iot_u8 rssi;
}module_send_factory_s;

typedef struct
{
    iot_u8      module_type;
    iot_u8      mode;
    iot_u8      timeout;
    iot_u8      result;
    iot_u8      extra_len[4];
    iot_u8      extra[];
}module_send_set_mode_s;

// opcode = 0xF3, module infomation callback
#define MODULE_S_MODULE_INFO_CB_MIN (sizeof(module_send_module_info_cb_s) + sizeof(mdoule_info_get_ip))
#define MODULE_S_MODULE_INFO_CB_MAX (sizeof(module_send_module_info_cb_s) + sizeof(mdoule_info_router))

typedef enum {
    MODULE_INFO_TYPE_ROUTER,
    MODULE_INFO_TYPE_GETIP,
} MODULE_INFO_TYPE;

typedef struct
{
    iot_u8 ssid[ROUTER_SSID_MAX];
}mdoule_info_router;

typedef struct
{
    iot_u8 ip_addr[4];
    iot_u8 ip_mask[4];
    iot_u8 gateway[4];
}mdoule_info_get_ip;

typedef struct
{
    iot_u8      module_type;
    iot_u8      info_type;
    iot_u8      info_len[4];
    iot_u8      info[];
}module_send_module_info_cb_s;

typedef struct
{
    iot_u8      module_type;
    iot_u8      info_type;
}mcu_send_module_info_cb_s;

void module_config_recv_init_cmd(void * arg);
void module_status_set(MODULE_STATUS Status);
MODULE_STATUS module_status_get();

//module spec
iot_s32 module_s_module_stauts(void);
iot_s32 module_d_set_mode(serial_msg_s *msg);
void    module_s_set_mode(iot_u8 mode, iot_u8 timeout, iot_u8 result, iot_u32 extra_len, void * extra);
iot_s32 module_d_module_info_cb(serial_msg_s *msg);
void    module_s_module_info_cb(iot_u8 info_type, iot_u32 info_len, iot_u8 * info);

//mcu spec
iot_s32 mcu_d_module_status(serial_msg_s *msg);
void    mcu_s_set_mode(iot_u8 mode, iot_u8 timeout, iot_u32 extra_len, void * extra);
iot_s32 mcu_d_set_mode(serial_msg_s *msg);
void    mcu_s_module_info_cb(iot_u8 info_type);
iot_s32 mcu_d_module_info_cb(serial_msg_s *msg);

//interface
void module_send_info_router(iot_u8 * ssid);
void module_send_info_ip(iot_u32 ip_addr, iot_u32 ip_mask, iot_u32 gateway);

void iot_module_status_cb(iot_u8 module_type, MODULE_STATUS status, iot_u32 timestamp);
void iot_module_set_reset(iot_u8 timeout);
void iot_module_set_smartconfig(iot_u8 timeout);
void iot_module_set_ap(iot_u8 timeout, char * ssid, char * passwd);
void iot_module_set_factroy(iot_u8 timeout);
void iot_module_set_mode_cb(iot_u8 mode, iot_u8 timeout, iot_u8 result);
void iot_module_set_factory_cb(iot_u8 ret, iot_u8 rssi);
void iot_module_info_get(iot_u8 info_type);
void iot_module_info_cb(iot_u8 info_type, iot_u8 info_len, iot_u8 * info);

#endif