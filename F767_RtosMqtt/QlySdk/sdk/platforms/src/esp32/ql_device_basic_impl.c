#include "esp_wifi.h"
#include "esp_smartconfig.h"
#include "ssl_lib.h"
#include "ssl.h"
#include "esp_spi_flash.h"
#include "sockets.h"
#include "api.h"
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

#define FLASH_DATA_BASEADDR   0x3FB
#define FLASH_SEC_SIZE  SPI_FLASH_SEC_SIZE
#define FLASH_SYS_SIZE  32
#define FLASH_USER_SIZE (FLASH_SEC_SIZE - 32)

typedef struct
{
    type_u8 flag;
    type_u8 pad[3];
} SAVE_FLAG;

SAVE_FLAG g_stSaveFlag = {1, {0, 0, 0}};

int IRAM_ATTR ql_persistence_data_save( const void* data, unsigned int data_len )
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

int IRAM_ATTR ql_persistence_data_load( void* data, unsigned int data_len )
{
    if(NULL == data || data_len > FLASH_SEC_SIZE)
    {
		ql_log_err(ERR_EVENT_INTERFACE, "data == NULL or data_len =%d", data_len);
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
    esp_err_t error = ESP_OK;

    error = esp_wifi_get_mac(ESP_IF_WIFI_STA, mac_arr);

    if(error != ESP_OK)
    {
		ql_log_err(ERR_EVENT_INTERFACE, "udp sock created err ==%d\r\n", error);
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
    ip_addr_t target_ip;

    if(NULL == name || NULL == ip)
    {
        return -1;
    }
    
    ret = netconn_gethostbyname(name, &target_ip);
    if(0 != ret)
    {
        return -1;
    }

    ntohl(target_ip.u_addr.ip4.addr);
    inet_ntoa_r(target_ip.u_addr.ip4.addr, ip, len);

    return 0;
}

ql_udp_socket_t *ql_udp_socket_create(unsigned int local_port)
{
    ql_udp_socket_t *socket = NULL;

    socket = (ql_udp_socket_t *)ql_malloc(sizeof(ql_udp_socket_t));
    ql_memset(socket, 0, sizeof(ql_udp_socket_t));
    socket->sockfd = -1;
    socket->local_port = local_port;

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
    struct	sockaddr_in server_addr = {0};
    int err = 0;

    if (sock == NULL)
    {
        return -1;
    }

    server_addr.sin_family = AF_INET;			
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(sock->local_port);

    sock->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock->sockfd == -1)	
    {
        ql_log_err(ERR_EVENT_INTERFACE, "udp create err");
        return -1;
    }

    err = bind(sock->sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if (err < 0) 
    {
        ql_log_err(ERR_EVENT_INTERFACE, "udp bind err");
        return -1;
    }

    return 0;
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

#define WEAK_S  __attribute__((weak))
WEAK_S ql_socket_t * ql_tcp_socket_create_ssl();
WEAK_S void ql_tcp_socket_destroy_ssl(ql_socket_t **sock);
WEAK_S int ql_tcp_connect_ssl(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms);
WEAK_S int ql_tcp_disconnect_ssl(ql_socket_t *sock);
WEAK_S int ql_tcp_send_ssl(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms);
WEAK_S int ql_tcp_recv_ssl(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms);


ql_socket_t * ql_tcp_socket_create()
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return ql_tcp_socket_create_ssl();
    else
        return _ql_tcp_socket_create();
}

void ql_tcp_socket_destroy(ql_socket_t **sock)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        ql_tcp_socket_destroy_ssl(sock);
    else
        _ql_tcp_socket_destroy(sock);
}

int ql_tcp_connect(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return ql_tcp_connect_ssl(sock, ip, port, timeout_ms);
    else
        return _ql_tcp_connect(sock, ip, port, timeout_ms);
}

int ql_tcp_disconnect(ql_socket_t *sock)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return ql_tcp_disconnect_ssl(sock);
    else
        return _ql_tcp_disconnect(sock);
}

int ql_tcp_send(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return ql_tcp_send_ssl(sock, buf, len, timeout_ms);
    else
        return _ql_tcp_send(sock, buf, len, timeout_ms);
}

int ql_tcp_recv(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
        return ql_tcp_recv_ssl(sock, buf, size, timeout_ms);
    else
        return _ql_tcp_recv(sock, buf, size, timeout_ms);
}

int32_t ql_set_wifi_info(uint8_t mode, wifi_info_t* info)
{
    wifi_config_t config;

    if(NULL == info)
    {
        return -1;
    }

    if(STATION_MODE == mode )
    {
        tcpip_adapter_init();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        
        esp_wifi_set_mode(WIFI_MODE_STA);
   
        if( esp_wifi_get_config(ESP_IF_WIFI_STA, &config) )
        {       
            if((0 == strcmp(config.sta.ssid, info->ssid)) && (0 == strcmp(config.sta.password, info->password)))
            {
                ql_log_warn("this wifi is same as before config \r\n");            
                return 0;
            }
            else
            {
                ql_memcpy(config.sta.ssid, info->ssid , ql_strlen(info->ssid)+1);
                ql_memcpy(config.sta.password, info->password , ql_strlen(info->password)+1);
                
                if(ESP_OK == esp_wifi_set_config(ESP_IF_WIFI_STA, &config))
                {
                    ql_log_info("[NEW] ssid:%s password:%s success!\r\n",config.sta.ssid,config.sta.password);
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
        esp_smartconfig_stop();
        tcpip_adapter_init();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        esp_wifi_set_mode(WIFI_MODE_AP);

        ql_memset(&config, 0x00,sizeof(wifi_config_t));
    
        if( esp_wifi_get_config(ESP_IF_WIFI_AP, &config) )
        {        
            if((0 == strcmp(config.ap.ssid, info->ssid)) && (0 == strcmp(config.ap.password, info->password)))
            {        
                ql_log_warn("this wifi is same as before config \r\n");            
                return 0;
            }
            else
            {               
                ql_memcpy(config.ap.ssid, info->ssid , ql_strlen(info->ssid)+1);
                //ql_log_info("ql_strlen(info->password) = %d ,info->password = %s\r\n",ql_strlen(info->password),info->password);
                if(0 == ql_strlen(info->password))
                {
                    ql_memset(config.ap.password, 0x00, sizeof(config.ap.password));
                    config.ap.authmode    =   WIFI_AUTH_OPEN;
                }
                else
                {
                    ql_memcpy(config.ap.password, info->password , ql_strlen(info->password)+1);
                    config.ap.authmode    =   WIFI_AUTH_WPA_WPA2_PSK;
                }
                config.ap.ssid_len    =   0;       
                config.ap.max_connection  =   1;  
                
                if(ESP_OK == esp_wifi_set_config(ESP_IF_WIFI_AP, &config))
                {
                    ql_log_info("[NEW] ssid:%s password:%s success!\r\n",config.ap.ssid,config.ap.password);
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

int32_t ql_get_wifi_info(int32_t mode,wifi_info_t* info)
{
    wifi_config_t config;

    if(NULL == info)
    {
        return -1;
    }
    
    if(STATION_MODE == mode )
    {
        if( esp_wifi_get_config(ESP_IF_WIFI_STA, &config))
        {   
            ql_memcpy(info->ssid, config.sta.ssid , ql_strlen(config.sta.ssid)+1);
            ql_memcpy(info->password, config.sta.password , ql_strlen(config.sta.password)+1);
            //printf("get wifi: ssid: %s ,password: %s \r\n",info->ssid,info->password);
        }
        else
        {
            ql_log_warn("STA WiFi is not config \r\n");
        }
    }
    else if(SOFTAP_MODE == mode)
    {
        if( esp_wifi_get_config(ESP_IF_WIFI_AP, &config) )
        {   
            ql_memcpy(info->ssid, config.ap.ssid , ql_strlen(config.ap.ssid)+1);
            ql_memcpy(info->password, config.ap.password , ql_strlen(config.ap.password)+1);
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
