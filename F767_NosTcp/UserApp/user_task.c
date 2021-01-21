

#include "user_task.h"

#include "tcp_client.h"
#include "printf.h"
#include "kfifo.h"


USER_TASK_TYPE gTask;

void app_run(void)
{
	if(gTask.t_2ms == 0)
    {
        gTask.t_2ms = Task_2ms;
		task_tcp_client();
    }
	if(gTask.t_5ms == 0)
    {
        gTask.t_5ms = Task_5ms;
    }
    if(gTask.t_10ms == 0)
    {
        gTask.t_10ms = Task_10ms;
    }
    if(gTask.t_20ms == 0)
    {
        gTask.t_20ms = Task_20ms;
    }
    if(gTask.t_50ms == 0)
    {
        gTask.t_50ms = Task_50ms;
        task_led_entry();
    }
    if(gTask.t_100ms == 0)
    {
        gTask.t_100ms = Task_100ms;
    }
    if(gTask.t_200ms == 0)
    {
        gTask.t_200ms = Task_200ms;
    }
    if(gTask.t_500ms == 0)
    {
        gTask.t_500ms = Task_500ms;
    }
    if(gTask.t_1000ms == 0)
    {
        task_test_c_entry();
        gTask.t_1000ms = Task_1000ms;
    }
}


uint8_t led_flag = 1;
void task_led_entry(void)
{
	HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, (GPIO_PinState)(led_flag & (1 << 0)));
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, (GPIO_PinState)(led_flag & (1 << 1)));
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, (GPIO_PinState)(led_flag & (1 << 2)));

	led_flag <<= 1;
	if(led_flag >= 0x10)
	{
		led_flag = 0x01;
	}
}

//uint8_t zsj[][70] =
//{
//"▬▬▬▬.◙.▬▬▬▬\r\n",
//"▂▄▄▓▄▄▂\r\n",
//"◢◤ █▀▀████▄▄▄▄▄◢◤\r\n",
//"█《*****》带你一起飞哦！█▀▀▀▀╬\r\n",
//"◥███████◤\r\n",
//"══╩══╩══期待******\r\n",
//};

uint8_t logo[][70] =
{
"  \\ | /                                    \r\n",
" - RT -     Thread Operating System        \r\n",
"  / | \\     3.1.4 build Jul 14 2020        \r\n",
"  2006 - 2019 Copyright by rt-thread team  \r\n",
};







void print_sys_log(void)
{
    uint8_t size = sizeof(logo)/sizeof(logo[0]);
    
    for(int i=0; i<size; i++)
    {
        printf("%s", logo[i]);
    }
}

uint32_t task_cnt = 0;

void task_test_c_entry(void)
{
	task_cnt++;
	printf("%s %04X\r\n", __func__, task_cnt);
    printf("printf: Hello World\r\n");
}




void app_timer_inc(void)
{
	if(gTask.PowerUpCnt>0)gTask.PowerUpCnt--;

	if(gTask.t_2ms>0)gTask.t_2ms--;
    if(gTask.t_5ms>0)gTask.t_5ms--;
    if(gTask.t_10ms>0)gTask.t_10ms--;
    if(gTask.t_20ms>0)gTask.t_20ms--;
    if(gTask.t_50ms>0)gTask.t_50ms--;
    if(gTask.t_100ms>0)gTask.t_100ms--;
    if(gTask.t_200ms>0)gTask.t_200ms--;
    if(gTask.t_500ms>0)gTask.t_500ms--;
    if(gTask.t_1000ms>0)gTask.t_1000ms--;
}



