

#include "tftlcd.h"
#include "lcd_print.h"
#include "stdlib.h"
#include "usart.h"


#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"



#if 1
uint8_t Buff_ShowString(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t size, uint8_t *p, uint8_t len)
{
    uint8_t res = 1;

     //判断是不是非法字符!
    while(len--)
    {
        if(*p == 0)
        {
            Buff_ShowChar(x, y, fc, bc, ' ', size, 0);
        }
        else
        {
            Buff_ShowChar(x, y, fc, bc, *p, size, 0);
        }
        x += size / 2;
        p++;

    }
    
    return 0;
}
#else

uint8_t Buff_ShowString(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint16_t width, uint16_t height, uint8_t size, uint8_t *p)
{
    uint8_t res = 1;
    uint8_t x0 = x;
    width += x;
    height += y;
    while(1) //判断是不是非法字符!
    {
        if(x >= width-size/2)
        {
            x = x0;
            y += size;
            res++;
        }
        if(y >= height)
            break; //退出

        Buff_ShowChar(x, y, fc, bc, *p, size, 0);
        x += size / 2;
        p++;
    }
    
    return res;
}

#endif




#define font_size   (12)
#define x_size      (176*2/font_size)   //一行最大的字节数
#define y_size      (220/font_size)     //最大行数

uint8_t log_buff[y_size][x_size] = {0};
extern SemaphoreHandle_t lcd_sync;
void lcd_print_buff(uint8_t * str, uint16_t len)
{
    xSemaphoreTake(lcd_sync, 50);
    int str_line = len/x_size+1;    //当前str要占用几行
    
    memcpy(log_buff[0], log_buff[str_line], x_size*(y_size-str_line));
    memset(log_buff[y_size-str_line], 0, x_size*str_line);
    memcpy(log_buff[y_size-str_line], str, len);
    
    for(int i=0; i<y_size; i++)
    {
        Buff_ShowString(0, i*font_size, GREEN, BLACK, font_size, log_buff[i], x_size);
        //my_printf("%s\r\n", log_buff[i]);
    }
    
    memset(str, 0, len);

    xSemaphoreGive(lcd_sync);

}

#define MAX_LCD_TEMP    200
char lcd_temp[MAX_LCD_TEMP] = {0};

void lcd_print(const char *format, ...)
{
    va_list arg_ptr;

	memset(lcd_temp, ' ', sizeof(lcd_temp));
    
	va_start(arg_ptr, format);
	vsprintf(lcd_temp, format, arg_ptr);
	va_end(arg_ptr);
    
    lcd_print_buff(lcd_temp, strlen(lcd_temp));
}



