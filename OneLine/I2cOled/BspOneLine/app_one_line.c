

#include "bsp_one_line.h"
#include "app_one_line.h"

uint32_t task_i = 0;
void OneLineTest(void)
{    
    if(gOneLine.rx_ok == 1)
    {
        gOneLine.rx_ok = 0;
        HAL_Delay(2000);
        one_line_send(gOneLine.rx_data, gOneLine.rx_len);
        task_i = 0;
    }
    else
    {
        task_i++;
        HAL_Delay(1000);
        if(task_i % 5 == 0)
        {
            one_line_send("1234567890abcdefghijklmn", 24);
        }
    }
}
