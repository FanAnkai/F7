/***************************************************************************************************
 * Filename: protocol.c
 * Description: receive, parse and send the frame of IOT uart protocol 
 *
 * Modify:
 * 2017.12.6  hept Create.
 * 
 ***************************************************************************************************/
#include "protocol.h"
#include "protocol_if.h"
#include "stdio.h"
#include "string.h"

#if (__SDK_TYPE__== 1)
#include "ql_logic_context.h"
#include "ql_device_thread_impl.h"
#endif

serial_protocol_s *common_protocol_define = NULL;

void protocol_define_set(serial_protocol_s *protocol_define)
{
    common_protocol_define = protocol_define;
}

serial_protocol_s * protocol_define_get(void)
{
    return common_protocol_define;
}

iot_s32 recv_opcode_fun_call(serial_opcode_e opcode_id, serial_msg_s *msg)
{
    if(common_protocol_define[opcode_id].recv == NULL) {
        IOT_ERR("%s is NULL\r\n", __FUNCTION__);
        return PROTOCOL_RET_ERR;
    }
    
    return common_protocol_define[opcode_id].recv->function(msg);
}

iot_s32 send_opcode_fun_call(serial_opcode_e opcode_id, serial_msg_s *msg)
{
    iot_s32 ret = -1;
    if(common_protocol_define[opcode_id].send == NULL) {
        IOT_ERR("%s is NULL\r\n", __FUNCTION__);
        return PROTOCOL_RET_ERR;
    }

    #if (__SDK_TYPE__== 1)
    if(ql_mutex_lock(g_client_handle.serial_mutex))
    {
        ret = common_protocol_define[opcode_id].send->function(msg);
        ql_mutex_unlock(g_client_handle.serial_mutex);
    }
    #else
        ret = common_protocol_define[opcode_id].send->function(msg);
    #endif
    return ret;
}

iot_u8 get_val_with_opcodeID(serial_opcode_e opcode_id)
{
    return common_protocol_define[opcode_id].opcode_val;
}

iot_s32 _check_opcode_field(frame_direct_e direct, iot_u8 opcode, iot_u16 FrameLen)
{
    iot_u32 i = 0;
    opcode_define_s *opcode_define = NULL;

    for(i = 0; i < SERIAL_OPCODE_COUNT; i++) {
        if(get_val_with_opcodeID(i) == opcode) {
            if(direct == FRAME_DIRECT_RECV) {
                opcode_define = common_protocol_define[i].recv;
            }
            else {
                opcode_define = common_protocol_define[i].send;
            }

            if(opcode_define == NULL) {
                IOT_ERR("opcode_define is NULL, direct:%d\r\n", direct);
                return -1;
            }

            if((FRAME_LEN_FIX == opcode_define->len_type) && (FrameLen == opcode_define->len_val)) {
                break;
            }
            else if((FRAME_LEN_FLEXIBLE == opcode_define->len_type) && (FrameLen >= opcode_define->len_min) && (FrameLen <= opcode_define->len_max))
            {
                break;
            }
            else {
                IOT_ERR("check the len of opcode is err! opcode:0x%02x, len:%d, direct:%d\r\n", opcode, FrameLen, direct);
                IOT_ERR("len_type:%d, len_val:%d, len_min:%d, len_max:%d\r\n", opcode_define->len_type, opcode_define->len_val, opcode_define->len_min, opcode_define->len_max);
                return -1;
            }
        }
    }

    if(i < SERIAL_OPCODE_COUNT) {
        // IOT_INFO("check the len of opcode is ok! opcode:0x%02x, len:%d, direct:%d\r\n", opcode, FrameLen, direct);
        // IOT_INFO("len_type:%d, len_val:%d, len_min:%d, len_max:%d\r\n", opcode_define->len_type, opcode_define->len_val, opcode_define->len_min, opcode_define->len_max);
        return common_protocol_define[i].opcode_id;
    }
    else {
        IOT_ERR("check the len of opcode is err! i:%d\r\n", i);
        return -1;
    }
}

void tx_buf(const iot_u8* pBuf, iot_u16 Length)
{
    /* add user codes here */
    uart_tx_buffer_for_cmd(pBuf, Length);
}

void _LOCATION_ tx_device_frame(iot_u8 Version, iot_u8 Type, iot_u16 Length, const iot_u8* pBuf)
{
    iot_u8 opcode_id = 0;
    iot_u16 i;
    iot_u8 FrameHeader[FRAME_HEADER_LEN];
    iot_u8 CheckSum = 0;

    if(0 != Length && NULL == pBuf)
    {
        IOT_ERR("TX buf is null\r\n");
        return;
    }

    if(Length > FRAME_DATA_LMT)
    {
        IOT_ERR("TX data too long\r\n");
        return;
    }

    //check the recive frame is ok
    opcode_id = _check_opcode_field(FRAME_DIRECT_SEND, Type, Length);
    if(opcode_id < SERIAL_OPCODE_MODULE_STATUS || opcode_id >=  SERIAL_OPCODE_COUNT)
    {
        IOT_ERR("%s opcode_id:%d check err\r\n", __FUNCTION__, opcode_id);
        return;
    }
    
    /* header */
    FrameHeader[0] = FRAME_MAGIC;
    FrameHeader[1] = Version;
    FrameHeader[2] = Type;
    FrameHeader[3] = 0;
    FrameHeader[4] = (iot_u8)(Length >> 8);
    FrameHeader[5] = (iot_u8)(Length & 0xFF);

    /* checksum */
    for(i = 0; i < FRAME_HEADER_LEN; i ++)
    {
        CheckSum += FrameHeader[i];
    }

    for(i = 0; i < Length; i ++)
    {
        CheckSum += pBuf[i];
    }
    //CheckSum = ~CheckSum;

    /* send */
    tx_buf(FrameHeader, sizeof(FrameHeader));
    if(0 != Length)
    {
        tx_buf(pBuf, Length);
    }
    tx_buf(&CheckSum, 1);

    /* print frame*/
	printf("\r\nMCU_TX:\r\n");
	for(int i=0; i<6; i++)
	{
		printf("%02X ", FrameHeader[i]);
	}
	for(int i=0; i<Length; i++)
	{
		printf("%02X ", pBuf[i]);
	}
	printf("\r\n\r\n");
	
    IOT_INFO("TX ver:%02X, type:%02X, len:%d\r\n\r\n",  Version,  Type, Length);
#if 0
    for(i = 0; i < FRAME_HEADER_LEN + Length + 1; i ++)
    {
        if(i < FRAME_HEADER_LEN)
            //PRINT_INFO("%02X ", FrameHeader[i]);
        else if(i < FRAME_HEADER_LEN + Length)
            //PRINT_INFO("%02X ", pBuf[i - FRAME_HEADER_LEN]);
        else
            //PRINT_INFO("%02X ", CheckSum);
        
        if((i+1)%16 == 0)
            //PRINT_INFO("\r\n");
    }
    //PRINT_INFO("\r\n");
#endif
#if (__PROTOCOL_TYPE__ == PROTOCOL_TYPE_MODULE)
    start_status_notify(10 * 1000);
#endif
    
}

#define RX_BUF_SIZE (FRAME_HEADER_LEN + FRAME_DATA_LMT + FRAME_CHECKSUM_LEN)
#define FRAME_BUF_SIZE (FRAME_HEADER_LEN + FRAME_DATA_LMT + FRAME_CHECKSUM_LEN)

typedef struct
{
    iot_u8 buf[RX_BUF_SIZE];
    iot_u32 in;
    iot_u32 out;
    volatile iot_u32 len;
}BUF_S;

typedef enum
{
    BUF_OK,
    BUF_ERR
}BUF_ERRNO;

BUF_S g_stRxBuf = {{0}, 0, 0, 0};
iot_u8 g_FrameBuf[FRAME_BUF_SIZE];

BUF_ERRNO buf_init(BUF_S* pBuf)
{
    if(NULL == pBuf)
        return BUF_ERR;

    memset(pBuf->buf, 0, RX_BUF_SIZE);
    pBuf->in = 0;
    pBuf->out = 0;
    pBuf->len = 0;

    return BUF_OK;
}

BUF_ERRNO buf_write_byte(BUF_S* pBuf, iot_u8 Byte)
{
    if(NULL == pBuf)
    {
        return BUF_ERR;
    }

    if(pBuf->len >= RX_BUF_SIZE)
    {
        return BUF_ERR;
    }

    pBuf->buf[pBuf->in ++] = Byte;
    if(RX_BUF_SIZE == pBuf->in)
    {
        pBuf->in = 0;
    }   

    pBuf->len++;
    
    return BUF_OK;
}

BUF_ERRNO buf_read_byte(BUF_S* pBuf, iot_u8* pByte)
{
    if(NULL == pBuf || NULL == pByte)
    {
        return BUF_ERR;
    }

    if(0 == pBuf->len && pBuf->buf[pBuf->out] == 0)
    {
        return BUF_ERR;
    }

    *pByte = pBuf->buf[pBuf->out];
    pBuf->buf[pBuf->out++] = 0;
    if(RX_BUF_SIZE == pBuf->out)
    {
        pBuf->out = 0;
    }
    uart_intr_disable_for_cmd();
    if(pBuf->len > 0)
        pBuf->len--;
    uart_intr_enable_for_cmd();

    return BUF_OK;
}

/***************************************************************************************************
 * FuncName: rx_byte
 * Description: receive one byte data, called by user to receive data for protocol
 * Modify: 2017.12.5 Create
 * Param: Byte - 
 * RetVal: no
 ***************************************************************************************************/
void rx_byte(iot_u8 Byte)
{
    /* copy to rx buffer */
    if(BUF_OK != buf_write_byte(&g_stRxBuf, Byte))
    {
        IOT_ERR("rx buffer full\r\n");
    }
}

void process_recv_frame(iot_u8* Frame)
{
    iot_s32 ret = 0;
    iot_s32 opcode_id = 0;
    iot_u16 Frame_len = 0;
    DEV_FRAME_HEAD_S stFrameHeader;
    serial_msg_s msg;

    if(NULL == Frame)
    {
        IOT_ERR("RX frame err\r\n");
        return;
    }

    if(common_protocol_define == NULL) {
        IOT_ERR("common_protocol_define is null, protocol don't initial!\r\n", FRAME_VER);
        return;
    }

    memcpy(&stFrameHeader, Frame, sizeof(stFrameHeader));
    Frame_len = STREAM_TO_UINT16_f(stFrameHeader.length, 0);
    
    IOT_INFO("RX ver:%02X, type:%02X, errcode:%02x, len:%d\r\n",  stFrameHeader.version,  stFrameHeader.type, stFrameHeader.errcode, Frame_len);

    //check the version
    if(stFrameHeader.version != FRAME_VER) {
        IOT_ERR("ver error, SDK FRAME_VER:%02x\r\n", FRAME_VER);
        return;
    }

    if(stFrameHeader.errcode != 0) {
        IOT_ERR("recv msg errcode:%02x\r\n", stFrameHeader.errcode);
        return;
    }

    //check the recive frame is ok
    opcode_id = _check_opcode_field(FRAME_DIRECT_RECV, stFrameHeader.type, Frame_len);
    if(opcode_id < SERIAL_OPCODE_MODULE_STATUS || opcode_id >=  SERIAL_OPCODE_COUNT)
    {
        IOT_ERR("%s opcode_id:%d check err\r\n", __FUNCTION__, opcode_id);
        return;
    }

    msg.opcode_id = opcode_id;
    msg.msg_len = Frame_len;
    if(msg.msg_len != 0)
        msg.arg = Frame + FRAME_HEADER_LEN;
    else
        msg.arg = NULL;

    ret = recv_opcode_fun_call(opcode_id, &msg);
    if(ret != PROTOCOL_RET_OK) {
        IOT_ERR("opcode_recv_fun_call err opcode_id:%02x, ret:%d\r\n", get_val_with_opcodeID(opcode_id), ret);
        return;
    }
}

/***************************************************************************************************
 * FuncName: serial_rx_handle
 * Description: receive frame, called periodic by user to drive the protocol
 * Modify: 2017.12.5 Create
 * Param: no
 * RetVal: no
 ***************************************************************************************************/
void serial_rx_handle(void)
{
    static FRAME_PARSE_STATUS_E ParseStatus = ST_WAIT_MAGIC;
    static iot_u16 FrameIndex = 0, DataLen = 0;
    iot_u8 CheckSum = 0;
    iot_u8 ReadByte = 0;
    iot_u32 DeliverFlag = 0;

#if (__PROTOCOL_TYPE__ == PROTOCOL_TYPE_MODULE)
    if(is_factorymode()) /* during factorymode, discard all frame */
    {
        while(BUF_OK == buf_read_byte(&g_stRxBuf, &ReadByte))
        {}
        return;
    }
#endif

    /* parse frame */
    while((0 == DeliverFlag) && (BUF_OK == buf_read_byte(&g_stRxBuf, &ReadByte))  )
    {
        //PRINT_INFO("RX: get byte %02X\r\n", ReadByte);
		//PRINT_INFO("%02X ", ReadByte);
        //PRINT_INFO("RX: status %d\r\n", ParseStatus);
        switch(ParseStatus)
        {
            case ST_WAIT_MAGIC:
                FrameIndex = 0;
                if(FRAME_MAGIC == ReadByte)
                {
                    g_FrameBuf[FrameIndex++] = ReadByte;
                    ParseStatus = ST_WAIT_VER;
                    //PRINT_INFO("RX: get magic\r\n");
                }
                break;
            case ST_WAIT_VER:
                if((FRAME_VER_1 == ReadByte) || (FRAME_VER_2 == ReadByte) || (FRAME_VER_3 == ReadByte))
                {
                    g_FrameBuf[FrameIndex++] = ReadByte;
                    ParseStatus = ST_WAIT_TYPE;
                    //PRINT_INFO("RX: get ver:%02X\r\n", ReadByte);
                }
                else
                {
                    ParseStatus = ST_WAIT_MAGIC;
                }
                break;
            case ST_WAIT_TYPE:
                g_FrameBuf[FrameIndex++] = ReadByte;
                ParseStatus = ST_WAIT_ERRCODE;
                //PRINT_INFO("RX: get type:%02X\r\n", ReadByte);
                break;
            case ST_WAIT_ERRCODE:
                g_FrameBuf[FrameIndex++] = ReadByte;
                ParseStatus = ST_WAIT_LENGTH_H;
                //PRINT_INFO("RX: get errcode:%02X\r\n", ReadByte);
                break;
            case ST_WAIT_LENGTH_H:
                g_FrameBuf[FrameIndex++] = ReadByte;
                DataLen = ((iot_u16)ReadByte) << 8;
                ParseStatus = ST_WAIT_LENGTH_L;
                break;
            case ST_WAIT_LENGTH_L:
                g_FrameBuf[FrameIndex++] = ReadByte;
                DataLen += ((iot_u16)ReadByte);
                if(0 == DataLen)
                {
                    ParseStatus = ST_WAIT_CHECKSUM;
                }
                else if(DataLen <= FRAME_DATA_LMT)
                {
                    ParseStatus = ST_WAIT_DATA;
                    //PRINT_INFO("RX: get len:%d\r\n", DataLen);
                }
                else
                {
                    ParseStatus = ST_WAIT_MAGIC;
                }
                break;
            case ST_WAIT_DATA:
                g_FrameBuf[FrameIndex++] = ReadByte; 
                if(FrameIndex == FRAME_HEADER_LEN + DataLen)
                {
                    ParseStatus = ST_WAIT_CHECKSUM;
                }
                break;
            case ST_WAIT_CHECKSUM:
                {
                iot_u16 i;
                CheckSum = 0;
                for(i = 0; i < FrameIndex; i ++)
                {
                    CheckSum += g_FrameBuf[i];
                }
                g_FrameBuf[FrameIndex++] = ReadByte;
                if(ReadByte == CheckSum)
                {
                    DeliverFlag = 1;
					PRINT_INFO("\r\nRX:\r\n");
					for(int i=0; i<FrameIndex; i++)
					{
						PRINT_INFO("%02X ", g_FrameBuf[i]);
					}
                    PRINT_INFO("\r\nRX: check ok\r\n\r\n");
                }
                else
                {
                    IOT_ERR("RX: check err, ver:%02X, type:%02X, len:%d\r\n", g_FrameBuf[1], g_FrameBuf[2], DataLen);

                    //PRINT_INFO("RX f_check:%02X, cal_check:%02X\r\n", g_FrameBuf[FrameIndex - 1], CheckSum);

                    //PRINT_INFO("RX Frame: \r\n");
                    int i;
                    for(i = 0; i < FrameIndex; i ++)
                    {
                        //PRINT_INFO("%02X ", g_FrameBuf[i]);
                        if((i+1)%16 == 0)
						{
                            //PRINT_INFO("\r\n");
						}
                    }
                    //PRINT_INFO("\r\n");
                }
                ParseStatus = ST_WAIT_MAGIC;
                }
                break;
            default:
                IOT_ERR("RX: status err %d\r\n", ParseStatus);
                break;
        }
    }

    /* deliver the frame */
    if(DeliverFlag)
    {
        process_recv_frame(g_FrameBuf);
    }
}

void protocol_init(void)
{
    if(BUF_OK != buf_init(&g_stRxBuf))
    {
        IOT_ERR("rx buffer init err!");
    }
    
    uart_init_for_cmd(115200);
}

