
#include "ninebot_protocol.h"
#include "uart_parse.h"

#include "usart.h"
#include "main.h"

#include "app_iap.h"

PARSE_STRUCT parse_uart_ble;
PARSE_STRUCT parse_uart_pc;

volatile uint8_t ble_is_recv = 0;
volatile uint8_t pc_is_recv = 0;

void uart_send_ble(uint8_t *tx_buffer, uint16_t tx_len)
{
    HAL_UART_Transmit(&huart3, tx_buffer, tx_len, 1000);
}

void uart_send_pc(uint8_t *tx_buffer, uint16_t tx_len)
{
    HAL_UART_Transmit(&huart3, tx_buffer, tx_len, 1000);
}



void recv_ble_farme(void *arg)
{
    ble_is_recv = 1;
}

void recv_pc_farme(void *arg)
{
    pc_is_recv = 1;
}

void parse_loop_ble(void)
{
    if(ble_is_recv != 0)
    {

        if(parse_uart_ble.frame_s.TargetID == LOCAL_ID)
        {
            switch(parse_uart_ble.frame_s.Cmd)
            {
            case CMD_CMAP_RD :
            case CMD_CMAP_WR :
            case CMD_CMAP_WR_NR :
            case CMD_CMAP_ACK_RD :
            case CMD_CMAP_ACK_WR :
            case CMD_CMAP_WR_PN :
            case CMD_CMAP_GRAP_RD :
                break;
            
            case CMD_IAP_BEGIN :
            case CMD_IAP_TRANS :
            case CMD_IAP_VERIFY:
            case CMD_MCU_RESET :
                break;
            
			case CMD_IAP_ACK:
				break;
				
            default:
                break;


            }
        }
        else if(parse_uart_ble.frame_s.TargetID == PC_ID)
        {
			
            switch(parse_uart_ble.frame_s.Cmd)
            {
            case CMD_CMAP_RD :
            case CMD_CMAP_WR :
            case CMD_CMAP_WR_NR :
                break;
            case CMD_CMAP_ACK_RD :
                break;
            case CMD_CMAP_ACK_WR :
                break;
            default:
                break;
            }
            
        }
        ble_is_recv = 0;
    }

}

void parse_loop_pc(void)
{
    if(pc_is_recv != 0)
    {

        if(parse_uart_pc.frame_s.TargetID == LOCAL_ID)
        {
            switch(parse_uart_pc.frame_s.Cmd)
            {
            case CMD_CMAP_RD :
            case CMD_CMAP_WR :
            case CMD_CMAP_WR_NR :
            case CMD_CMAP_ACK_RD :
            case CMD_CMAP_ACK_WR :
            case CMD_CMAP_WR_PN :
                break;
            case CMD_IAP_BEGIN :
            case CMD_IAP_TRANS :
            case CMD_IAP_VERIFY:
            case CMD_MCU_RESET :
                iap_rx_data(&parse_uart_pc);
                break;
            
			case CMD_IAP_ACK:
				break;
            
            default:
                break;
            }
        }
        else if(parse_uart_pc.frame_s.TargetID == BLE_ID)
        {
        }
        pc_is_recv = 0;
    }
}


void uart_receive_struct_init(void)
{
    parse_struct_init(&parse_uart_ble);
    parse_set_rec_callback(&parse_uart_ble, recv_ble_farme);
    parse_set_send_fun(&parse_uart_ble, uart_send_ble);

    
    parse_struct_init(&parse_uart_pc);
    parse_set_rec_callback(&parse_uart_pc, recv_pc_farme);
    parse_set_send_fun(&parse_uart_pc, uart_send_pc);

}



void receive_uart_ble(uint8_t *receive_buffer, uint16_t receive_len)
{
    parse_data(&parse_uart_ble, receive_buffer, receive_len);
}

void receive_uart_pc(uint8_t *receive_buffer, uint16_t receive_len)
{
    parse_data(&parse_uart_pc, receive_buffer, receive_len);
}


int uart_send_cmd(uint8_t source_id, uint8_t target_id, uint8_t cmd, uint16_t index)
{
    FRAME_STRUCT frame_tmp;
    frame_tmp.Version = USE_VERSION;
    frame_tmp.FrameDataLen = 0;
    frame_tmp.SourceID = source_id;
    frame_tmp.TargetID = target_id;
    frame_tmp.Cmd = cmd;
    frame_tmp.DataIndex = index;
    frame_tmp.frame_data = NULL;

    if((target_id == APP_ID) || (target_id == BLE_ID))
    {
        frame_tmp.send_frame_fun = uart_send_ble;
        return creat_send_cmd(&parse_uart_ble, &frame_tmp);
    }
    else if(target_id == PC_ID)
    {
        frame_tmp.send_frame_fun = uart_send_pc;
        return creat_send_cmd(&parse_uart_ble, &frame_tmp);
    }


    return -1;

}


int uart_send_data(uint8_t source_id, uint8_t target_id, uint8_t cmd, uint16_t index, uint8_t * buff, uint16_t len)
{
    FRAME_STRUCT frame_tmp;
    frame_tmp.Version = USE_VERSION;
    frame_tmp.FrameDataLen = 0;
    frame_tmp.SourceID = source_id;
    frame_tmp.TargetID = target_id;
    frame_tmp.Cmd = cmd;
    frame_tmp.DataIndex = index;
    frame_tmp.frame_data = buff;
    frame_tmp.FrameDataLen = len;

    if((target_id == APP_ID) || (target_id == BLE_ID))
    {
        frame_tmp.send_frame_fun = uart_send_ble;
        return creat_send_cmd(&parse_uart_ble, &frame_tmp);
    }
    else if(target_id == PC_ID)
    {
        frame_tmp.send_frame_fun = uart_send_pc;
        return creat_send_cmd(&parse_uart_ble, &frame_tmp);
    }


    return -1;
}


uint32_t iap_check_bin(uint8_t * buff, uint16_t len)
{
    uint32_t sum = 0;

    for(int i=0; i<len; i++)
    {
        sum += buff[i];
    }

    return sum;
}



