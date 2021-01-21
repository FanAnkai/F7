/*
 * buff_test.c
 *
 *  Created on: 2020年7月7日
 *      Author: yufei.fan
 */

#include "main.h"

//测试转义字符
void buff_test(void)
{
	char json_buff[] = "{\"results\":[{\"location\":{\"id\":\"WX4FBXXFKE4F\",\"name\":\"Beijing\"},\"now\":{\"temperature\":\"18\"}}]}";

	char hello[] = "\"HelloWorld!\"";
    
    json_buff[0] = json_buff[0];
    hello[0] = hello[0];


	int size = sizeof(json_buff);
	printf("sizeof(json_buff) = %d\n", size);

	size = sizeof(hello);
	printf("sizeof(hello) = %d\n", size);
}
