

#ifndef _APP_IAP_H_
#define _APP_IAP_H_


#include "main.h"
#include "ninebot_protocol.h"

/*
typedef enum
{
	STEP_IAP_BEGIN = 0,
	STEP_IAP_TX_CMD,
	STEP_IAP_TX_DATA,
	STEP_IAP_VERIFY,
	STEP_IAP_RESET,
	
} IAP_STEP_TYPE;
*/

#define IAP_RX_MAX      1024
#define UART_DMA_RX_MAX 1024

typedef struct
{
    uint8_t fifo_buff[IAP_RX_MAX];
	uint8_t dma_buff[UART_DMA_RX_MAX];

	uint32_t	file_size;		//bin文件大小
	uint32_t	write_pos;
	uint32_t	real_size;		//实际传输大小
	
	uint32_t	crc_file_get;	//获取到的整个文件的crc

	uint32_t	crc_file_cal;	//计算整个文件的crc
	
	uint8_t		wr_buf[IAP_RX_MAX];
	
	uint8_t		index_data;     //发送cmd时用的index
	
	uint8_t		iap_step;       //协议无法表示出当前步骤，需自己设定step标志
	uint8_t		pack_over_flag;
	uint16_t	time_out_cnt;
    
} IAP_DATA_TYPE;
extern IAP_DATA_TYPE gIapFream;



void check_iap(void);
void run_iap(void);

void uart_rx_init(void);
void uart3_rx_callback(void);
void iap_rx_data(PARSE_STRUCT * nbbus);

void iap_send_cmd(uint8_t cmd, uint8_t iap_ack);
void iap_send_data(uint8_t cmd, uint8_t index, uint8_t * buff, uint16_t len);

#endif


