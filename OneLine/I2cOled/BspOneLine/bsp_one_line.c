

#include "bsp_one_line.h"




ONE_LINE_TYPE gOneLine = {0};


#define ONE_LINE_PORT   GPIOB
#define ONE_LINE_PIN    GPIO_PIN_8
#define ONE_LINE_IRQ    EXTI9_5_IRQn

#define TX_PIN_H    HAL_GPIO_WritePin(ONE_LINE_PORT, ONE_LINE_PIN, GPIO_PIN_SET)
#define TX_PIN_L    HAL_GPIO_WritePin(ONE_LINE_PORT, ONE_LINE_PIN, GPIO_PIN_RESET)

#define RX_PIN  HAL_GPIO_ReadPin(ONE_LINE_PORT, ONE_LINE_PIN)



void set_pin_tx(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    HAL_NVIC_DisableIRQ(ONE_LINE_IRQ);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
    
    TX_PIN_L;
}


void set_pin_rx(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
    
    TX_PIN_L;
    
    HAL_NVIC_SetPriority(ONE_LINE_IRQ, 0, 0);
    HAL_NVIC_EnableIRQ(ONE_LINE_IRQ);
    
}




uint8_t  tx_flag = 0;        //启动发送
uint8_t  tx_first = 0;       //同步信号
uint8_t  tx_buf[MAX_BUFF_SIZE] = {0};
uint16_t tx_len = 0;         //要发送到长度
uint16_t tx_byte = 0;        //发送了多少字节
uint16_t tx_bit = 0;
uint16_t tx_tim_cnt = 0;     //定时器计数

uint8_t one_line_send(uint8_t * data, uint16_t len)
{
    if(len > MAX_BUFF_SIZE)
    {
        return 2;
    }
    if(tx_flag == 1)
    {
        return 1;
    }
    else
    {
        set_pin_tx(ONE_LINE_PORT, ONE_LINE_PIN);    //设置发送模式
        
        memcpy(tx_buf, data, len);
        tx_len = len;
        tx_flag = 1;
        tx_first = 1;

        tx_tim_cnt = 0;
        tx_bit = 0;
        tx_byte = 0;

        TX_PIN_L;
        return 0;
    }
}



#define TL_CNT  4   //8 //16


// 1/2ms调用 TL = (TL_CNT*0.5)ms
// 1/4ms调用 TL = (TL_CNT*0.25)ms
void send_timer_cb(void)
{
    if(tx_flag == 1)
    {
        tx_tim_cnt ++;
        if(tx_first == 1)
        {
            if(tx_tim_cnt < 10*TL_CNT)
            {
                TX_PIN_L;
            }
            else if(tx_tim_cnt < 10*TL_CNT+TL_CNT/4)
            {
                TX_PIN_H;
            }
            else
            {
                TX_PIN_L;
                tx_tim_cnt = 0;
                tx_first = 0;
            }
        }
        else
        {
            if(tx_buf[tx_byte] & (1<<tx_bit))
            {
                if(tx_tim_cnt < TL_CNT/4)
                {
                    TX_PIN_L;
                }
                else if(tx_tim_cnt < (TL_CNT))
                {
                    TX_PIN_H;
                }
                else
                {
                    TX_PIN_L;
                    tx_bit++;
                    tx_tim_cnt = 0;

                    if(tx_bit >= 8)
                    {
                        tx_bit = 0;
                        tx_byte++;

                        if(tx_byte >= tx_len)
                        {
                            TX_PIN_L;
                            tx_flag = 0;
                            memset(&gOneLine, 0, sizeof(gOneLine));
                            
                            set_pin_rx(ONE_LINE_PORT, ONE_LINE_PIN);    //设置接收模式
                            return;
                        }
                    }
                }
            }
            else
            {
                if(tx_tim_cnt < TL_CNT*3/4)
                {
                    TX_PIN_L;
                }
                else if(tx_tim_cnt < (TL_CNT))
                {
                    TX_PIN_H;
                }
                else
                {
                    TX_PIN_L;
                    tx_bit++;
                    tx_tim_cnt = 0;

                    if(tx_bit >= 8)
                    {
                        tx_bit = 0;
                        tx_byte++;

                        if(tx_byte >= tx_len)
                        {
                            TX_PIN_L;
                            tx_flag = 0;
                            memset(&gOneLine, 0, sizeof(gOneLine));
                            
                            set_pin_rx(ONE_LINE_PORT, ONE_LINE_PIN);    //设置接收模式
                            return;
                        }
                    }
                }
            }
            
        }
    }
}

uint8_t  rx_flag = 0;
uint8_t  rx_buf[MAX_BUFF_SIZE] = {0};
uint16_t rx_byte = 0;       //接收了多少字节
uint16_t rx_bit = 0;        //接收了多少位
uint16_t rx_tim_cnt = 0;    //定时器计数
uint16_t rx_tim_out = 0;

void rev_timer_cb(void)
{
    if(rx_flag == 1)
    {
        rx_tim_cnt++;
        if(rx_tim_cnt == (TL_CNT/2))
        {
            rx_buf[rx_byte] |= (RX_PIN << rx_bit);
            rx_bit++;
            if(rx_bit >= 8)
            {
                rx_bit = 0;
                rx_byte++;
            }
            
            
            rx_flag = 0;
            rx_tim_cnt = 0;
            
        }
    }
    
    if(rx_tim_out < TL_CNT*2)
    {
        rx_tim_out++;
        if(rx_tim_out == TL_CNT*2)
        {
            if(rx_byte > 0)
            {
                //rx_byte -= 1;
                memset(&gOneLine.rx_data, 0, sizeof(gOneLine.rx_data));
                memcpy(&gOneLine.rx_data, rx_buf, rx_byte);
                
                gOneLine.rx_len = rx_byte;
                
                //清除
                memset(&rx_buf, 0, sizeof(rx_buf));
                rx_byte = 0;
                rx_bit = 0;
                rx_flag = 0;
                rx_tim_cnt = 0;
                
                gOneLine.rx_ok = 1;
            }
            else
            {
                memset(&rx_buf, 0, sizeof(rx_buf));
                rx_byte = 0;
                rx_bit = 0;
                rx_flag = 0;
                rx_tim_cnt = 0;
            }

        }
    }
    
    
}



//下降沿触发中断
void rev_exit_cb(void)
{
    rx_tim_out = 0;
    
    rx_flag = 1;
    rx_tim_cnt = 0;
    
    
}


