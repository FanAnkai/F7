#include "ql_util.h"
#include "ql_include.h"

type_u16 wCRCTalbeAbs[] ={
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
	0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};

type_u16 crc16(type_u8* pchMsg, type_u32 wDataLen, type_u16 precrc)
{
	type_u16 wCRC = precrc;
	type_u32 i = 0;
	type_u8 chChar;

	for (i = 0; i < wDataLen; i++) {
		chChar = *pchMsg++;
		wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
		wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
	}

	return wCRC;
}

type_u16 STREAM_TO_UINT16_f(type_u8* p, type_u16 offset)
{
	return ((type_u16) ((type_u16) (*(p + offset)) << 8)
			+ (type_u16) (*(p + offset + 1)));
}

type_u32 STREAM_TO_UINT32_f(type_u8* p, type_u16 offset) {
	return ((type_u32) ((type_u32) (*(p + offset)) << 24)
			+ (type_u32) ((type_u32) (*(p + offset + 1)) << 16)
			+ (type_u32) ((type_u32) (*(p + offset + 2)) << 8)
			+ (type_u32) (*(p + offset + 3)));
}

type_u64 STREAM_TO_UINT64_f(type_u8* p, type_u16 offset) {
	return ((type_u64) ((type_u64) (*(p + offset)) << 56)
            + (type_u64) ((type_u64) (*(p + offset + 1)) << 48)
            + (type_u64) ((type_u64) (*(p + offset + 2)) << 40)
            + (type_u64) ((type_u64) (*(p + offset + 3)) << 32)
            + (type_u64) ((type_u64) (*(p + offset + 4)) << 24)
			+ (type_u64) ((type_u64) (*(p + offset + 5)) << 16)
			+ (type_u64) ((type_u64) (*(p + offset + 6)) << 8)
			+ (type_u64) (*(p + offset + 7)));
}

type_u8* UINT64_TO_STREAM_f(type_u8 *p, type_u64 u64)
{
	*(p)++ = (type_u8) ((u64) >> 56);
	*(p)++ = (type_u8) ((u64) >> 48);
	*(p)++ = (type_u8) ((u64) >> 40);
	*(p)++ = (type_u8) ((u64) >> 32);
	*(p)++ = (type_u8) ((u64) >> 24);
	*(p)++ = (type_u8) ((u64) >> 16);
	*(p)++ = (type_u8) ((u64) >> 8);
	*(p)++ = (type_u8) (u64);
	return p;
}

type_u8* UINT32_TO_STREAM_f(type_u8 *p, type_u32 u32)
{
	*(p)++ = (type_u8) ((u32) >> 24);
	*(p)++ = (type_u8) ((u32) >> 16);
	*(p)++ = (type_u8) ((u32) >> 8);
	*(p)++ = (type_u8) (u32);
	return p;
}

type_u8* UINT16_TO_STREAM_f(type_u8 *p, type_u16 u16)
{
	*(p)++ = (type_u8) ((u16) >> 8);
	*(p)++ = (type_u8) (u16);
	return p;
}

typedef union
{
    type_f32 f_val;
    type_u8 c[4];
}float_u;

type_f32 STREAM_TO_FLOAT32_f(type_u8* p, type_u16 offset)
{
    float_u f;
    f.c[0] = p[offset];
    f.c[1] = p[offset + 1];
    f.c[2] = p[offset + 2];
    f.c[3] = p[offset + 3];
    return f.f_val;
}

type_u8* FLOAT32_TO_STREAM_f(type_u8* p, type_f32 f32)
{
    float_u f;
    f.f_val = f32;
    p[0] = f.c[0];
    p[1] = f.c[1];
    p[2] = f.c[2];
    p[3] = f.c[3];
    return p;
}

void decrypt_msg(unsigned char *data, type_u16 offset, type_u16 length, int key1, int key2)
{
	int K1, K2, i, C1, C2;

	K1 = key1;
	K2 = key2;
	C1 = (K2 >> 10) % 1024 + 371;
	C2 = (K2 % 1024) + 829;

	for (i = offset; i < length; i++) {
		char temp = data[i] ^ K1;
		K1 = ((data[i] + K1) * C1 + C2) % 4096 + 111;
		data[i] = temp;
	}
	return;
}

void encrypt_msg(unsigned char *data, type_u16 offset, type_u16 length, int key1, int key2)
{
	int K1, K2, i, C1, C2;
	K1 = key1;
	K2 = key2;
	C1 = (K2 >> 10) % 1024 + 371;
	C2 = (K2 % 1024) + 829;

	for (i = offset; i < length; i++) {
		data[i] = data[i] ^ K1;
		K1 = ((data[i] + K1) * C1 + C2) % 4096 + 111;
	}
	return;
}


int mac_up_3(unsigned char *mac)
{
	int r = 0;
	int i = 0;
	for (i = 0; i < 3; i++) {
		r = (r << 8) + mac[i];
	}

	return r;
}

int hex2char(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

int tolower_convert(char chr)
{
	if (chr >= 'A' && chr <= 'Z') {
		return chr+32;
	} else {
		return chr;
	}
}

int v_strncpy(char *dest, int destsize, char *src, int srclen)
{
	int min = 0;
	if (NULL == dest || destsize < 0 || NULL == src || srclen <= 0) {
		return 0;
	}

	min = srclen < (destsize-1) ? srclen : (destsize-1);
	ql_memcpy(dest, src, min);
	dest[min] = '\0';

	return min;
}
int str2int(const char *str)
{
    int temp = 0;
    const char *ptr = str;  //ptr保存str字符串开头

    if (*str == '-' || *str == '+')  //如果第一个字符是正负号，
    {                      //则移到下一个字符
        str++;
    }
    while(*str != 0)
    {
        if ((*str < '0') || (*str > '9'))  //如果当前字符不是数字
        {                       //则退出循环
            break;
        }
        temp = temp * 10 + (*str - '0'); //如果当前字符是数字则计算数值
        str++;      //移到下一个字符
    }
    if (*ptr == '-')     //如果字符串是以“-”开头，则转换成其相反数
    {
        temp = -temp;
    }

    return temp;
}

type_f32 str2float(const char *str)
{
    type_f32 sumF = 0;
    int sumI = 0;
    int signFlag = 0;
    int precCnt = 0;
  
    /*Is less than 0 ?*/
    if(*str == '-')
    {
        signFlag = 1;
        str++;
    }
    /*The part of integer*/
    while(*str != '.' && *str != '\0')
    {
        sumI = 10*sumI + (*str - '0');
        str++;
    }
    /*Skip the dot*/
    if(*str == '.')
    {
        str++;
    }
    /*The part of fractional*/
    while(*str != '\0')
    {
        sumF = 10*sumF + (*str - '0');
        str++;
        precCnt ++;
    }
    while(precCnt --)
    {
        sumF *= (type_f32)0.1;
    }
    /*Combine integer and fractional*/
    sumF += sumI;
    /*Add sign*/
    if(signFlag == 1)
    {
        sumF = -sumF;
    }
    return sumF;
}
/*

input : num  len  prec_n
output : str

*/
int float2str(float num, char *str, int len, int prec_n)
{
    int     sumI;
    float   sumF;
    int temp;
    int count = 0;
    int len_cnt = len - 1;
    char *p, *pp, *str_begin;

    #define ADD_CHAR(c)    do{*(str++) = (c); if((--len_cnt) <= 0){*str = '\0';return 0;}}while(0)

    if(str == NULL) 
    {
        return -1;
    }

    str_begin = p = str;
    /*Is less than 0*/
    if(num < 0)
    {
        ADD_CHAR('-');
        num = -num;
        p++;
    }
    /*Get integer and fractional*/
    sumI = (int)num;
    sumF = num - sumI;
    /*The part of integer*/
    do
    {
        temp = sumI % 10;
        ADD_CHAR(temp + '0');
    }while(sumI /= 10);
    /*Reversal the integer*/
    pp = str - 1;
    while(p < pp)
    {
        *p = *p + *pp;
        *pp = *p - *pp;
        *p = *p -*pp;
        p++;
        pp--;
    }
    /*Add dot*/
    ADD_CHAR('.');
    /*The part of fractional*/
    do
    {
        temp = (int)(sumF * 10);
        ADD_CHAR(temp + '0');
        if((++count) == prec_n)
            break;
        sumF = sumF*10 - temp;
    }while(!(sumF > -0.000001 && sumF < 0.000001));
    /*Add string end*/
    *str = '\0';
    
    return (str - str_begin);
}

void hex_dump(char* name, unsigned char* buf, type_u32 len)
{
    int i;
    
    printf("\r\n");
    printf("######### %s:%d\r\n", name, len);
    for(i = 0; i < len; i ++)
    {
        if(0 == (i%16))
        {
            printf("\r\n");
        }
        printf("%02x ", buf[i]);
    }
    printf("\r\n");
}
