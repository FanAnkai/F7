


#include "tool.h"

//flag == 0	//从小到大
//flag == 1	//从大到小

uint8_t sort_buff(uint8_t *buff, uint16_t size, uint8_t flag)
{
    int i, j, temp1 = 0;

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

const uint64_t bit_buf[] =
{
		0x0000000000000001,
		0x0000000000000002,
		0x0000000000000004,
		0x0000000000000008,
		0x0000000000000010,
		0x0000000000000020,
		0x0000000000000040,
		0x0000000000000080,
		0x0000000000000100,
		0x0000000000000200,
		0x0000000000000400,
		0x0000000000000800,
		0x0000000000001000,
		0x0000000000002000,
		0x0000000000004000,
		0x0000000000008000,
		0x0000000000010000,
		0x0000000000020000,
		0x0000000000040000,
		0x0000000000080000,
		0x0000000000100000,
		0x0000000000200000,
		0x0000000000400000,
		0x0000000000800000,
		0x0000000001000000,
		0x0000000002000000,
		0x0000000004000000,
		0x0000000008000000,
		0x0000000010000000,
		0x0000000020000000,
		0x0000000040000000,
		0x0000000080000000,
		0x0000000100000000,
		0x0000000200000000,
		0x0000000400000000,
		0x0000000800000000,
		0x0000001000000000,
		0x0000002000000000,
		0x0000004000000000,
		0x0000008000000000,
		0x0000010000000000,
		0x0000020000000000,
		0x0000040000000000,
		0x0000080000000000,
		0x0000100000000000,
		0x0000200000000000,
		0x0000400000000000,
		0x0000800000000000,
		0x0001000000000000,
		0x0002000000000000,
		0x0004000000000000,
		0x0008000000000000,
		0x0010000000000000,
		0x0020000000000000,
		0x0040000000000000,
		0x0080000000000000,
		0x0100000000000000,
		0x0200000000000000,
		0x0400000000000000,
		0x0800000000000000,
		0x1000000000000000,
		0x2000000000000000,
		0x4000000000000000,
		0x8000000000000000,

};
uint8_t my_set_bit(void * data, uint8_t bit_index, uint8_t num_bool)
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
        *(uint64_t*)data &= ~ bit_buf[bit_index];
    }
    else if(num_bool == 1)
    {
        *(uint64_t*)data |= bit_buf[bit_index];
    }

    return 1;
}


uint16_t my_get_n_bit(uint64_t data, uint8_t bit_index, uint8_t n)
{
	if(bit_index > 64)
	{
		return 0;
	}
    if(n > 16)
    {
        return 0;
    }
    
    uint16_t num = 0;
    
    for(uint8_t i=0; i<n; i++)
    {
        num |= (data & bit_buf[bit_index+i]);
    }
    
    return num;

}







