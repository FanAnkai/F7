

#include "global.h"

void App_Run(void)
{
    if(gTask_Rrg.t_10ms == 0)
    {
        gTask_Rrg.t_10ms = Task_10ms;
		Tcp_Client();
		
    }
    if(gTask_Rrg.t_20ms == 0)
    {
        gTask_Rrg.t_20ms = Task_20ms;
    }
    if(gTask_Rrg.t_50ms == 0)
    {
        gTask_Rrg.t_50ms = Task_50ms;
        Task_Led();
		//Task_Test_C();
    }
    if(gTask_Rrg.t_100ms == 0)
    {
        gTask_Rrg.t_100ms = Task_100ms;
    }
    if(gTask_Rrg.t_200ms == 0)
    {
        gTask_Rrg.t_200ms = Task_200ms;
    }
    if(gTask_Rrg.t_500ms == 0)
    {
        gTask_Rrg.t_500ms = Task_500ms;
    }
    if(gTask_Rrg.t_1000ms == 0)
    {
		Task_Test_C();
        gTask_Rrg.t_1000ms = Task_1000ms;
		

    }
}


void App_Timer_Inc(void)
{
	if(gTask_Rrg.PowerUpCnt>0)gTask_Rrg.PowerUpCnt--;

    if(gTask_Rrg.t_10ms>0)gTask_Rrg.t_10ms--;
    if(gTask_Rrg.t_20ms>0)gTask_Rrg.t_20ms--;
    if(gTask_Rrg.t_50ms>0)gTask_Rrg.t_50ms--;
    if(gTask_Rrg.t_100ms>0)gTask_Rrg.t_100ms--;
    if(gTask_Rrg.t_200ms>0)gTask_Rrg.t_200ms--;
    if(gTask_Rrg.t_500ms>0)gTask_Rrg.t_500ms--;
    if(gTask_Rrg.t_1000ms>0)gTask_Rrg.t_1000ms--;
}



