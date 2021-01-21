

#include "main.h"
#include "usart.h"
#include "gpio.h"

#include "app_iap.h"
#include "nboot.h"
#include "flash.h"
#include "kfifo.h"
#include "uart_parse.h"
#include "nboot.h"

IAP_DATA_TYPE gIapFream = {0};

extern const Temp_Flash_Context flash_ctx;

uint32_t temp1 = 0;
uint32_t temp2 = 0;
uint32_t temp = NBOOT_SRAM_END;
void check_iap(void)
{
    uart_rx_init();
    temp1 = *(uint32_t *)IMAGE_SLOT_A_START;
    temp2 = *(uint32_t *)IMAGE_SLOT_B_START;
    if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == 1)
    {
        run_iap();
    }
    else if((temp1 > temp) && (temp2 > temp))
    {
        run_iap();
    }
}



uint32_t fifo_len;
uint8_t data_buff[512];
void run_iap(void)
{
    gIapFream.iap_step = CMD_IAP_BEGIN;
    while(1)
    {
        if((fifo_len = kfifo_len(&gFifoRx)) > 0)
        {
            kfifo_get(&gFifoRx, data_buff, fifo_len);
            receive_uart_pc(data_buff, fifo_len);

            memset(data_buff, 0, sizeof(data_buff));
            
            parse_loop_pc();
        }

        
    }

}


void iap_stat_reset(IAP_DATA_TYPE * iap)
{
    memset(iap, 0, sizeof(IAP_DATA_TYPE));
    iap->iap_step = CMD_IAP_BEGIN;

}


void iap_rx_data(PARSE_STRUCT * nbbus)
{
    uint8_t cmd     = nbbus->frame_s.Cmd;
    uint8_t * buff  = nbbus->frame_s.frame_data;
    uint16_t index  = nbbus->frame_s.DataIndex;
    uint16_t len    = nbbus->frame_s.FrameDataLen;


    switch(cmd)
    {
    case CMD_IAP_BEGIN:
        if(index == 0)
        {
            memcpy(&gIapFream.file_size, buff, 4);
            if((gIapFream.file_size < MAX_IMAGE_SIZE))
            {
                flash_ctx.backend->erase(IMAGE_SLOT_B_START, MAX_IMAGE_SIZE);
                gIapFream.iap_step = CMD_IAP_TRANS;
                iap_send_cmd(CMD_IAP_ACK, IAP_OK);
            }
            else
            {
                iap_send_cmd(CMD_IAP_ACK, IAPERROR_SIZE);
                iap_stat_reset(&gIapFream);
            }
        }
        else
        {
            iap_send_cmd(CMD_IAP_ACK, IAPERROR_INDEX);
            iap_stat_reset(&gIapFream);
        }
        break;

    case CMD_IAP_TRANS:
        if(index == gIapFream.index_data)
        {
            gIapFream.index_data++;
            flash_ctx.backend->write(IMAGE_SLOT_B_START+gIapFream.write_pos, buff, len);
            gIapFream.write_pos+=len;

            gIapFream.crc_file_cal += iap_check_bin(buff, len);

            if(gIapFream.write_pos >= gIapFream.file_size)
            {
                gIapFream.crc_file_cal = ~gIapFream.crc_file_cal;
                gIapFream.iap_step = CMD_IAP_VERIFY;
            }
            
            iap_send_cmd(CMD_IAP_ACK, IAP_OK);
        }
        else
        {
            iap_send_cmd(CMD_IAP_ACK, IAPERROR_INDEX);
            iap_stat_reset(&gIapFream);
        }
        break;

    case CMD_IAP_VERIFY:
        if(index == 0)
        {
            
            memcpy(&gIapFream.crc_file_get, buff, 4);
            if(gIapFream.crc_file_get == gIapFream.crc_file_cal)
            {
                iap_send_cmd(CMD_IAP_ACK, IAP_OK);
                gIapFream.iap_step = CMD_MCU_RESET;
            }
            else
            {
                iap_send_cmd(CMD_IAP_ACK, IAPERROR_CRC);
                iap_stat_reset(&gIapFream);
            }
        }
        else
        {
            iap_send_cmd(CMD_IAP_ACK, IAPERROR_INDEX);
            iap_stat_reset(&gIapFream);
        }
        break;

    case CMD_MCU_RESET:
        if(index == 0)
        {
            
            nboot_set_image_down_state(IMAGE_STATE_DOWNLOAD_OK, gIapFream.file_size);
            nboot_set_image_down_state(IMAGE_STATE_DOWNLOAD_OK, gIapFream.file_size);
            iap_stat_reset(&gIapFream);

            NVIC_SystemReset();
            
        }
        break;

    default:
        iap_stat_reset(&gIapFream);
        break;
    }
}



extern UART_HandleTypeDef huart3;

#if 0
void uart_rx_init(void)
{
    kfifo_init(&gFifoRx, gIapFream.fifo_buff, IAP_RX_MAX);

    HAL_UART_Receive_DMA(&huart3, gIapFream.dma_buff, UART_DMA_RX_MAX);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
}

uint32_t rx_len = 0;
void uart3_rx_callback(void)
{
    if(__HAL_UART_GET_IT(&huart3, UART_IT_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart3);

        rx_len = UART_DMA_RX_MAX - __HAL_DMA_GET_COUNTER(&hdma_usart3_rx);

        HAL_UART_DMAStop(&huart3);

        if(rx_len > 0)
        {
            kfifo_put(&gFifoRx, gIapFream.dma_buff, rx_len);
            memset(gIapFream.dma_buff, 0, UART_DMA_RX_MAX);
        }

        HAL_UART_Receive_DMA(&huart3, gIapFream.dma_buff, UART_DMA_RX_MAX);
    }

}

#else


void uart_rx_init(void)
{
    uint8_t temp_data = 0;
    kfifo_init(&gFifoRx, gIapFream.fifo_buff, IAP_RX_MAX);

    __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
    HAL_UART_Receive_IT(&huart3, &temp_data, 1);
}

uint8_t rx_data = 0;
void uart3_rx_callback(void)
{
    
    if(__HAL_UART_GET_IT(&huart3, UART_IT_RXNE) != RESET)
    {
        rx_data = huart3.Instance->RDR;
        kfifo_put(&gFifoRx, &rx_data, 1);
    }

    //这几句防止错误
    __HAL_UART_CLEAR_OREFLAG(&huart3);
    __HAL_UART_CLEAR_PEFLAG (&huart3);
    __HAL_UART_CLEAR_FEFLAG (&huart3);
    __HAL_UART_CLEAR_NEFLAG (&huart3);
    
}

#endif

void iap_send_cmd(uint8_t cmd, uint8_t iap_ack)
{
    uart_send_cmd(LOCAL_ID, PC_ID, cmd, iap_ack);
}


void iap_send_data(uint8_t cmd, uint8_t index, uint8_t * buff, uint16_t len)
{
    uart_send_data(LOCAL_ID, PC_ID, cmd, index, buff, len);

}


