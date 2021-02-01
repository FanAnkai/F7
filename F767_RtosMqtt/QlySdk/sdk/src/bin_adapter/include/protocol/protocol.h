#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "iot_interface.h"
#include "module_config.h"

#define BIN_SDK_VERSION     "03.13"

#define FRAME_HEADER_LEN    sizeof(DEV_FRAME_HEAD_S)
#define FRAME_DATA_LMT      __SDK_BIN_MODULE_DATA_MAX_LEN__ + 32
#define FRAME_CHECKSUM_LEN  1

#define FRAME_MAGIC     0xA5
#define FRAME_VER_1     0x01
#define FRAME_VER_2     0x02
#define FRAME_VER_3     0x03

#define FRAME_VER   FRAME_VER_3

typedef enum
{
    PROTOCOL_RET_ERR    = -1,
    PROTOCOL_RET_OK     = 0,
}serial_protocol_ret_e;

typedef enum
{
    FRAME_DIRECT_RECV,
    FRAME_DIRECT_SEND,
}frame_direct_e;

typedef enum
{
    ST_WAIT_MAGIC,
    ST_WAIT_VER,
    ST_WAIT_TYPE,
    ST_WAIT_LENGTH_H,
    ST_WAIT_LENGTH_L,
    ST_WAIT_ERRCODE,
    ST_WAIT_DATA,
    ST_WAIT_CHECKSUM,
}FRAME_PARSE_STATUS_E;

typedef struct
{
    iot_u8 magic;
    iot_u8 version;
    iot_u8 type;
    iot_u8 errcode;
    iot_u8 length[2];
}DEV_FRAME_HEAD_S;

typedef enum
{
    SERIAL_OPCODE_MODULE_STATUS,
    SERIAL_OPCODE_SET_MODE,
    SERIAL_OPCODE_MODULE_INFO_CB,
    SERIAL_OPCODE_INIT,
    SERIAL_OPCODE_SDK_STATUS,
    SERIAL_OPCODE_GET_ONLINE_TIME,
    SERIAL_OPCODE_DATA_CB,              //5
    SERIAL_OPCODE_UPLOADE_DPS,
    SERIAL_OPCODE_DOWNLOAD_DPS,
    SERIAL_OPCODE_TX_DATA,
    SERIAL_OPCODE_RX_DATA,
    SERIAL_OPCODE_OTA_SET,              //10
    SERIAL_OPCODE_OTA_INFO,
    SERIAL_OPCODE_OTA_UPGRADE,
    SERIAL_OPCODE_PATCH_LIST,
    SERIAL_OPCODE_PATCH_REQ,
    SERIAL_OPCODE_PATCH_END,            //15
    SERIAL_OPCODE_DATA_CHUNK,
    SERIAL_OPCODE_GET_INFO,
    SERIAL_OPCODE_INFO_CB,
    SERIAL_OPCODE_SET_CONFIG,
    SERIAL_OPCODE_SUB_DEV_ACT,          //20
    SERIAL_OPCODE_SUB_DEV_INACT,
    SERIAL_OPCODE_SUB_STATUS_CB,
    SERIAL_OPCODE_COUNT,
}serial_opcode_e;

typedef enum
{
    FRAME_LEN_FIX,          //len is fixed, must equal to len_val
    FRAME_LEN_FLEXIBLE,     //len is flexible, between len_min and len_max
}frame_len_type_e;

typedef struct
{
    serial_opcode_e     opcode_id;
    iot_u32             msg_len;
    void *              arg;
}serial_msg_s;

typedef struct
{
    frame_len_type_e    len_type;
    iot_u16            len_val;
    iot_u16            len_min;
    iot_u16            len_max;
    iot_s32 (* function)(serial_msg_s *msg);
}opcode_define_s;

typedef struct
{
    serial_opcode_e     opcode_id;
    iot_u8             opcode_val;
    opcode_define_s *   recv;
    opcode_define_s *   send;
}serial_protocol_s;

//opcode detail
typedef struct
{
    iot_u8      result;
}module_send_call_result_s;

//basic function
//opcode = 0x01, init
#define PRODUCT_KEY_LEN 16
#define MCU_VER_LEN 5
#define SERVER_ADDR_LEN_MAX 32

#define IS_DIGITAL(c)   (c >= '0' && c <= '9')
#define IS_DOT(c)   (c == '.')
#define IS_VALID_VER(v) (IS_DIGITAL(v[0]) && IS_DIGITAL(v[1]) && IS_DOT(v[2]) && IS_DIGITAL(v[3]) && IS_DIGITAL(v[4]) )
typedef struct
{
    iot_u8      product_id[4];
    iot_u8      product_key[PRODUCT_KEY_LEN];
    iot_u8      mcu_ver[MCU_VER_LEN];
    iot_u8      recvbuf_size[4];
    iot_u8      sendbuf_size[4];
    iot_u8      server_port[2];
    iot_u8      addr_len;
    iot_u8      server_addr[];
}mcu_send_init_s;

typedef struct
{
#if (__SDK_BIN_MODULE_TYPE__ == MODULE_TYPE_WIFI)
    iot_u8      mac[6];
#elif (__SDK_BIN_MODULE_TYPE__== MODULE_TYPE_2G || __SDK_BIN_MODULE_TYPE__== MODULE_TYPE_4G)
    iot_u8      imei[16];
#endif
    iot_u8      ver[5];
}module_send_init_s;

//opcode = 0x02, sdk status
typedef struct
{
    iot_u8      dev_status;
    iot_u8      timeout[4];
}opcode_sdk_status_s;

//opcode = 0x03, get online time
typedef struct
{
    iot_u8      TimeStamp[4];
    iot_u8      year;
    iot_u8      month;
    iot_u8      day;
    iot_u8      hour;
    iot_u8      minute;
    iot_u8      second;
    iot_u8      week;
}module_send_get_online_time_s;

//opcode = 0x04, date cb
typedef struct
{
    iot_u8      dev_id[32];
    iot_u8      info_type;
}module_send_data_list_s;

typedef struct
{
    iot_u8      seq[4];
    iot_u8      count[2];
    module_send_data_list_s data_list[];
}module_send_data_cb_s;

//data transmission
#define SUB_ID_LEN_MAX 32
//opcode = 0x11, upload dps
typedef struct
{
    iot_u8      seq[4];
    iot_u8      act;
    iot_u8      sub_id[SUB_ID_LEN_MAX];
}mcu_send_upload_dps_s;

//opcode = 0x12, download dps
typedef struct
{
    iot_u8      sub_id[SUB_ID_LEN_MAX];
}module_send_download_dps_s;

//opcode = 0x13, tx data
typedef struct
{
    iot_u8      seq[4];
    iot_u8      act;
    iot_u8      sub_id[SUB_ID_LEN_MAX];
}mcu_send_tx_data_s;

//opcode = 0x14, rx data
typedef struct
{
    iot_u8      TimeStamp[8];
    iot_u8      sub_id[SUB_ID_LEN_MAX];
}module_send_rx_data_s;

//about OTA
//opcode = 0x21, ota set
typedef struct
{
    iot_u8      expect_time[4];
    iot_u8      chunk_size[4];
}mcu_send_ota_set_s;

//opcode = 0x22, ota info
#define OTA_INFO_NAME_LEN 16
#define OTA_INFO_VER_LEN 6
typedef struct
{
    iot_u8      name[OTA_INFO_NAME_LEN];
    iot_u8      version[OTA_INFO_VER_LEN];
    iot_u8      total_len[4];
    iot_u8      file_crc[2];
}module_send_ota_info_s;

//opcode = 0x24, patch list
#define PATCH_NAME_LEN_MAX  16
#define PATCH_VER_LEN_MAX   16
#define PATCH_NUMBER_LEN    (sizeof(patch_list_number))
typedef struct
{
    iot_u8      name[PATCH_NAME_LEN_MAX];
    iot_u8      ver[PATCH_VER_LEN_MAX];
    iot_u8      total_len[4];
}patch_list_number;

typedef struct
{
    iot_u8      count[4];
    patch_list_number patch_number[];
}module_send_patch_list_s;

//opcode = 0x25, patch req
typedef struct
{
    iot_u8      name[16];
    iot_u8      ver[16];
}mcu_send_patch_req_s;

//opcode = 0x26, patch end
typedef struct
{
    iot_u8      name[16];
    iot_u8      ver[16];
    iot_u8      patch_state;
}mcu_send_patch_end_s;

//opcode = 0x27, data chunk
typedef struct
{
    iot_u8      chunk_stat;
    iot_u8      result;
    iot_u8      offset[4];
}mcu_send_data_chunk_s;

typedef struct
{
    iot_u8      chunk_stat;
    iot_u8      chunk_offset[4];
    iot_u8      chunk_size[4];
}module_send_data_chunk_s;

//about device infomation
//opcode = 0x31, get info
typedef struct
{
    iot_u8      info_type;
}mcu_send_get_info_s;

//opcode = 0x32, info cb
typedef struct
{
    iot_u8      info_type;
    iot_u8      data_len[2];
}module_send_info_cb_s;

//opcode = 0x33, set config
#define OPCODE_SET_CONFIG_LEN_MAX 16
typedef struct
{
    iot_u8      info_type;
    iot_u8      info_len;
    iot_u8      info[];
}mcu_send_set_config_s;

//opcode = 0x41, sub dev active
typedef struct
{
    iot_u8      sub_id[SUB_ID_LEN_MAX];
    iot_u8      sub_name[SUB_ID_LEN_MAX];
    iot_u8      sub_version[MCU_VER_LEN];
    iot_u8      sub_type[2];
}mcu_send_sub_dev_add;
typedef struct
{
    iot_u8      opt_type;
    iot_u8      data_seq[4];
    iot_u8      sub_count[2];
    mcu_send_sub_dev_add sub_info[];
}mcu_send_sub_dev_act_s;

//opcode = 0x42, sub dev inactive
typedef struct
{
    iot_u8      sub_id[SUB_ID_LEN_MAX];
    iot_u8      opt_type;
    iot_u8      data_seq[4];
}mcu_send_sub_dev_inact_s;

//opcode = 0x43, sub status cb
typedef struct
{
    iot_u8      sub_id[SUB_ID_LEN_MAX];
    iot_u8      info_type;
}module_send_sub_info;
typedef struct
{
    iot_u8      data_seq[4];
    iot_u8      sub_status;
    iot_u8      timestamp[4];
    iot_u8      sub_count[2];
    module_send_sub_info sub_info[];
}module_send_sub_status_cb_s;

void protocol_init(void);
void serial_rx_handle(void);
void protocol_define_set(serial_protocol_s *protocol_define);   //set the protocol
iot_s32 recv_opcode_fun_call(serial_opcode_e opcode_id, serial_msg_s *msg);
iot_s32 send_opcode_fun_call(serial_opcode_e opcode_id, serial_msg_s *msg);
iot_u8 get_val_with_opcodeID(serial_opcode_e opcode_id);

#endif
