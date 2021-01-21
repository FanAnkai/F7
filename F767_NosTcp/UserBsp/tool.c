

#include "tool.h"


uint16_t get_average(uint16_t *buff, uint16_t size, uint8_t rm_num)
{
    uint16_t i, j, temp1 = 0;
    uint32_t sum = 0;
    float average;
    //先排序
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
    //去除最大与最小
    for(i = rm_num; i < (size - rm_num); i++)
    {
        sum += buff[i];
    }
    average = sum / (size - (rm_num << 1));
    return (uint16_t)average;

}



int get_average_int16(short *buff, uint16_t size, uint8_t rm_num)
{
    uint16_t i, j = 0;
    int sum = 0;
	short temp = 0;
    float average;
    //先排序
    for(j = 0; j < size - 1; j++)
    {
        for(i = 0; i < size - j - 1; i++)
        {
            if(buff[i] > buff[i + 1])
            {
                temp = buff[i];
                buff[i] = buff[i + 1];
                buff[i + 1] = temp;
            }
        }
    }
    //去除最大与最小
    for(i = rm_num; i < (size - rm_num); i++)
    {
        sum += buff[i];
    }
    average = sum / (size - (rm_num << 1));
    return (int)average;

}

//设置一个数据的某个位
//返回1 成功
//返回0 失败
uint8_t my_set_bit(uint64_t * data, uint8_t bit_index, uint8_t num_bool)
{
	if(data == NULL)
	{
		return 0;
	}
	if((num_bool > 1) || (bit_index > 64))
	{
		return 0;
	}

	
	if(num_bool == 0)
	{
		*data &= ~(1<<bit_index);
	}
	else if(num_bool == 1)
	{
		*data |= (1<<bit_index);
	}

	return 1;
}








