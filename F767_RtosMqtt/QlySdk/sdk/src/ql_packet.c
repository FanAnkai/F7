#include "ql_include.h"
#include "ql_sm4.h"
#include "ql_patch.h"

ota_upgrade_ctx g_ota;
#ifdef LOCAL_SAVE
extern local_config_t g_local_config;
#endif
static char ota_compels_version[6] = {0};
static unsigned int ota_compels_id = 0;
extern type_u8  g_ota_chunk_retran_time;
extern type_u32 g_ota_chunk_retran_seq;
extern type_u32 g_ota_chunk_retran_offset;

static const char*const_str_data        = "data";
static const char*const_str_value       = "value";
static const char*const_str_res         = "res";
static const char*const_str_errcode     = "errcode";
static const char*const_str_ts          = "ts";
static const char*const_str_sbid        = "subid";
static const char*const_str_ota_owner[2]= {"module", "mcu"};

#define PKT_REQ_REG         "{\"req\":{\"product_id\":%d},\"data\":{\"product_sign\":\"%s\",\"rand\":%d,\"mac\":\"%s\",\"pf\":\"%02x\",\"crypt_type\":%d,\"sn\":\"%s\"}}"
#define PKT_REQ_AUTH        "{\"req\":{\"product_id\":%d},\"data\":{\"product_sign\":\"%s\",\"dev_sign\":\"%s\",\"rand\":%d,\"dev_id\":\"%s\",\"mcu_version\":\"%s\",\"wifi_version\":\"%s\",\"rst\":%d,\"unb\":%d,\"mcu\":\"%s\",\"wifi\":\"%s\"}}"
#define PKT_REQ_HRT         "{\"req\":{\"token\":\"%s\"}}"

#define PKT_REQ_UP_DPS      "{\"req\":{\"token\":\"%s\"},\"data\":%s,\"act\":%d}"
#define PKT_REQ_OTATRIGGER  "{\"req\":{\"token\":\"%s\"},\"data\":{\"upgradetype\":%d,\"product_id\":%d,\"version\":\"%s\",\"bin\":%d,\"owner\":%d,\"chunksize\":%d,\"brk_v\":\"%s\",\"brk_c\":%d,\"brk_s\":%d}}"
#define PKT_REQ_TX          "{\"req\":{\"token\":\"%s\"},\"data\":\"%s\",\"subid\":\"%s\"}"
#define PKT_REQ_DP_GET      PKT_REQ_HRT

#define PKT_RES_OTAPASSIVE  "{\"res\":{\"errcode\":%d,\"token\":\"%s\"}}"
#define PKT_RES_OTA_INFO    "{\"res\":{\"errcode\":%d,\"token\":\"%s\"},\"data\":{\"owner\":%d}}"
#define PKT_RES_OTA_CHUNK   "{\"res\":{\"errcode\":%d,\"token\":\"%s\"},\"data\":{\"owner\":%d,\"offset\":%d}}"
#define PKT_RES_OTA_CMD     "{\"res\":{\"errcode\":%d,\"token\":\"%s\"},\"data\":{\"owner\":%d,\"version\":\"%s\"}}"
#define PKT_RES_OTA_APPDOWN PKT_RES_OTAPASSIVE
#define PKT_RES_OTA_COMPELS PKT_RES_OTAPASSIVE

#define PKT_REQ_RST         PKT_REQ_HRT

#define PKT_REQ_SUB_ACT     "{\"req\":{\"token\":\"%s\"},\"data\":%s,\"act_t\":%d}"

#define PKT_REQ_SUB_INACT   PKT_REQ_SUB_ACT
#define PKT_GET_INFO        "{\"req\":{\"token\":\"%s\"},\"data\":{\"type\":%d,\"size\":%d}}"

#define PKT_REQ_UNBIND		        PKT_REQ_HRT

#define PKT_REQ_LOG_ONLINE          "{\"req\":{\"token\":\"%s\"},\"data\":%s}"

#define PKT_RES_PATCH_PASSIVE       PKT_RES_OTAPASSIVE
#define PKT_REQ_PATCH_LIST          PKT_REQ_LOG_ONLINE
#define PKT_REQ_PATCH_REQ           PKT_REQ_LOG_ONLINE
#define PKT_RES_PATCH_CHUNK         "{\"res\":{\"errcode\":%d,\"token\":\"%s\"},\"data\":{\"offset\":%d}}"
#define PKT_REQ_PATCH_FIN           PKT_REQ_LOG_ONLINE

cloud_packet_map_t g_ql_packet_map[] =
{
    { 0x0503, "updps",  PKT_REQ_UP_DPS },             // 0
    { 0x0504, "downdps",NULL},

    { 0x0401, "tx",     PKT_REQ_TX},                  // 2
    { 0x0402, "rx",     NULL},

    { 0x0301, "hrt",    PKT_REQ_HRT},                 // 4
    { 0x0101, "reg",    PKT_REQ_REG },
    { 0x0201, "auth",   PKT_REQ_AUTH },

    { 0x0a00, "0a00",   PKT_RES_OTAPASSIVE },         // 7
    { 0x0a01, "0a01",   PKT_REQ_OTATRIGGER },
    { 0x0a02, "0a02",   PKT_RES_OTA_INFO},
    { 0x0a03, "0a03",   PKT_RES_OTA_CHUNK },          //10
    { 0x0a04, "0a04",   PKT_RES_OTA_CMD },
    { 0x0a05, "0a05",   PKT_RES_OTA_APPDOWN },
    { 0x0a09, "0a09",   PKT_RES_OTA_COMPELS},
    #if  (__SDK_TYPE__== QLY_SDK_BIN)
    { 0x0302, "0302",   PKT_REQ_DP_GET},              //14
    #endif
    { 0x0b01, "0b01",   PKT_REQ_RST},                 //14/15
    { 0x0102, "0102",   PKT_REQ_SUB_ACT},
    { 0x0103, "0103",   PKT_REQ_SUB_INACT},
    { 0x0303, "0303",   PKT_GET_INFO},
    { 0x0403, "0403",   NULL},
    { 0x0404, "0404",   NULL},
    { 0x0b02, "0b02",   NULL},                        //20/21
    { 0x0b03, "0b03",   PKT_REQ_UNBIND},              //21
    { 0x0b04, "0b04",   NULL},                        //22
    { 0x0b05, "0b05",   NULL},                        //23
    { 0x0b06, "0b06",   PKT_REQ_LOG_ONLINE},           //24/25
   
    { 0x0a10, "0a10",   PKT_RES_PATCH_PASSIVE},       //26
    { 0x0a11, "0a11",   PKT_REQ_PATCH_LIST},          //27
    { 0x0a12, "0a12",   PKT_REQ_PATCH_REQ},           //28
    { 0x0a13, "0a13",   PKT_RES_PATCH_CHUNK},         //29
    { 0x0a14, "0a14",   PKT_REQ_PATCH_FIN},           //30/31

    { 0x0104, "0104",   NULL},                        //31
};
static type_u32 g_seq = 0;
type_u32 get_cur_seq_num(void)
{
	return g_seq++;
}

int is_packet_opcode_valid(int opcode, char** str)
{
    int i = 0;

    for(i = 0; i < PKT_CNT; i++)
    {
        if(PKT_OP_CODE(i) == opcode)
        {
            *str = PKT_OP_STR(i);
            break;
        }
    }

    return (i == PKT_CNT) ? 0 : 1;
}

int ql_crypt_gen_product_sign(type_u32 product_id, type_u8* product_key, type_u32 rand, type_u8* product_sign)
{
    #define       CIPHER_LEN 64
	unsigned char szinput[8];
   	unsigned char encry_data[CIPHER_LEN];
	type_size_t buff_size = CIPHER_LEN;
    int ret = 0;
    type_u32 pid = product_id;

    if( NULL == product_key || NULL == product_sign )
    {
        return -1;
    }

    rand = ql_htonl(rand);
	pid = ql_htonl(product_id);

	ql_memcpy(szinput, &pid, sizeof(pid));
	ql_memcpy(&szinput[sizeof(pid)], &rand, sizeof(rand));

	ql_memset(encry_data, 0, buff_size);
	ql_memset(product_sign, 0x00, QL_PRODUCT_SIGN_SIZE);

    ret = iot_aes_cbc128_encrypt(szinput, 8, encry_data, &buff_size, product_key);

	if (ret == 0) {
		if (iot_encode_base64((char*)product_sign, QL_PRODUCT_SIGN_SIZE, (const char *)encry_data, buff_size) > 0) {
            return 0;
		}
	}
	return -1;
}
int ql_crypt_gen_device_sign(const char* device_id, char* device_key, type_u32 rand, char* device_sign)
{
    #define       CIPHER_LEN 64
    char buf[128] = {0};
    unsigned char encry_data[CIPHER_LEN] = {0};
    int len = 0, ret = 0, rand_num = 0;
    type_size_t buff_size = CIPHER_LEN;

    ql_memcpy(buf, (char*)device_id, QL_DEVICE_ID_LEN);
    rand_num = ql_htonl(rand);

    ql_memcpy(buf + QL_DEVICE_ID_LEN, &rand_num, 4);
    len = QL_DEVICE_ID_LEN + 4;

    iot_aes_cbc128_encrypt((unsigned char*) buf, len, encry_data, &buff_size, (unsigned char*)device_key);
    if (ret == 0) {
        if (iot_encode_base64(device_sign, QL_DEVICE_SIGN_SIZE, (const char *)encry_data, buff_size) > 0) {
            return 0;
        }
    }
    return -1;
}
type_s32 creat_push_tx_pkt_v2(type_u8 * data, push_tx_pkt_t * push_tx_pkt_data)
{
	type_u8 * p = data;
	
	*(p++) = push_tx_pkt_data->token_len;
	ql_memcpy(p, push_tx_pkt_data->token, push_tx_pkt_data->token_len);
	p+=push_tx_pkt_data->token_len;
	*(p++) = push_tx_pkt_data->sub_id_len;
    ql_memcpy(p, push_tx_pkt_data->sub_id, push_tx_pkt_data->sub_id_len);
	p+=push_tx_pkt_data->sub_id_len;
	*(p++) = push_tx_pkt_data->data_len>>24 & 0xff;
	*(p++) = push_tx_pkt_data->data_len>>16 & 0xff;
	*(p++) = push_tx_pkt_data->data_len>>8 & 0xff;
	*(p++) = push_tx_pkt_data->data_len & 0xff;
	ql_memcpy(p, push_tx_pkt_data->data, push_tx_pkt_data->data_len);
	p+=push_tx_pkt_data->data_len;

	return p-data;
}
//为了避免每次都开辟一个128字节的局部数组
type_s32 create_reg_pkt(char* data, type_u32 data_size)
{
    int data_len = 0, rand_num = 0;
    char product_sign[QL_PRODUCT_SIGN_SIZE] = {0};
    rand_num = (int)(ql_random() % 65535);

    if (-1 == ql_crypt_gen_product_sign(g_client_handle.product_id, g_client_handle.product_aes_key, rand_num, (type_u8*)product_sign)) {
        return -1;
    }
    data_len = ql_snprintf(data, data_size, PKT_JSON_CTX(PKT_ID_REQ_REG), \
               g_client_handle.product_id, product_sign, rand_num, g_client_handle.mac_str, __SDK_PLATFORM__,g_client_handle.encrypt_type,g_client_handle.sn);
    return data_len;
}
//为了避免每次都开辟两个128字节的局部数组
type_s32 create_auth_pkt(char* data, type_u32 data_size)
{
    int data_len = 0, rand_num = 0;
    char product_sign[QL_PRODUCT_SIGN_SIZE] = {0};
    char device_sign[QL_DEVICE_SIGN_SIZE] = {0};
    char mcu_ota_ver[6] = {0};
    char wifi_ota_ver[6] = {0};

    rand_num = (int)(ql_random());

    if (-1 == ql_crypt_gen_product_sign(g_client_handle.product_id, g_client_handle.product_aes_key, rand_num, (type_u8*)product_sign))
    {
        return -1;
    }

    if (-1 == ql_crypt_gen_device_sign((const char*)(g_client_handle.deviceid), (char *)(g_client_handle.device_aes_key), rand_num, device_sign))
    {
        return -1;
    }
#ifdef LOCAL_SAVE  
    ql_memcpy(mcu_ota_ver, g_local_config.cfg_mcu_ota_ver, 5);
    ql_memcpy(wifi_ota_ver, g_local_config.cfg_wifi_ota_ver, 5);
    

    data_len = ql_snprintf(data, data_size, PKT_JSON_CTX(PKT_ID_REQ_AUTH),\
                            g_client_handle.product_id, product_sign, device_sign, rand_num, g_client_handle.deviceid,\
                            g_client_handle.mcu_version, DEVICE_VERSION, ql_local_dev_state_get(DEV_STATE_RST), ql_local_dev_state_get(DEV_STATE_UNB), mcu_ota_ver, wifi_ota_ver);
#else
    data_len = ql_snprintf(data, data_size, PKT_JSON_CTX(PKT_ID_REQ_AUTH),\
                            g_client_handle.product_id, product_sign, device_sign, rand_num, g_client_handle.deviceid,\
                            g_client_handle.mcu_version, DEVICE_VERSION, 0,0, mcu_ota_ver, wifi_ota_ver);
#endif
    return data_len;
}

type_s32 create_ota_check_pkt(char* data, type_u32 data_size, type_s32 ota_owner)
{
    int want_bin_location = 0, data_len = 0;
    want_bin_location = 1;

    /*1.get local ota info*/
    char brk_v[6] = {0};
    type_u16 brk_c = 0;
    type_u32 brk_s = 0;

#ifdef LOCAL_SAVE
    if(g_client_handle.ota_type == OTA_TYPE_BREAKPOINT)
    {
        if(ql_local_break_ota_info_get(brk_v, &brk_c, &brk_s) == -1)
            return -1;
    }
#endif

#if (__SDK_TYPE__== QLY_SDK_LIB)
    if(g_client_handle.ota_mcu_type == OTA_CHECK_TYPE_COMPELS) {
        data_len = ql_snprintf(data, data_size, PKT_JSON_CTX(PKT_ID_REQ_OTATRIGGER), g_client_handle.iottoken, g_client_handle.ota_mcu_type,\
            get_ota_compels_id(), get_ota_compels_version(), want_bin_location, ota_owner, g_client_handle.ota_mcu_chunk_size, brk_v, brk_c, brk_s );
    } else {
        data_len = ql_snprintf(data, data_size, PKT_JSON_CTX(PKT_ID_REQ_OTATRIGGER), g_client_handle.iottoken, g_client_handle.ota_mcu_type, \
            g_client_handle.product_id, g_client_handle.mcu_version, want_bin_location, ota_owner, g_client_handle.ota_mcu_chunk_size, brk_v, brk_c, brk_s);
    }
#elif (__SDK_TYPE__== QLY_SDK_BIN)
    if(OTA_OWNER_MCU == ota_owner) {
        if(g_client_handle.ota_mcu_type == OTA_CHECK_TYPE_COMPELS) {
            data_len = ql_snprintf(data, data_size, PKT_JSON_CTX(PKT_ID_REQ_OTATRIGGER), g_client_handle.iottoken, g_client_handle.ota_mcu_type,\
                get_ota_compels_id(), get_ota_compels_version(), 1, ota_owner, g_client_handle.ota_mcu_chunk_size);
        } else {
            data_len = ql_snprintf(data, data_size, PKT_JSON_CTX(PKT_ID_REQ_OTATRIGGER), g_client_handle.iottoken, g_client_handle.ota_mcu_type, \
                g_client_handle.product_id, g_client_handle.mcu_version, 1, ota_owner, g_client_handle.ota_mcu_chunk_size, brk_v, brk_c, brk_s);
        }
    }
    else if(OTA_OWNER_WIFI == ota_owner) {
        data_len = ql_snprintf(data, data_size, PKT_JSON_CTX(PKT_ID_REQ_OTATRIGGER), g_client_handle.iottoken, g_client_handle.ota_wifi_type, \
            g_client_handle.product_id, DEVICE_VERSION, want_bin_location, ota_owner, g_client_handle.ota_wifi_chunk_size, brk_v, brk_c, brk_s);
    }
#endif

    return data_len;
}

cloud_packet_t* create_lib_request(int pkt_id, type_u32 seq, void* arg)
{
    cloud_packet_t* pk = NULL;
	int data_len = 0;

    type_u8* encrypt_data = NULL;
	int encrypt_len = 0;

    ota_res_arg* ota_arg = NULL;
    ota_res_arg* patch_arg = NULL;

    char ota_ver[6] = {0};

    push_tx_pkt_t * push_tx_pkt_data = NULL;

    if(pkt_id == PKT_ID_REQ_UPLOAD_DPS || pkt_id == PKT_ID_REQ_TX_DATA || pkt_id == PKT_ID_REQ_SUB_ACT || pkt_id == PKT_ID_REQ_SUB_INACT || pkt_id == PKT_ID_REQ_LOG)
    {
        data_len = ql_strlen((char*)arg);
        data_len = MEM_ALIGN_SIZE(data_len + 128);//16 frame head + 74 json = 90
    }
    else if(pkt_id == PKT_ID_REQ_TX_DATA_V2)
    {
        push_tx_pkt_data = (push_tx_pkt_t *)arg;
		data_len = push_tx_pkt_data->pkt_len;
        data_len = MEM_ALIGN_SIZE(data_len + PACKET_HEAD_LENGTH+16);//16 frame head
    }
    else
    {
        data_len = 320;//LONGEST DATA is AUTH = 16 frame head + 230 json = 246
    }
    pk = (cloud_packet_t *)ql_malloc(data_len);
	
    if(pk == NULL)
    {
        ql_log_err( ERR_EVENT_INTERFACE, "malc pk[%d] err", data_len);
        return NULL;
    }
    else
    {
        ql_memset(pk, 0, data_len);
    }

    pk->pkt_hlen    = ql_htons(sizeof(cloud_packet_t));
    pk->pkt_version = ql_htons(PROTOCOL_VERSION);
    pk->pkt_flag    = ql_htons((DATATYPE_JSON & 0x7f) << 8 | (DIR_DEVICE_REQUEST & 0x01) << 15);
    pk->pkt_opcode  = ql_htons(PKT_OP_CODE(pkt_id));
    pk->pkt_seq     = ql_htonl(seq);

    switch(pkt_id)
    {
    case PKT_ID_REQ_UPLOAD_DPS:
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken, (type_u8*)arg, g_client_handle.data_act);
        break;
    case PKT_ID_REQ_TX_DATA:
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken, (type_u8*)arg,g_client_handle.sub_dev_id);
        break;
    case PKT_ID_REQ_TX_DATA_V2:
		pk->pkt_flag = ql_htons((DATATYPE_BINARY & 0x7f) << 8 | (DIR_DEVICE_REQUEST& 0x01) << 15);
		data_len = creat_push_tx_pkt_v2(pk->pkt_data, push_tx_pkt_data);
		#if 0
         for(i = 0; i < data_len; i++)
		 {
		 printf("%x ", pk->pkt_data[i]);
		 }
		 printf("\r\n");
        #endif
		break;
    case PKT_ID_REQ_SUB_ACT:
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken, (type_u8*)arg,g_client_handle.sub_opt_type);
        break;
    case PKT_ID_REQ_PATCHS_CHECK_LIST:
    case PKT_ID_REQ_PATCH_REQUEST:        
    case PKT_ID_REQ_PATCH_END:    
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken, (type_u8*)arg);
        break;
    case PKT_ID_REQ_SUB_INACT:
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken, (type_u8*)arg,g_client_handle.sub_opt_type);
        break;
    case PKT_ID_REQ_REG:
        data_len = create_reg_pkt(pk->pkt_data, data_len - PACKET_HEAD_LENGTH);
        break;
    case PKT_ID_REQ_AUTH:
        data_len = create_auth_pkt(pk->pkt_data, data_len - PACKET_HEAD_LENGTH);
        break;    
    #if  (__SDK_TYPE__== QLY_SDK_BIN)
    case PKT_ID_REQ_DP_GET:
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken);
        break;
    #endif
    case PKT_ID_GET_INFO:
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken, *(int*)arg, g_client_handle.recvbuf_size-PADDING_BUFFER_SIZE);
        break;
    case PKT_ID_REQ_HEART:
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken);
        break;
    case PKT_ID_RES_OTAPASSIVE:
    case PKT_ID_RES_OTA_COMPELS:
    case PKT_ID_RES_OTA_APPDOWN:
    case PKT_ID_RES_PATCH_PASSIVE:
        pk->pkt_flag    = ql_htons((DATATYPE_JSON & 0x7f) << 8 | (DIR_DEVICE_RESPOND& 0x01) << 15);   
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), *(int*)arg, g_client_handle.iottoken);
        break;
    case PKT_ID_REQ_OTATRIGGER:
        data_len = create_ota_check_pkt(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, *(int*)arg);
        break;
    case PKT_ID_RES_OTAFILE_INFO:
        pk->pkt_flag    = ql_htons((DATATYPE_JSON & 0x7f) << 8 | (DIR_DEVICE_RESPOND& 0x01) << 15);
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), *(int*)arg, g_client_handle.iottoken, g_ota.otafileinfo.owner );
        break;
    case PKT_ID_RES_OTAFILE_CHUNK:
        pk->pkt_flag    = ql_htons((DATATYPE_JSON & 0x7f) << 8 | (DIR_DEVICE_RESPOND& 0x01) << 15);
        ota_arg = (ota_res_arg*)arg;
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), ota_arg->error, g_client_handle.iottoken, ota_arg->owner, ota_arg->offset);
        break;
    case PKT_ID_RES_OTAUPGRADE_CMD:
        pk->pkt_flag    = ql_htons((DATATYPE_JSON & 0x7f) << 8 | (DIR_DEVICE_RESPOND& 0x01) << 15);
        if(get_ota_cur_owner() == OTA_OWNER_MCU)
        {
            ql_memcpy(ota_ver, g_ota.otafileinfo.version[OTA_OWNER_MCU], 5);
        }
        else if(get_ota_cur_owner() == OTA_OWNER_WIFI)
        {
            ql_memcpy(ota_ver, g_ota.otafileinfo.version[OTA_OWNER_WIFI], 5);
        }
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), *(int*)arg, g_client_handle.iottoken, get_ota_cur_owner(), ota_ver);
        break;
    case PKT_ID_REQ_RST:
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken);
        break;
	case PKT_ID_REQ_UNB:
		data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken);
		break;
    case PKT_ID_REQ_LOG:
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), g_client_handle.iottoken, (type_u8*)arg);
        break;    
    case PKT_ID_RES_PATCH_CHUNK:        
        pk->pkt_flag    = ql_htons((DATATYPE_JSON & 0x7f) << 8 | (DIR_DEVICE_RESPOND& 0x01) << 15);
        patch_arg = (ota_res_arg*)arg;                   
        data_len = ql_snprintf(pk->pkt_data, data_len - PACKET_HEAD_LENGTH, PKT_JSON_CTX(pkt_id), patch_arg->error, g_client_handle.iottoken, patch_arg->offset);
        break;
    
    
    default:
        break;
    }
    if(data_len < 0)
    {
        ql_free(pk);
        return NULL;
    }	
    #ifdef DEBUG_DATA_OPEN
    ql_printf("pk->pkt_data[%d]:%s\r\n", data_len, pk->pkt_data );
    #endif
    if((g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_AES||g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SM4) && pkt_id != PKT_ID_REQ_REG)
    {
        encrypt_len = data_len + (16 - data_len % 16);
        encrypt_data = (type_u8 *)ql_zalloc(encrypt_len+1);
        if(encrypt_data == NULL)
        {
            ql_free(pk);
            ql_log_err(ERR_EVENT_INTERFACE, "malc ecpt[%d] err", data_len);
            return NULL;
        }
        //iot_aes_cbc128_encrypt((type_u8*)pk->pkt_data, data_len, encrypt_data, (type_size_t*) &encrypt_len, g_client_handle.server_aes_key);

		if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_AES)
		{
			iot_aes_cbc128_encrypt((type_u8*)pk->pkt_data, data_len, encrypt_data, (type_size_t*) &encrypt_len, g_client_handle.server_aes_key);
		}
		else if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SM4)
		{
			/*add sm4 encrypt*/
			ql_sm4_encrypt_cbc(g_client_handle.server_aes_key,(type_u8*)pk->pkt_data,data_len,encrypt_data,encrypt_len);
		}
		
        ql_memcpy(pk->pkt_data, encrypt_data, encrypt_len);

		if(encrypt_data != NULL)
		{
	        ql_free(encrypt_data);
	        encrypt_data = NULL;
		}

        pk->pkt_len = sizeof(cloud_packet_t) + encrypt_len;
    }
    else
    {
        pk->pkt_len = sizeof(cloud_packet_t) + data_len;
    }

    return pk;
}

//md5(str_right(mac, 8)+dev_key+"iotqyl")
SEC_CODE void _calc_service_key()
{
	char temp[128] = {0};
	unsigned char result[64] = {0};
	int i = 0;

    #ifdef USE_SMPU
    char device_aes_key_temp[AESKEY_LEN + 1];
    ql_sec_memcpy(device_aes_key_temp, g_client_handle.device_aes_key, AESKEY_LEN + 1);
    ql_snprintf(temp, sizeof(temp), (const char *)"%s%s%s", (char *)(g_client_handle.mac_str+ql_strlen((const char *)g_client_handle.mac_str)-8), (char *)device_aes_key_temp, (char *)"iotqly");
    #else
	ql_snprintf(temp, sizeof(temp), (const char *)"%s%s%s", (char *)(g_client_handle.mac_str+ql_strlen((const char *)g_client_handle.mac_str)-8), (char *)g_client_handle.device_aes_key, (char *)"iotqly");
    #endif

	iot_encode_md5(result, sizeof(result), (unsigned char *)temp, ql_strlen(temp));

	for (i = 0; i < AESKEY_LEN; i++) {
		g_client_handle.server_aes_key[i] = tolower_convert(result[i]);
	}
	g_client_handle.server_aes_key[AESKEY_LEN] = '\0';
}

// return - version1 < version2
// return 0 version1 = version2
// return + version1 > version2
static int cloud_version_compare(char * version1, int version1_len, char * version2, int version2_len)
{
    int i = 0, j = 0;
    while (i < version1_len || j < version2_len) 
    {
        int num1 = 0, num2 = 0;
        while (i < version1_len && version1[i] != '.') 
        {
            num1 = num1 * 10 + version1[i] - '0';
            i++;
        }
        i++;
        while (j < version2_len && version2[j] != '.') 
        {
            num2 = num2 * 10 + version2[j] - '0';
            j++;
        }
        j++;
        if (num1 != num2) 
        {
            return num1 - num2;
        }
    }
    return 0;
}

#define CLOUD_SUPPORT_OTA_BREAKPOINT_VERSION "2.0.107"
void cloud_reginfo_store(const char *data, type_size_t len)
{
    char dvk[100] = {0};
    char real_dvk[64] = {0};
	cJSON *root = cJSON_Parse(data);
	if (root == NULL) {
		ql_client_set_status(NET_REGINFAIL, CLI_DISCONN_TYPE_NULL);
		return ;
	}
	cJSON *base_response = cJSON_GetObjectItem(root, const_str_res);
	cJSON *message_data = cJSON_GetObjectItem(root, const_str_data);
	if (base_response == NULL || message_data == NULL) {
		goto err;
	}
	cJSON *errcode = cJSON_GetObjectItem(base_response, const_str_errcode);
	if (errcode != NULL && (errcode->valueint != 0)) {
		goto err;
	}
	cJSON *dev_id = cJSON_GetObjectItem(message_data, "dev_id");
	cJSON *dev_key = cJSON_GetObjectItem(message_data, "dev_key");
	if (dev_id == NULL || dev_key == NULL) {
		goto err;
	}
	if (ql_strlen(dev_id->valuestring) == 0 || ql_strlen(dev_key->valuestring) == 0) {
		goto err;
	}

	ql_strncpy((char*)g_client_handle.deviceid, dev_id->valuestring, DEVICE_ID_LEN);
    g_client_handle.deviceid[DEVICE_ID_LEN] = '\0';

    cJSON *cld_ver  = cJSON_GetObjectItem(message_data, "cloud_version");
    if(cld_ver)
    {
        if(g_client_handle.ota_type == OTA_TYPE_BREAKPOINT && (cloud_version_compare(cld_ver->valuestring, strlen(cld_ver->valuestring), CLOUD_SUPPORT_OTA_BREAKPOINT_VERSION, strlen(CLOUD_SUPPORT_OTA_BREAKPOINT_VERSION)) < 0))
            g_client_handle.ota_type = OTA_TYPE_BASIC;
    }
    else if(g_client_handle.ota_type == OTA_TYPE_BREAKPOINT)
    {
        g_client_handle.ota_type = OTA_TYPE_BASIC;
    }

	int dlen = iot_decode_base64(dvk, 100, dev_key->valuestring, ql_strlen(dev_key->valuestring));
	int outlen = 64;

	iot_aes_cbc128_decrypt((unsigned char *)dvk, (type_size_t)dlen, (unsigned char *)real_dvk, (type_size_t*)&outlen, g_client_handle.product_aes_key);

    #ifdef USE_SMPU
    ql_sec_memcpy(g_client_handle.device_aes_key, real_dvk, AESKEY_LEN);
    #else
    ql_memcpy(g_client_handle.device_aes_key, real_dvk, AESKEY_LEN);
    #endif

    cJSON_Delete(root);

    //set service AES key.
	_calc_service_key();

	ql_client_set_status(NET_REGINFED, CLI_DISCONN_TYPE_NULL);

    g_client_handle.conn_interval = CONN_INTERVAL_NORMAL;

	return ;
err:
	ql_client_set_status(NET_REGINFAIL, CLI_DISCONN_TYPE_NULL);
	cJSON_Delete(root);
}
void cloud_auth_store(const char *data, type_size_t len)
{
	cJSON *root = cJSON_Parse(data);
	if (root == NULL) {
		ql_client_set_status(NET_AUTHFAIL, CLI_DISCONN_TYPE_NULL);
		return ;
	}
	cJSON *base_response = cJSON_GetObjectItem(root, const_str_res);
	cJSON *message_data = cJSON_GetObjectItem(root, const_str_data);
	if (base_response == NULL || message_data == NULL) {
		goto err;
	}
	cJSON* errcode = cJSON_GetObjectItem(base_response, const_str_errcode);
	if (errcode == NULL || errcode->valueint != 0) {
		goto err;
	}
	cJSON *iotid = cJSON_GetObjectItem(message_data, "iotid");
	cJSON *token = cJSON_GetObjectItem(message_data, "token");
	cJSON *timestamp = cJSON_GetObjectItem(message_data, const_str_ts);

	if (iotid == NULL || token == NULL || timestamp == NULL) {
		goto err;
	}

	if (ql_strlen(iotid->valuestring) == 0 || ql_strlen(token->valuestring) == 0) {
		goto err;
	}

	ql_strncpy((char*)g_client_handle.iotid, iotid->valuestring, IOTID_LEN);
    g_client_handle.iotid[IOTID_LEN] = '\0';

	ql_strncpy((char*)g_client_handle.iottoken, token->valuestring, TOKEN_LEN);
    g_client_handle.iottoken[TOKEN_LEN] = '\0';

    set_current_time(timestamp->valueint);

	cJSON_Delete(root);
	ql_client_set_status(NET_AUTHED, CLI_DISCONN_TYPE_NULL);
#ifdef LOCAL_SAVE
    ql_local_uplog_to_cloud();
    ql_local_log_to_cloud( LOG_TYPE_INFO, INFO_EVENT_DEV, NULL, get_current_time());
#endif
    iot_status_cb(DEV_STA_CONNECTED_CLOUD, get_current_time());
    g_client_handle.dev_status = 0;

    #ifdef LOCAL_SAVE
    ql_local_dev_state_reset(DEV_STATE_RST|DEV_STATE_UNB);    
    //ql_local_uplog_to_cloud();
    #endif

	return ;
err:
	ql_client_set_status(NET_AUTHFAIL, CLI_DISCONN_TYPE_NULL);
	cJSON_Delete(root);
}
void qlcloud_process_heart(const char* data, type_size_t len, int seq)
{
	cJSON *root = cJSON_Parse(data);
	if (root == NULL)
		return ;
	cJSON *data_value = cJSON_GetObjectItem(root, const_str_res);
	if (data_value == NULL)
		goto out;
    cJSON *errcode = cJSON_GetObjectItem(data_value, const_str_errcode);
	if (errcode != NULL && (errcode->valueint != 0))
		goto out;
	cJSON *timestamp = cJSON_GetObjectItem(root, const_str_value);
	if (timestamp == NULL)
		goto out;

	if (timestamp->valueint) {
        set_current_time(timestamp->valueint);
	}
out:
	cJSON_Delete(root);
}

#if (__SDK_TYPE__== QLY_SDK_LIB)
int qlcloud_dp_handle(char sub_id[32], type_u8 dpid, char* dp_data_str, type_u32 dp_data_str_len)
{
    int i = 0;
    type_u8 dptype = 0, u8_temp = 0, is_find_dpid = 0;
    type_u8* dp_data = NULL;
    type_u32 dp_data_len = 0, dp_bin_len = 0;
    type_s32 i32_temp = 0;
    type_f32 f32_temp = 0.0;
    for(i = 0; i < DOWN_DPS_CNT; i++)
    {
       if(iot_down_dps[i].dpid == dpid)
       {
           is_find_dpid = 1;
           dptype = iot_down_dps[i].dptype;
           switch(dptype)
           {
           case DP_TYPE_INT:
               i32_temp = str2int((const char *)dp_data_str);
               dp_data = (type_u8*)&i32_temp;
               dp_data_len = 4;
               break;
           case DP_TYPE_BOOL:
               u8_temp = (type_s8)str2int((const char *)dp_data_str);
               if(u8_temp!=0 && u8_temp!=1)
               {
                   ql_log_err( ERR_EVENT_RX_DATA, "bool[%d] invalid!", u8_temp);
                   return -1;
               }
               dp_data = &u8_temp;
               dp_data_len = 1;
               break;
           case DP_TYPE_ENUM:
               u8_temp = (type_s8)str2int((const char *)dp_data_str);
               dp_data = &u8_temp;
               dp_data_len = 1;
               break;
           case DP_TYPE_FLOAT:
               f32_temp = (type_f32)str2float((const char *)dp_data_str);
               dp_data = (type_u8*)&f32_temp;
               dp_data_len = 4;
               break;
           case DP_TYPE_STRING:
           case DP_TYPE_FAULT:
               dp_data = (type_u8*)dp_data_str;
               dp_data_len = dp_data_str_len;
               break;
           case DP_TYPE_BIN:
               if (0 != (ql_strlen(dp_data_str) & 0x3))
               {
                   ql_log_err( ERR_EVENT_RX_DATA, "%d not multiple of 4", (int)ql_strlen(dp_data_str));
                   return -1;                   
               }
               dp_bin_len = MEM_ALIGN_SIZE(ql_strlen(dp_data_str));
               dp_data = (type_u8*)ql_malloc(dp_bin_len);
               if(dp_data == NULL){
                   ql_log_err(ERR_EVENT_INTERFACE, "malc dp[%d] err", dp_bin_len);
                   return -1;
               }
               dp_data_len = iot_decode_base64((char *)dp_data, (type_size_t)dp_bin_len,(const char*)dp_data_str, dp_bin_len);
               if((int)dp_data_len<0)
               {
                   ql_free(dp_data);
                   return -1;
               }
               break;
           default:
               ql_log_err( ERR_EVENT_RX_DATA, "dp_t[%d] err", dptype);
               return -1;
           }

           if(iot_down_dps[i].dp_down_handle == NULL)
           {
               ql_log_err( ERR_EVENT_RX_DATA, "no dp func");
               if(DP_TYPE_BIN == dptype)
               {
                   ql_free(dp_data);
               }
               return -1;
           }
           else
           {
               iot_down_dps[i].dp_down_handle(sub_id, dp_data, dp_data_len);
               if(DP_TYPE_BIN == dptype)
               {
                   ql_free(dp_data);
               }
           }
       }
   }
   if(!is_find_dpid)
   {
       ql_log_err( ERR_EVENT_RX_DATA, "not find dpid[%d]", dpid);
   }
   return 0;
}
#elif (__SDK_TYPE__== QLY_SDK_BIN)

extern type_u32 bin_mcu_chunk_seq;
extern type_u32 bin_mcu_chunk_offset;

void cloud_dp_store(const char *data, type_size_t len)
{
    ql_set_dp_type(data);
   // hexdump("dp store", data, len);
    ql_client_set_status(NET_DP_GETED, CLI_DISCONN_TYPE_NULL);
}

int qlcloud_dp_handle_to_mcu(char sub_id[32],type_u8 dpid, type_u8* dp_data_str, type_u8*data_out, type_u16* data_out_len)
{
    int dp_index = 0;
    type_u8  dptype = 0, u8_temp = 0;;
    type_u8* dp_data = NULL;
    type_u8* dp_data_p = NULL;

    type_u32 dp_data_len = 0, dp_bin_len = 0;
    type_s32  i32_temp = 0;
    float f32_temp = 0;

    dptype = ql_get_dp_type(dpid);
    *data_out_len = 0;

	iot_s32 i=0;
	if(NULL == sub_id)
	{
		for(i=0;i<32;i++)
		{		
			*data_out++ = 0x00;         //sub_id
			*data_out_len += 1;
		}
	}
	else
	{	
		for(i=0;i<32;i++)
		{		
			if(i<ql_strlen(sub_id))
			{
				*data_out++ = sub_id[i];         //sub_id
				*data_out_len += 1;
			}
			else
			{
				*data_out++ = 0x00;         //sub_id
				*data_out_len += 1;
			}

		}
	}
	
    *data_out++ = dpid;         //id
    *data_out_len += 1;

    *data_out++ = dptype;       //type
    *data_out_len += 1;

    switch(dptype)
    {
    case DP_TYPE_INT:
        i32_temp = str2int(dp_data_str);
        UINT16_TO_STREAM_f(data_out, 4);            //len
        UINT32_TO_STREAM_f(data_out+2, i32_temp);   //data
        *data_out_len += 6;
        break;
    case DP_TYPE_BOOL:
        u8_temp = (type_s8)str2int(dp_data_str);
        if(u8_temp!=0 && u8_temp!=1)
        {
            ql_log_err( ERR_EVENT_RX_DATA, "bool[%d] invalid!", u8_temp);
            return-1;
        }
        UINT16_TO_STREAM_f(data_out, 1);            //len
        *(data_out+2) = u8_temp;                    //data
        *data_out_len += 3;
        break;
    case DP_TYPE_ENUM:
        u8_temp = (type_s8)str2int(dp_data_str);
        UINT16_TO_STREAM_f(data_out, 1);            //len
        *(data_out+2) = u8_temp;                    //data
        *data_out_len += 3;
        break;
    case DP_TYPE_FLOAT:
        f32_temp = str2float(dp_data_str);
        UINT16_TO_STREAM_f(data_out, 4);            //len
        FLOAT32_TO_STREAM_f(data_out + 2, f32_temp); //data                
        *data_out_len += 6;
        break;
    case DP_TYPE_STRING:
    case DP_TYPE_FAULT:
        dp_data_len = ql_strlen(dp_data_str);
        UINT16_TO_STREAM_f(data_out, dp_data_len);  //len
        data_out += 2;
        *data_out_len+=(2+dp_data_len);
        while(dp_data_len--)
        {
            *data_out++ = *dp_data_str++;          //data
        }
        break;
    case DP_TYPE_BIN:
        dp_bin_len = MEM_ALIGN_SIZE(ql_strlen(dp_data_str));
        dp_data = (type_u8*)ql_malloc(dp_bin_len);
        if(dp_data == NULL){
            return-1;
        }
        dp_data_p = dp_data;
        dp_data_len = iot_decode_base64(dp_data, dp_bin_len,(const char*)dp_data_str, dp_bin_len);
        if(dp_data_len<0)
        {
            ql_free(dp_data);
            return -1;
        }

        UINT16_TO_STREAM_f(data_out, dp_data_len);  //len
        data_out += 2;
        *data_out_len+=(2+dp_data_len);
        while(dp_data_len--)
        {
            *data_out++ = *dp_data_p++;              //data
        }
        ql_free(dp_data);
        break;
    default:
        ql_log_err(ERR_EVENT_RX_DATA, "dp_t[%d] err", dptype);
        return -1;
        break;
    }

    return 0;
}
#endif

DATA_LIST_T  g_data_list;
void qlcloud_parse_data(type_u16 pkt_opcode, int seq, const char* data, type_size_t data_len)
{
    type_u32 i = 0, sub_act = 0, timestamp = 0;

    cJSON *root = NULL, *json_res = NULL,  *json_value = NULL, *json_dev_list = NULL, *sub_item = NULL;

    if(data_len == 0 || data == NULL)
    {
        ql_log_err(ERR_EVENT_RX_DATA,"parse data err");
        return;
    }

    root = cJSON_Parse(data);
    if (root == NULL)
    {
        ql_log_err(ERR_EVENT_RX_DATA,"parse root err");
        return;
    }

    json_res = cJSON_GetObjectItem(root, "res");
    if (json_res == NULL)
    {        
        ql_log_err(ERR_EVENT_RX_DATA,"json_res err");
        goto out;
    }

    sub_item = NULL;
    sub_item = cJSON_GetObjectItem(json_res, "err_num");
    if(sub_item == NULL)
    {
        ql_log_err(ERR_EVENT_RX_DATA,"err_num err");
        goto out;
    }
    g_data_list.count = sub_item->valueint;

    if(g_data_list.count > 0)
    {
        g_data_list.info = (DATA_INFO_T*)ql_zalloc( sizeof(DATA_INFO_T) * g_data_list.count );
        if(g_data_list.info == NULL)
        {
            ql_log_err(ERR_EVENT_INTERFACE,"malc list err");
            goto out;
        }
        
        json_value = cJSON_GetObjectItem(root, "value");
        if (json_value == NULL || json_value->child == NULL)
        {        
            ql_log_err(ERR_EVENT_RX_DATA,"json_value err");
            goto out;
        } 
        
        json_dev_list = json_value->child;
        for(i = 0; i < g_data_list.count; i++)
        {
            if(json_dev_list == NULL)
            {            
                ql_log_err(ERR_EVENT_RX_DATA,"value child[%d] err",i);
                goto out;
            }

            sub_item = NULL;
            sub_item = cJSON_GetObjectItem( json_dev_list,"d");
            if(sub_item == NULL)
            {
                ql_log_err(ERR_EVENT_RX_DATA,"parse d err");
                goto out;
            }

            if(strcmp(sub_item->valuestring,"") == 0)
                g_data_list.info[i].dev_id = NULL;
            else
                g_data_list.info[i].dev_id = sub_item->valuestring;

            sub_item = NULL;
            sub_item = cJSON_GetObjectItem(json_dev_list,"err");
            if(sub_item == NULL)
            {
                ql_log_err(ERR_EVENT_RX_DATA,"parse d err");
                goto out;
            }
            g_data_list.info[i].info_type = (DATA_INFO_TYPE_T)sub_item->valueint;
            
            json_dev_list = json_dev_list->next;
        }
    }

    if (pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_SUB_ACT) || pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_SUB_INACT))
    {        
        sub_item = NULL;
        sub_item = cJSON_GetObjectItem(root, "act_t");
        if(sub_item == NULL)
        {
            ql_log_err(ERR_EVENT_RX_DATA,"pars act_t err");
            goto out;
        }

        if(pkt_opcode == PKT_OP_CODE(PKT_ID_REQ_SUB_ACT))
            sub_act = (sub_item->valueint == 0)? SUB_STA_BIND_ONLINE: SUB_STA_ONLINE;
        else
            sub_act = (sub_item->valueint == 0)? SUB_STA_OFFLINE: SUB_STA_UNB_OFFLINE;

        sub_item = NULL;
        sub_item = cJSON_GetObjectItem(root, "ts");
        if(sub_item == NULL)
        {
            ql_log_err(ERR_EVENT_RX_DATA,"pars ts err");
            goto out;
        }
        timestamp = sub_item->valueint;

        iot_sub_status_cb(sub_act, timestamp, &g_data_list, seq);
        
    }
    else
    {
        iot_data_cb( seq , &g_data_list);
    }

out:
    if(g_data_list.info)
    {
        ql_free(g_data_list.info);
        g_data_list.info = NULL;
    }
    g_data_list.count = 0;

    cJSON_Delete(root);
}

void qlcloud_sub_forced_unb_data( int seq, const char* data, type_size_t data_len)
{
    type_u32 i = 0, ts = 0;

    cJSON *root = NULL, *json_res = NULL,  *json_value = NULL, *json_dev_list = NULL, *sub_item = NULL;

    if(data_len == 0 || data == NULL)
    {
        ql_log_err(ERR_EVENT_RX_DATA,"parse data err");
        return;
    }

    root = cJSON_Parse(data);
    if (root == NULL)
    {
        ql_log_err(ERR_EVENT_RX_DATA,"parse root err");
        return;
    }

    json_res = cJSON_GetObjectItem(root, "res");
    if (json_res == NULL)
    {        
        ql_log_err(ERR_EVENT_RX_DATA,"json_res err");
        goto out;
    }

    sub_item = NULL;
    sub_item = cJSON_GetObjectItem(json_res, "count");
    if(sub_item == NULL)
    {
        ql_log_err(ERR_EVENT_RX_DATA,"count err");
        goto out;
    }
    g_data_list.count = sub_item->valueint;

    if(g_data_list.count > 0)
    {
        g_data_list.info = (DATA_INFO_T *)ql_zalloc( sizeof(DATA_INFO_T) * g_data_list.count );
        if(g_data_list.info == NULL)
        {
            ql_log_err(ERR_EVENT_INTERFACE,"malc list err");
            goto out;
        }
        
        json_value = cJSON_GetObjectItem(root, "value");
        if (json_value == NULL || json_value->child == NULL)
        {        
            ql_log_err(ERR_EVENT_RX_DATA,"json_value err");
            goto out;
        } 
        
        json_dev_list = json_value->child;
        for(i = 0; i < g_data_list.count; i++)
        {
            if(json_dev_list == NULL)
            {            
                ql_log_err(ERR_EVENT_RX_DATA,"value child[%d] err",i);
                goto out;
            }

            sub_item = NULL;
            sub_item = cJSON_GetObjectItem( json_dev_list,"sub");
            if(sub_item == NULL)
            {
                ql_log_err(ERR_EVENT_RX_DATA,"parse sub err");
                goto out;
            }
           g_data_list.info[i].dev_id = sub_item->valuestring;
           json_dev_list = json_dev_list->next;
        }
    }

    sub_item = NULL;
    sub_item = cJSON_GetObjectItem(root, "ts");
    if(sub_item == NULL)
    {
        ql_log_err(ERR_EVENT_RX_DATA,"ts err");
        goto out;
    }
    ts = (type_u32)sub_item->valueint;
    
    iot_sub_status_cb(SUB_STA_PASSIVE_UNBIND, ts, &g_data_list, seq );
  
out:
    if(g_data_list.info)
    {
        ql_free(g_data_list.info);
        g_data_list.info = NULL;
    }
    g_data_list.count = 0;

    cJSON_Delete(root);
}

//{"data": [ {"i": 101,"v": "12","t": 1},{"i": 102,"v": "abc","t": 3}], "expire": 123456}
void qlcloud_process_downdps(const char* data, type_size_t len, int seq)
{
    type_u8 arr_cnt = 0, i = 0, dpid = 0;
    cJSON *root = NULL, *data_arr = NULL, *expire = NULL, *arr_item = NULL, *sub_item = NULL;

    cJSON *sub_id_json = NULL;
    char* p_sub_id= NULL;
    
    #if (__SDK_TYPE__== QLY_SDK_BIN)
    static type_u8  dp_mcu_data[1028] = {0};
    type_u16  dp_mcu_data_total_len = 0;
    type_u16  dp_mcu_data_len = 0;
    #endif

	root = cJSON_Parse(data);
	if (root == NULL)
		return ;
	data_arr = cJSON_GetObjectItem(root, const_str_data);
	if (data_arr == NULL)
		goto out;

	expire = cJSON_GetObjectItem(root, "expire");
	if (expire == NULL || (type_u32)expire->valueint < get_current_time())
		goto out;

    sub_id_json = cJSON_GetObjectItem(root, const_str_sbid);
    if (sub_id_json == NULL)
    {
        p_sub_id = NULL;
    }   
    else
    {
        if(ql_strlen(sub_id_json->valuestring) == 0)
        {
            p_sub_id = NULL;
        }
        else if(ql_strlen(sub_id_json->valuestring) <= SUB_DEV_ID_MAX)
        {
            p_sub_id = sub_id_json->valuestring;
        }
        else
        {
            ql_log_err(ERR_EVENT_RX_DATA,"Down:subid err");
            goto out;
        }
    }
    
    arr_cnt = cJSON_GetArraySize(data_arr);

    if(arr_cnt > 20)
    {
        ql_log_err(ERR_EVENT_RX_DATA, "dp_c %d>max", arr_cnt);
        goto out;
    }


    for(i = 0; i < arr_cnt; i++)
    {
        arr_item = cJSON_GetArrayItem(data_arr, i);
        if(arr_item)
        {
            sub_item = cJSON_GetObjectItem(arr_item, "i");
            if(sub_item == NULL)
            {
                goto out;
            }
            dpid = (type_u8)sub_item->valueint;
            sub_item = cJSON_GetObjectItem(arr_item, "v");
            if(sub_item == NULL)
            {
                goto out;
            }
            #if (__SDK_TYPE__== QLY_SDK_LIB)
                #if (__SDK_PLATFORM__ == 0x19)
                extern iot_s32 dp_down_handle(char sub_id[32], iot_u8 dpid, char* dp_data_str, iot_u32 dp_data_str_len);
                if(dp_down_handle(p_sub_id, dpid, sub_item->valuestring, ql_strlen(sub_item->valuestring)) < 0)
                {
                    goto out;
                }
                #else
                if(qlcloud_dp_handle(p_sub_id, dpid, sub_item->valuestring, ql_strlen(sub_item->valuestring)) < 0)
                {
                    goto out;
                }
                #endif
            #elif (__SDK_TYPE__== QLY_SDK_BIN)

                if(qlcloud_dp_handle_to_mcu(p_sub_id,dpid, sub_item->valuestring, dp_mcu_data + dp_mcu_data_total_len, &dp_mcu_data_len) < 0)
                {
                    goto out;
                }
				
				dp_mcu_data_total_len += dp_mcu_data_len;
                //hexdump("to mcu dp", dp_mcu_data, dp_mcu_data_total_len);
            #endif
        }
    }
    #if (__SDK_TYPE__== QLY_SDK_BIN)
        //send to uart
        if(dp_mcu_data_total_len > 1028)
        {
            goto out;
        }
		#ifndef DEBUG_WIFI_OTA_OPEN
        module_tx_data_downlink(dp_mcu_data, dp_mcu_data_total_len);
		#endif
    #endif

out:
	cJSON_Delete(root);
}

void qlcloud_process_rx_data(const char* data, type_size_t len, type_u32 seq)
{
    cJSON *sub_id_json = NULL;
    char* p_sub_id = NULL;

	cJSON *root = cJSON_Parse(data);
	if (root == NULL)
		return ;
	cJSON *data_value = cJSON_GetObjectItem(root, const_str_data);
	if (data_value == NULL)
		goto out;

	cJSON *timestamp = cJSON_GetObjectItem(root, const_str_ts);
	if (timestamp == NULL)
		goto out;

	sub_id_json = cJSON_GetObjectItem(root, const_str_sbid);
	if (sub_id_json == NULL)
    {
        p_sub_id = NULL;
    }   
    else
    {
        if(ql_strlen(sub_id_json->valuestring) == 0)
        {
            p_sub_id = NULL;
        }
        else if(ql_strlen(sub_id_json->valuestring) < 17)
        {
            p_sub_id = sub_id_json->valuestring;
        }
        else
        {
            ql_log_err(ERR_EVENT_RX_DATA, "Rx:subid err");
            goto out;
        }
    }
    iot_rx_data_cb(p_sub_id, timestamp->valueint, (type_u8*)(data_value->valuestring), ql_strlen(data_value->valuestring));
out:
	cJSON_Delete(root);
}

void qlcloud_process_rx_data_v2(const char* data, type_size_t len, type_u32 seq)
{
    int i = 0;
	push_rx_pkt_t * push_rx_pkt = NULL;

    #ifdef DEBUG_DATA_OPEN
        ql_printf("rx_data_v2 data[%d]:\r\n", len);
        for(i=0; i<len; i++)
        {
            ql_printf("%02x ", data[i]);
        }
        ql_printf("\r\n");
    #endif

    push_rx_pkt = (push_rx_pkt_t *)ql_zalloc(sizeof(push_rx_pkt_t));
    if(push_rx_pkt == NULL)
    {
        goto out;
    }
    
	/*sub_id_len*/
	push_rx_pkt->sub_id_len = *(data++);
    #ifdef DEBUG_DATA_OPEN
         ql_printf("sub_id_len:%d | ", push_rx_pkt->sub_id_len);
    #endif
    if(push_rx_pkt->sub_id_len == 0)
    {
        push_rx_pkt->sub_id = NULL;
    }
    else if(push_rx_pkt->sub_id_len <= SUB_DEV_ID_MAX)
    {
        push_rx_pkt->sub_id = ql_zalloc(push_rx_pkt->sub_id_len+1);
        if(push_rx_pkt->sub_id == NULL)
        {
            goto out;
        }
        ql_memcpy(push_rx_pkt->sub_id, data, push_rx_pkt->sub_id_len);
        data += push_rx_pkt->sub_id_len;
        #ifdef DEBUG_DATA_OPEN
            ql_printf("sub_id:%s | ", push_rx_pkt->sub_id);
        #endif
    }
    else
    {
        ql_log_err(ERR_EVENT_RX_DATA, "Rx:v2 subid err");
        goto out;
    }

	/*ts*/
	push_rx_pkt->ts = 0;
	for(i = 8; i > 0; i--)
	{
		push_rx_pkt->ts |= (type_u64)((*data)&0xff) << 8*(i - 1);
		data++;
	}
    #ifdef DEBUG_DATA_OPEN
        ql_printf("ts:%llu | ", push_rx_pkt->ts);
    #endif

	/*data_len*/
	for(i = 4; i > 0; i--)
	{
		push_rx_pkt->data_len |=  ((*data)&0xff) << 8*(i - 1);        
		data++;
	} 
    #ifdef DEBUG_DATA_OPEN
        ql_printf("data_len:%d | ", push_rx_pkt->data_len);
    #endif

	/*data*/
	push_rx_pkt->data = ql_zalloc(push_rx_pkt->data_len+1);
    if(push_rx_pkt->data == NULL)
    {
        goto out;
    }
	ql_memcpy(push_rx_pkt->data, data, push_rx_pkt->data_len);
    #ifdef DEBUG_DATA_OPEN
        ql_printf("data:%s\r\n", push_rx_pkt->data);
    #endif

    iot_rx_data_cb(push_rx_pkt->sub_id, push_rx_pkt->ts, push_rx_pkt->data, push_rx_pkt->data_len);
out:
	if(push_rx_pkt->data != NULL)   ql_free(push_rx_pkt->data);
	if(push_rx_pkt->sub_id != NULL) ql_free(push_rx_pkt->sub_id);
	if(push_rx_pkt != NULL)         ql_free(push_rx_pkt);
}

#if (__SDK_TYPE__== QLY_SDK_BIN)
void ql_udp_process_rx_data_v2(const char* data, type_u32 len, type_u32 ts, type_u32 seq)
{
    int i = 0;
	push_rx_pkt_t * push_rx_pkt = NULL;

    #ifdef DEBUG_DATA_OPEN
        ql_printf("udp_rx_data_v2 data[%d]:\r\n", len);
        for(i=0; i<len; i++)
        {
            ql_printf("%02x ", data[i]);
        }
        ql_printf("\r\n");
    #endif

    push_rx_pkt = (push_rx_pkt_t *)ql_zalloc(sizeof(push_rx_pkt_t));
    if(push_rx_pkt == NULL)
    {
        goto out;
    }
    
	/*sub_id_len*/
	push_rx_pkt->sub_id_len = 0;
    push_rx_pkt->sub_id = NULL;
    #ifdef DEBUG_DATA_OPEN
         ql_printf("sub_id_len:%d | ", push_rx_pkt->sub_id_len);
    #endif

	/*ts*/
	push_rx_pkt->ts = ts;
    #ifdef DEBUG_DATA_OPEN
        ql_printf("ts:%d | ", (type_u32)push_rx_pkt->ts);
    #endif

	/*data_len*/
	for(i = 4; i > 0; i--)
	{
		push_rx_pkt->data_len |=  ((*data)&0xff) << 8*(i - 1);        
		data++;
	} 
    #ifdef DEBUG_DATA_OPEN
        ql_printf("data_len:%d | ", push_rx_pkt->data_len);
    #endif

	/*data*/
	push_rx_pkt->data = ql_zalloc(push_rx_pkt->data_len);
    if(push_rx_pkt->data == NULL)
    {
        goto out;
    }
    ql_memset(push_rx_pkt->data, 0x00, push_rx_pkt->data_len);
	ql_memcpy(push_rx_pkt->data, data, push_rx_pkt->data_len);
    #ifdef DEBUG_DATA_OPEN
    ql_printf("data[%d]:\r\n", push_rx_pkt->data_len);
    for(i = 0; i < push_rx_pkt->data_len; i++)
        ql_printf("%02x ", push_rx_pkt->data[i]);
    ql_printf("\r\n");
    #endif

    iot_rx_data_cb(push_rx_pkt->sub_id, push_rx_pkt->ts, push_rx_pkt->data, push_rx_pkt->data_len);
out:
	if(push_rx_pkt->data != NULL)   ql_free(push_rx_pkt->data);
	if(push_rx_pkt->sub_id != NULL) ql_free(push_rx_pkt->sub_id);
	if(push_rx_pkt != NULL)         ql_free(push_rx_pkt);
}
#endif

#if 0/*log online switch*/
//{"req":{"token":"K91xz8Bi1PTr6/x6XphAUw"},"data":{"sw":0}}
void qlcloud_process_log_sw(const char* data, type_size_t len)
{
    type_u8 dp_sw = 0;
    cJSON *root = NULL, *data_arr = NULL, *sub_item = NULL;
    
    root = cJSON_Parse(data);
    if (root == NULL)
        return ;
    
    data_arr = cJSON_GetObjectItem(root, const_str_data);
    if (data_arr == NULL)
        goto out;

    sub_item = cJSON_GetObjectItem(data_arr, "sw");
    if(sub_item == NULL)
    {
        goto out;
    }
    ql_log_info("log_sw： dp_sw = %d \r\n");
    set_log_online_switch((type_u8)sub_item->valueint);
out:
	cJSON_Delete(root);
}
#endif

void qlcloud_process_otapassive(const char *in, int inlen, type_u32 pkt_seq)
{
    int i_owner = 0, ret = 0;

    cJSON *root = cJSON_Parse((const char *)in);
    if (root == NULL) {
        ret = -1;
        goto fatal_err;
    }

    cJSON *owner = cJSON_GetObjectItem(root, "owner");
    if (owner == NULL) {
        cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
    }

    i_owner = (int)(owner->valueint);

    if(OTA_OWNER_WIFI == i_owner) 
    {
        #if (__SDK_TYPE__== QLY_SDK_BIN)
            if (g_client_handle.ota_wifi_status >= OTA_UPGRADE_CMD_GET && g_client_handle.ota_wifi_status <= OTA_UPGRADE_CMD_BACK) 
            {
                ql_log_err(ERR_EVENT_RX_DATA, "wifi:%s wait rb", const_str_ota_owner[0]);
                ret = -2;
            } 
            else if(g_client_handle.ota_mcu_status > OTA_FILECHUNK_BEGIN &&  g_client_handle.ota_mcu_status < OTA_UPGRADE_CMD_GET)
            {
                ql_log_err(ERR_EVENT_RX_DATA, "mcu:%s upgrading", const_str_ota_owner[1]);
                ret = -2;
            } 
            else 
            {
                g_client_handle.ota_wifi_type = OTA_CHECK_TYPE_NEED_PASSIVE;
                set_ota_wifi_status(OTA_INIT);
                ret = 0;
            }
        #elif (__SDK_TYPE__== QLY_SDK_LIB)
            ql_log_err(ERR_EVENT_RX_DATA, "pls select mcu fw.");
            set_ota_mcu_status(OTA_INIT);
            ret = -1;
        #endif
    } 
    else if(OTA_OWNER_MCU == i_owner) 
    {
        switch(get_download_status())
        {
            case DEV_DOWNLOAD_FREE:
            case DEV_DOWNLOAD_OTA:
                if (g_client_handle.ota_mcu_status >= OTA_UPGRADE_CMD_GET &&  g_client_handle.ota_mcu_status <= OTA_UPGRADE_CMD_BACK) 
                {
                    ql_log_err(ERR_EVENT_RX_DATA, "mcu:%s wait rb", const_str_ota_owner[1]);
                    ret = -2;
                }                 
                else if(g_client_handle.ota_mcu_status > OTA_FILECHUNK_BEGIN &&  g_client_handle.ota_mcu_status < OTA_UPGRADE_CMD_GET)
                {
                    ql_log_err(ERR_EVENT_RX_DATA, "mcu:%s upgrading", const_str_ota_owner[1]);
                    ret = -2;
                } 
                else 
                {
                    g_client_handle.ota_mcu_type = OTA_CHECK_TYPE_NEED_PASSIVE;
                    set_ota_mcu_status(OTA_INIT);
                    ret = 0;
                }
                break;            
#if (__SDK_TYPE__== QLY_SDK_LIB)
            case DEV_DOWNLOAD_PATCH:                
                ql_log_err(ERR_EVENT_STATU,"ota_p down %d",get_download_status());
                ret = -2;
                break;
#endif
            default:
                ql_log_err(ERR_EVENT_STATU, "ota_p sta err");
                break;
        }
    } 
    else 
    {
        ql_log_err(ERR_EVENT_RX_DATA, "owner err:%d", i_owner);
        set_ota_mcu_status(OTA_INIT);
        #if (__SDK_TYPE__== QLY_SDK_BIN)
        set_ota_wifi_status(OTA_INIT);
        #endif
        ret = -1;
    }

    cJSON_Delete(root);
fatal_err:
    passive_start_response(pkt_seq, ret);
}
unsigned int get_ota_compels_id(void)
{
	return ota_compels_id;
}

type_s8 * get_ota_compels_version(void)
{
	return (type_s8 *)ota_compels_version;
}
void qlcloud_process_ota_compels(const char *in, int inlen, type_u32 pkt_seq)
{
	cJSON *root = NULL;
    cJSON *owner = NULL;
    cJSON *version = NULL;
    cJSON *product_id = NULL;

    int ret = 0;

    if (g_client_handle.ota_mcu_status == OTA_UPGRADE_CMD_GET) {
        ret = -2;
        goto fatal_err;
    }

    root = cJSON_Parse((const char *)in);
	if (root == NULL) {
        ret = -1;
        goto fatal_err;
	}

	owner = cJSON_GetObjectItem(root, "owner");
	if (owner == NULL) {
		cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
	}
    ret = (int)owner->valueint;
    if (ret != OTA_OWNER_MCU) {
		cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
    }

	product_id = cJSON_GetObjectItem(root, "product_id");
	if (product_id == NULL) {
		cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
	}
    ota_compels_id = (int)product_id->valueint;

    version = cJSON_GetObjectItem(root, "version");
    if (version == NULL) {
        cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
    }
    ql_memset(ota_compels_version, 0, sizeof(ota_compels_version));
    v_strncpy((char *)ota_compels_version, sizeof(ota_compels_version), version->valuestring, ql_strlen(version->valuestring));
    if (ota_compels_version[2] != '.') {
		cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
    }
    ql_log_info("ota compels id:%d,v:%s\n", ota_compels_id, ota_compels_version);

    g_client_handle.ota_mcu_type = OTA_CHECK_TYPE_NEED_COMPELS;
    set_ota_mcu_status(OTA_INIT);

	cJSON_Delete(root);
fatal_err:
    compels_upgrade_response(pkt_seq, ret);
}
static void qlcloud_clear_ota_info()
{
    //do not clear version
    g_ota.otafileinfo.flen= 0;
    g_ota.otafileinfo.fcrc= 0;
    g_ota.otafileinfo.owner = 0;

    g_ota.cur_len = 0;
    g_ota.exptect_offset = 0;
    g_ota.cur_crc = 0;
}

#ifdef LOCAL_SAVE
extern local_config_t g_local_config;
#endif
void qlcloud_process_otafileinfo(const char *in, int inlen, type_u32 pkt_seq)
{
	int ret = -1;
	cJSON *root = NULL;
	cJSON *base_response = NULL;
	cJSON *errcode = NULL;
	cJSON *item = NULL;
	cJSON *temp = NULL;
    cJSON *name = NULL;
	type_u32 owner = 0, file_len = 0;

	root = cJSON_Parse((const char *)in);
	if (root == NULL) {
        ret = -1;
		goto fatal_err;
	}

    item = cJSON_GetObjectItem(root, "value");
    if (item == NULL) {
        cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
    }

    temp = cJSON_GetObjectItem(item, "owner");
    if (temp == NULL) {
        cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
    }
    owner = temp->valueint;
    
    if( owner != OTA_OWNER_MCU && owner != OTA_OWNER_WIFI )
    {
        ql_log_err(ERR_EVENT_RX_DATA, "owner:%d", owner);        
        cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
    }

    #if (__SDK_TYPE__== QLY_SDK_LIB)
    if (owner != OTA_OWNER_MCU || g_client_handle.ota_mcu_status != OTA_VERSION_CHECK ) {
        cJSON_Delete(root);
        ret = -2;
		goto fatal_err;
    }//END SDK LIB
    #elif (__SDK_TYPE__== QLY_SDK_BIN)
    if( (owner == OTA_OWNER_WIFI && g_client_handle.ota_wifi_status == OTA_VERSION_CHECK))
    {
    }
    else if(owner == OTA_OWNER_MCU && g_client_handle.ota_mcu_status == OTA_VERSION_CHECK)
    {
    }
    else
    {
        cJSON_Delete(root);
        ret = -2;
		goto fatal_err;
    }
    #endif//END SDK_BIN

	base_response = cJSON_GetObjectItem(root, "res");
	if (base_response == NULL) {
		cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
	}

	errcode = cJSON_GetObjectItem(base_response, "errcode");
	if (errcode == NULL) {
		cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
	}

	ret = (int)(errcode->valueint);

	if  (ret == 0)
    {
    }
	else if (ret == 2)
    {
        //just skip to waiting upgrade cmd status.
        ql_log_info("latest %s firmware has been downloaded\r\n", const_str_ota_owner[owner]);
		cJSON_Delete(root);
        ret = 2;
		goto fatal_err;
	}
    else
    {
		cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
    }

	temp = cJSON_GetObjectItem(item, "ftotallen");
	if (temp == NULL) {
		cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
	}

    file_len = (unsigned int)temp->valueint;

    if(0 == file_len) {
        ql_log_info("current %s firmware version is latest\r\n", const_str_ota_owner[owner]);
        cJSON_Delete(root);
        ret = 1;

        if(ql_patch_check_tm_set)
        {
            ql_patch_check_tm_set(ql_get_current_sysclock());
        }
		goto fatal_err;
    }

    if(get_download_status() != DEV_DOWNLOAD_OTA)
    {
        ql_log_err(ERR_EVENT_STATU,"ota_f:downl %d",get_download_status());
        cJSON_Delete(root);
        ret = -1;
        
        goto fatal_err;
    }

	name = cJSON_GetObjectItem(item, "name");
	if (name == NULL || name->valuestring == NULL || ql_strlen(name->valuestring) > 16) {
		cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
	}

	temp = cJSON_GetObjectItem(item, "version");
	if (temp == NULL) {
		cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
	}
    ql_log_info("ready to download %s new version:%s\r\n", const_str_ota_owner[owner], temp->valuestring);
    if(qlcloud_is_version_valid((const type_u8*)temp->valuestring) < 0)
    {
		cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
    }
    qlcloud_clear_ota_info();

    g_ota.otafileinfo.owner = owner;
	g_ota.otafileinfo.flen = file_len;
    ql_memcpy(g_ota.otafileinfo.version[owner], temp->valuestring, 5);

	temp = cJSON_GetObjectItem(item, "fcrc");
	if (temp == NULL) {
		cJSON_Delete(root);
        ret = -1;
		goto fatal_err;
	}
	g_ota.otafileinfo.fcrc = (unsigned short)temp->valueint;

    #if (__SDK_TYPE__== QLY_SDK_LIB)
	    set_ota_mcu_status(OTA_FILEINFO_GET);//END SDK LIB
    #elif (__SDK_TYPE__== QLY_SDK_BIN)
        if(owner == OTA_OWNER_MCU)
        {
            set_ota_mcu_status(OTA_FILEINFO_GET);
        }
        else
        {
            set_ota_wifi_status(OTA_FILEINFO_GET);
        }
    #endif//END SDK BIN
    
    if(owner == OTA_OWNER_MCU)
        iot_ota_info_cb( name->valuestring, g_ota.otafileinfo.version[owner], file_len, g_ota.otafileinfo.fcrc);
#if (__SDK_TYPE__== QLY_SDK_BIN)
    else if(owner == OTA_OWNER_WIFI)
        iot_ota_wifi_info_cb( name->valuestring, g_ota.otafileinfo.version[owner], file_len, g_ota.otafileinfo.fcrc);
#endif

#ifdef LOCAL_SAVE
    int cfg_len = 0;
    local_config_t *p = NULL;
    if(g_client_handle.ota_type == OTA_TYPE_BREAKPOINT && ql_strlen(g_local_config.cfg_brk_ota_info.version[owner]) != 0 && g_local_config.cfg_brk_ota_info.fcrc != 0 &&
       strcmp(g_local_config.cfg_brk_ota_info.version[owner],g_ota.otafileinfo.version[owner]) == 0 && g_local_config.cfg_brk_ota_info.fcrc == g_ota.otafileinfo.fcrc)
    {
        /*break point*/
        if(g_local_config.cfg_brk_cur_offset == g_ota.otafileinfo.flen && g_local_config.cfg_brk_cur_crc == g_ota.otafileinfo.fcrc)
        {
            ret = 2;            
        }
        else
        {
            g_ota.exptect_offset = g_local_config.cfg_brk_cur_offset;
            g_ota.cur_crc = g_local_config.cfg_brk_cur_crc;
            g_ota.cur_len = g_local_config.cfg_brk_cur_offset;        
            ql_log_info("break point [%d-%d]\r\n", g_ota.cur_len, g_ota.cur_crc);
        }
    }
    else
    {     
        /*not support breakpoint*/
        if(g_client_handle.ota_type == OTA_TYPE_BASIC)
        {        
            ql_memset(&g_local_config.cfg_brk_ota_info, 0x00, sizeof(binfile_info));
            g_local_config.cfg_brk_cur_offset = 0;
            g_local_config.cfg_brk_cur_crc = 0;
        }
        else/*no break point or ota modify*/
        {
            g_local_config.cfg_brk_ota_info.fcrc  =  g_ota.otafileinfo.fcrc;
            g_local_config.cfg_brk_ota_info.flen  =  g_ota.otafileinfo.flen;
            g_local_config.cfg_brk_ota_info.owner =  g_ota.otafileinfo.owner;
            ql_memcpy(g_local_config.cfg_brk_ota_info.version, g_ota.otafileinfo.version, sizeof(g_local_config.cfg_brk_ota_info.version));
            g_local_config.cfg_brk_cur_offset = 0;    
            g_local_config.cfg_brk_cur_crc = 0;
        }

        if(ql_local_break_ota_info_set( LOCAL_BREAK_OTA_SET_T_ALL ) == -1)
            ret = -1;
        //printf("%s [local] ver: %s ,crc: %d, offset: %d\r\n", __func__, g_local_config.cfg_brk_ota_info.version[g_local_config.cfg_brk_ota_info.owner], g_local_config.cfg_brk_ota_info.fcrc, g_local_config.cfg_brk_cur_offset);
    }
#endif

    cJSON_Delete(root);
fatal_err:
    binfile_info_response(pkt_seq, ret, owner);
}
int _check_binfile_chunk(type_u8  isend, type_u32 offset, type_u32 datalen, type_u8*  data)
{
	unsigned short precrc = 0;
	int len = 0;

	if (data == NULL || datalen <= 0) {
		return -1;
	}

	if (g_ota.exptect_offset != offset) {
		return -1;
	}

	len = g_ota.cur_len + datalen;
	if (len > g_ota.otafileinfo.flen) {
		return -1;
	}

	if (offset == 0) {
		precrc = 0xFFFF;
		g_ota.cur_crc = crc16(data, datalen, precrc);
	} else {
		g_ota.cur_crc = crc16(data, datalen, g_ota.cur_crc);
	}

	/* last chunk packet, check the length and crc. */
	if (isend) {
		if (len != g_ota.otafileinfo.flen) {
			return -1;
		}

		if (g_ota.cur_crc != g_ota.otafileinfo.fcrc) {
			return -1;
		}
	}

	g_ota.cur_len = len;
	g_ota.exptect_offset += datalen;

	return 0;
}

void qlcloud_process_otafilechunk(const char *in, int inlen, type_u32 pkt_seq)
{
	type_s8      owner = 0, is_end = 0;
	type_u32    offset = 0, data_len = 0;
	type_u8*    data = NULL;
	int ret = -1;
    static int chunk_num = 0;
    static type_u32 ota_start = 0;
	if (inlen <= 6) {        
		goto fatal_err;
	}

	if (in[0] != OTA_OWNER_WIFI && in[0] != OTA_OWNER_MCU) {
		goto fatal_err;
	}

	owner    = in[0];
	is_end   = in[1];
	offset   = STREAM_TO_UINT32_f((type_u8*)in, 2);
	data_len = inlen - 6;
	data     = (type_u8*)(in + 6);

    if ( offset > 0 && offset <= g_ota_chunk_retran_offset )
    {
        ql_log_warn("ota chunk overlap!");
        return ;
    }

	if (owner == OTA_OWNER_WIFI) {
        #if (__SDK_TYPE__== QLY_SDK_LIB)

        ret = -1;
		goto fatal_err;

        #elif (__SDK_TYPE__== QLY_SDK_BIN)
        if( offset == 0 )
        {
            if(g_client_handle.ota_wifi_status == OTA_FILEINFO_BACK)
            {
                #ifdef LOCAL_SAVE
                ql_local_ota_ver_set(owner, DEVICE_VERSION, OTA_FILEINFO_BACK);
                #endif
            }
            else
            {
                ret = -2;
                goto fatal_err;
            }
        }//ota_chunk_offset != 0
        else if(g_client_handle.ota_wifi_status == OTA_FILECHUNK_BEGIN_BACK || g_client_handle.ota_wifi_status == OTA_FILECHUNK_STREAM_BACK)
        {
        }
        else
        {
            ret = -2;
            goto fatal_err;
        }

        ret = qlcloud_ota_wifi_chunk_cb(is_end, offset, data_len, data);
        if (ret == 0)
        {
            ret = _check_binfile_chunk(is_end, offset, data_len, data);
            if (ret == 0)
            {
				if (offset == 0 && is_end == 0)
                {
					set_ota_wifi_status(OTA_FILECHUNK_BEGIN);
				}
                else if (is_end == 0)
			    {
					set_ota_wifi_status(OTA_FILECHUNK_STREAM);
				}
                else
                {
					set_ota_wifi_status(OTA_FILECHUNK_END);
				}
			}
            else
            {
                ql_log_err(ERR_EVENT_RX_DATA, "wifi:chunk err!");
                set_ota_wifi_status(OTA_INIT);
                goto fatal_err;
            }
        }
        else
        {
            ret = -1;
            goto fatal_err;
        }
        #endif//end QLY_SDK_BIN
	}
    else //(owner == OTA_OWNER_MCU)
    {
        #ifdef LOCAL_SAVE
        if( offset == 0 ||(ota_start == 0 && offset == g_local_config.cfg_brk_cur_offset) )
        #else
        if( offset == 0 )
        #endif
        {
            if(g_client_handle.ota_mcu_status == OTA_FILEINFO_BACK)
            {
                #ifdef LOCAL_SAVE
                ql_local_ota_ver_set(owner, g_client_handle.mcu_version, OTA_FILEINFO_BACK);
                #endif
            }
            else
            {
                ret = -2;
                goto fatal_err;
            }
            ota_start = 1;
        }//ota_chunk_offset != 0
        else if(g_client_handle.ota_mcu_status == OTA_FILECHUNK_BEGIN_BACK || g_client_handle.ota_mcu_status == OTA_FILECHUNK_STREAM_BACK)
        {
        }
        else
        {
            ret = -2;
            goto fatal_err;
        }

        ret = iot_chunk_cb(is_end, offset, data_len, (const type_s8*)data);
        
        if(ret == 0)
        {
            ret = _check_binfile_chunk(is_end, offset, data_len, data);
            
            if (ret == 0)
            {
				if (offset == 0 && is_end == 0)
                {
					set_ota_mcu_status(OTA_FILECHUNK_BEGIN);                    
                    chunk_num++;
				}
                else if (is_end == 0)
			    {
					set_ota_mcu_status(OTA_FILECHUNK_STREAM);                    
                    chunk_num++;
				}
                else
                {
					set_ota_mcu_status(OTA_FILECHUNK_END);                    
                    chunk_num = 0;
				}
			}
            else
            {
                ql_log_err(ERR_EVENT_RX_DATA, "mcu:chunk fail!");
                set_ota_mcu_status(OTA_INIT);
                goto fatal_err;
            }
            #if (__SDK_TYPE__== QLY_SDK_BIN)
                bin_mcu_chunk_offset = offset;
                bin_mcu_chunk_seq    = pkt_seq;
                #ifndef DEBUG_WIFI_OTA_OPEN
                    //bin version, send to uart without response to cloud
                    return;
                #endif
            #endif//END SDK BIN

#ifdef LOCAL_SAVE
            /*每20个包存一次offset ; 最后一个包清除断点续传相关信息*/
            if( g_client_handle.ota_type == OTA_TYPE_BREAKPOINT && (chunk_num % 20 == 0 || is_end))
            {                
                int cfg_len = 0;
                local_config_t *p = NULL;

                if(is_end)
                    g_local_config.cfg_brk_cur_offset = offset + data_len;
                else
                    g_local_config.cfg_brk_cur_offset = offset + g_client_handle.ota_mcu_chunk_size;

                g_local_config.cfg_brk_cur_crc = g_ota.cur_crc;
                if(ql_local_break_ota_info_set( LOCAL_BREAK_OTA_SET_T_OFFSET ) == -1)
                    ret = -1; 
            }
#endif
        }
        else
        {
            //the value user return err
            ret = -1;
            goto fatal_err;
        }
	}
    g_ota_chunk_retran_time = 0;
    g_ota_chunk_retran_seq = pkt_seq;
    g_ota_chunk_retran_offset = offset;
fatal_err:
    binfile_chunk_response(pkt_seq, ret, offset, owner);
}

void qlcloud_process_otacmd(const char *in, int inlen, type_u32 pkt_seq)
{
    type_u8 i_owner = 0;
    type_s32 ret = 0;

    cJSON *root = cJSON_Parse((const char *)in);
    if (root == NULL) {
        ret = -1;
        goto fatal_err;
    }

    cJSON *owner = cJSON_GetObjectItem(root, "owner");
    if (owner == NULL) {
        cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
    }

    i_owner = (type_u8)(owner->valueint);
    if( i_owner != OTA_OWNER_MCU && i_owner != OTA_OWNER_WIFI )
    {
        ql_log_err(ERR_EVENT_RX_DATA, "owner:%d", i_owner);        
        cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
    }

    set_ota_cur_owner(i_owner);
    ql_log_info("recv %s upgrade cmd\r\n", const_str_ota_owner[i_owner]);

    if(OTA_OWNER_WIFI == i_owner) {
        #if (__SDK_TYPE__== QLY_SDK_LIB)
            set_ota_mcu_status(OTA_INIT);
            ret = -1;
        #elif (__SDK_TYPE__== QLY_SDK_BIN)
            if(g_client_handle.ota_wifi_status == OTA_FILECHUNK_END_BACK) {
                set_ota_wifi_status(OTA_UPGRADE_CMD_GET);
                ret = 0;
            } else {
                ret = -2;
            }
        #endif//END SDK BIN
    } else {
        if(g_client_handle.ota_mcu_status == OTA_FILECHUNK_END_BACK) {
            set_ota_mcu_status(OTA_UPGRADE_CMD_GET);
            ret = 0;
        } else {
            ret = -2;
            ql_log_err(ERR_EVENT_RX_DATA,"fw not download");
        }
    }

    cJSON_Delete(root);
fatal_err:
    binfile_upgrade_response(pkt_seq, ret);
}

void qlcloud_process_ota_appdown(const char *in, int inlen, type_u32 pkt_seq)
{
    int i_owner = 0, ret = 0;

    cJSON *root = cJSON_Parse((const char *)in);
    if (root == NULL) {
        ret = -1;
        goto fatal_err;
    }

    cJSON *owner = cJSON_GetObjectItem(root, "owner");
    if (owner == NULL) {
        cJSON_Delete(root);
        ret = -1;
        goto fatal_err;
    }

    i_owner = (int)(owner->valueint);

    if(OTA_OWNER_WIFI == i_owner) 
    {
        #if (__SDK_TYPE__== QLY_SDK_BIN)
            if (g_client_handle.ota_wifi_status >= OTA_UPGRADE_CMD_GET && g_client_handle.ota_wifi_status <= OTA_UPGRADE_CMD_BACK) 
            {
                ql_log_err(ERR_EVENT_RX_DATA, "wifi:%s wait rb", const_str_ota_owner[0]);
                ret = -2;
            } 
            else 
            {
                g_client_handle.ota_wifi_type = OTA_CHECK_TYPE_NEED_APPDOWN;
                set_ota_wifi_status(OTA_INIT);
                ret = 0;
            }
        #elif (__SDK_TYPE__== QLY_SDK_LIB)
            ql_log_err(ERR_EVENT_RX_DATA, "pls select mcu fw.");
            set_ota_mcu_status(OTA_INIT);
            ret = -1;
        #endif
    } 
    else if(OTA_OWNER_MCU == i_owner) 
    {
        if (g_client_handle.ota_mcu_status >= OTA_UPGRADE_CMD_GET &&  g_client_handle.ota_mcu_status <= OTA_UPGRADE_CMD_BACK) 
        {
            ql_log_err(ERR_EVENT_RX_DATA, "mcu:%s wait rb", const_str_ota_owner[1]);
            ret = -2;
        } 
        else 
        {
            g_client_handle.ota_mcu_type = OTA_CHECK_TYPE_NEED_APPDOWN;
            set_ota_mcu_status(OTA_INIT);
            ret = 0;
        }
    } 
    else 
    {
        ql_log_err(ERR_EVENT_RX_DATA, "owner err:%d", i_owner);
        set_ota_mcu_status(OTA_INIT);
        #if (__SDK_TYPE__== QLY_SDK_BIN)
        set_ota_wifi_status(OTA_INIT);
        #endif
        ret = -1;
    }

    cJSON_Delete(root);
fatal_err:
    appdown_start_response(pkt_seq, ret);
}

//{"t":0,"v":{"i":"用户id""e":"邮箱""c":"手机号的国家代码""p":"手机号""n":"用户昵称"}}
//{"t":1,"v":[{"i":"用户id","e":"邮箱","c":"手机号的国家代码","p":"手机号","n":"用户昵称"},{"i":"用户id","e":"邮箱","c":"手机号的国家代码","p":"手机号","n":"用户昵称"}]}
//{"t":2,"v":{"b":1}}
void qlcloud_process_info(const char* data, type_size_t len, int seq)
{
    type_u8 arr_cnt = 0, i = 0;
    cJSON *root = NULL, *t_item = NULL, *v_item = NULL, *arr_item = NULL, *sub_item = NULL;

    user_info_t * master_info = NULL;
    share_info_t * share_info = NULL;
    dev_info_t * dev_info = NULL;

	root = cJSON_Parse(data);
	if (root == NULL)
		return ;
	t_item = cJSON_GetObjectItem(root, "t");
	if (t_item == NULL)
		goto out;
    v_item = cJSON_GetObjectItem(root, "v");
    if (v_item == NULL)
		goto out;

    switch(t_item->valueint)
    {
        case INFO_TYPE_USER_MASTER:
            if(v_item->type == cJSON_NULL)
            {
                ql_log_warn("no bound user!\r\n");
                iot_info_cb(t_item->valueint, (void *)master_info);
                goto out;
            }

            master_info = (user_info_t *)ql_malloc(sizeof(user_info_t));
            sub_item = cJSON_GetObjectItem(v_item, "i");
            if (sub_item == NULL)
                goto out;
            master_info->id = sub_item->valuestring;
            sub_item = cJSON_GetObjectItem(v_item, "e");
            if (sub_item == NULL)
                goto out;
            master_info->email = sub_item->valuestring;
            sub_item = cJSON_GetObjectItem(v_item, "c");
            if (sub_item == NULL)
                goto out;
            master_info->country_code = sub_item->valuestring;
            sub_item = cJSON_GetObjectItem(v_item, "p");
            if (sub_item == NULL)
                goto out;
            master_info->phone = sub_item->valuestring;
            sub_item = cJSON_GetObjectItem(v_item, "n");
            if (sub_item == NULL)
                goto out;
            master_info->name = sub_item->valuestring;
            iot_info_cb(t_item->valueint, (void *)master_info);
            break;
        case INFO_TYPE_USER_SHARE:
            if(v_item->type == cJSON_NULL)
            {
                ql_log_warn("no share user!\r\n");
                iot_info_cb(t_item->valueint, (void *)master_info);
                goto out;
            }

            arr_cnt = cJSON_GetArraySize(v_item);
            share_info = (share_info_t *)ql_malloc(sizeof(share_info_t));
            share_info->count = arr_cnt;
            share_info->user = ql_malloc(arr_cnt * sizeof(user_info_t));
            for(i = 0; i < arr_cnt; i++)
            {
                arr_item = cJSON_GetArrayItem(v_item, i);
                sub_item = cJSON_GetObjectItem(arr_item, "i");
                if (sub_item == NULL)
                    goto out;
                share_info->user[i].id = sub_item->valuestring;
                sub_item = cJSON_GetObjectItem(arr_item, "e");
                if (sub_item == NULL)
                    goto out;
                share_info->user[i].email = sub_item->valuestring;
                sub_item = cJSON_GetObjectItem(arr_item, "c");
                if (sub_item == NULL)
                    goto out;
                share_info->user[i].country_code = sub_item->valuestring;
                sub_item = cJSON_GetObjectItem(arr_item, "p");
                if (sub_item == NULL)
                    goto out;
                share_info->user[i].phone = sub_item->valuestring;
                sub_item = cJSON_GetObjectItem(arr_item, "n");
                if (sub_item == NULL)
                    goto out;
                share_info->user[i].name = sub_item->valuestring;
            }
            iot_info_cb(t_item->valueint, (void *)share_info);
            break;
        case INFO_TYPE_DEVICE:
            dev_info = (dev_info_t *)ql_malloc(sizeof(dev_info_t));
            sub_item = cJSON_GetObjectItem(v_item, "b");
            if (sub_item == NULL)
                goto out;
            dev_info->is_bind = sub_item->valueint;
            ql_memcpy(dev_info->sdk_ver, DEVICE_VERSION, 6);
            //printf("dev_info->is_bind=%d,dev_info->sdk_ver = %s\n",dev_info->is_bind,dev_info->sdk_ver);
            iot_info_cb(t_item->valueint, (void *)dev_info);
            break;
        default:
            goto out;
    }

out:
	cJSON_Delete(root);
    if(master_info != NULL) ql_free(master_info);
    if(share_info != NULL) {
        if(share_info->user != NULL)
            ql_free(share_info->user);
        ql_free(share_info);
    }
    if(dev_info != NULL) ql_free(dev_info);
}


void qlcloud_process_bindinfo(type_u16 optype, const char* data)
{
    cJSON *root = cJSON_Parse(data);
    if (root == NULL)
        return ;
    
    cJSON *data_value = cJSON_GetObjectItem(root, const_str_res);
    if (data_value == NULL)
        goto out;
    
    cJSON *errcode = cJSON_GetObjectItem(data_value, const_str_errcode);
    if (errcode == NULL)
        goto out;
    
    if(errcode->valueint == 0){
        if(PKT_ID_RCV_BIND_INFO == optype)
        {
            cloud_bind_state_set(BIND_STATE_ED, get_current_time(),0); 
        }
        else if(PKT_ID_REQ_UNB == optype)
        {
            cloud_unbound_state_set(UNB_STATE_UNB_ED, get_current_time(), 0);
        }     
        else
        {}
    }
    else
    {
        if(PKT_ID_RCV_BIND_INFO == optype)
        {
            ql_log_err(ERR_EVENT_NULL,"bind err = %d",errcode->valueint);
        }
        else if(PKT_ID_REQ_UNB == optype)
        {
            ql_log_err(ERR_EVENT_NULL,"unb err = %d",errcode->valueint);
        }     
        else
        {}
    }


out:
    cJSON_Delete(root);
}



