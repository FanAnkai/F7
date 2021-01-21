
#ifndef LCD_PRINT_H_
#define LCD_PRINT_H_

#include "main.h"


void Buff_ShowChar(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t num, uint8_t size, uint8_t mode);
void lcd_print_buff(uint8_t * str, uint16_t len);
void lcd_print(const char *format, ...);

#endif



