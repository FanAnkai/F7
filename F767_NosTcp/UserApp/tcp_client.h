
#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H


#include "stdio.h"
#include "string.h"


#include "main.h"
#include "lwip.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"
#include "tcp.h"


#define	TCP_CLIENT_PORT			8080	//定义tcp client要连接的远端端口


#define TCP_BUFF_MAX		1024
#define UART_DMA_BUFF_MAX	512
typedef struct
{
	uint8_t		work_step;
	
	uint8_t		is_connect;	//连接标志
	uint8_t		connect_stat;	//状态判断，用于打印数据

	uint8_t 	wait_send;
	uint8_t 	wait_send_len;
	
	uint8_t		tx_buff[TCP_BUFF_MAX];
	uint8_t		rx_buff[TCP_BUFF_MAX];
	
	uint8_t		dma_buff[UART_DMA_BUFF_MAX];

	uint8_t		remot_ip[4];
	uint32_t	remot_port;
	
	uint8_t		local_ip[4];
	uint8_t		local_mask[4];
	uint8_t		local_gw[4];

	
	
}USER_TCP_TYPE;
extern USER_TCP_TYPE gTcpReg;


void task_tcp_client(void);
void send_data_poll(struct tcp_pcb* tcp_pcb);


void tcp_client_set_remoteip(void);
void tcp_client_test(void);//TCP Client测试函数
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err);
void tcp_client_error(void *arg,err_t err);
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb);
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
void tcp_client_senddata(struct tcp_pcb *tpcb, uint8_t * data, uint16_t len);
void tcp_client_connection_close(struct tcp_pcb *tpcb);


#endif

