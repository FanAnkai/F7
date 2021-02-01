#ifndef __QL_UDP_H__
#define __QL_UDP_H__

void *qlcloud_udp_server(void *para);

#define QL_UDP_FRAME_MAX_SIZE      256


typedef struct
{
    uint8_t     send_recieve_flag;	
    uint32_t    productid;
    uint8_t     mac_address[6];
    uint8_t     user_id[4];
    uint32_t    seq;
	uint32_t    timestamp;
    uint32_t    data_len;
}DCONN_HANDER;


typedef enum UDP_PKT_TYPE {
    UDP_PKT_TYPE_AP_SMART               = 16,    //AP配网
    UDP_PKT_TYPE_AP_DOWNLINK_REQ        = 19,    //直连控制请求（app）
    UDP_PKT_TYPE_AP_DOWNLINK_RCV        = 20,    //直连控制回复（dev）
    UDP_PKT_TYPE_AP_UPLINK_REQ          = 21,    //直连设备主动同步（dev）
    UDP_PKT_TYPE_AP_UPLINK_RCV          = 22,    //直连设备同步回复（app）
}UDP_PKT_TYPE_T;

#define UDP_PKT_HEARD_LEN           27
#define DCONN_ENCRYPT_OFFSET  5
#define DCONN_UDP_BUF_LEN   (1024 + 512)
extern void  ql_udp_parse_data(ql_udp_socket_t* sock, unsigned char *pData, uint32_t data_len);
extern int32_t udp_conn_recv(uint8_t* pData, uint32_t data_len);
extern int32_t  udp_conn_send(uint8_t* pData, uint32_t data_len, uint32_t Seq,UDP_PKT_TYPE_T PacketFlag);


#endif /* __CLOUD_UDP_H__ */
