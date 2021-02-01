#ifndef QINGLIANCLOUD_CLOUD_DISPATCH_ACCESS_H_
#define QINGLIANCLOUD_CLOUD_DISPATCH_ACCESS_H_

#include "ql_include.h"

typedef struct _dispatch_ipinfo_s {
	char ip[16];
	int  port;
} dispatch_ipinfo;

int qlcloud_get_dispatch_ipinfo(dispatch_ipinfo *ipinfo, struct client_handle* client, ql_socket_t *socket);

#endif /* QINGLIANCLOUD_CLOUD_DISPATCH_ACCESS_H_ */
