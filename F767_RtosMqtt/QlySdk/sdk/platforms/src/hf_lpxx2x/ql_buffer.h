#ifndef _QL_BUFFER_H_
#define _QL_BUFFER_H_

#include "ql_include.h"

#define PRINT_INFO  printf
#define PRINT_ERR  printf

#define RAW_BUF_SIZE 8192

// #define RAW_BUFFER
#define TCPIP_BUFFER
#ifdef TCPIP_BUFFER
#define TCP_BUF_USE
#define UDP_BUF_USE
#endif

typedef enum
{
    BUF_ERR = -1,
    BUF_OK,
}BUF_ERRNO;

typedef struct
{
    uint8_t buf[RAW_BUF_SIZE];
    uint32_t in;
    uint32_t out;
    volatile uint32_t len;
}RAWBUF_S;

#ifdef TCPIP_BUFFER
typedef enum
{
#ifdef TCP_BUF_USE
    DATA_TCP,
#endif
#ifdef UDP_BUF_USE
    DATA_UDP,
#endif
    DATA_TYPE_NUM
}DATABUF_TYPE_E;
#endif

typedef enum
{
    DATA_IOCTL_RESET,
    DATA_IOCTL_SET_DATA,
    DATA_IOCTL_GET_DATA,
    DATA_IOCTL_GET_LEN
}DATABUF_IOCTL_E;

#ifdef RAW_BUFFER
extern RAWBUF_S g_RxRawBuf;
#endif

BUF_ERRNO rawbuf_init(RAWBUF_S* pBuf);
BUF_ERRNO rawbuf_write_byte(RAWBUF_S* pBuf, uint8_t Byte);
BUF_ERRNO rawbuf_read_byte(RAWBUF_S* pBuf, uint8_t* pByte);
void rx_byte_to_rawbuf(uint8_t Byte);
int data_buf_ioctl(DATABUF_IOCTL_E ioctl, DATABUF_TYPE_E type, uint8_t* pbuf, uint32_t len);
void ql_buffer_init();

#endif

