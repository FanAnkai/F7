#include "module_config.h"
#include "protocol.h"

iot_u16 STREAM_TO_UINT16_f(iot_u8* p, iot_u16 offset)
{
	return ((iot_u16) ((iot_u16) (*(p + offset)) << 8)
			+ (iot_u16) (*(p + offset + 1)));
}

iot_u32 STREAM_TO_UINT32_f(iot_u8* p, iot_u16 offset) {
	return ((iot_u32) ((iot_u32) (*(p + offset)) << 24)
			+ (iot_u32) ((iot_u32) (*(p + offset + 1)) << 16)
			+ (iot_u32) ((iot_u32) (*(p + offset + 2)) << 8)
			+ (iot_u32) (*(p + offset + 3)));
}

iot_u64 STREAM_TO_UINT64_f(iot_u8* p, iot_u16 offset) {
	return ((iot_u64) ((iot_u64) (*(p + offset)) << 56)
            + (iot_u64) ((iot_u64) (*(p + offset + 1)) << 48)
            + (iot_u64) ((iot_u64) (*(p + offset + 2)) << 40)
            + (iot_u64) ((iot_u64) (*(p + offset + 3)) << 32)
            + (iot_u64) ((iot_u64) (*(p + offset + 4)) << 24)
			+ (iot_u64) ((iot_u64) (*(p + offset + 5)) << 16)
			+ (iot_u64) ((iot_u64) (*(p + offset + 6)) << 8)
			+ (iot_u64) (*(p + offset + 7)));
}

iot_u8* UINT64_TO_STREAM_f(iot_u8 *p, iot_u64 u64)
{
	*(p)++ = (iot_u8) ((u64) >> 56);
	*(p)++ = (iot_u8) ((u64) >> 48);
	*(p)++ = (iot_u8) ((u64) >> 40);
	*(p)++ = (iot_u8) ((u64) >> 32);
	*(p)++ = (iot_u8) ((u64) >> 24);
	*(p)++ = (iot_u8) ((u64) >> 16);
	*(p)++ = (iot_u8) ((u64) >> 8);
	*(p)++ = (iot_u8) (u64);
	return p;
}

iot_u8* UINT32_TO_STREAM_f(iot_u8 *p, iot_u32 u32)
{
	*(p)++ = (iot_u8) ((u32) >> 24);
	*(p)++ = (iot_u8) ((u32) >> 16);
	*(p)++ = (iot_u8) ((u32) >> 8);
	*(p)++ = (iot_u8) (u32);
	return p;
}

iot_u8* UINT16_TO_STREAM_f(iot_u8 *p, iot_u16 u16)
{
	*(p)++ = (iot_u8) ((u16) >> 8);
	*(p)++ = (iot_u8) (u16);
	return p;
}

typedef union
{
    iot_f32 f_val;
    iot_u8 c[4];
}float_u;

iot_f32 STREAM_TO_FLOAT32_f(iot_u8* p, iot_u16 offset)
{
    float_u f;
    f.c[0] = p[offset];
    f.c[1] = p[offset + 1];
    f.c[2] = p[offset + 2];
    f.c[3] = p[offset + 3];
    return f.f_val;
}

iot_u8* FLOAT32_TO_STREAM_f(iot_u8* p, iot_f32 f32)
{
    float_u f;
    f.f_val = f32;
    p[0] = f.c[0];
    p[1] = f.c[1];
    p[2] = f.c[2];
    p[3] = f.c[3];
    return p;
}

void hex_dump(char* name, unsigned char* buf, iot_u32 len)
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


