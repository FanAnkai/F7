#include "ql_dispatch_access.h"

#define MAGIC_SSL 0xF1EB
#define MAGIC_AES 0xF1EC
#define FACTOR    32463
static type_u16   dpc_magic = 0;

int qlcloud_create_dispatch_request(char *data, unsigned char *outbuf, int outbufsize)
{
	int datalen = 0;
	int len = 0;
	unsigned int salt = 0;
	unsigned short crc = 0;

	if (data == NULL || outbuf == NULL) {
		return -1;
	}

	if (outbufsize <= 12) {
		return -1;
	}

	datalen = strlen(data);
	if (datalen == 0 || datalen + 12 >= outbufsize) {
		return -1;
	}

	salt = (unsigned int)ql_random();

	UINT32_TO_STREAM_f(outbuf, salt);
	UINT16_TO_STREAM_f(outbuf+4, dpc_magic);
	UINT32_TO_STREAM_f(outbuf+6, datalen);
	len = v_strncpy((char *)(outbuf+10), outbufsize - 10, data, datalen);

	len += 10;
	crc = crc16(outbuf, len, 0XFFFF);
	UINT16_TO_STREAM_f(outbuf+len, crc);

	encrypt_msg(outbuf, 4, 8+datalen, salt, FACTOR);

	return 12+datalen;
}

int _parse_dispatch_ipinfo(char *ipnode, dispatch_ipinfo *ipinfo)
{
	char *ptr = ipnode;
	while (*ptr != '\0' && *ptr != ':') {
		ptr++;
	}

	if (*ptr != ':') {
		return -5;
	}

	if (ptr - ipnode >= 16) {
		return -6;
	}

	ql_memset(ipinfo, 0x00, sizeof(dispatch_ipinfo));

	ql_memcpy(ipinfo->ip, ipnode, ptr-ipnode);
	ipinfo->port = atoi(ptr+1);

	return 0;
}

int qlcloud_parse_dispatch_info(unsigned char *data, int datalen, dispatch_ipinfo *ipinfo)
{
    int len = 0;
    unsigned int salt = 0;
    unsigned short lcrc = 0;
    unsigned short crc = 0;
    unsigned short magic = 0;
    int ret = 0;
    unsigned char temp[256];

    if (data == NULL) {
        return -1;
    }

    if (datalen <= 12) {
        return -2;
    }

    salt = STREAM_TO_UINT32_f(data, 0);

    decrypt_msg(data, 4, datalen-4, salt, FACTOR);

    lcrc = crc16(data, datalen-2, 0xFFFF);

    magic = STREAM_TO_UINT16_f(data, 4);
    if (magic != dpc_magic) {
    	return -3;
    }

    len = STREAM_TO_UINT32_f(data, 6);
    v_strncpy((char *)temp, sizeof(temp), (char *)(data+10), len);

    crc = STREAM_TO_UINT16_f(data, 10+len);
    if (lcrc != crc) {
        ret = -4;
    } else {
    	ret = _parse_dispatch_ipinfo((char *)temp, ipinfo);
    }

    return ret;
}

#define CONNECTION_TIMEOUT	2000//ms
int qlcloud_open_dispatch_socket(ql_socket_t *socket, struct client_handle* client)
{
	int ret = 0;
	char ip[32];
	ql_memset(ip, 0x00, sizeof(ip));

    ret = ql_gethostbyname(client->dispatch_ip, ip, sizeof(ip));
	if (ret == -1) {       
        ql_log_err(ERR_EVENT_INTERFACE,"get ip err");
		return -1;
	}
    ret = ql_tcp_connect(socket, (unsigned char *)ip, client->dispatch_port, CONNECTION_TIMEOUT);

    if (ret < 0) {
        ql_log_err(ERR_EVENT_INTERFACE, "dispch conn err: %d", ret);
		return -1;
	}

	return 0;
}

void qlcloud_close_dispatch_socket(ql_socket_t *socket)
{
	ql_tcp_disconnect(socket);
}

int qlcloud_get_dispatch_ipinfo(dispatch_ipinfo *ipinfo, struct client_handle* client, ql_socket_t *socket)
{
    unsigned char sendbuf[128];
    unsigned char rbuf[128] = {0};
    int ret = 0;
    int slen = 0;
    int i = 0;
    int result = -1;

    if (ipinfo == NULL || client == NULL || socket == NULL) {
    	return -1;
    }

    dpc_magic = (client->encrypt_type == QL_ENCRYPT_TYPE_SSL) ? MAGIC_SSL : MAGIC_AES;
    
    if (qlcloud_open_dispatch_socket(socket, client) == -1) {
       ql_log_err(ERR_EVENT_INTERFACE, "dispch open err");
    	return -1;
    }

    slen = qlcloud_create_dispatch_request(client->mac_str, sendbuf, sizeof(sendbuf));
    if (-1 == slen) {
    	qlcloud_close_dispatch_socket(socket);
        ql_log_err(ERR_EVENT_INTERFACE, "dispch req err");
        return -1;
    }

    for (i = 0; i < 2; i++) {
        ret = ql_tcp_send(socket, sendbuf, slen, CONNECTION_TIMEOUT);
        if (ret == -1) {
            ql_log_err(ERR_EVENT_INTERFACE, "dispch send err");
            qlcloud_close_dispatch_socket(socket);
            return -1;
        }
        else
        {
            if (ret != slen)
            {
                ql_log_err(ERR_EVENT_INTERFACE, "dispch send err");
                continue;
            }
            else
            {
            }
        }

        ret = ql_tcp_recv(socket, rbuf, sizeof(rbuf), CONNECTION_TIMEOUT);
        if(ret == -1)
        {
            ql_log_err(ERR_EVENT_INTERFACE, "dispch recv err");
        }
        else
        {
            ret = qlcloud_parse_dispatch_info(rbuf, ret, ipinfo);
            if (0 == ret) {
                result = 0;
                break;
            }
            else
            {
                ql_log_err(ERR_EVENT_INTERFACE, "dispch parse err:%d", ret);
            }
        }
        ql_sleep_s(2);
    }

	qlcloud_close_dispatch_socket(socket);

    return result;
}
