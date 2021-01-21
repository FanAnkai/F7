/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/

#if !defined(SOCKET_ERROR)
/** error in socket operation */
#define SOCKET_ERROR -1
#endif

#if defined(WIN32)
/* default on Windows is 64 - increase to make Linux and Windows the same */
#define FD_SETSIZE 1024
#include <winsock2.h>
#include <ws2tcpip.h>
#define MAXHOSTNAMELEN 256
#define EAGAIN WSAEWOULDBLOCK
#define EINTR WSAEINTR
#define EINVAL WSAEINVAL
#define EINPROGRESS WSAEINPROGRESS
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ENOTCONN WSAENOTCONN
#define ECONNRESET WSAECONNRESET
#define ioctl ioctlsocket
#define socklen_t int
#else
#define INVALID_SOCKET SOCKET_ERROR

#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"



#include "ethernetif.h"


#include <string.h>
#include <stdlib.h>


#endif


/**
This simple low-level implementation assumes a single connection for a single thread. Thus, a static
variable is used for that connection.
On other scenarios, the user must solve this by taking into account that the current implementation of
MQTTPacket_read() has a function pointer for a function call to get the data to a buffer, but no provisions
to know the caller or other indicator (the socket id): int (*getfn)(unsigned char*, int)
*/

static int mysock;

/************************************************************************
** 函数名称: transport_sendPacketBuffer
** 函数功能: 以TCP方式发送数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败
************************************************************************/
int32_t transport_sendPacketBuffer( uint8_t* buf, int32_t buflen)
{
    int32_t rc;
    rc = write(mysock, buf, buflen);
    return rc;
}

/************************************************************************
** 函数名称: transport_getdata
** 函数功能: 以阻塞的方式接收TCP数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int count：数据长度
** 出口参数: <=0接收数据失败
************************************************************************/
int32_t transport_getdata(uint8_t* buf, int32_t count)
{
    int32_t rc;
    //这个函数在这里不阻塞
    rc = recv(mysock, buf, count, 0);
    return rc;
}

/**
return >=0 for a socket descriptor, <0 for an error code
@todo Basically moved from the sample without changes, should accomodate same usage for 'sock' for clarity,
removing indirections
*/
#if 0
int transport_open(char* addr, int port)
{
    int* sock = &mysock;
    int type = SOCK_STREAM;
    struct sockaddr_in address;
#if (AF_INET6 == 10)
    struct sockaddr_in6 address6;
#endif
    int rc = -1;
#if defined(WIN32)
    short family;
#else
    sa_family_t family = AF_INET;
#endif
    struct addrinfo *result = NULL;
    struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};
    static struct timeval tv;

    *sock = -1;
    if (addr[0] == '[')
        ++addr;

    if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
    {
        struct addrinfo* res = result;

        /* prefer ip4 addresses */
        while (res)
        {
            if (res->ai_family == AF_INET)
            {
                result = res;
                break;
            }
            res = res->ai_next;
        }

#if (AF_INET6 == 10)
        if (result->ai_family == AF_INET6)
        {
            address6.sin6_port = htons(port);
            address6.sin6_family = family = AF_INET6;
            address6.sin6_addr = ((struct sockaddr_in6*)(result->ai_addr))->sin6_addr;
        }
        else
#endif
            if (result->ai_family == AF_INET)
            {
                address.sin_port = htons(port);
                address.sin_family = family = AF_INET;
                address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
            }
            else
                rc = -1;

        freeaddrinfo(result);
    }

    if (rc == 0)
    {
        *sock =	socket(family, type, 0);
        if (*sock != -1)
        {
#if defined(NOSIGPIPE)
            int opt = 1;

            if (setsockopt(*sock, SOL_SOCKET, SO_NOSIGPIPE, (void*)&opt, sizeof(opt)) != 0)
                Log(TRACE_MIN, -1, "Could not set SO_NOSIGPIPE for socket %d", *sock);
#endif

            if (family == AF_INET)
                rc = connect(*sock, (struct sockaddr*)&address, sizeof(address));
#if (AF_INET6 == 10)
            else
                rc = connect(*sock, (struct sockaddr*)&address6, sizeof(address6));
#endif
        }
    }
    if (mysock == INVALID_SOCKET)
        return rc;

    tv.tv_sec = 1;  /* 1 second Timeout */
    tv.tv_usec = 0;
    setsockopt(mysock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
    return mysock;
}

#else


/************************************************************************
** 函数名称: transport_open
** 函数功能: 打开一个接口，并且和服务器 建立连接
** 入口参数: char* servip:服务器域名
**           int   port:端口号
** 出口参数: <0打开连接失败
************************************************************************/
int32_t transport_open(int8_t* servip, int32_t port)
{
    int32_t *sock = &mysock;
    int32_t ret;
//	int32_t opt;
    struct sockaddr_in addr;

    //初始换服务器信息
    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    //填写服务器端口号
    addr.sin_port = PP_HTONS(port);
    //填写服务器IP地址
    addr.sin_addr.s_addr = inet_addr((const char*)servip);

    //创建SOCK
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    //连接服务器
    ret = connect(*sock, (struct sockaddr*)&addr, sizeof(addr));
    if(ret != 0)
    {
        //关闭链接
        close(*sock);
        //连接失败
        return -1;
    }
    //连接成功,设置超时时间1000ms
//	opt = 1000;
//	setsockopt(*sock,SOL_SOCKET,SO_RCVTIMEO,&opt,sizeof(int));

    //返回套接字
    return *sock;
}

#endif

/************************************************************************
** 函数名称: transport_close
** 函数功能: 关闭套接字
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败
************************************************************************/
int transport_close(void)
{

    int rc;
//	rc = close(mysock);
    rc = shutdown(mysock, SHUT_WR);
    rc = recv(mysock, NULL, (size_t)0, 0);
    rc = close(mysock);
    return rc;
}



