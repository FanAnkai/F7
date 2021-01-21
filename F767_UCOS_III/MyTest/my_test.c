

#include "my_test.h"
#include "usart.h"
#include "stdio.h"

#include "task.h"




uint8_t	buff[100] = {"test_debug"};
void test_debug(void)
{
    DBG_INFO("mq_receive data : %s \r\n", buff);

}



int print_test(void)
{
	char buff[10] = {0};
    char * p = buff;

	for(int i=0; i<10; i++)
	{
         *p = 'a';
         p++;
        //buff++;   //不能直接写buff++;
	}
	printf("%s\r\n", buff);

	return 0;
}

typedef struct
{
	unsigned long  tv_sec;
	unsigned long  tv_usec;

}TIME_TYPE;

typedef struct
{
	int     code;
	int     flags;

	TIME_TYPE	time;

}EV_TYPE;

EV_TYPE ev = {0};


#define ITP_KEYPAD_DOWN     (1<<0)
#define ITP_KEYPAD_UP       (1<<1)
#define ITP_KEYPAD_REPEAT   (1<<2)

void test_print(void)
{
    ev.time.tv_sec  = 0;
    ev.time.tv_usec = 999999;
    
    ev.flags    = 7;
    ev.code     = 5;
    
    
    printf("TestFunc key: time=%ld.%ld,code=%d,down=%d,up=%d,repeat=%d,flags=0x%X\r\n", 
                ev.time.tv_sec,
                ev.time.tv_usec,
                ev.code,
                (ev.flags & ITP_KEYPAD_DOWN)    ? 1 : 0,
                (ev.flags & ITP_KEYPAD_UP)      ? 1 : 0,
                (ev.flags & ITP_KEYPAD_REPEAT)  ? 1 : 0,
                ev.flags);
    
//        printf("TestFunc key: code=%d,down=%d,up=%d,repeat=%d,flags=0x%X\r\n", 
//                ev.code,
//                (ev.flags & ITP_KEYPAD_DOWN)    ? 1 : 0,
//                (ev.flags & ITP_KEYPAD_UP)      ? 1 : 0,
//                (ev.flags & ITP_KEYPAD_REPEAT)  ? 1 : 0,
//                ev.flags);
//                
//        printf("TestFunc key: time=%ld.%ld\r\n", 
//                ev.time.tv_sec,
//                ev.time.tv_usec / 1000);
}

//TestFunc key: time=0.1325376016,code=0,down=434,up=1,repeat=1,flags=0x0

int app_test(void)
{
    
    printf("%s\r\n", __func__);
	//left_move();

	//gps_test();
	//test_pn_size();
	//test_priority_level();
	//print_test();
    //test_print();
    test_struct_pack();


	return 0;
}



