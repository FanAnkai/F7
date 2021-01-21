

#include "tcp_client.h"
#include "printf.h"
#include "kfifo.h"

//TCP服务器发送数据内容

uint8_t tcp_client_sendbuf[] = "NUCLEO STM32F767ZI TCP Client Is Online\r\n";


USER_TCP_TYPE gTcpReg = {0};

uint8_t recv_buff[100] = {0};
uint8_t send_buff[100] = {0};


err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{
    err_t ret_err;
	struct pbuf *p_temp;
	uint32_t data_len = 0;
	
	if(err!= ERR_OK)
    {
        if(p)
		{
			pbuf_free(p);
		}
        ret_err=err;
    }
	
	if(p!=NULL)//当处于连接状态并且接收到的数据不为空时
    {
        for(p_temp=p; p_temp!=NULL; p_temp=p_temp->next) //遍历完整个pbuf链表
        {
            //判断要拷贝到TCP_CLIENT_RX_BUFSIZE中的数据是否大于TCP_CLIENT_RX_BUFSIZE的剩余空间，如果大于
            //的话就只拷贝TCP_CLIENT_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
            if(p_temp->len < (TCP_BUFF_MAX-my_fifo_len(&gFifoRx))) 
			{
				my_fifo_put(&gFifoRx, p_temp->payload, p_temp->len);
				data_len += p_temp->len;
			}
            else
			{
				break;
			}
        }
        tcp_recved(tpcb,p->tot_len);	//用于获取接收数据,通知LWIP可以获取更多数据
        pbuf_free(p);				//释放内存
        data_len = 0;
	}

	return ret_err;
}
//lwIP tcp_err函数的回调函数
void tcp_client_error(void *arg,err_t err)
{
	gTcpReg.is_connect = 0;
    printf("%s  %d\r\n", __func__, __LINE__);
}
//lwIP tcp_poll的回调函数
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
    err_t ret_err;
	send_data_poll(tpcb);
    return ret_err;
}
//lwIP tcp_sent的回调函数(当从远端主机接收到ACK信号后发送数据)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    LWIP_UNUSED_ARG(len);
	//有数据发送
	send_data_poll(tpcb);
	
    return ERR_OK;
}
//此函数用来发送数据
void tcp_client_senddata(struct tcp_pcb *tpcb, uint8_t * data, uint16_t len)
{
	tcp_write(tpcb,data,len,1);
	tcp_output(tpcb);
}

//关闭与服务器的连接
void tcp_client_connection_close(struct tcp_pcb *tpcb)
{
    tcp_abort(tpcb);//终止连接,删除pcb控制块
    tcp_arg(tpcb,NULL);
    tcp_recv(tpcb,NULL);
    tcp_sent(tpcb,NULL);
    tcp_err(tpcb,NULL);
    tcp_poll(tpcb,NULL,0);

	gTcpReg.is_connect = 0;
}


err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    if(err==ERR_OK)
    {
		tcp_recv(tpcb,tcp_client_recv);  	//初始化LwIP的tcp_recv回调功能
		tcp_err(tpcb,tcp_client_error); 	//初始化tcp_err()回调函数
		tcp_sent(tpcb,tcp_client_sent);		//初始化LwIP的tcp_sent回调功能
		tcp_poll(tpcb,tcp_client_poll,1); 	//初始化LwIP的tcp_poll回调功能
		gTcpReg.is_connect = 1; 				//标记连接到服务器了
		err=ERR_OK;
    }
    else
    {
        tcp_client_connection_close(tpcb);//关闭连接
    }
    return err;
}



uint8_t remoteip[] = {192, 168, 31, 105};
struct tcp_pcb* tcp_pcb = NULL;
ip_addr_t rmtipaddr;

void task_tcp_client(void)
{
    static uint16_t i = 0;
    static uint16_t num = 0;

	
	if(gTcpReg.work_step == 0)
	{
		tcp_pcb = tcp_new();	//创建一个新的pcb

		if(tcp_pcb)			//创建成功
		{
			IP4_ADDR(&rmtipaddr,remoteip[0],remoteip[1],remoteip[2],remoteip[3]);
			tcp_connect(tcp_pcb,(const ip_addr_t *)&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
			gTcpReg.work_step = 1;
		}
		else
		{
			gTcpReg.work_step = 0;
		}
	}
	else if(gTcpReg.work_step == 1)
	{
		i++;
		if(i == 500)//发送数据
		{
			i = 0;
			num++;

			if(gTcpReg.is_connect == 0)//断开连接
			{
				tcp_client_connection_close(tcp_pcb);//关闭TCP Client连接
				tcp_pcb = (struct tcp_pcb*)tcp_new();	//创建一个新的pcb

				if(tcp_pcb) 		//创建成功
				{
					tcp_connect(tcp_pcb,(const ip_addr_t *)&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
				}
			}
			else
			{
				//发送心跳
				my_fifo_put(&gFifoTx, tcp_client_sendbuf, sizeof(tcp_client_sendbuf));

				
			}
		}

		if(gTcpReg.is_connect == 1)//已连接
		{
			if(gTcpReg.connect_stat == 0)
			{
				gTcpReg.connect_stat = 1;
				printf("\r\n%s\r\n", "ConnectSuccess");
			}

			//有数据接收
			while(my_fifo_len(&gFifoRx) > 0)
			{
				uint8_t get_len = 0;
				get_len = my_fifo_get(&gFifoRx, recv_buff, sizeof(recv_buff));

				print_buff(recv_buff, get_len);
			}
			
			send_data_poll(tcp_pcb);

		}
		else
		{
			if(gTcpReg.connect_stat == 1)
			{
				gTcpReg.connect_stat = 0;
				printf("\r\n%s\r\n", "Disconnected");
			}
		}

		MX_LWIP_Process();
	}

}


uint16_t tx_fifo_len = 0;
void send_data_poll(struct tcp_pcb* tcp_pcb)
{
	//有数据发送
	tx_fifo_len = my_fifo_len(&gFifoTx);
	if(tx_fifo_len > 0)
	{
		uint8_t get_len = 0;
		get_len = my_fifo_get(&gFifoTx, send_buff, sizeof(send_buff));

		tcp_client_senddata(tcp_pcb, send_buff, get_len);
	}
}


