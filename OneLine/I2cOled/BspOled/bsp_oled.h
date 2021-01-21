
#ifndef BSP_OLED_H_
#define BSP_OLED_H_



#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "i2c.h"

#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"

#define OLED0561_ADD    0x78

extern uint8_t qt_image[1024];
extern uint8_t nvidia_image[1024];

void OLED_Init(void);
void OLED_SetPos(uint8_t x, uint8_t y);
void OLED_Fill(uint8_t fill_Data);
void OLED_CLS(void);

void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size);
void oled_show_str(uint8_t x, uint8_t y, uint8_t ch[], uint8_t text_size);
void oled_show_pic(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[]);


#endif


