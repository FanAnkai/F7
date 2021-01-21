
#include "tcp_client.h"
#include "printf.h"

USER_TCP_TYPE gTcpReg = {0};

struct tcp_pcb * ptcp = NULL;


void local_ip_init(ip4_addr_t *ipaddr, ip4_addr_t *netmask, ip4_addr_t *gw)
{
	gTcpReg.local_ip[0] = 192;
	gTcpReg.local_ip[1] = 168;
	gTcpReg.local_ip[2] = 31;
	gTcpReg.local_ip[3] = 99;

	gTcpReg.local_mask[0] = 255;
	gTcpReg.local_mask[1] = 255;
	gTcpReg.local_mask[2] = 255;
	gTcpReg.local_mask[3] = 0;

	gTcpReg.local_gw[0] = 192;
	gTcpReg.local_gw[1] = 168;
	gTcpReg.local_gw[2] = 31;
	gTcpReg.local_gw[3] = 1;
	

	uint8_t buff[4];
	memcpy(buff, gTcpReg.local_ip, 4);
	IP4_ADDR(ipaddr, buff[0], buff[1], buff[2], buff[3]);

	memcpy(buff, gTcpReg.local_mask, 4);
	IP4_ADDR(netmask, buff[0], buff[1], buff[2], buff[3]);

	memcpy(buff, gTcpReg.local_gw, 4);
	IP4_ADDR(gw, buff[0], buff[1], buff[2], buff[3]);

}


void remot_ip_init(ip4_addr_t *ipaddr, uint16_t* port)
{
	gTcpReg.remot_ip[0] = 192;
	gTcpReg.remot_ip[1] = 168;
	gTcpReg.remot_ip[2] = 31;
	gTcpReg.remot_ip[3] = 105;

	gTcpReg.remot_port = 8080;

	uint8_t buff[4];
	memcpy(buff, gTcpReg.remot_ip, 4);
	IP4_ADDR(ipaddr, buff[0], buff[1], buff[2], buff[3]);

	*port = gTcpReg.remot_port;
}


ip4_addr_t	remot_ip;
uint16_t	remot_port;
uint16_t	task_i = 0;

void tcp_clinet_poll(void)
{
	
	
	if(gTcpReg.work_step == 0)
	{
		remot_ip_init(&remot_ip, &remot_port);
		ptcp = tcp_new();
		if(ptcp == NULL)
		{
			printf("%s", "tcp_new failed\r\n");
			return;
		}
		else
		{
			tcp_connect(ptcp, &remot_ip, remot_port, tcp_client_connect_cb);
		}
		
		gTcpReg.work_step = 1;
	}
	else if(gTcpReg.work_step == 1)
	{
		task_i++;
		if(task_i > 10000)
		{
			task_i = 0;
			if(gTcpReg.is_connect == 0)
			{
				tcp_client_connection_close(ptcp);
				gTcpReg.work_step = 0;
			}
		}
	}
}

err_t tcp_client_connect_cb(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	
	if(err==ERR_OK)
    {
		tcp_recv(tpcb, tcp_client_recv_cb);		//接收回调功能
		tcp_err(tpcb, tcp_client_err_cb);		//出错回调函数
		tcp_sent(tpcb, tcp_client_sent_cb);		//远端成功接收回调
		tcp_poll(tpcb, tcp_client_poll_cb, 1); 	//定期回调功能
		
		err=ERR_OK;
		gTcpReg.is_connect = 1;
		task_i = 0;
    }
    else
    {
        tcp_client_connection_close(tpcb);//关闭连接
    }
    return err;
}



err_t tcp_client_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	return err;
}

void  tcp_client_err_cb(void *arg, err_t err)
{
	gTcpReg.is_connect = 0;
	gTcpReg.work_step = 0;
	task_i = 0;
}


err_t tcp_client_sent_cb(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	//远端成功接收回调
	return 0;
}


err_t tcp_client_poll_cb(void *arg, struct tcp_pcb *tpcb)
{
	return 0;
}



//关闭与服务器的连接
void tcp_client_connection_close(struct tcp_pcb *tpcb)
{
    //移除回调
    tcp_abort(tpcb);//终止连接,删除pcb控制块
    tcp_arg(tpcb,NULL);
    tcp_recv(tpcb,NULL);
    tcp_sent(tpcb,NULL);
    tcp_err(tpcb,NULL);
    tcp_poll(tpcb,NULL,0);
	
	gTcpReg.is_connect = 0;
	gTcpReg.work_step = 0;
	task_i = 0;
	
}






