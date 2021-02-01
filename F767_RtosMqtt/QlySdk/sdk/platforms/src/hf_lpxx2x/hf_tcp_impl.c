#include "hf_tcp_impl.h"
#include "hsf.h"
#include "ql_buffer.h"
#include "ql_include.h"

PROCESS(hf_tcp_process, "hf_tcp_process");

struct hf_tcp_client_st hf_tcp_client;
enum TCP_CLIENT_FLAG g_tcp_connect_flag = TCP_CLIENT_WAIT;

void tcp_process_start(void)
{
    process_start(&hf_tcp_process, NULL);
}

void post_event_tcp_process(void)
{
	process_post(&hf_tcp_process, PROCESS_EVENT_CONTINUE, NULL);
}

extern void post_event_logic_main_process(void);
static int app_tcp_recv_callback(NETSOCKET socket, unsigned char *data, unsigned short len)
{
	int ret = 0;

	// ql_log_info("app_tcp_recv_callback len:%d\r\n", len);

	ret = data_buf_ioctl(DATA_IOCTL_SET_DATA, DATA_TCP, data, len);
	if(ret < 0)
	{
		ql_log_err("data_buf_ioctl err:%d\r\n", ret);
	}

	post_event_logic_main_process();
	return 0;
}

static void app_tcp_connect_callback(NETSOCKET socket)
{
	ql_log_info("socket %d connected.\r\n", socket);
	ql_buffer_init();
	g_tcp_connect_flag = TCP_CLIENT_CONNECTED;

	post_event_tcp_process();
}

static void app_tcp_close_callback(NETSOCKET socket)
{
	ql_log_info("socket %d closed, current socket is %d.\r\n", socket, hf_tcp_client.tcp_sockfd);
	if(socket == hf_tcp_client.tcp_sockfd)
	{
		g_tcp_connect_flag = TCP_CLIENT_CLOSING;
	}

	post_event_tcp_process();
}

static void app_tcp_send_callback(NETSOCKET socket)
{
	// ql_log_info("TCP data send ack.\r\n");
	g_tcp_connect_flag = TCP_CLIENT_SENDACK;
	
	post_event_tcp_process();
}

static NETSOCKET tcp_client_connect(uip_ipaddr_t *addr, unsigned short port)
{
	struct tcp_socket t_socket;
	
	uip_ipaddr_copy(&t_socket.r_ip, addr);
	t_socket.r_port = port;
	t_socket.recv_callback = app_tcp_recv_callback;
	t_socket.connect_callback = app_tcp_connect_callback;
	t_socket.close_callback = app_tcp_close_callback;
	t_socket.send_callback = app_tcp_send_callback;
	t_socket.recv_data_maxlen = hf_tcp_client.tcp_recive_max_len;

	return hfnet_tcp_connect(&t_socket);
}

PROCESS_THREAD(hf_tcp_process, ev, data)
{
	PROCESS_BEGIN();
	static struct etimer timer_sleep;

	etimer_set(&timer_sleep, 3 * CLOCK_SECOND);
	ql_log_info("hf_tcp_process is begin\r\n");

	ql_buffer_init();

	while(1) 
	{
		PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER || ev == PROCESS_EVENT_CONTINUE || ev == resolv_event_found);

		if(ev == PROCESS_EVENT_TIMER)
		{
			if((g_tcp_connect_flag == TCP_CLIENT_DNSING)||(g_tcp_connect_flag == TCP_CLIENT_CONNECTING))
			{
				ql_log_err("DNS or connect tiemout. g_tcp_connect_flag:%d\r\n", g_tcp_connect_flag);
				
				g_tcp_connect_flag = TCP_CLIENT_WAIT;
			}

			if((g_tcp_connect_flag == TCP_CLIENT_SENDING)||(g_tcp_connect_flag == TCP_CLIENT_RECING))
			{
				ql_log_err("Send or recv tiemout.\r\n");
			}
		}
	
		if(g_tcp_connect_flag == TCP_CLIENT_DNS)
		{
			uip_ipaddr_t addr;

			ql_log_info("g_tcp_connect_flag == TCP_CLIENT_DNS\r\n");
			
			if(uiplib_ipaddrconv(hf_tcp_client.dns_name, &addr) == 0)
			{
				ql_log_info("DNS:%s.\r\n", hf_tcp_client.dns_name);
				resolv_query(hf_tcp_client.dns_name);
				g_tcp_connect_flag = TCP_CLIENT_DNSING;
			}
			else
			{
				ql_log_info("This is IP addr %s, don't need to DNS\r\n", hf_tcp_client.dns_name);
				strcpy(hf_tcp_client.dns_ip, hf_tcp_client.dns_name);
				g_tcp_connect_flag = TCP_CLIENT_DNSOK;
			}
			
			etimer_set(&timer_sleep, hf_tcp_client.dns_timout * CLOCK_SECOND);
			continue;
		}
		else if((g_tcp_connect_flag == TCP_CLIENT_DNSING)&&(ev == resolv_event_found))
		{
			char *pHostName = (char*)data;
			if(strcmp(pHostName, hf_tcp_client.dns_name))
				continue;

			uip_ipaddr_t addr;
			uip_ipaddr_t *addrptr = &addr;
			resolv_lookup(pHostName, &addrptr);
			ql_log_info("Resolv IP = %d.%d.%d.%d\n", addrptr->u8[0], addrptr->u8[1], addrptr->u8[2], addrptr->u8[3] );
			sprintf(hf_tcp_client.dns_ip, "%d.%d.%d.%d", addrptr->u8[0], addrptr->u8[1], addrptr->u8[2], addrptr->u8[3]);
	
			g_tcp_connect_flag = TCP_CLIENT_DNSOK;
			continue;
		}
		else if(g_tcp_connect_flag == TCP_CLIENT_CONNECT)
		{
			uip_ipaddr_t addr;
			if(uiplib_ipaddrconv(hf_tcp_client.tcp_ip, &addr) == 0)
			{
				ql_log_info("hf_tcp_client.tcp_ip:%s is error\r\n", hf_tcp_client.tcp_ip);
				g_tcp_connect_flag = TCP_CLIENT_WAIT;
			}
			else
			{
				hf_tcp_client.tcp_sockfd = tcp_client_connect(&addr, hf_tcp_client.tcp_port);
				if(hf_tcp_client.tcp_sockfd >= 0)
                {
					g_tcp_connect_flag = TCP_CLIENT_CONNECTING;
                    etimer_set(&timer_sleep, hf_tcp_client.tcp_connect_timout_ms * CLOCK_MINI_SECOND);
                }
				else
					g_tcp_connect_flag = TCP_CLIENT_WAIT;
			}
		}
		else if(g_tcp_connect_flag == TCP_CLIENT_CLOSING)
		{
			hf_tcp_client.tcp_sockfd = -1;
			g_tcp_connect_flag = TCP_CLIENT_CLOSED;
		}
		else//connected/sendakc/recved/closed
		{
			// etimer_set(&timer_sleep, 1 * CLOCK_SECOND);
		}
	}
	
	PROCESS_END();
}