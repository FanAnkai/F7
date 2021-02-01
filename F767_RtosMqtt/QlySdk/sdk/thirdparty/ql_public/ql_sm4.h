/**
 * \file sm4.h
 */
#ifndef XYSSL_SM4_H
#define XYSSL_SM4_H

#define SM4_ENCRYPT     1
#define SM4_DECRYPT     0

/**
 * \brief          SM4 context structure
 */
typedef struct
{
    int mode;                   /*!<  encrypt/decrypt   */
    unsigned long sk[32];       /*!<  SM4 subkeys       */
}
sm4_context;


#ifdef __cplusplus
extern "C" {
#endif

int ql_sm4_encrypt_ecb(unsigned char *key,
                     unsigned char *input,
                     int input_len,
                     unsigned char *output,
                     int output_size);

int ql_sm4_decrypt_ecb(unsigned char *key,
                     unsigned char *input,
                     int input_len,
                     unsigned char *output,
                     int output_size);

int ql_sm4_encrypt_cbc(unsigned char *key,
                     unsigned char *input,
                     int input_len,
                     unsigned char *output,
                     int output_size);

int ql_sm4_decrypt_cbc(unsigned char *key,
                     unsigned char *input,
                     int input_len,
                     unsigned char *output,
                     int output_size);


#ifdef __cplusplus
}
#endif

#endif /* sm4.h */
