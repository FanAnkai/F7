#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
// #include <sys/select.h>
#include <string.h>
#include <unistd.h>
// #include <arpa/inet.h>
#include <sys/types.h>
// #include <sys/socket.h>
// #include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
// #include <netdb.h>
#include <time.h>
// #include <netinet/in.h>
// #include <net/if.h>

#include "hsf.h"
#include "hf_tcp_impl.h"
#include "hf_udp_impl.h"
#include "ql_buffer.h"

#ifndef USE_SSL
#else
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "ql_device_basic_impl.h"
#include "ql_log.h"

void *ql_malloc(unsigned int size)
{
    return hfmem_malloc(size);
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
    return hfmem_free(p);
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
int ql_persistence_data_save( const void* data, type_u32 data_len )
{
    type_s32 ret;

    hfuflash_erase_page(0, 1);
    ret = hfuflash_write(0, data, data_len);
    if(ret >= 0)
    {
        return 0;
    }
    else
    {
        ql_log_err("ql_persistence_data_save err:%d", ret);
        return -1;
    }
}

int ql_persistence_data_load( void* data, type_u32 data_len )
{
    type_s32 ret;

    ret = hfuflash_read(0, data, data_len);
    if(ret >= 0)
    {
        return 0;
    }
    else
    {
        ql_log_err("ql_persistence_data_save err:%d", ret);
        return -1;
    }
}
#endif

//get the mac address of device,assign to a array size 6 bytes
int qlcloud_get_mac_arr(unsigned char mac_arr[6])
{
    int ret = 0;
    ret = hfwifi_read_sta_mac_address(mac_arr);
    if(ret == 0)
    {
        return 0;
    }
    else
    {
        ql_log_err("qlcloud_get_mac_arr err:%d\r\n", ret);
        return -1;
    }
}

unsigned int ql_get_current_sysclock(void)
{
    unsigned int sysclock;
    sysclock = hfsys_get_time();
    return sysclock/1000;
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
    hfsys_sleep(ms);
}

void ql_sleep_s(unsigned int s)
{
    hfsys_sleep(s*1000);
}

int ql_gethostbyname(const char *name, char *ip, char len)
{
    int start_time = 0, current_time = 0;

	ql_log_info("ql_gethostbyname\r\n");

	strcpy(hf_tcp_client.dns_name, name);
	hf_tcp_client.dns_timout = DNS_TIMEOUT;
	start_time = hfsys_get_time();
	current_time = hfsys_get_time();
	g_tcp_connect_flag = TCP_CLIENT_DNS;

	ql_log_info("current_time:%d\r\n", current_time);
    post_event_tcp_process();
	while(current_time - start_time < hf_tcp_client.dns_timout*1000 && g_tcp_connect_flag != TCP_CLIENT_DNSOK)
	{
		process_run();
		current_time = hfsys_get_time();
	}
	ql_log_info("current_time:%d\r\n", current_time);

	if(g_tcp_connect_flag == TCP_CLIENT_DNSOK)
	{
		ql_log_info("DNS success, hf_tcp_client.dns_ip:%s\r\n", hf_tcp_client.dns_ip);
		strcpy(ip, hf_tcp_client.dns_ip);
		return 0;
	}
	else
	{
        ql_log_err("ql_gethostbyname err\r\n");
		return -1;
	}
}

ql_udp_socket_t *ql_udp_socket_create(unsigned int local_port)
{
    ql_udp_socket_t *socket = NULL;

    socket = (ql_udp_socket_t *)malloc(sizeof(ql_udp_socket_t));
    memset(socket, 0, sizeof(ql_udp_socket_t));
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
    return hf_udp_bind(sock);
}
int ql_udp_close(ql_udp_socket_t *sock)
{
    if (sock && sock->sockfd != -1) {
        hfnet_udp_close(sock->sockfd);
        sock->sockfd = -1;
    }
    return 0;
}

int ql_udp_send(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    if (sock == NULL || sock->sockfd == -1) {
        return -1;
    }

    if(hf_udp_client.udp_sockfd != sock->sockfd)
    {
        ql_log_err("hf_udp_client.udp_sockfd != sock->sockfd\r\n");
        return -1;
    }
    return hfnet_udp_sendto(hf_udp_client.udp_sockfd, buf, len, &(hf_udp_client.udp_peeraddr), hf_udp_client.udp_peerport);
}

int ql_udp_recv(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int ret = 0;
    ret = data_buf_ioctl(DATA_IOCTL_GET_DATA, DATA_UDP, buf, len);
    if(ret > 0)
    {
        sock->sockfd = hf_udp_client.udp_sockfd;
        sock->remote_ip = hf_udp_client.udp_peeraddr.u8;
        sock->remote_port = hf_udp_client.udp_peerport;
    }

    return ret;
}

struct ql_socket_t {
    int sockfd;
};

ql_socket_t *ql_tcp_socket_create()
{
    ql_socket_t *socket = NULL;

    socket = (ql_socket_t *)ql_malloc(sizeof(ql_socket_t));

    return socket;
}

void ql_tcp_socket_destroy(ql_socket_t **sock)
{
    if (sock && *sock) {
        ql_free(*sock);
    }

    *sock = NULL;
}

int ql_tcp_connect(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms)
{
    int start_time = 0, current_time = 0;

	ql_log_info("ql_tcp_connect\r\n");

	strcpy(hf_tcp_client.tcp_ip, ip);
	hf_tcp_client.tcp_port = port;
	hf_tcp_client.tcp_recive_max_len = g_client_handle.recvbuf_size;
	hf_tcp_client.tcp_connect_timout_ms = timeout_ms*5; //because HF SDK hardware timer is not accuracy when short internal
	start_time = hfsys_get_time();
	current_time = hfsys_get_time();
	g_tcp_connect_flag = TCP_CLIENT_CONNECT;

    post_event_tcp_process();
	while(current_time - start_time < hf_tcp_client.tcp_connect_timout_ms && g_tcp_connect_flag != TCP_CLIENT_CONNECTED)
	{
		process_run();
		current_time = hfsys_get_time();
	}

	if(g_tcp_connect_flag == TCP_CLIENT_CONNECTED)
	{
		ql_log_info("tcp connect success, hf_tcp_client.tcp_socket:%d\r\n", hf_tcp_client.tcp_sockfd);
		sock->sockfd = hf_tcp_client.tcp_sockfd;
		return 0;
	}
	else
	{
        ql_log_err("ql_tcp_connect err\r\n");
		return -1;
	}
}

int ql_tcp_disconnect(ql_socket_t *sock)
{
    if (sock && sock->sockfd != -1) {
        hfnet_tcp_disconnect(sock->sockfd);
        sock->sockfd = -1;
    }
    return 0;
}

int ql_tcp_send(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int ret = 0;
    unsigned int start_time = 0, current_time = 0;

    timeout_ms = timeout_ms*5;//because HF SDK hardware timer is not accuracy when short internal

    // ql_log_info("ql_tcp_send, len:%d, timeout_ms:%d\r\n", len, timeout_ms);

    start_time = hfsys_get_time();
	current_time = hfsys_get_time();
    
    ret = hfnet_tcp_send(sock->sockfd, buf, len);
    if(ret > 0)
    {
        g_tcp_connect_flag = TCP_CLIENT_SENDING;

        while((start_time + timeout_ms > current_time) && g_tcp_connect_flag != TCP_CLIENT_SENDACK)
        // while((start_time + 10000 > current_time) && g_tcp_connect_flag != TCP_CLIENT_SENDACK)
        {
            process_run();
            current_time = hfsys_get_time();
        }
        if(current_time - start_time > 2000)
        {
            ql_log_info("tcp send ack time > 2S, start:%d, current:%d, time:%d\r\n", start_time, current_time, (current_time-start_time));
        }

        if(g_tcp_connect_flag != TCP_CLIENT_SENDACK)
        {
            ql_log_err("hfnet_tcp_send ack timeout, s:%d, c:%d, to:%d, t:%d, g_tcp_connect_flag:%d\r\n", start_time, current_time, timeout_ms, start_time-current_time, g_tcp_connect_flag);
            return 0;
        }

        g_tcp_connect_flag = TCP_CLIENT_CONNECTED;
        return ret;
    }
    else
    {
        ql_log_err("ql_tcp_send err:%d\r\n", ret);
        return -1;
    }
}

int ql_tcp_recv(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
{
    int ret = 0;
	unsigned int start_time = 0, current_time = 0;

    if(g_tcp_connect_flag == TCP_CLIENT_CLOSED)
    {
        ql_log_err("g_tcp_connect_flag == TCP_CLIENT_CLOSED\r\n");
        return -1;
    }

	start_time = hfsys_get_time();
	current_time = hfsys_get_time();

	while((start_time + timeout_ms > current_time) && ret == 0)
	{
		ret = data_buf_ioctl(DATA_IOCTL_GET_DATA, DATA_TCP, buf, size);
		if(ret < 0)
		{
			ql_log_err("data_buf_ioctl err:%d\r\n", ret);
			return -1;
		}
        if(ret == 0)
        {
            process_run();
            current_time = hfsys_get_time();
        }
	}

	return ret;
}
