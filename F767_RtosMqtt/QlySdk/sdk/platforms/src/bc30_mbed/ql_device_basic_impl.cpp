#include "mbed.h"
#include "WiFiStackInterface.h"
#include "ql_device_basic_impl.h"
#include "rda5991h_wland.h"

#include "ql_log.h"

#define RDA5981_FLASH_ALIGNMENT               256
#define RDA5981_FLASH_SIZE_ALIGN(size)        (((size) + RDA5981_FLASH_ALIGNMENT - 1) & (~(RDA5981_FLASH_ALIGNMENT-1)))
#define RDA_FLASH_DATA_BASEADDR                   0x180fc000
#define RDA5981_FLASH_SEC_SIZE                4096

extern WiFiStackInterface* iot_network_stack;
TCPSocket t_socket;

void *ql_malloc(unsigned int size)
{
    return malloc(size);
}

void *ql_zalloc(unsigned int size)
{
    void* p = ql_malloc(size);
    if(p){}else{return p;}
    ql_memset(p, 0, size);
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


int ql_persistence_data_save( const void* data, unsigned int data_len )
{
    int ret = 0;
    int align_len = 0;

    //RDA5981 flash must be written through the entire page.So calculate the actual write length.
    align_len = RDA5981_FLASH_SIZE_ALIGN(data_len);

    //if the pointer is null or the actual len is overflow,return the err number -1.
    if(NULL == data || align_len > RDA5981_FLASH_SEC_SIZE)
    {
        return -1;
    }

    ret = rda5981_erase_flash(RDA_FLASH_DATA_BASEADDR, RDA5981_FLASH_SEC_SIZE);
    if(ret != 0)
    {
        return -1;
    }

    ret = rda5981_write_flash(RDA_FLASH_DATA_BASEADDR, (char *)data, align_len);
    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

int ql_persistence_data_load( void* data, unsigned int data_len )
{
    int ret = 0;
    if(NULL == data || data_len > RDA5981_FLASH_SEC_SIZE)
    {
        return -1;
    }

    ret = rda5981_read_flash(RDA_FLASH_DATA_BASEADDR, (char *)data, data_len);
    if (ret != 0)
    {
        return -1;
    }

    return 0;
}

int qlcloud_get_mac_arr(unsigned char mac_arr[6])
{
// for self test virtual mac
#if 0
    char  mac[] = {0x66,0x00,0x00,0x00,0x00,0x0C};
    ql_memcpy(mac_arr, mac, 6);
    return 0;
#endif
    if(NULL == iot_network_stack)
    {
        printf("get mac err \r\n");
        return -1;
    }
    const char *mac = iot_network_stack->get_mac_address();    
	sscanf(mac,"%2x:%2x:%2x:%2x:%2x:%2x",&mac_arr[0],&mac_arr[1],&mac_arr[2],&mac_arr[3],&mac_arr[4],&mac_arr[5]);
    
    return 0;
}

unsigned int ql_get_current_sysclock(void)
{
    return (unsigned int)time(NULL);
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
    wait_ms( ms );
}

void ql_sleep_s(unsigned int s)
{
    wait( s );
}

struct ql_udp_socket {
	UDPSocket*    	p_usock;
    char 			local_ip[16];
    unsigned short  local_port;
    char 			remote_ip[16];
    unsigned short  remote_port;
};

ql_udp_socket_t *ql_udp_socket_create(unsigned int local_port)
{
    ql_udp_socket_t *socket = NULL;
	
    socket = (ql_udp_socket_t *)ql_malloc(sizeof(ql_udp_socket_t));
	if (!socket) {
		return NULL;
	}
	memset(socket, 0, sizeof(ql_udp_socket_t));
    socket->p_usock = new UDPSocket();

	if (!socket->p_usock) {
	   ql_free(socket);
	   return NULL;
    }
	
	socket->local_port = local_port;
    
    return socket;
}

void ql_udp_socket_destroy(ql_udp_socket_t **sock)
{
    if (sock && *sock) {
		delete (*sock)->p_usock;
		ql_free(*sock);
    }

    *sock = NULL;
}

int ql_udp_bind(ql_udp_socket_t *sock)
{
	int ret = 0;
	int udp_broadcast = 1;

	ret = sock->p_usock->open(iot_network_stack);
	if(ret){
		return ret;
	}
	ret = sock->p_usock->bind(sock->local_port);
	if(ret){
		return ret;
	}
	ret = sock->p_usock->setsockopt(0, NSAPI_UDP_BROADCAST, &udp_broadcast, sizeof(udp_broadcast));
	if(ret){
		return ret;
	}
	
	return 0;
}
int ql_udp_close(ql_udp_socket_t *sock)
{
    return sock->p_usock->close();
}

int ql_udp_send(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    return sock->p_usock->sendto(sock->remote_ip, sock->remote_port, buf, len);
}

int ql_udp_recv(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
	int ret = 0;
	SocketAddress address;

    sock->p_usock->set_timeout(timeout_ms);
    ret = sock->p_usock->recvfrom(&address, buf, len);
    

    if(ret == NSAPI_ERROR_WOULD_BLOCK)
    {
        ret = 0;
        return 0;
    }

	ql_memcpy(sock->remote_ip, address.get_ip_address(), strlen(address.get_ip_address()));
	sock->remote_port = address.get_port();
    return ret;
}

struct ql_socket_t {
	TCPSocket* p_sock;
};

ql_socket_t *ql_tcp_socket_create(void)
{
    ql_socket_t *socket = NULL;
    
    socket = (ql_socket_t *)ql_malloc(sizeof(ql_socket_t));
    if (!socket) {
        return NULL;
    }
    socket->p_sock = new TCPSocket();
    if (!socket->p_sock) {
        ql_free(socket);
        return NULL;
    }
    return socket;
}

void ql_tcp_socket_destroy(ql_socket_t **sock)
{
    if (sock && *sock) {
		delete (*sock)->p_sock;
        ql_free(*sock);
    }

    *sock = NULL;
}

int ql_tcp_connect(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms)
{
	int ret = 0;
	ret = sock->p_sock->open(iot_network_stack);
	if (ret){
		return ret;
	}
	ret = sock->p_sock->connect((const char*)ip, port);
	sock->p_sock->set_timeout(timeout_ms);
	return ret;
}

int ql_tcp_disconnect(ql_socket_t *sock)
{
    return sock->p_sock->close();
}

int ql_tcp_send(ql_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int ret = 0;
    sock->p_sock->set_timeout(timeout_ms);
    ret = sock->p_sock->send(buf, len);
    return ret;
}

int ql_tcp_recv(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
{
    int ret = 0;
    sock->p_sock->set_timeout(timeout_ms);
    ret = sock->p_sock->recv(buf, size);

    //printf("TCP recv size = %d\r\n",size);
    if(ret == NSAPI_ERROR_WOULD_BLOCK)
    {
        ret = 0;
        //printf("TCP recv time out,recv size = %d,ret = %d\r\n",size,ret);
    }

    return ret;
}
int ql_gethostbyname(const char *name, char *ip, char len)
{
	ql_strncpy(ip, name, len);
	return 0;
}

#define AP_IP       "192.168.5.1"
#define AP_MSK      "255.255.255.0"
#define AP_GW       "192.168.5.255"
#define AP_START    "192.168.5.100"
#define AP_END      "192.168.5.200"
char ssid_ap[32 + 1];
char passwd_ap[64 + 1];

int ql_set_wifi_info(unsigned char mode,wifi_info_t* info)
{    

    if(NULL == info)
    {
        return -1;
    }
    if(STATION_MODE == mode )
    {        
        iot_s32 ret;

        /*1.disconnect*/        
        if(iot_network_stack)
            iot_network_stack->disconnect();
        
        ret = rda5981_flash_read_sta_data(ssid_ap, passwd_ap);
        if (ret == 0 && strlen(ssid_ap)) 
        {
            if((0 == strcmp((const char *)ssid_ap,(const char *)info->ssid)) && (0 == strcmp((const char *)passwd_ap,(const char *)info->password)))
            {
               printf("new sta config is same as before\r\n");
               return 0;
            } 
        }
        //printf("SSID=%s,passwd=%s\r\n", SSID,passwd);
        rda5981_flash_write_sta_data((char *)info->ssid, (char *)info->password);

    }
    else if(SOFTAP_MODE == mode)
    {        
        unsigned char channel = 0;        
        unsigned int ret = 0;

        /*1.disconnect*/
        if(iot_network_stack)
            iot_network_stack->disconnect();
        
        /*2.set ap config*/
        ql_memset(ssid_ap, 0, sizeof(ssid_ap));
        ql_memset(passwd_ap, 0, sizeof(passwd_ap));
        
        if(!rda5981_flash_read_ap_data( ssid_ap, passwd_ap, &channel))
        {
                    
            if((0 == strcmp((const char *)ssid_ap,(const char *)info->ssid)) && (0 == strcmp((const char *)passwd_ap,(const char *)info->password)))
            {   
                printf("this wifi is same as before config \r\n");   
                return 0;
            }
            else
            {
                //printf("debug read_ap: ssid:%s,pwd:%s;\r\n new_ap:ssid:%s,pwd:%s \r\n",ssid_ap,passwd_ap,info->ssid,info->password);
                ql_memcpy(ssid_ap,info->ssid,ql_strlen((const char *)info->ssid)+1);                
                ql_memcpy(passwd_ap,info->password,ql_strlen((const char *)info->password)+1);
                channel = 6;

                /*3.save to flash*/
                ret = rda5981_flash_write_ap_data(ssid_ap, passwd_ap, channel);
                return ret;
            }
        }    
        else
        {
            printf("read_ap err\r\n");            
            return -1 ;   
        }
        
    }   
    else
    {   
        return -1 ;   
    }    
    return 0;
}




