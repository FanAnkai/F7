

#include "test_c.h"
#include "lcd_print.h"

typedef struct
{
	int a;
	int b;
	int c;
	char d;
	double e;
	int f;
} TEST;



uint8_t queue_buff[1000] = {0};
void test_queue(void)
{
	printf("%s\r\n", __FUNCTION__);
	
	TEST * t = (TEST *)queue_buff;
	
	printf("sizeof(TEST) = %d\r\n", sizeof(TEST));
	printf("%x-%x = %d\r\n", (int)t, (int)t-1, (int)t-(int)(t-1));
	
	printf("\r\n");
}

void test_size(void)
{
	printf("%s\r\n", __FUNCTION__);
	
	printf("sizeof(long long) = %d\n",	sizeof(long long));
	printf("sizeof(long) = %d\n",		sizeof(long));
	printf("sizeof(double) = %d\n",		sizeof(double));
	printf("sizeof(int) = %d\n",		sizeof(int));
	printf("sizeof(char) = %d\n",		sizeof(char));
	
	printf("sizeof(test1_t) = %d\n", sizeof(test1_t));
	printf("sizeof(test2_t) = %d\n", sizeof(test2_t));
	printf("sizeof(test3_t) = %d\n", sizeof(test3_t));
	printf("sizeof(test4_t) = %d\n", sizeof(test4_t));
	printf("sizeof(test5_t) = %d\n", sizeof(test5_t));
	
	printf("\r\n");
}



void test_c(void)
{
	test_queue();
	test_size();
}

