
#ifndef __PRINTF_H
#define __PRINTF_H


#include "usart.h"
#include "main.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"

void print_buff(uint8_t * buff, uint16_t len);
void print_buff_eof(uint8_t * buff, uint16_t len);
void my_printf(const char *format, ...);


#endif

