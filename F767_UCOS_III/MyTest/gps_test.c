/*
 * gps_test.c
 *
 *  Created on: 2020年7月7日
 *      Author: yufei.fan
 */

#include "main.h"




//计算n秒内最大的偏移值
#define COG_BUF_SIZE    3
uint8_t is_out_max_diff(float *buf, uint8_t size, float new, float set_max_diff)
{
	uint8_t i=0;
	float left = 0;
	float right = 0;

	left = new-set_max_diff;
	right = new+set_max_diff;

	if(left<0)
	{
		left = 360+left;
	}
	if(right>360)
	{
		right = right-360;
	}

	printf("left = %f\n", left);
	printf("right = %f\n", right);

	for(i=0; i<size; i++)
	{
		if((buf[i]<left) && (buf[i]>right))
		{
			return 1;
		}
	}

	return 0;
}

uint8_t cal_data = 0;
void gps_test(void)
{
	float gps_cog[3] = {10, 350, 20};
	float gps_cog_new = 20.1;

	cal_data = is_out_max_diff(gps_cog, 3, gps_cog_new, 30);


	printf("%d\n", cal_data);

}


