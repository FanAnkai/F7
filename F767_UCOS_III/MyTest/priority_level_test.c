/*
 * priority_level_test.c
 *
 *  Created on: 2020年7月7日
 *      Author: yufei.fan
 */


#include "main.h"

void test_priority_level(void)
{
	int a = 0x00;
	//if(a&0x04 == 0x04)--->if(a&(4 == 4))
	if((a&0x04) == 0x04)
	{
		printf("== > &");
	}
	else
	{
		printf("& > ==");
	}
}

