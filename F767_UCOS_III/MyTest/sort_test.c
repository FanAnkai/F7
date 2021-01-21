/*
 * sort_test.c
 *
 *  Created on: 2020年7月7日
 *      Author: yufei.fan
 */

#include "main.h"



//flag == 0	//从小到大
//flag == 1	//从大到小

uint8_t float_sort_buff(float *buff, uint16_t size, uint8_t flag)
{
    int i, j = 0;
	float temp1 = 0;

    if(flag == 0)
    {
        for(j = 0; j < size - 1; j++)
        {
            for(i = 0; i < size - j - 1; i++)
            {
                if(buff[i] > buff[i + 1])
                {
                    temp1 = buff[i];
                    buff[i] = buff[i + 1];
                    buff[i + 1] = temp1;
                }
            }
        }
    }
    else if(flag == 1)
    {
        for(j = 0; j < size - 1; j++)
        {
            for(i = size - 2; i >= 0; i--)
            {
                if(buff[i] < buff[i + 1])
                {
                    temp1 = buff[i];
                    buff[i] = buff[i + 1];
                    buff[i + 1] = temp1;
                }
            }
        }
    }
	return 0;
}



