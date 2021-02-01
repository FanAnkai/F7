#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

// suppress Error C4996: 'gethostbyname': Use getaddrinfo() or GetAddrInfoW() instead or
// define _WINSOCK_DEPRECATED_NO_WARNINGS to disable deprecated API warnings
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <ws2tcpip.h>
#include <winsock.h>
#include <iphlpapi.h>
#include <windows.h>

#ifndef USE_SSL
#else
#error  SSL is not yet implemented on Windows platform
#endif

#include "ql_device_basic_impl.h"
#include "ql_log.h"

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

unsigned char g_device_mac_address[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
int g_device_mac_address_is_set = 0;

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
    free(p);
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

int ql_strncmp(const char *str1, const char *str2, unsigned int n)
{
    unsigned int len1 = 0, len2 = 0;
    if( str1 == NULL || str2 == NULL )
    {
        return -1;
    }
    len1 = ql_strlen(str1);len2 = ql_strlen(str2);
    if( (len1 != len2) || (len1 > n) )
    {
        return -1;
    }
    return strcmp(str1, str2);
}

#ifdef LOCAL_SAVE

char* p_file_name = "config.bin";

int ql_persistence_data_save( const void* data, type_u32 data_len )
{
    int rc = 0;
    FILE * file;
    
    //Opens a binary file to update both reading and writing. The file must exist.
    file = fopen( p_file_name, "rb+" );
    
    if( file == NULL )
    {
        //Opens or creates an empty binary file for both reading and writing, if the file already exists, its contents are destroyed.
        file = fopen(p_file_name, "wb+" );
        if( file == NULL )
        {
            ql_log_err(ERR_EVENT_INTERFACE, "save open err");
            return -1;
        }
    }

    rc = fwrite(data, sizeof(unsigned char), data_len, file);
    if( rc == data_len )
    {
        rc = 0;
    }
    else
    {
        rc = -1;
    }

    fclose(file);
    
    return rc;
}

int ql_persistence_data_load( void* data, type_u32 data_len )
{
    int rc = 0;
    FILE * file;    

    file = fopen( p_file_name, "rb+" );
    
    if( file == NULL )
    {
        file = fopen(p_file_name, "wb+" );
        if( file == NULL )
        {
            ql_log_err(ERR_EVENT_INTERFACE, "load open err");
            return -1;
        }
    }

    rc = fread(data, sizeof(unsigned char), data_len, file);
    //"rc == 0" means the first to read the file
    if( rc == data_len || rc == 0 )
    {
        rc = 0;
    }
    else
    {
        rc = -1;
    }

    fclose(file);
    
    return rc;
}
#endif

//get the mac address of device,assign to a array size 6 bytes
int qlcloud_get_mac_arr(unsigned char mac_arr[6])
{
#if 0
    // for test
//    memcpy(mac_arr, "\x11\x11\x11\x22\x33\x44", 6);
//    memcpy(mac_arr, "\x08\x00\x27\x06\x43\xd1", 6);
    memcpy(mac_arr, "\x08\x00\x00\xAA\x55\x01", 6);
    return 0;
#else
    if (g_device_mac_address_is_set) {
        memcpy(mac_arr, g_device_mac_address, 6);
        return 0;
    } else {
        IP_ADAPTER_INFO *adapter_info_buffer = NULL;
        DWORD adapter_info_buffer_size = sizeof(adapter_info_buffer) * 10;

        adapter_info_buffer = malloc(adapter_info_buffer_size);
        if (adapter_info_buffer == NULL) {
            return -1;
        }

        DWORD ret = GetAdaptersInfo(adapter_info_buffer, &adapter_info_buffer_size);
        
        if (ret == ERROR_BUFFER_OVERFLOW) {
            // try again

            if (adapter_info_buffer != NULL) {
                free(adapter_info_buffer);
            }

            adapter_info_buffer = malloc(adapter_info_buffer_size);
            if (adapter_info_buffer == NULL) {
                return -1;
            }

            ret = GetAdaptersInfo(adapter_info_buffer, &adapter_info_buffer_size);
        }

        if (ret != NO_ERROR)
        {
            if (adapter_info_buffer != NULL) {
                free(adapter_info_buffer);
            }

            ql_log_err(ERR_EVENT_NULL, "GetAdaptersInfo() failed with error: %d\n", ret);

            return -1;
        }

        IP_ADAPTER_INFO *adapter_info = adapter_info_buffer;

        int found_valid = 0;
        while (adapter_info) {
            if (adapter_info->AddressLength != 6) {
                continue;
            }

            ql_log_info("found network adapter %02x:%02x:%02x:%02x:%02x:%02x (%s)\r\n",
                (int)adapter_info->Address[0], (int)adapter_info->Address[1], (int)adapter_info->Address[2],
                (int)adapter_info->Address[3], (int)adapter_info->Address[4], (int)adapter_info->Address[5],
                adapter_info->Description);

            if (!found_valid) {
                if (adapter_info->AddressLength == 6) {  // to be noticed
                    memcpy(mac_arr, adapter_info->Address, 6);
                    found_valid = 1;
                }
            }

            adapter_info = adapter_info->Next;
        }

        if (adapter_info_buffer != NULL) {
            free(adapter_info_buffer);
        }

        if (found_valid) {
            ql_log_info("use mac address %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                (int)mac_arr[0], (int)mac_arr[1], (int)mac_arr[2],
                (int)mac_arr[3], (int)mac_arr[4], (int)mac_arr[5]);

            return 0;
        } else {
            return -1;
        }
    }
#endif
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
    Sleep(ms);
}

void ql_sleep_s(unsigned int s)
{
    Sleep(s * 1000);
}

int ql_gethostbyname(const char *name, char *ip, char len)
{
    struct hostent *ht = NULL;
    WORD wVersionRequested = MAKEWORD(2, 0);
    WSADATA data;

    WSAStartup(wVersionRequested, &data);

    ht = gethostbyname(name);
    if (NULL == ht) {
        ql_log_err(ERR_EVENT_INTERFACE, "gethostbyname null!\r\n");
        return -1;
    }

    if (inet_ntop(ht->h_addrtype, ht->h_addr, ip, len)) {
        return 0;
    } else {
        ql_log_err(ERR_EVENT_INTERFACE, "inet_ntop err!\r\n");
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
    if (sock == NULL) {
        return -1;
    }

    sock->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock->sockfd == -1) {
        return -1;
    }

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(sock->local_port);

    int re_flag = 1;
    int re_len=sizeof(int);
    setsockopt(sock->sockfd, SOL_SOCKET, SO_REUSEADDR, &re_flag, re_len);

    if(bind(sock->sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        return -1;
    }
    return 0;
}
int ql_udp_close(ql_udp_socket_t *sock)
{
    if (sock && sock->sockfd != -1) {
        closesocket(sock->sockfd);
        sock->sockfd = -1;
    }
    return 0;
}

int ql_udp_send(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    int ret = -1;
    int sentlen = 0;
    int totallen = 0;

    if (sock == NULL || sock->sockfd == -1) {
        return ret;
    }

    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = sock->remote_ip;
    addr.sin_port = sock->remote_port;

    struct timeval tv;
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sock->sockfd, &wfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    if(ret = select(sock->sockfd + 1, NULL, &wfds, NULL, &tv) >= 0) {
        if(FD_ISSET(sock->sockfd, &wfds))
        {
            totallen = len;
            while (sentlen < totallen) {
                ret = sendto(sock->sockfd, buf + sentlen, totallen - sentlen, 0, (struct sockaddr *)&addr, sizeof(addr));
                if (ret < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                        continue;
                    }else {
                        break;
                    }
                }
                sentlen += ret;
            }
            if(ret > 0) {
                ret = totallen;
            }
        }
    } else {
        ql_log_err(ERR_EVENT_INTERFACE, "udp send select err:%d\n", ret);
    }

    return ret;
}

int ql_udp_recv(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    struct timeval tv;
    fd_set readfds;
    int n=0;
    int ret = -1;

    struct sockaddr_in remote_addr;
    bzero(&remote_addr, sizeof(remote_addr));
    int addr_len = sizeof(remote_addr);

    FD_ZERO(&readfds);
    FD_SET(sock->sockfd, &readfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    if(ret = select(sock->sockfd + 1, &readfds, NULL, NULL, &tv) >= 0) {
        if(FD_ISSET(sock->sockfd, &readfds))
        {
           if((n = recvfrom(sock->sockfd, buf, len, 0, (struct sockaddr*)&remote_addr, &addr_len )) >= 0)
           {
               sock->remote_port = remote_addr.sin_port;
               sock->remote_ip   = remote_addr.sin_addr.s_addr;
               ret = n;
           }
        } else {
            ret = 0;
        }
    } else {
        ql_log_err(ERR_EVENT_INTERFACE, "udp recv select err:%d\n", ret);
    }
    return ret;
}

#ifndef USE_SSL

struct ql_socket_t {
    SOCKET sockfd;
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
    struct sockaddr_in addr;
    struct timeval tv;
    fd_set wfds;
    int retval;
    int optlen;
    int fstatus;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, (char *)ip, &addr.sin_addr);

    if (sock == NULL) {
        return -1;
    }

    sock->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock->sockfd == -1) {
        return -1;
    }

    // set to non-blocking mode
    unsigned long flag = 1;
    if (ioctlsocket(sock->sockfd, FIONBIO, (unsigned long *)&flag) < 0) {
        closesocket(sock->sockfd);
        return -1;
    }

    if (connect(sock->sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        return 0;
    }

    int last_error = WSAGetLastError();
    if (last_error != WSAEWOULDBLOCK) {
        closesocket(sock->sockfd);
        return -1;
    }

    FD_ZERO(&wfds);
    FD_SET(sock->sockfd, &wfds);

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    retval = select(sock->sockfd + 1, NULL, &wfds, NULL, &tv);
    if (retval <= 0) {
        closesocket(sock->sockfd);
        return -1;
    }

    optlen = sizeof(int);
    if (getsockopt(sock->sockfd, SOL_SOCKET, SO_ERROR, &retval, &optlen) == 0 && retval == 0) {
        return 0;
    }

    closesocket(sock->sockfd);

    return -1;
}

int ql_tcp_disconnect(ql_socket_t *sock)
{
    if (sock && sock->sockfd != -1) {
        closesocket(sock->sockfd);
        sock->sockfd = -1;
    }
    return 0;
}

int ql_tcp_send(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    struct timeval tv;
    fd_set wfds;
    int retval;
    int sentlen = 0;
    int totallen = 0;

    if (sock == NULL || sock->sockfd == -1) {
        return -1;
    }

    FD_ZERO(&wfds);
    FD_SET(sock->sockfd, &wfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    retval = select(sock->sockfd + 1, NULL, &wfds, NULL, &tv);
    if (retval < 0) {
        return -1;
    } else if (retval == 0){
        return 0; //timeout
    }

    totallen = len;
    while (sentlen < totallen) {
        retval = send(sock->sockfd, buf + sentlen, totallen - sentlen, 0);
        if (retval < 0) {
            int last_error = WSAGetLastError();

            if (last_error == EAGAIN || last_error == EWOULDBLOCK) {
                continue;
            } else if (last_error == EINTR) {
                continue;
            }
            return -1;
        }

        sentlen += retval;
    }

    return totallen;
}

int ql_tcp_recv(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
{
    struct timeval tv;
    fd_set rfds;
    int retval;


    FD_ZERO(&rfds);
    FD_SET(sock->sockfd, &rfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    retval = select(sock->sockfd + 1, &rfds, NULL, NULL, &tv);
    if (retval < 0) {
        return -1;
    } else if (0 == retval) {
        return 0;
    }


    retval = recv(sock->sockfd, buf, size, 0);
    if (retval == 0) {
        return -1; //the peer has performed an orderly shutdown.
    } else if (retval > 0) {
    } else {
        int last_error = WSAGetLastError();

        if (last_error == EAGAIN || last_error == EWOULDBLOCK || last_error == EINTR) {
            return 0;
        } else {
            return -1;
        }
    }
    return retval;
}

#else

#error  SSL is not yet implemented on Windows platform

#endif  // #ifndef USE_SSL
