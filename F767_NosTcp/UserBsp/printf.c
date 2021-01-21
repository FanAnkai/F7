

#include "printf.h"



struct __FILE
{
    int handle;
};

FILE __stdout;

int fputc(int ch, FILE *f)
{
    while((USART3->ISR & 0X40)==0);
    USART3->TDR = (uint8_t) ch;
    return ch;
}

#if 1

void print_buff(uint8_t * buff, uint16_t len)
{
    for(int i=0; i<len; i++)
	{
		fputc(buff[i], &__stdout);
	}
}

#else

void print_buff(uint8_t * buff, uint16_t len)
{
    HAL_UART_Transmit_IT(&huart3, buff, len);
}

#endif

#define MAX_PRINT_SIZE  512


void my_printf(const char *format, ...)
{
    char my_print[MAX_PRINT_SIZE] = {0};
    
    va_list arg_ptr;

	memset(my_print, '\0', sizeof(my_print));
    
	va_start(arg_ptr, format);
	vsprintf(my_print, format, arg_ptr);
	va_end(arg_ptr);
    
	HAL_UART_Transmit_IT(&huart3, (uint8_t *)my_print, strlen(my_print));
}

