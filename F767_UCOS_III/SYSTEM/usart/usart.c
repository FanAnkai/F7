#include "usart.h"
#include "delay.h"

//如果使用os,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif

//加入以下代码,支持printf函数,而不需要选择use MicroLIB
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
    int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
    while(( USART3->ISR & 0X40) == 0); //循环发送,直到发送完毕
    USART3->TDR = (uint8_t)ch;
    return ch;
}
#endif

#if EN_USART3_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
uint8_t USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
uint16_t USART_RX_STA = 0;     //接收状态标记

uint8_t aRxBuffer[RXBUFFERSIZE];//HAL库使用的串口接收缓冲
UART_HandleTypeDef UART3_Handler; //UART句柄

//初始化IO 串口1
//bound:波特率
void uart_init(u32 bound)
{
    //UART 初始化设置
    UART3_Handler.Instance = USART3;			//USART3
    UART3_Handler.Init.BaudRate = bound;		//波特率
    UART3_Handler.Init.WordLength = UART_WORDLENGTH_8B;	//字长为8位数据格式
    UART3_Handler.Init.StopBits = UART_STOPBITS_1;		//一个停止位
    UART3_Handler.Init.Parity = UART_PARITY_NONE;		//无奇偶校验位
    UART3_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;	//无硬件流控
    UART3_Handler.Init.Mode = UART_MODE_TX_RX;			//收发模式
    HAL_UART_Init(&UART3_Handler);						//HAL_UART_Init()会使能UART3

    HAL_UART_Receive_IT(&UART3_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE); //该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量

}

//UART底层初始化，时钟使能，引脚配置，中断配置
//此函数会被HAL_UART_Init()调用
//huart:串口句柄

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_Initure;

    if(huart->Instance == USART3)//如果是串口1，进行串口1 MSP初始化
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();			//使能GPIOA时钟
        __HAL_RCC_USART3_CLK_ENABLE();			//使能USART3时钟

        GPIO_Initure.Pin = GPIO_PIN_8;			//PA9
        GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
        GPIO_Initure.Pull = GPIO_PULLUP;			//上拉
        GPIO_Initure.Speed = GPIO_SPEED_FAST;		//高速
        GPIO_Initure.Alternate = GPIO_AF7_USART3;	//复用为USART3
        HAL_GPIO_Init(GPIOD, &GPIO_Initure);	   	//初始化PA9

        GPIO_Initure.Pin = GPIO_PIN_9;			//PA10
        HAL_GPIO_Init(GPIOD, &GPIO_Initure);	   	//初始化PA10

#if EN_USART3_RX
        HAL_NVIC_EnableIRQ(USART3_IRQn);				//使能USART3中断通道
        HAL_NVIC_SetPriority(USART3_IRQn, 3, 3);			//抢占优先级3，子优先级3
#endif
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)//如果是串口1
    {
        if(( USART_RX_STA & 0x8000) == 0) //接收未完成
        {
            if(USART_RX_STA & 0x4000)//接收到了0x0d
            {
                if(aRxBuffer[0] != 0x0a)USART_RX_STA = 0; //接收错误,重新开始
                else USART_RX_STA |= 0x8000;	//接收完成了
            }
            else //还没收到0X0D
            {
                if(aRxBuffer[0] == 0x0d)USART_RX_STA |= 0x4000;
                else
                {
                    USART_RX_BUF[USART_RX_STA & 0X3FFF] = aRxBuffer[0] ;
                    USART_RX_STA++;
                    if(USART_RX_STA > (USART_REC_LEN - 1))USART_RX_STA = 0; //接收数据错误,重新开始接收
                }
            }
        }

    }
}

//串口1中断服务程序
void USART3_IRQHandler(void)
{
    u32 timeout = 0;
    u32 maxDelay = 0x1FFFF;
#if SYSTEM_SUPPORT_OS	//使用OS
    OSIntEnter();
#endif

    HAL_UART_IRQHandler(&UART3_Handler);	//调用HAL库中断处理公用函数

    timeout = 0;
    while(HAL_UART_GetState(&UART3_Handler) != HAL_UART_STATE_READY) //等待就绪
    {
        timeout++;		//超时处理
        if(timeout > maxDelay)break;
    }

    timeout = 0;
    while(HAL_UART_Receive_IT(&UART3_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK) //一次处理完成之后，重新开启中断并设置RxXferCount为1
    {
        timeout++;		//超时处理
        if(timeout > maxDelay)break;
    }
#if SYSTEM_SUPPORT_OS	//使用OS
    OSIntExit();
#endif
}
#endif


