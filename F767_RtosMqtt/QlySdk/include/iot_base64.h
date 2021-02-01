#ifndef __IOT_BASE64_H__
#define __IOT_BASE64_H__

/*
 * -1 fail, else success
 */
int iot_encode_base64(char *pDest, unsigned int dstlen, const char *pSrc, unsigned int srclen);
/*
 * -1 fail, else success
 */
int iot_decode_base64(char *pDest, unsigned int dstlen, const char *pSrc, unsigned int srclen);

#endif
