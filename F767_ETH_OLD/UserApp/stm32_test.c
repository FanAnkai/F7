
#include "global.h"


//结构体相关对齐问题
void sizeof_struct(void)
{
    typedef struct
    {
        short a;
        char b;
        short c;

    } SIZE_TYPE1;

    typedef struct
    {
        short a;
        char b;
        int d;
        short c;

    } SIZE_TYPE2;
    typedef struct
    {
        short a;

        short c;
        char b;
        int d;

    } SIZE_TYPE3;

    printf("SIZE_TYPE1 = %d\r\n", sizeof(SIZE_TYPE1));
    printf("SIZE_TYPE2 = %d\r\n", sizeof(SIZE_TYPE2));
    printf("SIZE_TYPE3 = %d\r\n", sizeof(SIZE_TYPE3));


}

#define CLEAR_NUM	100
unsigned int period = 100;

float Kp = 0.8;
float Ki = 0.01;
float Kd = 0.2;

float Ep = 0.00;
float Old_Ep = 0.0;
float Ei = 0.0;
float Ed = 0.0;

float current_level = 0.0;//当前水平
float goal_level = 2500.0;//目标水平
float water_in = 0.0;//单位时间进水
float water_out = 0.0;//单位时间出水

void Pid_Ctrl(void)
{
	static uint8_t i = 0;
	if(i == 0)
	{
		Old_Ep = goal_level - current_level;
		i++;
	}

	current_level = current_level+(water_in - water_out);
	//if(current_level <= 0.0)
		//current_level = 0;
	Ep = goal_level - current_level;
	Ei += Ep;

	Ed = Old_Ep-Ep;

	water_in = Kp*Ep + Ki*Ei + Kd*Ed;

	if(water_in <= 0.0)
		water_in = 0.0;

	water_out = 500 - water_in;

	printf("%4.1f  %4.1f\r\n", water_in, current_level);
	Old_Ep = Ep;
}

void TestBreak()//断点测试
{
	int a = 5;
	int b = 6;
	int c = 26;
	
	int temp = a;
	
	int i = 0;
	
	for(i=0; i<b; i++)
	{
		a *=temp;
	}
	
	a = a%c;
	
	printf("a = %d\r\n", a);
	
}

void Task_Test_C(void)
{
	//Pid_Ctrl();
	TestBreak();
}
