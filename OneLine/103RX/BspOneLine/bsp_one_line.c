

#include "bsp_one_line.h"

ONE_LINE_TYPE gOneLine = {0};

//初始化
void one_line_comm_init(void)
{
    set_pin_rx(ONE_LINE_PORT, ONE_LINE_PIN);    //设置接收模式
}


static uint8_t  tx_flag = 0;        //启动发送
static uint8_t  tx_first = 0;       //同步信号
static uint8_t  tx_buf[ONE_LINE_BUF_SIZE] = {0};
static uint16_t tx_len = 0;         //要发送到长度
static uint16_t tx_byte = 0;        //发送了多少字节
static uint16_t tx_bit = 0;
static uint16_t tx_tim_cnt = 0;     //定时器计数

//发送函数
uint8_t one_line_send(uint8_t * data, uint16_t len)
{
    if(len > ONE_LINE_BUF_SIZE)
    {
        return 2;
    }
    if(tx_flag == 1)
    {
        return 1;
    }
    else
    {
        gOneLine.tx_ok = 0;
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



static void send_timer_cb(void)
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
                            
                            gOneLine.tx_ok = 1;
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
                            
                            gOneLine.tx_ok = 1;
                            set_pin_rx(ONE_LINE_PORT, ONE_LINE_PIN);    //设置接收模式
                            return;
                        }
                    }
                }
            }
            
        }
    }
}

static uint8_t  rx_flag = 0;
static uint8_t  rx_buf[ONE_LINE_BUF_SIZE] = {0};
static uint16_t rx_byte = 0;       //接收了多少字节
static uint16_t rx_bit = 0;        //接收了多少位
static uint16_t rx_tim_cnt = 0;    //定时器计数
static uint16_t rx_tim_out = 0;

static void rev_timer_cb(void)
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

// 1/2ms调用 TL(位周期) = (TL_CNT*0.5)ms
// 1/4ms调用 TL(位周期) = (TL_CNT*0.25)ms

//定时器回调函数    推荐定时范围：1/16ms 到 1/2ms
void one_line_timer_cb(void)
{
    rev_timer_cb();
    send_timer_cb();
}

//下降沿中断回调
void one_line_exit_cb(void)
{
    rx_tim_out = 0;
    
    rx_flag = 1;
    rx_tim_cnt = 0;
    
}


