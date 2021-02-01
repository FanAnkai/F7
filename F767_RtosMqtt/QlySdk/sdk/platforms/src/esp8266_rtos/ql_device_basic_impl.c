#include "FreeRTOSConfig.h"
#include "lwip/ip4_addr.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_libc.h"
#include "spi_flash.h"
#include "sockets.h"
#include "portmacro.h"
#include "openssl/ssl.h"

#include "ql_include.h"
#include "ql_device_basic_impl.h"
#include "ql_log.h"

void *ql_malloc(unsigned int size)
{
    return malloc(size);
}

void *ql_zalloc(unsigned int size)
{
    void* p = ql_malloc(size);
    if(p)
    {
        ql_memset(p, 0, size);
    }
    return p;
}

void ql_free(void *p)
{
    return free(p);
}

void ql_memcpy(void *dst, const void *src, unsigned int n)
{
    memcpy(dst, src, n);
    return;
}

void ql_memset(void *p, int c, unsigned int n)
{
    memset(p, c, n);
    return;
}
/*
 *   p1<p2  return < 0
 *   p1=p2  return    0
 *   p1>p2  return > 0
*/
int ql_memcmp(const void *p1, const void *p2, unsigned int n)
{
    return memcmp(p1, p2, n);
}

#ifdef LOCAL_SAVE
/*
* flash data-area map
*
            |-----------------------4096Byte---------------------------|
0xF8000 |------------------------certificate-------------------------|
0xF9000 |-sys1 32B-|---------------user1 4064B----------------------|
0xFA000 |-sys2 32B-|---------------user2 4064B----------------------|
0xFB000 |------------------------save flag--------------------------|
*/

#define FLASH_DATA_BASEADDR   0xF8

#define FLASH_SEC_SIZE  SPI_FLASH_SEC_SIZE
#define FLASH_SYS_SIZE  32
#define FLASH_USER_SIZE (FLASH_SEC_SIZE - 32)

typedef struct
{
    uint8 flag;
    uint8 pad[3];
} SAVE_FLAG;

SAVE_FLAG g_stSaveFlag = {1, {0, 0, 0}};

int ICACHE_FLASH_ATTR ql_persistence_data_save( const void* data, unsigned int data_len )
{
    if(NULL == data || data_len > FLASH_SEC_SIZE)
    {
        return -1;
    }

    spi_flash_read((FLASH_DATA_BASEADDR + 3) * FLASH_SEC_SIZE, (unsigned int *)&g_stSaveFlag, sizeof(SAVE_FLAG));

    if(1 == g_stSaveFlag.flag) /* if section 1 is used currently, save to section 2 */
    {
        spi_flash_erase_sector(FLASH_DATA_BASEADDR + 2);
        spi_flash_write((FLASH_DATA_BASEADDR + 2) * FLASH_SEC_SIZE, (unsigned int *)data, data_len);

        /* set save-flag */
        g_stSaveFlag.flag = 2;
        spi_flash_erase_sector(FLASH_DATA_BASEADDR + 3);
        spi_flash_write((FLASH_DATA_BASEADDR + 3) * FLASH_SEC_SIZE, (unsigned int *)&g_stSaveFlag, sizeof(SAVE_FLAG));
    }
    else
    {
        spi_flash_erase_sector(FLASH_DATA_BASEADDR + 1);
        spi_flash_write((FLASH_DATA_BASEADDR + 1) * FLASH_SEC_SIZE, (unsigned int *)data, data_len);

         /* set save-flag */
        g_stSaveFlag.flag = 1;
        spi_flash_erase_sector(FLASH_DATA_BASEADDR + 3);
        spi_flash_write((FLASH_DATA_BASEADDR + 3) * FLASH_SEC_SIZE, (unsigned int *)&g_stSaveFlag, sizeof(SAVE_FLAG));
    }
    

    return 0;
}

int ICACHE_FLASH_ATTR ql_persistence_data_load( void* data, unsigned int data_len )
{
    if(NULL == data || data_len > FLASH_SEC_SIZE)
    {
        os_printf("data == NULL or data_len =%d",data_len);
        return -1;
    }
    
    spi_flash_read((FLASH_DATA_BASEADDR + 3) * FLASH_SEC_SIZE, (unsigned int *)&g_stSaveFlag, sizeof(SAVE_FLAG));

    if(1 == g_stSaveFlag.flag)
    {
        spi_flash_read((FLASH_DATA_BASEADDR + 1) * FLASH_SEC_SIZE, (unsigned int *)data, data_len);
    }
    else
    {
        spi_flash_read((FLASH_DATA_BASEADDR + 2) * FLASH_SEC_SIZE, (unsigned int *)data, data_len);
    }
    return 0;
}
#endif

int qlcloud_get_mac_arr(unsigned char mac_arr[6])
{
    if(!wifi_get_macaddr(STATION_IF, mac_arr))
    {
        return -1;
    }
    
    return 0;
}

unsigned int ql_get_current_sysclock(void)
{
    return system_get_time()/1000000;
}

static unsigned int randSeed = 1;
unsigned int ql_random(void)
{
    randSeed = randSeed * 1103515245 + 12345;
    return((unsigned)(randSeed/65536) % 32768);
}
unsigned int ql_srand(unsigned int seed)
{
    randSeed = seed;

    return 0;
}

void ql_sleep_ms(unsigned int ms)
{
    vTaskDelay(ms / portTICK_RATE_MS);
}

void ql_sleep_s(unsigned int s)
{
    vTaskDelay((s * 1000) / portTICK_RATE_MS);
}

int ql_gethostbyname(const char *name, char *ip, char len)
{
    int ret;
    //ip_addr_t target_ip;
    unsigned int target_ip;

    if(NULL == name || NULL == ip)
    {
        return -1;
    }
    
    ret = netconn_gethostbyname(name, (ip_addr_t*)&target_ip);
    if(0 != ret)
    {
        return -1;
    }

    ntohl(target_ip);
    inet_ntoa_r(target_ip, ip, len);
    
    return 0;
}

ql_udp_socket_t *ql_udp_socket_create(unsigned int local_port)
{
    ql_udp_socket_t *socket = NULL;

    socket = (ql_udp_socket_t *)ql_malloc(sizeof(ql_udp_socket_t));
    ql_memset(socket, 0, sizeof(ql_udp_socket_t));
    socket->sockfd = -1;
    socket->local_port = local_port;

#if (__SDK_TYPE__== QLY_SDK_BIN)
    if(SOFTAP_MODE == ql_get_wifi_opmode())
    {
        socket->remote_ip = inet_addr("100.5.168.192");
        socket->remote_port = 30000;
        #ifdef DEBUG_DATA_OPEN
            ql_printf("%s , %d ip:%s, port:%d\r\n", __func__, __LINE__, inet_ntoa(socket->remote_ip), socket->remote_port);
        #endif
    }
#endif
    return socket;
}

void ql_udp_socket_destroy(ql_udp_socket_t **sock)
{
    if (sock && *sock) {
        ql_free(*sock);
    }

    *sock = NULL;
}

int ql_udp_bind(ql_udp_socket_t *sock)
{
    if (sock == NULL)
    {
        return -1;
    }

    struct	sockaddr_in server_addr;
    ql_memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;							
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(sock->local_port);
    server_addr.sin_len = sizeof(server_addr);
    
    sock->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock->sockfd == -1)	
    {
        ql_log_err(ERR_EVENT_INTERFACE, "udp create err");
        return -1;
    }

    if(0 != bind(sock->sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        ql_log_err(ERR_EVENT_INTERFACE, "udp bind err");
        return -1;
    }
}

int ql_udp_close(ql_udp_socket_t *sock)
{
    if (sock && sock->sockfd != -1) {
        close(sock->sockfd);
        sock->sockfd = -1;
    }
    return 0;
}

int ql_udp_send(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int ret = -1;
    struct sockaddr_in to;
    socklen_t tolen;
    int nNetTimeout = timeout_ms;

    setsockopt(sock->sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));

    ql_memset(&to, 0, sizeof(to));
    tolen = sizeof(struct sockaddr_in);
    to.sin_len = sizeof(struct sockaddr_in);
    to.sin_family = AF_INET;	
    to.sin_addr.s_addr = htonl(sock->remote_ip);
    to.sin_port = htons(sock->remote_port);
    ret = sendto(sock->sockfd, buf, len, 0, (struct sockaddr *)&to, tolen);
    if(ret >= 0)
    {
        if(ret != len)
        {
            ql_log_err(ERR_EVENT_TX_DATA, "udp_s:len:%d,ret:%d", len, ret);
        }
    }
    else
    {
        ql_log_err(ERR_EVENT_TX_DATA,"udp_s,err:%d", ret);
        ret = -1;
    }

    return ret;
}

int ql_udp_recv(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int ret = -1;
    struct sockaddr_in from;
    socklen_t fromlen;
    //int nNetTimeout = timeout_ms;
    fd_set readfds;
    struct timeval tv;

    FD_ZERO(&readfds);
    FD_SET(sock->sockfd, &readfds);    
    tv.tv_sec = timeout_ms / 1000;    
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(sock->sockfd + 1, &readfds, NULL, NULL, &tv);
    if(ret >= 0)
    {
        //setsockopt(sock->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
        if(FD_ISSET(sock->sockfd, &readfds))
        {
            ql_memset(&from, 0, sizeof(from));
            fromlen = sizeof(struct sockaddr_in);
            ret = recvfrom(sock->sockfd, buf, len, 0, (struct sockaddr *)&from, &fromlen);
            if(ret > 0)
            {
                sock->remote_ip = ntohl(from.sin_addr.s_addr);
                sock->remote_port = ntohs(from.sin_port);
            }
            else
            {
                ql_log_err(ERR_EVENT_RX_DATA, "udp_r err:%d", ret);
                ret = -1;
            }
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
        ql_log_err(ERR_EVENT_RX_DATA, "udp_r select err:%d", ret);
        ret = -1;
    }

    return ret;
}

struct ql_socket_t {
    int sockfd;
    SSL_CTX *ctx;
    SSL *ssl;
};

ql_socket_t * _ql_tcp_socket_create()
{
    ql_socket_t *socket = NULL;

    socket = (ql_socket_t *)ql_malloc(sizeof(ql_socket_t));

    return socket;
}

void _ql_tcp_socket_destroy(ql_socket_t **sock)
{
    if (sock && *sock) {
        ql_free(*sock);
    }

    *sock = NULL;
}

int _ql_tcp_connect(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms)
{
    struct sockaddr_in addr;

    if(NULL == sock || NULL == ip) 
    {
        return -1;
    }

#ifdef DEBUG_DATA_OPEN
    //ql_printf("ql_tcp_connect %s:%d\r\n", ip, port);
#endif
    sock->sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock->sockfd == -1) {
        ql_log_err(ERR_EVENT_INTERFACE, "tcp socket err");
        close(sock->sockfd);
        return -1;
    }

    ql_memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr	= inet_addr(ip);
    
    if(0 != connect(sock->sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)))
    {
        ql_log_err(ERR_EVENT_INTERFACE, "tcp conn err");
        close(sock->sockfd);
        sock->sockfd = -1;
        return -1;
    }

    return 0;
}

int _ql_tcp_disconnect(ql_socket_t *sock)
{
    if (sock && sock->sockfd != -1) {
        close(sock->sockfd);
        sock->sockfd = -1;
    }
    return 0;
}

int _ql_tcp_send(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int ret = -1;
    int nNetTimeout = timeout_ms;

    setsockopt(sock->sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));
    
    ret = write(sock->sockfd, buf, len) ;
    if(ret >= 0)
    {
        if(ret != len)
        {
            ql_log_err(ERR_EVENT_TX_DATA, "tcp_s len:%d,ret:%d", len, ret);
        }
    }
    else
    {
        ql_log_err(ERR_EVENT_TX_DATA, "tcp_s err:%d", ret);
        ret = -1;
    }

    return ret;
}

int _ql_tcp_recv(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
{
    int ret = -1;
    int nNetTimeout = timeout_ms;
    fd_set readfds;
    struct timeval tv;

    FD_ZERO(&readfds);
    FD_SET(sock->sockfd, &readfds);    
    tv.tv_sec = timeout_ms / 1000;    
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(sock->sockfd + 1, &readfds, NULL, NULL, &tv);
    if(ret >= 0)
    {
        if(FD_ISSET(sock->sockfd, &readfds))
        {
            //setsockopt(sock->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
            ret = read(sock->sockfd, buf, size);
            if(ret >= 0)
            {
                
            }
            else
            {
                ql_log_err(ERR_EVENT_RX_DATA, "tcp_r err:%d", ret);
                ret = -1;
            }
        }
    }
    else
    {
        ql_log_err(ERR_EVENT_RX_DATA, "tcp_r select err:%d", ret);
        ret = -1;
    }

    return ret;
}

#define VERIFY_SERVER_CERTIFICATE

#ifdef VERIFY_SERVER_CERTIFICATE
const unsigned char ca_crt[] = {
  0x30, 0x82, 0x02, 0xcc, 0x30, 0x82, 0x01, 0xb4, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x09, 0x00, 0xf5, 0x79, 0x3f, 0x7a, 0x91, 0x95, 0x95, 0xa6,
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
  0x0b, 0x05, 0x00, 0x30, 0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55,
  0x04, 0x03, 0x13, 0x0b, 0x71, 0x69, 0x6e, 0x67, 0x6c, 0x69, 0x61, 0x6e,
  0x79, 0x75, 0x6e, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x36, 0x30, 0x37, 0x31,
  0x33, 0x30, 0x39, 0x34, 0x33, 0x34, 0x31, 0x5a, 0x17, 0x0d, 0x32, 0x36,
  0x30, 0x37, 0x31, 0x31, 0x30, 0x39, 0x34, 0x33, 0x34, 0x31, 0x5a, 0x30,
  0x16, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x0b,
  0x71, 0x69, 0x6e, 0x67, 0x6c, 0x69, 0x61, 0x6e, 0x79, 0x75, 0x6e, 0x30,
  0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
  0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30,
  0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xc0, 0x71, 0xa2, 0x32,
  0xa0, 0xb2, 0x31, 0x47, 0x0c, 0xf5, 0x3e, 0x4c, 0x29, 0x93, 0x25, 0x9f,
  0xe6, 0x4a, 0x12, 0x6d, 0xea, 0xfd, 0x11, 0x0b, 0x7d, 0x91, 0x16, 0xff,
  0xbb, 0x47, 0x1d, 0xfe, 0x3d, 0xc8, 0x5f, 0x6c, 0x30, 0x63, 0x20, 0x87,
  0xb4, 0xc3, 0x10, 0x87, 0x9a, 0x3b, 0x5b, 0xfc, 0x99, 0xa7, 0x79, 0x10,
  0x7e, 0x44, 0x30, 0x14, 0x5f, 0x50, 0x8f, 0x91, 0x54, 0xb6, 0x74, 0xe3,
  0xe8, 0x67, 0xcb, 0x39, 0xef, 0x1e, 0x2c, 0x53, 0x81, 0x4c, 0x91, 0x7e,
  0x14, 0x2a, 0x5c, 0xf7, 0x87, 0x40, 0x4d, 0xfd, 0x26, 0x99, 0xea, 0x41,
  0xed, 0x53, 0xa0, 0x48, 0x60, 0x17, 0x63, 0xd6, 0xc2, 0x27, 0xab, 0x39,
  0x6e, 0x57, 0xc6, 0x28, 0x08, 0x4c, 0xc1, 0x89, 0x3b, 0x66, 0xd5, 0xbe,
  0x34, 0xdd, 0xb9, 0x46, 0x5e, 0x6e, 0x55, 0xb6, 0x7b, 0x17, 0x18, 0x2b,
  0x75, 0x98, 0x61, 0x6a, 0x4c, 0x63, 0xf7, 0xaa, 0xe8, 0x4d, 0xf3, 0x9f,
  0x52, 0x78, 0x37, 0xad, 0x5b, 0xa1, 0xe5, 0xb1, 0xab, 0x20, 0x1c, 0xe6,
  0x69, 0x2d, 0x10, 0x91, 0xa1, 0x19, 0x83, 0x6e, 0xe0, 0x92, 0x28, 0xf5,
  0xcd, 0x65, 0xaa, 0xf8, 0x60, 0x62, 0xda, 0x81, 0x36, 0xb0, 0x46, 0x1b,
  0x32, 0x66, 0xb1, 0x3f, 0xa3, 0xf8, 0x5a, 0x34, 0xdc, 0xc8, 0xed, 0x5b,
  0x43, 0x6b, 0x1c, 0x67, 0xfe, 0x1f, 0x73, 0x30, 0x52, 0x08, 0x02, 0x30,
  0x66, 0xa8, 0xfe, 0x00, 0xec, 0xc9, 0x8b, 0x0b, 0xb6, 0x1f, 0xe1, 0xa0,
  0x25, 0x29, 0xfb, 0x25, 0x66, 0x0a, 0x3d, 0x80, 0x41, 0x40, 0x66, 0xfe,
  0xd9, 0x67, 0x4a, 0xa8, 0x81, 0xe0, 0x39, 0x57, 0x90, 0x2a, 0x81, 0x5c,
  0xeb, 0x5a, 0x3c, 0x02, 0xe2, 0x1e, 0x5d, 0x5d, 0x87, 0x74, 0xce, 0x7d,
  0x3c, 0xbd, 0xbb, 0xb7, 0x18, 0xbb, 0x51, 0xde, 0x9b, 0xbd, 0x00, 0x23,
  0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x1d, 0x30, 0x1b, 0x30, 0x0c, 0x06,
  0x03, 0x55, 0x1d, 0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30,
  0x0b, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06,
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
  0x0b, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x89, 0xbb, 0x35, 0x50,
  0x06, 0x3b, 0xbc, 0x3c, 0xb3, 0xc5, 0xcd, 0x7a, 0xe2, 0x3c, 0x58, 0x73,
  0x57, 0xe6, 0x86, 0x10, 0x80, 0x61, 0x76, 0x33, 0x63, 0x85, 0xdf, 0xf8,
  0x35, 0x4f, 0x12, 0x64, 0xa7, 0x94, 0x7e, 0xdf, 0x04, 0x26, 0x8d, 0x5d,
  0xfa, 0xf3, 0x30, 0x26, 0x11, 0xfb, 0xbb, 0x98, 0x9b, 0x74, 0x66, 0x04,
  0xde, 0x4d, 0x22, 0xf8, 0x6d, 0xae, 0x2e, 0x25, 0x32, 0xd2, 0x52, 0x52,
  0xe3, 0xea, 0x6f, 0xa0, 0x70, 0xdc, 0x07, 0x8b, 0x0f, 0x28, 0x00, 0x2f,
  0xc3, 0xab, 0xf3, 0x2f, 0x5a, 0x9a, 0x44, 0x87, 0x93, 0x69, 0xcd, 0x30,
  0x1a, 0x71, 0x59, 0xb7, 0x15, 0xb4, 0xfe, 0xcd, 0xd0, 0x0c, 0x9e, 0xa2,
  0x34, 0x36, 0xf1, 0x49, 0x9b, 0xf7, 0x13, 0x9c, 0x29, 0x52, 0xfc, 0x1e,
  0x19, 0xb9, 0x78, 0xde, 0xa5, 0x57, 0xdc, 0xae, 0xa2, 0xcd, 0xeb, 0x77,
  0x98, 0x27, 0x9e, 0x1f, 0x14, 0x02, 0xee, 0x9a, 0x7a, 0x7a, 0x77, 0xe1,
  0xdd, 0xad, 0x84, 0xfd, 0xe1, 0xc6, 0x46, 0x0c, 0x28, 0x87, 0x17, 0x06,
  0x94, 0x07, 0xe3, 0x15, 0x2f, 0x55, 0xef, 0x19, 0x88, 0x5d, 0xf3, 0xca,
  0x46, 0xcb, 0xe9, 0xb2, 0x38, 0xda, 0x21, 0x93, 0x69, 0x95, 0xfd, 0x6c,
  0xa0, 0x60, 0x7b, 0xce, 0x37, 0xba, 0x1e, 0x01, 0x62, 0x54, 0xeb, 0x8f,
  0x63, 0x43, 0xf6, 0xb1, 0x78, 0x3e, 0x65, 0x65, 0x53, 0xa2, 0xb0, 0xb9,
  0xdf, 0xdd, 0x64, 0x57, 0x4a, 0x79, 0xc7, 0xc6, 0x9e, 0x7c, 0x62, 0x2f,
  0xb3, 0x80, 0x9b, 0xec, 0x0f, 0x0f, 0x93, 0xcf, 0xea, 0x12, 0x39, 0x65,
  0xfb, 0xd2, 0xda, 0x04, 0x81, 0x61, 0xa6, 0xe8, 0x46, 0xff, 0x13, 0x5f,
  0x48, 0x3d, 0xdb, 0x46, 0xa8, 0xad, 0xb4, 0x20, 0x14, 0xe1, 0x02, 0xdd,
  0x2d, 0xdb, 0x23, 0x45, 0x5a, 0xb9, 0x0a, 0x7c, 0xf3, 0x9e, 0xc8, 0x81
};
unsigned int ca_crt_len = 720;
#endif

#define OPENSSL_FRAGMENT_SIZE  2048
ql_socket_t * _ql_tcp_socket_create_ssl()
{
    ql_socket_t *socket = NULL;
    SSL_METHOD *meth = NULL;
    SSL_CTX *ctx = NULL;

    socket = (ql_socket_t *)ql_malloc(sizeof(ql_socket_t));

    meth = (SSL_METHOD *)TLSv1_1_client_method();
    if (meth == NULL) 
    {
        ql_free(socket);
        return NULL;
    }

    ctx = SSL_CTX_new(meth);
    if (ctx == NULL) 
    {
        ql_free(socket);
        return NULL;
    }
    socket->ctx = ctx;
#ifdef VERIFY_SERVER_CERTIFICATE
    X509 *cacrt = d2i_X509(NULL, ca_crt, ca_crt_len);
    if(cacrt){
        SSL_CTX_add_client_CA(ctx, cacrt);
        
#ifdef DEBUG_DATA_OPEN
        ql_printf("load ca crt OK\n");
#endif
    }
    else{
        ql_free(socket);
        return NULL;
    }
    //SSL_CTX_load_verify_locations(ctx, "./cacert.pem", NULL);
    
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
#else
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
#endif

    SSL_CTX_set_default_read_buffer_len(ctx, OPENSSL_FRAGMENT_SIZE);

    return socket;
}

void _ql_tcp_socket_destroy_ssl(ql_socket_t **sock)
{
    if (sock && *sock) 
    {
        if ((*sock)->ctx)
        {
            SSL_CTX_free((*sock)->ctx);
            (*sock)->ctx = NULL;
        }
        ql_free(*sock);
    }

    *sock = NULL;
}

int _ql_tcp_connect_ssl(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms)
{
    struct sockaddr_in addr;
    int retval;

    if (sock == NULL || NULL == ip) 
    {
        return -1;
    }
#ifdef DEBUG_DATA_OPEN
    //ql_printf("ql_tcp_connect %s:%d\r\n", ip, port);
#endif
    sock->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock->sockfd == -1) 
    {
        ql_log_err(ERR_EVENT_INTERFACE, "ssl sock err");
        close(sock->sockfd);
        return -1;
    }

    ql_memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr	= inet_addr(ip);

    if (connect(sock->sockfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) 
    {
        ql_log_err(ERR_EVENT_INTERFACE, "ssl conn err:%d", errno);
        close(sock->sockfd);
        sock->sockfd = -1;
        return -1;
    }

    SSL *ssl = NULL;
    ssl = SSL_new(sock->ctx);
    if (ssl == NULL)
    {
        ql_log_err(ERR_EVENT_INTERFACE, "ssl_new err");
        close(sock->sockfd);
        sock->sockfd = -1;
        return -1;
    }

    if (0 == SSL_set_fd(ssl, sock->sockfd))
    {
        ql_log_err(ERR_EVENT_INTERFACE, "ssl_set_fd failed.");
        SSL_free(ssl);
        ssl = NULL;
        close(sock->sockfd);
        sock->sockfd = -1;
        return -1;
    }

    retval = SSL_connect(ssl);
    if (retval != 1) 
    {
        ql_log_err(ERR_EVENT_INTERFACE, "ssl_conn err:%d", SSL_get_error(ssl, retval));
        SSL_free(ssl);
        ssl = NULL;
        close(sock->sockfd);
        sock->sockfd = -1;
        return -1;
    }

    sock->ssl = ssl;

    return 0;
}

int _ql_tcp_disconnect_ssl(ql_socket_t *sock)
{
    if (sock)
    {
        if (sock->ssl) 
        {
            SSL_shutdown(sock->ssl);
            SSL_free(sock->ssl);
            sock->ssl = NULL;
        }
        if (sock->sockfd != -1) 
        {
            close(sock->sockfd);
            sock->sockfd = -1;
        }
    }

    return 0;
}

int _ql_tcp_send_ssl(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int retval;
    int sentlen = 0;
    int totallen = 0;
    int err = 0;

    if (sock == NULL || sock->sockfd == -1 || sock->ssl == NULL) 
    {
        return -1;
    }
#ifdef DEBUG_DATA_OPEN

    /*
    int i = 0;
    ql_printf("_ql_tcp_send_ssl %d:\r\n", len);
    for(i = 0; i < len; i ++)
    {
        ql_printf("%c", buf[i]);
    }
    ql_printf("\r\n");
    */
#endif
    totallen = len;
    while (sentlen < totallen) 
    {
        retval = SSL_write(sock->ssl, buf + sentlen, totallen - sentlen);
        if (retval <= 0) 
        {
            ql_log_err(ERR_EVENT_TX_DATA, "ssl_s err:-0x%02x", -retval);
            err = SSL_get_error(sock->ssl, retval);
            if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ)
            {
                return 0;
            }
            else 
            {
                return -1;
            }
        }
        sentlen += retval;
    }

    return totallen;
}

int _ql_tcp_recv_ssl(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
{
    int retval;
    int err;
    int nNetTimeout = timeout_ms;

    setsockopt(sock->sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
    
    retval = SSL_read(sock->ssl, buf, size);
    if (retval < 0) 
    {
        err = SSL_get_error(sock->ssl, retval);
        if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) 
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else if (retval == 0)
    {
        return -1;
    }

    return retval;
}

ql_socket_t * ql_tcp_socket_create()
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return _ql_tcp_socket_create_ssl();
    else
        return _ql_tcp_socket_create();
}

void ql_tcp_socket_destroy(ql_socket_t **sock)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        _ql_tcp_socket_destroy_ssl(sock);
    else
        _ql_tcp_socket_destroy(sock);
}

int ql_tcp_connect(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return _ql_tcp_connect_ssl(sock, ip, port, timeout_ms);
    else
        return _ql_tcp_connect(sock, ip, port, timeout_ms);
}

int ql_tcp_disconnect(ql_socket_t *sock)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return _ql_tcp_disconnect_ssl(sock);
    else
        return _ql_tcp_disconnect(sock);
}

int ql_tcp_send(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return _ql_tcp_send_ssl(sock, buf, len, timeout_ms);
    else
        return _ql_tcp_send(sock, buf, len, timeout_ms);
}

int ql_tcp_recv(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return _ql_tcp_recv_ssl(sock, buf, size, timeout_ms);
    else
        return _ql_tcp_recv(sock, buf, size, timeout_ms);
}

struct station_config 
{
    uint8_t ssid [32];
    uint8_t password [64];
    uint8_t bssid_set;
    uint8_t bssid [6];
};

struct softap_config {
    uint8_t ssid[32];
    uint8_t password[64];
    uint8_t ssid_len;	// Note: Recommend to set it according to your ssid
    uint8_t channel;	// Note: support 1 ~ 13
    AUTH_MODE authmode;	// Note: Don't support AUTH_WEP in softAP mode.
    uint8_t ssid_hidden;	// Note: default 0
    uint8_t max_connection;	// Note: default 4, max 4
    uint8_t beacon_interval;	// Note: support 100 ~ 60000 ms, default 100
};

int32_t ql_set_wifi_info(uint8_t mode,wifi_info_t* info)
{

    if(NULL == info)
    {
        return -1;
    }
    if(STATION_MODE == mode )
    { 

        wifi_set_opmode(STATION_MODE);
        struct station_config config;
        wifi_station_dhcpc_set_maxtry(5);

        ql_memset(&config,0x00,sizeof( struct station_config));
        if( wifi_station_get_config(&config) )
        {       
            if((0 == strncmp(config.ssid,info->ssid,32)) && (0 == strncmp(config.password,info->password,64)))
            {        
                ql_log_warn("this wifi existed\r\n");            
                return 0;
            }
            else
            {     
                ql_memset(&config,0x00,sizeof(struct station_config));
                ql_memcpy(config.ssid, info->ssid , 32);
                ql_memcpy(config.password, info->password , 64);

                if(TRUE == wifi_station_set_config(&config))
                {
                    printf("[NEW]ssid:%s passwd:%s\r\n",config.ssid,config.password);
                }
                else
                {
                    ql_log_err(ERR_EVENT_INTERFACE, "station con err");
                    return -1;
                }    
            }
        }
    }
    else if(SOFTAP_MODE == mode)
    {     
        smartconfig_stop();
        wifi_set_opmode(SOFTAP_MODE);
        struct  softap_config config;
        ql_memset(&config, 0x00,sizeof(struct softap_config ));
    
        if( wifi_softap_get_config(&config) )
        {        
            if((0 == strcmp(config.ssid,info->ssid)) && (0 == strcmp(config.password,info->password)))
            {        
                ql_log_warn("this wifi is same as before config \r\n");            
                return 0;
            }
            else
            {               
                ql_memcpy(config.ssid, info->ssid , ql_strlen(info->ssid)+1);
                //ql_log_info("ql_strlen(info->password) = %d ,info->password = %s\r\n",ql_strlen(info->password),info->password);
                if(0 == ql_strlen(info->password))
                {
                    ql_memset(config.password, 0x00, sizeof(config.password));
                    config.authmode    =   AUTH_OPEN;
                }
                else
                {
                    ql_memcpy(config.password, info->password , ql_strlen(info->password)+1);
                    config.authmode    =   AUTH_WPA_WPA2_PSK;
                }
                config.ssid_len    =   0;       
                config.max_connection  =   1;  
                
                if(TRUE == wifi_softap_set_config(&config))
                {
                    ql_log_info("[NEW] ssid:%s password:%s success!\r\n",config.ssid,config.password);
                }
                else
                {
                    ql_log_err(ERR_EVENT_INTERFACE, "ap config err");
                    return -1;
                }                
            }
        }
    }   
    else
    {   
        return -1 ;   
    }

    return 0;
}

#if (__SDK_TYPE__== QLY_SDK_BIN)
int32_t ql_get_wifi_opmode()
{
    return wifi_get_opmode();
}
#endif

int32_t ql_get_wifi_info(int32_t mode,wifi_info_t* info)
{
    if(NULL == info)
    {
        return -1;
    }
    
    if(STATION_MODE == mode )
    {    
        struct station_config config;
        if( wifi_station_get_config(&config) )
        {   
            ql_memcpy(info->ssid, config.ssid , ql_strlen(config.ssid)+1);
            ql_memcpy(info->password, config.password , ql_strlen(config.password)+1);
            //printf("get wifi: ssid: %s ,password: %s \r\n",info->ssid,info->password);
        }
        else
        {
            ql_log_warn("STA WiFi is not config \r\n");
        }
    }
    else if(SOFTAP_MODE == mode)
    {
        struct softap_config config;
        if( wifi_softap_get_config(&config) )
        {   
            ql_memcpy(info->ssid, config.ssid , ql_strlen(config.ssid)+1);
            ql_memcpy(info->password, config.password , ql_strlen(config.password)+1);
            //printf("get wifi: ssid: %s ,password: %s \r\n",info->ssid,info->password);
        }
        else
        {
            ql_log_warn("AP WiFi is not config \r\n");
        }
    }
    else
    {   
        return -1 ;   
    }

    return 0;
}
