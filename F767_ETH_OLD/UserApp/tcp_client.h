
#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H


#include "global.h"

void Tcp_Client(void);	

#define TCP_CLIENT_RX_BUFSIZE	200		//定义tcp client最大接收数据长度
#define	TCP_CLIENT_PORT			8080	//定义tcp client要连接的远端端口

//tcp服务器连接状态
enum tcp_client_states
{
    ES_TCPCLIENT_NONE = 0,		//没有连接
    ES_TCPCLIENT_CONNECTED,		//连接到服务器了
    ES_TCPCLIENT_CLOSING,		//关闭连接
};
//LWIP回调函数使用的结构体
struct tcp_client_struct
{
    uint8_t state;               //当前连接状
    struct tcp_pcb *pcb;    //指向当前的pcb
    struct pbuf *p;         //指向接收/或传输的pbuf
};
struct ip_addr
{
    uint32_t addr;
};


void tcp_client_set_remoteip(void);
void tcp_client_test(void);//TCP Client测试函数
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err);
void tcp_client_error(void *arg,err_t err);
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb);
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es);
void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct * es );


#endif

