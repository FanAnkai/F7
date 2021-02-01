#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "ql_device_basic_impl.h"
#include "ql_log.h"

//#define HEAP_STAT
#ifdef HEAP_STAT
type_u32 heap_peak = 0;
type_u32 heap_current = 0;
void *ql_malloc(unsigned int size)
{
    void * p = malloc(4 + size);

    if(!p) 
        return p;

    heap_current += size;
    *((type_u32*)p) = size;

    if(heap_current > heap_peak)
        heap_peak = heap_current;

    return (p + 4);
}

void ql_free(void *p)
{
    if(!p)
        return;
    
    heap_current -= *((type_u32*)(p - 4));
    return free(p - 4);
}

void ql_heap_stat_dump()
{
    printf("\r\n Heap peak:%d, current:%d\r\n", heap_peak, heap_current);
}

#else
void *ql_malloc(unsigned int size)
{
    return malloc(size);
}

void ql_free(void *p)
{
    return free(p);
}
#endif

void *ql_zalloc(unsigned int size)
{
    void* p = ql_malloc(size);
    if(p)
    {
        ql_memset(p, 0, size);
    }
    return p;
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

#if (__SDK_PLATFORM__ == 0x19)
extern char* p_file_name ;
#else
char* p_file_name = "config";
#endif
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
    if( rc <= data_len || rc == 0 )
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


#if (__SDK_PLATFORM__ == 0x17)

#define MAC_LEN  10
int qlcloud_get_mac_arr(uint8_t *macbuff)
 {
    int fd,offset=0x04;
    uint8_t mac[MAC_LEN]={0};
    fd = open("/dev/mtd2", O_RDONLY );
    if (fd < 0) {
        ql_log_err(ERR_EVENT_NULL, "open mtd2 err");
        close(fd);
        return -1;
    }
    //lseek(fd, offset, SEEK_SET);
    if (read(fd, mac, MAC_LEN) != MAC_LEN) {
        ql_log_err(ERR_EVENT_NULL, "read mtd2 err");
        close(fd);
        return -1;
    }
    close(fd);
    memcpy(macbuff,mac + 4,MAC_LEN - 4);
    return 0;
 }
#elif (__SDK_PLATFORM__ == 0x19)
extern char android_dev_mac[6];
int qlcloud_get_mac_arr(uint8_t *macbuff)
{
    memcpy(macbuff, android_dev_mac, 6);
    return 0;
}

#else
//get the mac address of device,assign to a array size 6 bytes
int qlcloud_get_mac_arr(unsigned char mac_arr[6])
{
    #define MAXINTERFACES 16    /* max interface num */  

    int fd, i = 0;
    int if_len;
    struct ifreq buf[MAXINTERFACES];    /* ifreq array */  
    struct ifconf ifc;                  /* ifconf */  

// for self test virtual mac
#if 0
    char  mac[] = {0x66,0x00,0x00,0x00,0x00,0x05};
    ql_memcpy(mac_arr, mac, 6);
    return 0;
#endif
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)      
    {
       ql_log_err( ERR_EVENT_NULL, "not find socket!");
       return -1;
    }

    ifc.ifc_len = sizeof(buf);  
    ifc.ifc_buf = (caddr_t) buf;  
   
    /* get interface list */  
    if (ioctl(fd, SIOCGIFCONF, (char *) &ifc) == -1)  
    {  
        ql_log_err( ERR_EVENT_NULL, "not find interface!");
        close(fd);
        return -1;  
    }
    
    /* interface num */
    if_len = ifc.ifc_len / sizeof(struct ifreq);

    for(i = 0; i < if_len; i++) /* traversal interfaces */  
    {
        //get interface flags
        if (ioctl(fd, SIOCGIFFLAGS, &buf[i]) == 0) 
        {
            if ( buf[i].ifr_flags & IFF_LOOPBACK ) 
            {
                // don't count loopback,do nothing
            }
            else
            {
                if (!(ioctl(fd, SIOCGIFHWADDR, (char *) &buf[i])))  
                {
                    //use the first valid mac 
                    break;
                }  
                else  
                {
                    continue;
                }
            }
        }
        else
        {
            continue;
        }
    }//�Cend for
    
    close(fd);

    if( i == if_len )
    {
       ql_log_err( ERR_EVENT_NULL, "not find mac!");
       return -1;
    }
    else
    {
        ql_memcpy(mac_arr, buf[i].ifr_hwaddr.sa_data, 6);
    }
    return 0;
}
#endif
unsigned int ql_get_current_sysclock(void)
{
    #if __SDK_PLATFORM__ == 0x17 || __SDK_PLATFORM__ == 0x11
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec;
    #else
        return (unsigned int)time(NULL);
    #endif
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
    usleep(1000*ms);
}

void ql_sleep_s(unsigned int s)
{
    sleep(s);
}

int ql_gethostbyname(const char *name, char *ip, char len)
{
    struct hostent *ht = NULL;

    res_init();

    ht = gethostbyname(name);
    if (NULL == ht) {        
        ql_log_err( ERR_EVENT_INTERFACE, "gethostbyname err!");
        return -1;
    }

    if (inet_ntop(ht->h_addrtype, ht->h_addr, ip, len)) {
        return 0;
    } else {
        ql_log_err( ERR_EVENT_INTERFACE, "inet_ntop err!");
        return -1;
    }
}

ql_udp_socket_t *ql_udp_socket_create(unsigned int local_port)
{
    ql_udp_socket_t *socket = NULL;

    socket = (ql_udp_socket_t *)ql_malloc(sizeof(ql_udp_socket_t));
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
        close(sock->sockfd);
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
        ql_log_err( ERR_EVENT_INTERFACE, "udp_s select err:%d", ret);
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
        if( errno == EINTR )
        {
            ql_log_err(ERR_EVENT_INTERFACE, "u_rv1 r:%d,e:%d", ret, errno);
            return 0;
        }
        else
        {
            ql_log_err(ERR_EVENT_INTERFACE, "u_rv2 r:%d,e:%d", ret, errno);
            return -1;
        }
    }
    return ret;
}

struct ql_socket_t {
    int sockfd;
};

ql_socket_t *_ql_tcp_socket_create() 
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
    struct timeval tv;
    fd_set wfds;
    int retval;
    socklen_t optlen;
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
        return -2;
    }


    fstatus = fcntl(sock->sockfd, F_GETFL) | O_NONBLOCK;
    if (fcntl(sock->sockfd, F_SETFL, fstatus) == -1) {
        close(sock->sockfd);
        return -3;
    }

    if (connect(sock->sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        return 0;
    }

    if (errno != EINPROGRESS) {
        close(sock->sockfd);
        return -4;
    }

    FD_ZERO(&wfds);
    FD_SET(sock->sockfd, &wfds);

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    retval = select(sock->sockfd + 1, NULL, &wfds, NULL, &tv);
    if (retval <= 0) {
        close(sock->sockfd);
        return -5;
    }

    optlen = sizeof(int);
    if (getsockopt(sock->sockfd, SOL_SOCKET, SO_ERROR, &retval, &optlen) == 0 && retval == 0) {
        return 0;
    }

    close(sock->sockfd);

    return -6;
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
    struct timeval tv;
    fd_set wfds;
    int retval;
    int sentlen = 0;
    int totallen = 0;

    if (sock == NULL || sock->sockfd == -1) {  
        ql_log_err(ERR_EVENT_LOCAL_DATA, "t_sd sock err");
        return -1;
    }

    FD_ZERO(&wfds);
    FD_SET(sock->sockfd, &wfds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    retval = select(sock->sockfd + 1, NULL, &wfds, NULL, &tv);
    if (retval < 0) {
        ql_log_err(ERR_EVENT_INTERFACE, "t_sd1 r:%d,e:%d", retval, errno);
        return -1;
    } else if (retval == 0){
        ql_log_warn("tcp send timeout\r\n");
        return 0; //timeout
    }

    totallen = len;
    while (sentlen < totallen) {
        retval = send(sock->sockfd, buf + sentlen, totallen - sentlen, 0);
        if (retval < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else if (errno == EINTR) {
                continue;
            }
            ql_log_err(ERR_EVENT_INTERFACE, "t_sd2 r:%d,e:%d", retval, errno);
            return -1;
        }

        sentlen += retval;
    }

    return totallen;
}

int _ql_tcp_recv(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
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
        if( errno == EINTR )
        {
            ql_log_err(ERR_EVENT_INTERFACE, "t_rv4 r:%d,e:%d", retval, errno);
            return 0;
        }
        else
        {
            ql_log_err(ERR_EVENT_INTERFACE, "t_rv1 r:%d,e:%d", retval, errno);
            return -1;
        }
    } else if (0 == retval) {
        return 0;
    }

    retval = recv(sock->sockfd, buf, size, 0);
    if (retval == 0) {        
        ql_log_err(ERR_EVENT_INTERFACE, "t_rv2 r:%d,e:%d", retval, errno);
        return -1; //the peer has performed an orderly shutdown.
    } else if (retval > 0) {
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            return 0;
        } else {        
            ql_log_err(ERR_EVENT_INTERFACE, "t_rv3 r:%d,e:%d", retval, errno);
            return -1;
        }
    }
    return retval;
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
    {
        if(ql_tcp_socket_create_ssl)
        {
            return ql_tcp_socket_create_ssl();
        }
        else
        {
            ql_log_err( ERR_EVENT_INTERFACE,"ssl create undef!");
        }
    }
    else
    {
        return _ql_tcp_socket_create();
    }
}

void ql_tcp_socket_destroy(ql_socket_t **sock)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
    {
        if(ql_tcp_socket_destroy_ssl)
        {
            ql_tcp_socket_destroy_ssl(sock);
        }
        else
        {
            ql_log_err(ERR_EVENT_INTERFACE,"ssl destroy undef!");
        }
    }
    else
    {
        _ql_tcp_socket_destroy(sock);
    }
}

int ql_tcp_connect(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
    {
        if(ql_tcp_connect_ssl)
        {
            return ql_tcp_connect_ssl(sock, ip, port, timeout_ms);
        }
        else
        {
            ql_log_err(ERR_EVENT_INTERFACE, "ssl_conn undef!");
        }
    }
    else
    {
        return _ql_tcp_connect(sock, ip, port, timeout_ms);
    }
}

int ql_tcp_disconnect(ql_socket_t *sock)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
    {
        if(ql_tcp_disconnect_ssl)
        {
            return ql_tcp_disconnect_ssl(sock);
        }
        else
        {
            ql_log_err(ERR_EVENT_INTERFACE, "ssl_discon undef!");
        }
    }
    else
    {
        return _ql_tcp_disconnect(sock);
    }
}

int ql_tcp_send(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
    {
        if(ql_tcp_send_ssl)
        {
            return ql_tcp_send_ssl(sock, buf, len, timeout_ms);
        }
        else
        {
            ql_log_err(ERR_EVENT_INTERFACE, "ssl_s undef!");
        }
    }
    else
    {
        return _ql_tcp_send(sock, buf, len, timeout_ms);
    }
}

int ql_tcp_recv(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms)
{
    if(g_client_handle.encrypt_type == QL_ENCRYPT_TYPE_SSL)
    {
        if(ql_tcp_recv_ssl)
        {
             return ql_tcp_recv_ssl(sock, buf, size, timeout_ms);
        }
        else
        {
            ql_log_err(ERR_EVENT_INTERFACE, "ssl_r undef!");
        }
    }
    else
    {   
        return _ql_tcp_recv(sock, buf, size, timeout_ms);
    }
}

