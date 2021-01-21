

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


