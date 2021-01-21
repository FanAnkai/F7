#ifndef __UART_PARSE_H__
#define __UART_PARSE_H__

#include "ninebot_protocol.h"


//mem_cmd
#define CMD_CMAP_RD			0x01 ///<查询控制表数据
#define CMD_CMAP_WR			0x02 ///<写控制表数据，需要目标节点应答
#define CMD_CMAP_WR_NR		0x03 ///<写控制表数据，无需应答
#define CMD_CMAP_ACK_RD		0x04 ///<对查询指令的应答指令
#define CMD_CMAP_ACK_WR		0x05 ///<对有应答写入指令（0x02）的应答指令

#define CMD_CMAP_GRAP_RD    0x0D    //曲线数据读取命令

//mem_pn_cmd
#define CMD_CMAP_WR_PN		0x19
#define CMD_CMAP_ACK_WR_PN	0x19



//mem_ack
#define CMD_CMAP_WRERROR_OK		0x00
#define CMD_CMAP_WRERROR_AUTH	0x01 ///<写入的地址没有写入权限
#define CMD_CMAP_WRERROR_BUSY	0x02 ///<正在操作控制表，不允许写入
#define CMD_CMAP_WRERROR_SIZE	0x03 ///<写入数据超范围
#define CMD_CMAP_WRERROR_FORM	0x04 ///<写入数据格式错误


//iap_cmd
#define CMD_IAP_BEGIN	0x07 ///<固件下载起始帧
#define CMD_IAP_TRANS	0x08 ///<固件下载数据帧
#define CMD_IAP_VERIFY	0x09 ///<固件下载校验帧
#define CMD_MCU_RESET	0x0A ///<芯片复位指令
#define CMD_IAP_ACK		0x0B ///<固件下载应答指令

//iap_err
#define IAP_OK				0x00
#define IAPERROR_SIZE		0x01 // 固件大小超范围
#define IAPERROR_ERASE		0x02 // 擦除FLASH失败
#define IAPERROR_WRITEFLASH 0x03 // 写入FLASH失败
#define IAPERROR_UNLOCK		0x04 // 车辆未处于锁车/可固件更新状态
#define IAPERROR_INDEX		0x05 // 数据索引错误
#define IAPERROR_BUSY		0x06 // IAP正忙（如正在进行FLASH写入）
#define IAPERROR_FORM		0x07 // 数据格式错误（如发的送数据长度不是8的整数倍）
#define IAPERROR_CRC		0x08 // 数据校验失败
#define IAPERROR_OTHER		0x09 // 芯片复位失败
#define IAPERROR_INDEX_SEQ	0x0A // 数据索引错误
#define	IAP_NO_ACK			0x0B // 无ACK




extern PARSE_STRUCT parse_uart_ble;
extern PARSE_STRUCT parse_uart_pc;


void uart_receive_struct_init(void);
void receive_uart_ble(uint8_t *receive_buffer,uint16_t receive_len);
void receive_uart_pc(uint8_t *receive_buffer,uint16_t receive_len);
void parse_loop_ble(void);
void parse_loop_pc(void);

int uart_send_cmd(uint8_t source_id, uint8_t target_id, uint8_t cmd, uint16_t index);
int uart_send_data(uint8_t source_id, uint8_t target_id, uint8_t cmd, uint16_t index, uint8_t * buff, uint16_t len);

void uart_send_ble(uint8_t *tx_buffer, uint16_t tx_len);
void uart_send_pc(uint8_t *tx_buffer, uint16_t tx_len);

uint32_t iap_check_bin(uint8_t * buff, uint16_t len);


#endif

