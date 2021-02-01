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
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTT.h"
extern unsigned int os_tick;
char expired(Timer* timer)
{
	long left = timer->end_time - os_tick;
	return (left < 0);
}

/*
void countdown_ms(Timer* timer, unsigned int timeout) {
	timer->end_time = os_tick + timeout;
}

int left_ms(Timer* timer) {
	long left = timer->end_time - os_tick;
	return (left < 0) ? 0 : left;
}
*/

void countdown(Timer* timer, unsigned int timeout) {
	timer->end_time = os_tick + timeout;
}

void InitTimer(Timer* timer) {
	timer->end_time = 0;
	timer->systick_period = 0;
}
#include <errno.h>
int my_socket_read(Network* n, unsigned char* buffer, int len, int timeout_ms) {
	int recvlen = 0;
	recvlen = ql_tcp_recv(n->my_socket, buffer, len, timeout_ms);
	if (recvlen == -1) {
		ql_tcp_disconnect(n->my_socket);

		if (n->disconn_cb) {
            ql_log_err( ERR_EVENT_INTERFACE, "sock_rd err");
			n->disconn_cb(CLI_DISCONN_TYPE_RD_CLS);
		}
	}
	return recvlen;
}

int my_socket_write(Network* n, unsigned char* buffer, int len, int timeout_ms) {
	int rc = 0;

	rc = ql_tcp_send(n->my_socket, buffer, len, timeout_ms);
	if (rc == -1) {
		ql_tcp_disconnect(n->my_socket);
		if (n->disconn_cb) {
            ql_log_err( ERR_EVENT_INTERFACE, "sock_wt err");
			n->disconn_cb(CLI_DISCONN_TYPE_WRT_CLS);
		}
	}
	return rc;
}


void my_socket_disconnect(Network* n)
{
	if (n && n->my_socket) {
		ql_tcp_disconnect(n->my_socket);
	}
}

void NewNetwork(Network* n, void (*disconn_cb)(CLI_STATU_T), ql_socket_t *socket)
{
	n->my_socket = socket;
	n->mqttread = my_socket_read;
	n->mqttwrite = my_socket_write;
	n->disconnect = my_socket_disconnect;

	n->disconn_cb = disconn_cb;
}

int ConnectNetwork(Network* n, char* addr, int port)
{
	int ret = 0;

	if (n->my_socket == NULL) {
		return -1;
	}

	ret = ql_tcp_connect(n->my_socket, (unsigned char *)addr, port, 5000);
	if (ret < 0) {
		return ret;
	}

	return 0;
}
