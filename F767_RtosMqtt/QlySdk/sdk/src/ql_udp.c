#include "ql_logic_context.h"
#include "ql_util.h"
#include "ql_udp.h"

#define FACTOR_1                        17445
#define FACTOR_2                        32463
#define FACTOR_3                        34354
#define FACTOR_4                        23458554
#define MODE_FACTOR                     32760

#define UDP_SERVER_PORT                 30001

#define Cmd_No_QueryLocalDevice			17
#define IOTIDITEM_LEN	                16
#define MACITEM_LEN	                    6
#define TOKENITEM_LEN	                44
#define DEVICENAMEITEM_LEN	            32
#define OFFSET_W	                    5
#define MIN_MSG_SIZE                    25

#define CMD_AP_SMART                    19
#define SSID_PWD_LEN                    32
#define AP_SMART_PADING_LEN             13
extern int is_application_authed(void);

#pragma pack (push, 1)
struct tagFindDevice_back
{
	unsigned char 	    send_recieve_flag;	// send 0, recieve 1
	int     	        random;
	unsigned char 	    iotid[IOTIDITEM_LEN];
	unsigned short 	    sOrder;	 //命令序列号
	unsigned short 	    cmdid;	// 命令iD Cmd_No_QueryLocalDevice
	int			        productid;	//命令ID
	unsigned char		device_name[DEVICENAMEITEM_LEN];
	unsigned char		status;  //设备状态
	unsigned char		mac_address[MACITEM_LEN];
	unsigned char		token[TOKENITEM_LEN];
	unsigned short	    chk_sum;
};

struct tagApSmart_back//114字节
{
    unsigned char       send_recieve_flag;	// send 0, recieve 1
    int                 random;
    unsigned char       iotid[IOTIDITEM_LEN];
    unsigned short      sOrder;	 //命令序列号
    unsigned short      cmdid;	// 命令iD Cmd_No_QueryLocalDevice
    int                 productid;	//命令ID
    unsigned char       mac_address[MACITEM_LEN];//mac
    unsigned char       ssid[SSID_PWD_LEN];//wifi ssid
    unsigned char       password[SSID_PWD_LEN];  //wifi password	
    unsigned char       pad[AP_SMART_PADING_LEN];//填充
    unsigned short      chk_sum;
};

#pragma pack(pop)

static ql_udp_socket_t *udp_socket = NULL;

static unsigned short get_chksum(unsigned char * buf, unsigned short len)
{
	unsigned short chksum = 0;
	int i = 0;
	for (i = 0; i < len; i++)
		chksum += buf[i];
	return chksum;
}


/*
dir|random|iotid|seq|cmdid|productid|device_name|status|mac_addr|token|chk_sum
1B	 4B		 16B  2B  2B      4B		32			1B	   6B	   32	2B
0/1|-salt-|---------------------------encrypt---------------------------------|
*/
static void send_device_iottoken(ql_udp_socket_t* sock, unsigned short recv_sOrder)
{
	int random_token, ret = 0;
	unsigned short chk_sum = 0;
	unsigned char device_status = 0;
	struct tagFindDevice_back find_device_response_packet;
	unsigned short find_device_response_packet_len = sizeof(struct tagFindDevice_back);

	ql_memset(&find_device_response_packet, 0x00, find_device_response_packet_len);

	//flag
	find_device_response_packet.send_recieve_flag = 0x01;	 //receive;

	//random
	random_token = ((unsigned int) ql_random()) % 30000 + 1000;
	find_device_response_packet.random = ql_htonl(random_token);

	//iot id
	ql_memcpy(find_device_response_packet.iotid, g_client_handle.iotid, IOTIDITEM_LEN);
	//token
	ql_memcpy(find_device_response_packet.token, g_client_handle.iottoken, TOKENITEM_LEN);

    ql_memcpy(find_device_response_packet.mac_address, g_client_handle.dev_mac_hex, MACITEM_LEN);

	//product id
	find_device_response_packet.productid = ql_htonl(g_client_handle.product_id);

	//order
	find_device_response_packet.sOrder = ql_htons(recv_sOrder);

	//command id
	find_device_response_packet.cmdid =ql_htons(Cmd_No_QueryLocalDevice);

	// status
	find_device_response_packet.status = device_status;

	//chksum
	chk_sum = get_chksum((unsigned char*) &find_device_response_packet, find_device_response_packet_len - 2);
	chk_sum = chk_sum % MODE_FACTOR;
	find_device_response_packet.chk_sum = ql_htons(chk_sum);

	//encrypt
	int key1 = random_token % FACTOR_1 + FACTOR_1;
	encrypt_msg((unsigned char*) &find_device_response_packet, OFFSET_W, find_device_response_packet_len, key1, FACTOR_2);
	//send immedy
	ret = ql_udp_send(sock, (unsigned char*)&find_device_response_packet, sizeof(find_device_response_packet), 2000);
	if (ret < 0) {
		ql_log_err(ERR_EVENT_INTERFACE, "token send err!");
	}
}

#if (__SDK_TYPE__== QLY_SDK_BIN)
#define DISCOVER_LOCAL_DEV              18
typedef struct
{
    uint8_t send_recieve_flag;	
    uint32_t productid;
    uint8_t mac_address[6];
    uint8_t user_id[4];
    uint32_t seq;	
    uint32_t timestamp;
    //uint8_t json_data[0];
}AP_MODE_HANDER;

static void send_device_mac(ql_udp_socket_t* sock, unsigned short recv_sOrder)
{
    int random_token, ret = 0;
    unsigned short chk_sum = 0;
    unsigned char device_status = 0;
    struct tagFindDevice_back find_device_response_packet;
    unsigned short find_device_response_packet_len = sizeof(struct tagFindDevice_back);

    ql_memset(&find_device_response_packet, 0x00, find_device_response_packet_len);

    //flag
    find_device_response_packet.send_recieve_flag = 0x03;	 //receive;

    //random
    random_token = ((unsigned int) ql_random()) % 30000 + 1000;
    find_device_response_packet.random = ql_htonl(random_token);
    
    ql_memcpy(find_device_response_packet.mac_address,g_client_handle.dev_mac_hex,sizeof(g_client_handle.dev_mac_hex));
 
    //product id
    find_device_response_packet.productid = ql_htonl(g_client_handle.product_id);

    //order
    find_device_response_packet.sOrder = ql_htons(recv_sOrder);

    //command id
    find_device_response_packet.cmdid =ql_htons(DISCOVER_LOCAL_DEV);

    // status
    find_device_response_packet.status = device_status;

    //chksum
    chk_sum = get_chksum((unsigned char*) &find_device_response_packet, find_device_response_packet_len - 2);
    chk_sum = chk_sum % MODE_FACTOR;
    find_device_response_packet.chk_sum = ql_htons(chk_sum);

#if 0//MAC debug
    printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\r\n",find_device_response_packet.mac_address[0],find_device_response_packet.mac_address[1],find_device_response_packet.mac_address[2],find_device_response_packet.mac_address[3],find_device_response_packet.mac_address[4],find_device_response_packet.mac_address[5]);
#endif

    //encrypt
    int key1 = random_token % FACTOR_1 + FACTOR_1;
    encrypt_msg((unsigned char*) &find_device_response_packet, OFFSET_W, find_device_response_packet_len, key1, FACTOR_2);
    //send immedy
    ret = ql_udp_send(sock, (unsigned char*)&find_device_response_packet, sizeof(find_device_response_packet), 2000);
    if (ret < 0) {
        ql_log_err(ERR_EVENT_INTERFACE,"mac send err");
    }
}


/*AP 配网设置*/
int32_t send_ap_smart_ack(ql_udp_socket_t* sock, unsigned short recv_sOrder, wifi_info_t info)
{
    int random_token, ret = 0;
    unsigned short chk_sum = 0;
    
    struct tagApSmart_back ap_smart_response_packet;
    unsigned short ap_smart_response_packet_len = sizeof(struct tagApSmart_back);

    ql_memset(&ap_smart_response_packet, 0x00, ap_smart_response_packet_len);

    //flag
    ap_smart_response_packet.send_recieve_flag = 17;    //receive;

    //random
    random_token = ((unsigned int) ql_random()) % 30000 + 1000;
    ap_smart_response_packet.random = ql_htonl(random_token);
    
    ql_memcpy(ap_smart_response_packet.mac_address,g_client_handle.dev_mac_hex,sizeof(g_client_handle.dev_mac_hex));
 
    //product id
    ap_smart_response_packet.productid = ql_htonl(g_client_handle.product_id);

    //order
    ap_smart_response_packet.sOrder = ql_htons(recv_sOrder);

    //command id
    ap_smart_response_packet.cmdid =ql_htons(DISCOVER_LOCAL_DEV);

    // ssid
    ql_memcpy(ap_smart_response_packet.ssid, info.ssid, ql_strlen(info.ssid));
    ap_smart_response_packet.ssid[ql_strlen(info.ssid)] = '\0';
    // password
    ql_memcpy(ap_smart_response_packet.password, info.password, ql_strlen(info.password));    
    ap_smart_response_packet.password[ql_strlen(info.password)] = '\0';    
    //chksum
    chk_sum = get_chksum((unsigned char*) &ap_smart_response_packet, ap_smart_response_packet_len - 2);
    chk_sum = chk_sum % MODE_FACTOR;
    ap_smart_response_packet.chk_sum = ql_htons(chk_sum);

#if 0//MAC debug
    printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\r\n",find_device_response_packet.mac_address[0],find_device_response_packet.mac_address[1],find_device_response_packet.mac_address[2],find_device_response_packet.mac_address[3],find_device_response_packet.mac_address[4],find_device_response_packet.mac_address[5]);
#endif

    //encrypt
    int key1 = random_token % FACTOR_1 + FACTOR_1;
    encrypt_msg((unsigned char*) &ap_smart_response_packet, OFFSET_W, ap_smart_response_packet_len, key1, FACTOR_2);
    //send immedy
    ret = ql_udp_send(sock, (unsigned char*)&ap_smart_response_packet, sizeof(ap_smart_response_packet), 2000);
    if (ret < 0) {
        ql_log_err(ERR_EVENT_INTERFACE, "UDP:send err");
    }
}

extern struct iot_context iot_ctx;
/*****************************************************************************
函数名称 : udp_conn_send
功能描述 : udp报文发送函数
传出参数 : pData    :       数据内容
           data_len:    数据长度
           Seq     :    序列号
           PacketFlag:  发送数据类型
                        20：AP模式控制ack回复
                        21: UDP 设备端同步
                        
返回参数 :  0 : 成功
           -1 : 失败
*****************************************************************************/
int32_t  udp_conn_send(uint8_t* pData, uint32_t data_len, uint32_t Seq,UDP_PKT_TYPE_T PacketFlag)
{
    DCONN_HANDER* pHeader = NULL;
    uint8_t *pPacketBeforeEncrypt = NULL, *pPacketAfterEncrypt = NULL;

    uint16_t LengthBeforeEncrypt = UDP_PKT_HEARD_LEN + data_len;
    uint16_t LengthAfterEncrypt = LengthBeforeEncrypt + (16 - (LengthBeforeEncrypt - DCONN_ENCRYPT_OFFSET) % 16);;

    uint16_t Length = 0;
    int32_t ret = 0;

    /*  pPacketBeforeEncrypt  |----Header----|-------------pBuf---------------|  */
    /*  pPacketAfterEncrypt   |-5B-|--------------AES encrypt-----------------|  */
    if(NULL == pData || data_len == 0)
    {
    
        if(UDP_PKT_TYPE_AP_DOWNLINK_REQ == PacketFlag && NULL == pData && data_len == 0)
        {
            ;
        }
        else
        {
            ql_log_err(ERR_EVENT_TX_DATA, "udp_con_send err!");
            return -1;
        }
    }
    /*create befor encrypt data*/
    pPacketBeforeEncrypt = (uint8_t*)ql_malloc(LengthBeforeEncrypt);
    if(NULL == pPacketBeforeEncrypt)
    {
        return -1;
    }
	ql_memset(pPacketBeforeEncrypt,0x00,LengthBeforeEncrypt);
	

    pPacketAfterEncrypt = (uint8_t*)ql_malloc(LengthAfterEncrypt);
    if(NULL == pPacketAfterEncrypt)
    {
        ret = -1;
        goto out;
    }
	ql_memset(pPacketAfterEncrypt,0x00,LengthAfterEncrypt);
    /* packet before encrypt */
    switch (PacketFlag)
    {
    case UDP_PKT_TYPE_AP_UPLINK_REQ:
        pPacketBeforeEncrypt[0] = UDP_PKT_TYPE_AP_UPLINK_REQ;
        break;
    case UDP_PKT_TYPE_AP_DOWNLINK_REQ:
        pPacketBeforeEncrypt[0] = UDP_PKT_TYPE_AP_DOWNLINK_RCV;
        break;
    default:
        break;
        
    }

    /*product id*/
    UINT32_TO_STREAM_f(pPacketBeforeEncrypt + 1,g_client_handle.product_id);
    /*mac*/
    qlcloud_get_mac_arr(pPacketBeforeEncrypt + 5);
    /*Seq*/
    UINT32_TO_STREAM_f(pPacketBeforeEncrypt + 15,Seq);
    /*timestamp*/
    UINT32_TO_STREAM_f(pPacketBeforeEncrypt + 19,get_current_time());
    /*data_len*/
    UINT32_TO_STREAM_f(pPacketBeforeEncrypt + 23,data_len);

    /*data*/
    if(UDP_PKT_TYPE_AP_UPLINK_REQ == PacketFlag)
    {
        ql_memcpy(pPacketBeforeEncrypt + UDP_PKT_HEARD_LEN, pData, data_len);
    }

    #if 0/*debug*/
    int i = 0;
    printf("udp send before encry[%d]:\r\n", UDP_PKT_HEARD_LEN + data_len);
    for(i = 0; i < UDP_PKT_HEARD_LEN + data_len; i++)
    {
        printf("%02x ", pPacketBeforeEncrypt[i]);
    }
    printf("\r\n");

    
    printf("udp send key:\r\n");
    for(i = 0; i < 16; i++)
    {
        printf("%02x ", iot_ctx.product_key[i]);
    }
    printf("\r\n");

    #endif

    //packet_dump("udp sendpkt src:", pPacketBeforeEncrypt, LengthBeforeEncrypt);

    /* packet after encrypt */    
    ql_memcpy(pPacketAfterEncrypt, pPacketBeforeEncrypt, DCONN_ENCRYPT_OFFSET);
    Length = LengthAfterEncrypt - DCONN_ENCRYPT_OFFSET ;

    /**************************************/
    ret = iot_aes_cbc128_encrypt(pPacketBeforeEncrypt + DCONN_ENCRYPT_OFFSET, LengthBeforeEncrypt - DCONN_ENCRYPT_OFFSET, 
                                        pPacketAfterEncrypt + DCONN_ENCRYPT_OFFSET, (unsigned int*)&Length,  (unsigned char *)iot_ctx.product_key);
    
    if(-1 == ret)
    {
        ql_log_err(ERR_EVENT_INTERFACE, "udp send encry err");
        goto out;
    }
    
    //packet_dump("udp conn send pkt", pPacketAfterEncrypt, LengthAfterEncrypt);
    ret = ql_udp_send(udp_socket, (unsigned char*)pPacketAfterEncrypt, (DCONN_ENCRYPT_OFFSET+Length), 100);
    if (ret < 0) {
        ql_log_err(ERR_EVENT_INTERFACE, "UDP: send error");
    }

 	ql_log_info("[UDP SEND] : Seq= %d , PacketFlag = %d ,SendFlag = %d.\r\n", Seq, PacketFlag,pPacketBeforeEncrypt[0]);

#if 0/*debug*/
    printf("udp send after encry[%d]:\r\n", DCONN_ENCRYPT_OFFSET+Length);
    for(i = 0; i < DCONN_ENCRYPT_OFFSET+Length; i++)
    {
        printf("%02x ", pPacketAfterEncrypt[i]);
    }
    printf("\r\n");
#endif

out:
    if(pPacketBeforeEncrypt)
    {   
        ql_free(pPacketBeforeEncrypt); 
    	pPacketBeforeEncrypt = NULL;
    }
    if(pPacketAfterEncrypt)
    {
        ql_free(pPacketAfterEncrypt); 
	    pPacketAfterEncrypt = NULL;
    }

    if(ret < 0)
        return ret;
    else
        return 0;
}

/*****************************************************************************
函数名称 : udp_conn_recv
功能描述 : udp报文接收函数
传出参数 : pData    :       数据内容
           data_len:    数据长度
           Packetflag:  发送数据类型
                        0x13: udp app端控制报文
                        0x16: udp 设备端同步回复报文
返回参数 :     0 : 成功
          -1 : 失败
*****************************************************************************/
int32_t udp_conn_recv(uint8_t* pData, uint32_t data_len)
{
    uint8_t *pPacketAfterDecrypt = NULL;
    uint32_t LengthAfterDecrypt = 0;
    uint32_t ret = 0;
        
	int i=0;
	uint8_t local_mac[6];
    qlcloud_get_mac_arr(local_mac);
    //printf("mac : %02x:%02x:%02x:%02x:%02x:%02x \r\n", local_mac[0], local_mac[1], local_mac[2], local_mac[3], local_mac[4], local_mac[5]);
    /*  pBuf                      |----Header----|-------------Data---------------|  */
    /*  pPacketAfterDecrypt       |-5B-|--------------AES decrypt-----------------|  */

    DCONN_HANDER Header;
    if(NULL == pData || data_len < 0 || data_len > DCONN_UDP_BUF_LEN)
    {
        ret = -1;
        goto out;
    }

    //packet_dump("udp_conn_recv src", pData, data_len);
    Header.send_recieve_flag = pData[0];
    Header.productid = STREAM_TO_UINT32_f(pData, 1);

    if(Header.productid != g_client_handle.product_id)
    {
        ql_log_err(ERR_EVENT_LOCAL_DATA, "product_id err.");
        ret = -1;
        goto out;
    }
    
    pPacketAfterDecrypt = (uint8_t*)ql_malloc(data_len);
    if(NULL == pPacketAfterDecrypt)
    {
        ret = -1;
        goto out;
    }
	ql_memset(pPacketAfterDecrypt,0x00,data_len);

    /* packet after decrypt */
    LengthAfterDecrypt = data_len - DCONN_ENCRYPT_OFFSET;  
    ret = iot_aes_cbc128_decrypt(pData + DCONN_ENCRYPT_OFFSET, data_len - DCONN_ENCRYPT_OFFSET,
                             pPacketAfterDecrypt + DCONN_ENCRYPT_OFFSET, (type_size_t *)&LengthAfterDecrypt,
                             (unsigned char*)iot_ctx.product_key);

    if(-1 == ret)
    {
        ql_log_err(ERR_EVENT_INTERFACE, "decrypt err.");
        goto out;
    }
    ql_memcpy(pPacketAfterDecrypt, pData, DCONN_ENCRYPT_OFFSET);
    LengthAfterDecrypt += DCONN_ENCRYPT_OFFSET;
    pPacketAfterDecrypt[LengthAfterDecrypt] = '\0';

    /* process data */
    for(i = 0 ;i < 6; i++)
    {   
        if( pPacketAfterDecrypt[DCONN_ENCRYPT_OFFSET + i] != local_mac[i] )
        {
			/*this packet not send to me*/
			ql_log_warn("\r\nthis packet not send to me\r\n");      
            ret  = 0;
            goto out;
        }
        Header.mac_address[i] = pPacketAfterDecrypt[DCONN_ENCRYPT_OFFSET + i];
    }
    Header.seq      = STREAM_TO_UINT32_f(pPacketAfterDecrypt, 15);
    Header.timestamp= STREAM_TO_UINT32_f(pPacketAfterDecrypt, 19);
    /*update time*/
    set_current_time(Header.timestamp); 
    Header.data_len = STREAM_TO_UINT32_f(pPacketAfterDecrypt, 23);

    if(UDP_PKT_TYPE_AP_DOWNLINK_REQ == Header.send_recieve_flag)
    {   
        if(0x06 == module_status_get())
        {
            /*回复控制ack*/
            udp_conn_send(NULL, 0, Header.seq,Header.send_recieve_flag);
        }
        else   
        {
            ql_log_err(ERR_EVENT_LOCAL_DATA, "NO STATION JOIN AP");
            return -1;
        }
        
        ql_udp_process_rx_data_v2(pPacketAfterDecrypt + (UDP_PKT_HEARD_LEN - 4),  LengthAfterDecrypt - (UDP_PKT_HEARD_LEN - 4), Header.timestamp, Header.seq);
    }
    else if(UDP_PKT_TYPE_AP_UPLINK_RCV == Header.send_recieve_flag)
    {
        //DATA_LIST_T data_list = {0, NULL};
        //iot_data_cb(Header.seq, &data_list);
    }
    ql_log_info("[UDP RECV] :PacketFlag = %d, Seq= %d.\r\n", Header.send_recieve_flag, Header.seq);
out:
    if(pPacketAfterDecrypt)
    {    
        ql_free(pPacketAfterDecrypt);
    	pPacketAfterDecrypt = NULL;
    }
    
    return ret;
}

/*局域网报文接收函数*/
void  ql_udp_parse_data(ql_udp_socket_t* sock, unsigned char *pData, uint32_t data_len)
{
    uint8_t PacketFlag;
    int32_t ret;

    if(NULL == sock)
    {
        ql_log_err(ERR_EVENT_NULL, "arg err.\r\n");
        return;
    }

    if(NULL == pData)
    {
        ql_log_err(ERR_EVENT_NULL, "pusrdata err.\r\n");
        return;
    }

    //packet_dump("udp recv raw", pData, data_len);
    PacketFlag = *(uint8_t*)pData;

    if(UDP_PKT_TYPE_AP_DOWNLINK_REQ == PacketFlag || UDP_PKT_TYPE_AP_UPLINK_RCV == PacketFlag)
    {
        ret = udp_conn_recv(pData, data_len);
    }
    
}
#endif


static void platform_data_parser(ql_udp_socket_t* sock, unsigned char* data, int data_len)
{
	unsigned short chk_sum = 0;
	unsigned short scmdid;
    type_s32 productid = 0;
    unsigned short order;

	if (sock == NULL) {
		return;
	}

	if (data_len < MIN_MSG_SIZE) {
 		return;
	}

#if (__SDK_TYPE__== QLY_SDK_BIN)
    if(UDP_PKT_TYPE_AP_DOWNLINK_REQ !=  (type_u8)data[0] && UDP_PKT_TYPE_AP_UPLINK_RCV !=  (type_u8)data[0])
#endif
    {
    	//前5个字节没有加密
    	decrypt_msg(data, OFFSET_W, data_len, ((int) STREAM_TO_UINT32_f(data, 1)) % FACTOR_1 + FACTOR_1, FACTOR_2);
    	chk_sum = get_chksum(data, data_len - sizeof(chk_sum));
    	if (STREAM_TO_UINT16_f(data, data_len - sizeof(chk_sum)) != (chk_sum % MODE_FACTOR)) {
    		ql_log_err(ERR_EVENT_RX_DATA,"checksum err");
    		return;
    	}
    }

	switch (data[0]) {
	case 0:   
#if (__SDK_TYPE__== QLY_SDK_BIN)
    case 2:/*AP 配网的发现*/
#endif
	case 10:
		productid = STREAM_TO_UINT32_f(data, 25);
        ql_log_info("udp pkt[%d], id:%d\r\n", data_len, productid);

		scmdid = STREAM_TO_UINT16_f(data, 23);
		order = STREAM_TO_UINT16_f(data, 21);        
		if (scmdid == Cmd_No_QueryLocalDevice) {
			send_device_iottoken(sock, order);
		}
#if (__SDK_TYPE__== QLY_SDK_BIN)        
        else if(scmdid == DISCOVER_LOCAL_DEV){
            send_device_mac(sock,order);
        }
#endif
		break;
#if (__SDK_TYPE__== QLY_SDK_BIN)   
    case UDP_PKT_TYPE_AP_SMART:/*AP配网*/
        if(0x01 == get_ap_smart_status())
        {        
            scmdid = STREAM_TO_UINT16_f(data, 23);
            order = STREAM_TO_UINT16_f(data, 21);        
            if(scmdid == CMD_AP_SMART)
            {          
                wifi_info_t info;    
                int ret = 0;
                int i = 0;
               /*1.get wifi info*/
                ql_memset(&info, 0x00, sizeof(info));
                ql_memcpy(info.ssid, data + 39 , SSID_PWD_LEN);
                ql_memcpy(info.password, data + 71, SSID_PWD_LEN);
                ql_log_info("[ap_smart]:ssid:%s,password:%s \r\n", info.ssid, info.password);

                /*2.send ack to app*/
                i = 3;
                while(i--)
                {           
                    ql_log_info("send_ap_smart_ack\r\n");
                    send_ap_smart_ack( sock, order, info);
                    ql_sleep_s(1);
                }

                /*3.set wifi*/
                if( (ret = ql_set_wifi_info(STATION_MODE,&info)) < 0)
                {          
                    ql_log_err(ERR_EVENT_INTERFACE, "set wifi err");            
                    return ;
                }   
                /*4.ap smart success*/
                 ap_smart_cb(); 
            }
        }
        else
        {
            ql_log_warn("The mode not in ap\r\n");
        }
        break;
    case UDP_PKT_TYPE_AP_DOWNLINK_REQ:  /*AP透传下发数据*/
    case UDP_PKT_TYPE_AP_UPLINK_RCV:    /*AP透传上报数据*/
    #if (__SDK_TYPE__== QLY_SDK_BIN)
        if(SOFTAP_MODE == ql_get_wifi_opmode())
        {       
            ql_udp_parse_data(sock,data,data_len);
        }
    #endif
        break;
#endif
	default:
		break;
	}
}

#define UDP_CONTROL_CLOSE 0
#define UDP_CONTROL_OPEN 1
static type_u8 g_udp_control_flag = 0;
void udp_control(type_u8 control_type)
{
	g_udp_control_flag = control_type;
}

unsigned char g_udp_buf[QL_UDP_FRAME_MAX_SIZE];
void ql_udp_server_task(void)
{
	int len;
    unsigned char *buf = g_udp_buf;
	static int udp_task_first = 0;

	if(g_udp_control_flag == UDP_CONTROL_OPEN)
	{
		if(udp_task_first == 0)
		{
			udp_socket = ql_udp_socket_create(UDP_SERVER_PORT);
			if (udp_socket == NULL) {
				return;
			}

			if(ql_udp_bind(udp_socket)){
				ql_log_err(ERR_EVENT_INTERFACE, "udp bind err.");
				return;
			}
			udp_task_first = 1;
		}

		len = ql_udp_recv(udp_socket, buf, QL_UDP_FRAME_MAX_SIZE, 2000);

		if (len < 0) {
			ql_log_err(ERR_EVENT_INTERFACE, "udp recv err:%d", len);
		} else {
			#if (__SDK_TYPE__== QLY_SDK_BIN)        
			if ((is_application_authed() ||  get_ap_smart_status()) && len) 
			#else
			if (is_application_authed() && len)
			#endif
			{
				buf[len] = 0;
				ql_log_info("ql_udp_recv len:%d, buf:%s\r\n", len, buf);
				platform_data_parser(udp_socket, buf, len);
			}
		}
	}
	else
	{
		if(udp_task_first == 1)
		{
			ql_udp_close(udp_socket);
			ql_udp_socket_destroy(&udp_socket);
            udp_socket = NULL;
			udp_task_first = 0;
		}
	}
	
}

void *qlcloud_udp_server(void *para)
{
    for(;;)
	{
		ql_udp_server_task();
		ql_sleep_s(1);
	}
}
