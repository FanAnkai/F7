
#include "hf_udp_impl.h"
#include "ql_buffer.h"

struct hf_udp_client_st hf_udp_client;

static int app_udp_recv_callback(NETSOCKET socket, 
	unsigned char *data, 
	unsigned short len,
	uip_ipaddr_t *peeraddr, 
	unsigned short peerport)
{
	int ret = 0;

	hf_udp_client.udp_sockfd = socket;
	memcpy(&(hf_udp_client.udp_peeraddr), peeraddr, sizeof(uip_ipaddr_t));
	hf_udp_client.udp_peerport = peerport;

	ret = data_buf_ioctl(DATA_IOCTL_SET_DATA, DATA_UDP, data, len);
	if(ret < 0)
	{
		ql_log_err("data_buf_ioctl err:%d\r\n", ret);
	}

	return 0;
}

int hf_udp_bind(ql_udp_socket_t *sock)
{
	struct udp_socket u_socket;

	if (sock == NULL) {
        return -1;
    }
	
	u_socket.l_port = sock->local_port;
	u_socket.recv_callback = app_udp_recv_callback;
	u_socket.recv_data_maxlen = 1024;
	sock->sockfd = hfnet_udp_create(&u_socket);
	ql_log_info("%s, sock->sockfd:%d\r\n", __FUNCTION__, sock->sockfd);
	if(sock->sockfd<0)
	{
		ql_log_err("create udp failed\r\n");
		return -1;
	}
	return 0;
}

