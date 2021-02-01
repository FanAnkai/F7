/*
 * Copyright (c) 2018 qinglianyun.
 * All rights reserved.
 */

#ifndef HF_TCP_IMPL_H_
#define HF_TCP_IMPL_H_

#define DNS_TIMEOUT 10

struct hf_tcp_client_st
{
    char dns_name[32];
    char dns_ip[32];
	unsigned int dns_timout;
	int tcp_sockfd;
	char tcp_ip[32];
	unsigned int tcp_port;
	unsigned int tcp_recive_max_len;
	unsigned int tcp_connect_timout_ms;
};

enum TCP_CLIENT_FLAG
{
	TCP_CLIENT_WAIT=0x0,
	TCP_CLIENT_START,
	TCP_CLIENT_DNS,
	TCP_CLIENT_DNSING,
	TCP_CLIENT_DNSOK,
	TCP_CLIENT_CONNECT,
	TCP_CLIENT_CONNECTING,
	TCP_CLIENT_CONNECTED,
	TCP_CLIENT_SENDING,
	TCP_CLIENT_SENDACK,
	TCP_CLIENT_RECING,
	TCP_CLIENT_RECED,
	TCP_CLIENT_CLOSING,
	TCP_CLIENT_CLOSED
};

void tcp_process_start(void);
void post_event_tcp_process(void);

extern struct hf_tcp_client_st hf_tcp_client;
extern enum TCP_CLIENT_FLAG g_tcp_connect_flag;

#endif /* HF_TCP_IMPL_H_ */
