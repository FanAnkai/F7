#ifndef QINGLIANCLOUD_CLOUD_UTIL_H_
#define QINGLIANCLOUD_CLOUD_UTIL_H_
#include "ql_include.h"

type_u8* UINT16_TO_STREAM_f(type_u8 *p, type_u16 u16);
type_u8* UINT32_TO_STREAM_f(type_u8 *p, type_u32 u32);
type_u8* UINT64_TO_STREAM_f(type_u8 *p, type_u64 u64);
type_u16 STREAM_TO_UINT16_f(type_u8* p, type_u16 offset);
type_u32 STREAM_TO_UINT32_f(type_u8* p, type_u16 offset);
type_u64 STREAM_TO_UINT64_f(type_u8* p, type_u16 offset);
type_f32 STREAM_TO_FLOAT32_f(type_u8* p, type_u16 offset);
type_u8* FLOAT32_TO_STREAM_f(type_u8* p, type_f32 f32);

void decrypt_msg(unsigned char *data, type_u16 offset, type_u16 length, type_s32 key1, type_s32 key2);
void encrypt_msg(unsigned char *data, type_u16 offset, type_u16 length, type_s32 key1, type_s32 key2);

type_u16 crc16(type_u8* pchMsg, type_u32 wDataLen, type_u16 precrc);
type_s32 mac_up_3(unsigned char *mac);

type_s32 hex2char(char c);

type_s32 tolower_convert(char chr);

type_s32 v_strncpy(char *dest, type_s32 destsize, char *src, type_s32 srclen);

#define ql_ntohl(A)  ((((type_u32)(A) & 0xff000000) >> 24) | (((type_u32)(A) & 0x00ff0000) >> 8) | (((type_u32)(A) & 0x0000ff00) << 8) | (((type_u32)(A) & 0x000000ff) << 24))
#define ql_ntohs(A)  ((((type_u16)(A) & 0xff00) >> 8) | (((type_u16)(A) & 0x00ff) << 8))
#define ql_htonl(A)  ql_ntohl(A)
#define ql_htons(A)  ql_ntohs(A)

int str2int(const char *str);
type_f32 str2float(const char *str);
int float2str(float num, char *str, int len, int prec_n);
void hex_dump(char* name, unsigned char* buf, type_u32 len);



#endif /* QINGLIANCLOUD_CLOUD_UTIL_H_ */
