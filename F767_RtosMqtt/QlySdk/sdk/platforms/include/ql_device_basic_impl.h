/*
 * Copyright (c) 2016 qinglianyun.
 * All rights reserved.
 */

/************************************************************************
 sdk依赖的最基本平台功能，此文件内的每个接口接入厂商必须实现，否则链接找不到对象
************************************************************************/

#ifndef QL_DEVICE_BASIC_IMPL_H_
#define QL_DEVICE_BASIC_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ql_snprintf
#define ql_snprintf snprintf
#endif

#ifndef ql_strlen
#define ql_strlen   strlen
#endif

#ifndef ql_strncpy
#define ql_strncpy  strncpy
#endif

#ifndef ql_strcpy
#define ql_strcpy   strcpy
#endif

/******************************基本操作类接口********************************/

extern void *ql_malloc(unsigned int size);

extern void ql_free(void *p);

extern void *ql_zalloc(unsigned int size);

extern void ql_memcpy(void *dst, const void *src, unsigned int n);

extern void ql_memset(void *p, int c, unsigned int n);

extern int ql_memcmp(const void *p1, const void *p2, unsigned int n);

/*获得当前系统时钟时间，相对开机时的系统时钟-单位秒*/
extern unsigned int ql_get_current_sysclock(void);

extern unsigned int ql_random(void);

extern unsigned int ql_srand(unsigned int seed);

/*sleep当前线程，不需要指定线程id，跟linux下的sleep功能（用法）一样，单位是ms*/
extern void ql_sleep_ms(unsigned int ms);

extern void ql_sleep_s(unsigned int s);

extern int qlcloud_get_mac_arr(unsigned char mac_arr[6]);

/******************************tcp socket接口 ***************************/
/*
 * socket接口，可以是tcp socket或者SSL socket, 当在初始化接口中flag为1时，该部分实现必须为ssl; flag为0时，该部分实现必须为普通tcp socket
 * SDK不关心socket的实现是阻塞或非阻塞
 * 注意：SDK内部只会调用一次ql_tcp_socket_create来创建维持TCP长连接的socket，
 * 如果连接失败或者断开连接（包括recv和send失败），SDK会先调用ql_tcp_disconnect，然后再调用ql_tcp_connect继续连接
 */

/*厂商需要实现 struct ql_socket_t 结构体的定义*/

typedef struct ql_socket_t ql_socket_t;
/*
 * udp socket接口，如设备需要使用局域网功能，请实现此结构
*/
#if (__SDK_PLATFORM__ == 0x12)
typedef struct ql_udp_socket ql_udp_socket_t;
#else
    typedef struct ql_udp_socket {
        int sockfd;
        unsigned int local_ip;
        unsigned int local_port;
        unsigned int remote_ip;
        unsigned int remote_port;
    }ql_udp_socket_t;
#endif
/*创建 tcp socket句柄  malloc from heap*/
extern ql_socket_t *ql_tcp_socket_create(void);

/*创建 udp socket句柄  malloc from heap
* @param local_port 本地端口
*/
extern ql_udp_socket_t *ql_udp_socket_create(unsigned int local_port);

/*销毁tcp socket句柄*/
extern void ql_tcp_socket_destroy(ql_socket_t **sock);

/*销毁udp socket句柄*/
extern void ql_udp_socket_destroy(ql_udp_socket_t **sock);
/**连接服务器，保持tcp长连接
 * @param sock tcp socket句柄
 * @param ip 服务器的IP字符串地址，如"192.168.1.1"
 * @param port 服务器的端口号，此处为本机字节序，使用时需转成网络字节序
 * @param timeout_ms 超时时间，单位：毫秒
 *
 * @return 0 表示连接成功
 *         -1 表示连接失败
 */
extern int ql_tcp_connect(ql_socket_t *sock, unsigned char *ip, unsigned short port, unsigned int timeout_ms);
/**绑定UDP端口
 * @param sock udp socket句柄
 *
 * @return  0 绑定成功
 *         -1 绑定失败
 */
extern int ql_udp_bind(ql_udp_socket_t *sock);
/**关闭udp句柄
 * @param sock udp socket句柄
 *
 * @return  0 关闭成功
 *         -1 关闭失败
 */
extern int ql_udp_close(ql_udp_socket_t *sock);
/**发送udp数据
 * @param sock udp socket句柄
 * @param buf 待发送数据的首地址
 * @param len 待发送数据的大小（字节数）
 * @param timeout_ms 超时时间，单位：毫秒
 *
 * @return  0 关闭成功
 *         -1 关闭失败
 */
extern int ql_udp_send(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms);
/**接收udp数据
 * @param sock udp socket句柄
 * @param buf 接收缓冲区的首地址
 * @param len 接收缓冲区buf的大小
 * @param timeout_ms 超时时间，单位：毫秒
 *
 * @return -1 表示发生错误
 *          0 表示在timeout_ms时间内没有收到数据，属于正常情况
 *         >0 表示收到的字节数
 */
extern int ql_udp_recv(ql_udp_socket_t *sock, unsigned char *buf, unsigned int len, unsigned int timeout_ms);

/**断开连接
 * @note 不要释放socket handler
 * @param sock tcp socket句柄
 *
 * @return  0 表示成功
 *         -1 表示失败
 */
extern int ql_tcp_disconnect(ql_socket_t *sock);

/**发送数据
 * @param sock tcp socket句柄
 * @param buf 待发送数据的首地址
 * @param len 待发送数据的大小（字节数）
 * @param timeout_ms 超时时间，单位：毫秒
 *
 * @return -1 表示发生错误，SDK在发现返回-1后会调用ql_tcp_disconnect, 然后再调用ql_tcp_connect保持长连接
 *         0 表示再timeout_ms时间内没有将数据发送出去
 *         正数 表示发送成功的字节数
 */
extern int ql_tcp_send(ql_socket_t *sock,  unsigned char *buf, unsigned int len, unsigned int timeout_ms);


/**接收数据
 * @param sock tcp socket句柄
 * @param buf 接收缓冲区的首地址
 * @param size 接收缓冲区buf的大小
 * @param timeout_ms 超时时间，单位：毫秒
 *
 * @return -1 表示发生错误，SDK在发现返回-1后会调用txd_tcp_disconnect，, 然后再调用ql_tcp_connect保持长连接
 *         0 表示在timeout_ms时间内没有收到数据，属于正常情况
 *         正数 表示收到的字节数
 */
extern int ql_tcp_recv(ql_socket_t *sock, unsigned char *buf, unsigned int size, unsigned int timeout_ms);

/**DNS解析
 * @param name - domain hostname.
 * @param ip - an IPv4 address in standard dot notation, 比如192.168.0.1
 * @param len - buffsize to store ip.
 *
 * @return -1 失败
 * 			0   成功
 */
extern int ql_gethostbyname(const char *name, char *ip, char len);

#ifdef LOCAL_SAVE
extern int ql_persistence_data_save( const void* data, unsigned int data_len );

extern int ql_persistence_data_load( void* data, unsigned int data_len );
#endif

#define NULL_MODE       0x00
#define STATION_MODE    0x01
#define SOFTAP_MODE     0x02
#define STATIONAP_MODE  0x03

typedef struct 
{
    unsigned char ssid [32];   
    unsigned char password [64];
}wifi_info_t;
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* QL_DEVICE_BASIC_IMPL_H_ */
