#ifndef __CLOUD_ASSEMBLE_FRAME_H__
#define __CLOUD_ASSEMBLE_FRAME_H__
#include "ql_include.h"

enum {
	//请求方向
    DIR_DEVICE_REQUEST              = 0,
    DIR_DEVICE_RESPOND              = 1,

	//数据格式
    DATATYPE_JSON                   = 1,
    DATATYPE_KLV                    = 2,//not used.
    DATATYPE_BINARY                 = 3,
    DATATYPE_QUERY                  = 4,

    //ota owner
    OTA_OWNER_WIFI                  = 0,
    OTA_OWNER_MCU                   = 1,
};

#define QL_ENCRYPT_TYPE_AES        0
#define QL_ENCRYPT_TYPE_SSL        1
#define QL_ENCRYPT_TYPE_SM4        2

typedef struct cloud_packet{
    type_u32 pkt_len;
    type_u16 pkt_hlen;
    type_u16 pkt_version;
    type_u16 pkt_flag;
    type_u16 pkt_opcode;
    type_u32 pkt_seq;
    char     pkt_data[];
} cloud_packet_t;


typedef enum PKT_ID
{
    PKT_ID_REQ_UPLOAD_DPS,          //0
    PKT_ID_RCV_DOWNLOAD_DPS,

    PKT_ID_REQ_TX_DATA,             // 2
    PKT_ID_RCV_RX_DATA,

    PKT_ID_REQ_HEART,               // 4
    PKT_ID_REQ_REG,
    PKT_ID_REQ_AUTH,

    PKT_ID_RES_OTAPASSIVE,          // 7
    PKT_ID_REQ_OTATRIGGER,
    PKT_ID_RES_OTAFILE_INFO,        // 9
    PKT_ID_RES_OTAFILE_CHUNK,
    PKT_ID_RES_OTAUPGRADE_CMD,      //11
    PKT_ID_RES_OTA_APPDOWN,
    PKT_ID_RES_OTA_COMPELS,
    #if  (__SDK_TYPE__== 1)         //QLY_SDK_BIN not include the ql_logic_contex.h
    PKT_ID_REQ_DP_GET,              //14
    #endif
    PKT_ID_REQ_RST,                 //15/16
    PKT_ID_REQ_SUB_ACT,
    PKT_ID_REQ_SUB_INACT,
    PKT_ID_GET_INFO,
    PKT_ID_REQ_TX_DATA_V2,
    PKT_ID_RCV_RX_DATA_V2,          //20/21
    PKT_ID_RCV_BIND_INFO,
    PKT_ID_REQ_UNB,
    PKT_ID_REQ_SHAREINFO_ADD,
    PKT_ID_REQ_SHAREINFO_DELETE,
    PKT_ID_REQ_LOG,
    PKT_ID_RES_PATCH_PASSIVE,       //26/27
    PKT_ID_REQ_PATCHS_CHECK_LIST,
    PKT_ID_REQ_PATCH_REQUEST,
    PKT_ID_RES_PATCH_CHUNK,
    PKT_ID_REQ_PATCH_END,
    PKT_ID_RES_SUB_FORCED_UNB,      //31
}PKT_ID_t;

typedef struct cloud_packet_map{
    type_u16     op_code;
    char*        op_str;
    const char* json_ctx;
} cloud_packet_map_t;

extern cloud_packet_map_t g_ql_packet_map[];

#define PKT_CNT             (sizeof(g_ql_packet_map)/sizeof(cloud_packet_map_t))
#define PKT_OP_CODE(pkt_id)  g_ql_packet_map[pkt_id].op_code
#define PKT_OP_STR(pkt_id)   g_ql_packet_map[pkt_id].op_str
#define PKT_JSON_CTX(pkt_id) g_ql_packet_map[pkt_id].json_ctx

typedef struct _binfile_chunk_data {
    type_s8     owner;
    type_s8     isend;
    type_u32    offset;
    type_u32    datalen;
    type_u8*    data;
} binfile_chunk_data;

typedef struct _ota_res_arg {
    type_s8     owner;
    type_u32    offset;
    type_s32    error;
}ota_res_arg;

typedef struct push_tx_pkt{
    type_u32        pkt_len;
    type_u8         token_len;
    type_u8*        token;
    type_u8         sub_id_len;
    type_u8*        sub_id;
    type_u32        data_len;
    const type_u8*  data;
} push_tx_pkt_t;

typedef struct push_rx_pkt{
    type_u8      sub_id_len;
    type_u8*     sub_id;
    type_u64     ts;
    type_u32     data_len;
    type_u8*     data;
} push_rx_pkt_t;

#define QL_PRODUCT_SIGN_SIZE        128
#define QL_DEVICE_SIGN_SIZE         128
#define QL_DEVICE_ID_LEN            16

#define QL_CRYPT_SERVICE_SALT       "iotqly"

const type_u8 * packet_opcode_message(int opcode);
type_u32 get_ota_compels_id(void);
type_s8 * get_ota_compels_version(void);

void cloud_reginfo_store(const char *data, type_size_t len);
void cloud_auth_store(const char *data, type_size_t len);
void qlcloud_process_heart(const char* data, type_size_t len, int seq);
void qlcloud_process_rx_data(const char* data, type_size_t len, type_u32 seq);
void qlcloud_process_rx_data_v2(const char* data, type_size_t len, type_u32 seq);
void qlcloud_process_downdps(const char* data, type_size_t len, int seq);
void qlcloud_process_otapassive(const char *in, int inlen, type_u32 pkt_seq);
void qlcloud_process_ota_compels( const char *in, int inlen, type_u32 pkt_seq);
void qlcloud_process_otafileinfo(const char *in, int inlen, type_u32 pkt_seq);
void qlcloud_process_otafilechunk(const char *in, int inlen, type_u32 pkt_seq);
void qlcloud_process_otacmd(const char *in, int inlen, type_u32 pkt_seq);
void qlcloud_process_ota_appdown(const char *in, int inlen, type_u32 pkt_seq);
void qlcloud_process_info(const char* data, type_size_t len, int seq);
void qlcloud_process_bindinfo(type_u16 optype, const char* data);
void qlcloud_parse_data(type_u16 pkt_opcode, int seq, const char* data, type_size_t data_len);
void qlcloud_sub_forced_unb_data( int seq, const char* data, type_size_t data_len);

cloud_packet_t* create_lib_request(int pkt_id, type_u32 seq, void* arg);
type_u32 get_cur_seq_num(void);
int is_packet_opcode_valid(int opcode, char** str);

#define MEM_ALIGNMENT                   4
#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT-1))

extern struct client_handle g_client_handle;


#endif /* __FRAME_H__ */
