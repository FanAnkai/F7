#include "stdio.h"
#include "string.h"

#include "ql_buffer.h"

#ifdef RAW_BUFFER
RAWBUF_S g_RxRawBuf;
#endif
#ifdef TCPIP_BUFFER
RAWBUF_S g_RxDataBuf[DATA_TYPE_NUM];
#endif

BUF_ERRNO rawbuf_init(RAWBUF_S* pBuf)
{
    if(NULL == pBuf)
    {
        return BUF_ERR;
    }

    memset(pBuf->buf, 0, RAW_BUF_SIZE);
    pBuf->in = 0;
    pBuf->out = 0;
    pBuf->len = 0;

    return BUF_OK;
}

BUF_ERRNO rawbuf_write_byte(RAWBUF_S* pBuf, uint8_t Byte)
{
    if(NULL == pBuf)
    {
        return BUF_ERR;
    }

    if((pBuf->in+RAW_BUF_SIZE-pBuf->out)%RAW_BUF_SIZE >= RAW_BUF_SIZE)
    {
        return BUF_ERR;
    }

    pBuf->buf[pBuf->in ++] = Byte;
    if(RAW_BUF_SIZE == pBuf->in)
    {
        pBuf->in = 0;
    }   

    pBuf->len = (pBuf->in+RAW_BUF_SIZE-pBuf->out)%RAW_BUF_SIZE;

    return BUF_OK;
}

BUF_ERRNO rawbuf_read_byte(RAWBUF_S* pBuf, uint8_t* pByte)
{
    if(NULL == pBuf || NULL == pByte)
    {
        return BUF_ERR;
    }

    if(0 == (pBuf->in+RAW_BUF_SIZE-pBuf->out)%RAW_BUF_SIZE)
    {
        return BUF_ERR;
    }

    *pByte = pBuf->buf[pBuf->out];
    pBuf->buf[pBuf->out++] = 0;
    if(RAW_BUF_SIZE == pBuf->out)
    {
        pBuf->out = 0;
    }

    pBuf->len = (pBuf->in+RAW_BUF_SIZE-pBuf->out)%RAW_BUF_SIZE;

    return BUF_OK;
}

#ifdef RAW_BUFFER
void rx_byte_to_rawbuf(uint8_t Byte)
{
    /* copy to rx buffer */
    if(BUF_OK != rawbuf_write_byte(&g_RxRawBuf, Byte))
    {
        ql_log_err("rx buffer full\r\n");
    }
}
#endif

#ifdef TCPIP_BUFFER
int data_buf_ioctl(DATABUF_IOCTL_E ioctl, DATABUF_TYPE_E type, uint8_t* pbuf, uint32_t len)
{
    int i = 0;

    switch(ioctl)
    {
        case DATA_IOCTL_RESET:
            rawbuf_init(&g_RxDataBuf[type]);
            break;
        case DATA_IOCTL_SET_DATA:
            if(len + g_RxDataBuf[type].len > RAW_BUF_SIZE)
            {
                ql_log_err("rx buffer will full, BUF_SIZE:%d, current len:%d, write len:%d\r\n", RAW_BUF_SIZE, g_RxDataBuf[type].len, len);
                return 0;
            }
            for(i = 0; i < len; i++)
            {
                if(BUF_OK != rawbuf_write_byte(&g_RxDataBuf[type], pbuf[i]))
                {
                    ql_log_err("rx buffer full\r\n");
                    break;
                }
            }
            return i;
            break;
        case DATA_IOCTL_GET_DATA:
            if(g_RxDataBuf[type].len == 0)
            {
                return 0;
            }
            if(len > g_RxDataBuf[type].len)
            {
                len = g_RxDataBuf[type].len;
            }
            for(i = 0; i < len; i++)
            {
                if(BUF_OK != rawbuf_read_byte(&g_RxDataBuf[type], pbuf++))
                {
                    ql_log_err("rx buffer empty\r\n");
                    break;
                }
            }
            return i;
            break;
        case DATA_IOCTL_GET_LEN:
            return g_RxDataBuf[type].len;
            break;
        default:
            return BUF_ERR;
    }

    return BUF_OK;
}
#endif

void ql_buffer_init()
{
#ifdef RAW_BUFFER
    rawbuf_init(&g_RxRawBuf);
#endif
#ifdef TCP_BUF_USE
    data_buf_ioctl(DATA_IOCTL_RESET, DATA_TCP, NULL, NULL);
#endif
#ifdef UDP_BUF_USE
    data_buf_ioctl(DATA_IOCTL_RESET, DATA_UDP, NULL, NULL);
#endif
}

