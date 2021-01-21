

#include "can_bit.h"



/**
 * @brief 从符合摩托罗拉LSB格式的buff中获取一个int类型数字
 *
 * @param buff 数组指针
 * @param addr 起始地址位
 * @param len  位长度 
 * @return int类型数字
 */
int moto_lsb_get_bit(uint8_t * buff, uint8_t addr, uint8_t len)
{
    uint8_t i = 0;

    uint8_t buf_x = addr / 8;			//代表buf[x];
    uint8_t index_u8 = addr % 8;;		//一个数据中实时的idnex

    int temp = 0;

    if((len + index_u8) > ((buf_x + 1) * 8)) //为防止下边的buf_x减越界
    {
        return -1;
    }
    if(buff == NULL)
    {
        return -2;
    }
    if(len >= 32)
    {
        return -3;
    }

    for(i = 0; i < len; i++)
    {
        temp |= ((buff[buf_x] >> index_u8) & (0x01)) << i;
        index_u8++;
        if(index_u8 >= 8)
        {
            index_u8 = 0;
            buf_x--;	//经过开始的判断，这里安全换行，往上走

        }
    }
	
	return temp;
    
}


/**
 * @brief 将一个uint32_t数字按照摩托罗拉LSB格式打包到一个buff中
 *
 * @param buff 数组指针
 * @param data 将要打包的数据
 * @param addr 起始地址位
 * @param len  位长度 
 * @return int类型数字
 */
int moto_lsb_pack_bit(uint8_t * buff, uint32_t data, uint8_t addr, uint8_t len)
{
    uint8_t i = 0;

    uint8_t buf_x = addr / 8;			//代表buf[x];
    uint8_t index_u8 = addr % 8;;		//一个数据中实时的idnex

    if((len + index_u8) > ((buf_x + 1) * 8)) //为防止下边的buf_x减越界
    {
        return -1;
    }
    if(buff == NULL)
    {
        return -2;
    }

    for(i = 0; i < len; i++)
    {
        buff[buf_x] |= ((data >> i) & (0x01)) << index_u8;

        index_u8++;

        if(index_u8 >= 8)
        {
            index_u8 = 0;
            buf_x--;	//经过开始的判断，这里安全换行，往上走

        }
    }

    return 0;
}






