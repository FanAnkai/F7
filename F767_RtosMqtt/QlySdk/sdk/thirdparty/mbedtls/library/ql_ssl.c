#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#define ql_log_err printf

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    mbedtls_fprintf( (FILE *) ctx, "%s:%04d: %s", file, line, str );
    fflush(  (FILE *) ctx  );
}

struct ql_socket_t {
    mbedtls_net_context sockfd;
    mbedtls_ssl_context ssl_ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_config conf;
    //mbedtls_x509_crt cacert;
};
typedef struct ql_socket_t ql_socket_t;

ql_socket_t* ql_tcp_socket_create_ssl()
{
    ql_socket_t *socket = NULL;
    const char *pers = "ssl_client";
    int ret = 1;

    socket = (ql_socket_t *)mbedtls_calloc(1, sizeof(ql_socket_t));

    /*
     * 0. Initialize network
     */
    mbedtls_net_init( &(socket->sockfd) );
    
    /*
     * 1. Initialize the RNG
     */
    mbedtls_ctr_drbg_init( &(socket->ctr_drbg) );
    mbedtls_entropy_init( &(socket->entropy) );
    if( ( ret = mbedtls_ctr_drbg_seed( &(socket->ctr_drbg), mbedtls_entropy_func, &(socket->entropy), 
        (const unsigned char *) pers, strlen( pers ) ) ) != 0 )
    {
        ql_log_err( "ssl_seed:%d", ret );
    }

     /*
     * 2. Initialize certificates
     */
    #if 0
    mbedtls_x509_crt_init( &(socket->cacert) );
    ret = mbedtls_x509_crt_parse( &(socket->cacert), (const unsigned char *) mbedtls_test_cas_pem,
                          mbedtls_test_cas_pem_len );
    if( ret < 0 )
    {
        ql_log_err("mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
    }
    #endif
    
    /*
     * 3. Initialize config
     */
    mbedtls_ssl_config_init( &(socket->conf) );
    if( ( ret = mbedtls_ssl_config_defaults( &(socket->conf),
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        ql_log_err("ssl_init:%d", ret );
        return NULL;
    }

    mbedtls_ssl_conf_authmode( &(socket->conf), MBEDTLS_SSL_VERIFY_NONE );
    //mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
    mbedtls_ssl_conf_rng( &(socket->conf), mbedtls_ctr_drbg_random, &(socket->ctr_drbg) );
    mbedtls_ssl_conf_dbg( &(socket->conf), my_debug, stdout );
    
    return socket;
}

void ql_tcp_socket_destroy_ssl(ql_socket_t **sock)
{
    if (sock && *sock) {
        mbedtls_ssl_config_free( &((*sock)->conf) );
        //mbedtls_x509_crt_free( &((*sock)->cacert) );
        mbedtls_ctr_drbg_free( &((*sock)->ctr_drbg) );
        mbedtls_entropy_free( &((*sock)->entropy) );

        mbedtls_free(*sock);
    }

    *sock = NULL;
}

int ql_tcp_connect_ssl(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms)
{
    char port_str[10];
    int ret = 1;
    
    if(sock == NULL || ip == NULL)
        return -1;

    timeout_ms = timeout_ms;

    /*
    * Connect network
    */
    sprintf(port_str, "%d", port);
    if( ( ret = mbedtls_net_connect( &(sock->sockfd), (char*)ip,
                                         port_str, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        ql_log_err("ssl_conn:%d", ret );
        return -1;
    }

    /* Init SSL */
    mbedtls_ssl_init( &(sock->ssl_ctx) );
    if( ( ret = mbedtls_ssl_setup( &(sock->ssl_ctx), &(sock->conf) ) ) != 0 )
    {
        ql_log_err("ssl_setup:%d", ret );
        return -1;
    }
    mbedtls_ssl_set_bio( &(sock->ssl_ctx), &(sock->sockfd), mbedtls_net_send, mbedtls_net_recv, /*mbedtls_net_recv_timeout*/NULL );

     /*
     * Handshake
     */
    while( ( ret = mbedtls_ssl_handshake( &(sock->ssl_ctx) ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            ql_log_err("ssl_hand:-0x%x", -ret );
            return -1;
        }
    }

    return 0;
}

int ql_tcp_disconnect_ssl(ql_socket_t *sock)
{
    if(sock == NULL)
        return -1;
    
    mbedtls_ssl_close_notify( &(sock->ssl_ctx) );
    mbedtls_net_free( &(sock->sockfd));
    mbedtls_ssl_free( &(sock->ssl_ctx));

    return 0;
}

int ql_tcp_send_ssl(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    struct timeval tv;
    int retval;
    int sentlen = 0;
    int totallen = 0;

    if (sock == NULL || buf == NULL) 
    {
        return -1;
    }

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt((sock->sockfd).fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));

    totallen = len;
    while (sentlen < totallen) {
        retval = mbedtls_ssl_write(&(sock->ssl_ctx), buf + sentlen, totallen - sentlen);
        if (retval < 0) {
            if (retval == MBEDTLS_ERR_SSL_WANT_READ || retval == MBEDTLS_ERR_SSL_WANT_READ) {
                continue;//return 0;
            } else {
                ql_log_err("ssl_write:%d", retval );
                return -1;
            }
        }
        sentlen += retval;
    }

    return totallen;
}

int ql_tcp_recv_ssl(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
{
    struct timeval tv;
    int retval;

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt((sock->sockfd).fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

    retval = mbedtls_ssl_read(&(sock->ssl_ctx), buf, size);
    if (retval < 0)
    {
        if (retval == MBEDTLS_ERR_SSL_WANT_READ || retval == MBEDTLS_ERR_SSL_WANT_WRITE || retval == MBEDTLS_ERR_NET_RECV_FAILED)
        {
           return 0;
        }
        else
        {
            return -1;
        }
    }

    return retval;
}

/* just for libmbedtls.a to be linked */
void iot_ssl_load()
{
    return;
}