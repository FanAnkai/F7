
#ifndef BSP_ONE_LINE_H_
#define BSP_ONE_LINE_H_



#include "config_one_line.h"

/*
 *  协议结构体
 */
typedef struct
{
    uint8_t             rx_data[ONE_LINE_BUF_SIZE]; //接收到的数据
    uint16_t            rx_len; //接收到的长度
    volatile uint8_t    rx_ok;  //接收完成
    volatile uint8_t    tx_ok;  //发送完成

} ONE_LINE_TYPE;


extern ONE_LINE_TYPE gOneLine;


/*
 *  初始化
 */
void one_line_comm_init(void);

/*
 *  发送函数
 */
uint8_t one_line_send(uint8_t * data, uint16_t len);


/*
 *  定时器回调函数    推荐定时范围：1/16ms 到 1/2ms
 *  建议设置较高优先级
 *  1/2ms调用 TL(位周期) = (TL_CNT*0.5)ms
 *  1/4ms调用 TL(位周期) = (TL_CNT*0.25)ms
 */
void one_line_timer_cb(void);

/*
 *  下降沿中断回调
 *  建议设置较高优先级
 */
void one_line_exit_cb(void);


#endif


