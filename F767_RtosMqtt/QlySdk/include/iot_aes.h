#ifndef __IOT_AES_H__
#define __IOT_AES_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * return 
 *  0 success
 *  -1 fail.
 */
int iot_aes_cbc128_encrypt(unsigned char *in, unsigned int inlen, unsigned char *out, unsigned int *outlen, unsigned char *aes_key);
/*
 * return 
 *  0 success
 *  -1 fail.
 */
int iot_aes_cbc128_decrypt(unsigned char *in, unsigned int inlen, unsigned char *out, unsigned int *outlen, unsigned char *aes_key);

#ifdef __cplusplus
}
#endif

#endif
