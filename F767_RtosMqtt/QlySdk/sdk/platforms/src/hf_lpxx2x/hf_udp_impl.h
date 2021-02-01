/*
 * Copyright (c) 2018 qinglianyun.
 * All rights reserved.
 */

#ifndef HF_UDP_IMPL_H_
#define HF_UDP_IMPL_H_

#include "hsf.h"
#include "ql_include.h"

struct hf_udp_client_st
{
	int udp_sockfd;
	uip_ipaddr_t udp_peeraddr;
	unsigned int udp_peerport;
};

int hf_udp_bind(ql_udp_socket_t *sock);

extern struct hf_udp_client_st hf_udp_client;

#endif /* HF_UDP_IMPL_H_ */
