/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of and a contribution to the lwIP TCP/IP stack.
 *
 * Credits go to Adam Dunkels (and the current maintainers) of this software.
 *
 * Christiaan Simons rewrote this file to get a more stable echo example.
 *
 **/

 /* This file was modified by ST */


#include "config.h"
#include "lwip/tcp.h"
//#include "lwip/stats.h"
#include "lwip/mem.h"
#include "lwip/memp.h"

#include "tcp_server.h"

#include "net_AS.h"

extern LCAS_DEV_STATE_t devState;

//#define SYSEM


#if LWIP_TCP

//u8_t data_buff[610]={0x30,};
static struct tcp_pcb *tcp_server_pcb;
//uint8_t FrameSendRequest;
struct tcp_server_struct SvrStruct;
struct tcp_server_struct *g_es;
u8_t esstate_save;
struct tcp_pcb *g_pcb;
//struct pbuf *g_p;
GCSVC_ST gcsvc;
//extern GCDBG_ST gcdbg;

uint8_t ConnTime=0;

struct tcp_pcb *netAS_PCB;

signed char retERR;
uint32_t  sndCnt = 0;

volatile char Clarity_Connect = STATE_COMM_NOT_CONNECTED;	// CJKIM
volatile char g_TcpWindowState = STATE_TCP_NORMAL_WND;		//(+)210722 CJKIM, TCP Zero Window

const char* pkt_code_str(int code);

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_server_error(void *arg, err_t err);
static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_server_send(struct tcp_pcb *tpcb, struct tcp_server_struct *es);
static void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es);
static void tcp_server_connection_abort(struct tcp_pcb *tpcb, struct tcp_server_struct *es);		//(+)210722 CJKIM, added

err_t tcp_GCDB_Send(struct tcp_pcb *tpcb, struct tcp_server_struct *es, struct pbuf *gcd_pbuf);

extern void MSS_MAC_enable_intr();				//(+) 210621 CJKIM, For atomic tx .
extern void MSS_MAC_disable_intr();				//(+) 210621 CJKIM, For atomic tx .

extern void comm_disconnected_uvd_init();				//(+)210716 CJKIM, re-init uvd state when network disconnecting


/**
  * @brief  Initializes the tcp server
  * @param  None
  * @retval None
  */
void tcp_server_init(void)
{
	/* create new tcp pcb */

	tcp_server_pcb = tcp_new();

	if (tcp_server_pcb != NULL)
	{
		err_t err;

		/* bind pcb to port IP_PORT_NUMBER */
		err = tcp_bind(tcp_server_pcb, IP_ADDR_ANY, IP_PORT_NUMBER);

		if (err == ERR_OK)
		{
			/* start tcp listening for echo_pcb */
			tcp_server_pcb = tcp_listen(tcp_server_pcb);

			/* initialize LwIP tcp_accept callback function */
			tcp_accept(tcp_server_pcb, tcp_server_accept);
		}
		else 
		{
			DEBUG_E("() Can not bind pcb\n");
		}
	}
	else
	{
		DEBUG_E("() Can not create new pcb\n");
	}
}

/**
  * @brief  This function is the implementation of tcp_accept LwIP callback
  * @param  arg: not used
  * @param  newpcb: pointer on tcp_pcb struct for the newly created tcp connection
  * @param  err: not used 
  * @retval err_t: error status
  */
#if KCJ_TCP_SERVER
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	err_t ret_err;
	struct tcp_server_struct *es;

	//DEBUG_I(DBG_LEVEL_INFO, "() Client is connected \r\n");

	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	/* set priority for the newly accepted tcp connection newpcb */
	tcp_setprio(newpcb, TCP_PRIO_MIN);

	/* allocate structure es to maintain tcp connection informations */
	//es = (struct tcp_server_struct *)mem_malloc(sizeof(struct tcp_server_struct));
	es = &SvrStruct;
	g_es = es;

	newpcb->flags |= TF_NODELAY;

	if (es != NULL)
	{
		es->state = ES_ACCEPTED;
		es->pcb = newpcb;
		//   es->p = NULL;
		es->transmitBuf_length = 0;

		/* pass newly allocated es structure as argument to newpcb */
		tcp_arg(newpcb, es);
		/* initialize lwip tcp_recv callback function for newpcb  */ 
		tcp_recv(newpcb, tcp_server_recv);
		/* initialize lwip tcp_err callback function for newpcb  */
		tcp_err(newpcb, tcp_server_error);
#if 0 // RYU_TCP     
		/* initialize lwip tcp_poll callback function for newpcb */
		tcp_poll(newpcb, tcp_server_poll, 1);
#endif
		// RYU
		netAS_PCB = newpcb;

		//(-/+) 210611 CJKIM, For Debug
		DEBUG_I(DBG_LEVEL_INFO, "() TCP MSS[%d]\r\n", tcp_mss(newpcb));
	
		netConnection();	
//		LEDStateChange = 0;
		Clarity_Connect = STATE_COMM_CONNECTED;

		ret_err = ERR_OK;
	}
	else
	{
		/* return memory error */
		ret_err = ERR_MEM;
	}
	return ret_err;  
}
#else
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  err_t ret_err;
  struct tcp_server_struct *es;

dfp("Client is connected \r\n");

#if TCP_SERVER_CONNECTION_ONLY_ONE
	if(devState.connected == YES) {
	iprintf("Client connect ==> abort!!  \r\n");		
		ret_err = ERR_ABRT;
		return ret_err;
	}
#endif

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  /* set priority for the newly accepted tcp connection newpcb */
  tcp_setprio(newpcb, TCP_PRIO_MIN); //  tcp_setprio(newpcb, TCP_PRIO_MAX);

  /* allocate structure es to maintain tcp connection informations */
  //es = (struct tcp_server_struct *)mem_malloc(sizeof(struct tcp_server_struct));
  es = &SvrStruct;
  g_es = es;

 if (es != NULL)
  {
    es->state = ES_ACCEPTED;
    es->pcb = newpcb;
    es->p = NULL;
    
    /* pass newly allocated es structure as argument to newpcb */
    tcp_arg(newpcb, es);
    /* initialize lwip tcp_recv callback function for newpcb  */ 
    tcp_recv(newpcb, tcp_server_recv);
    /* initialize lwip tcp_err callback function for newpcb  */
    tcp_err(newpcb, tcp_server_error);
#if 1 // RYU_TCP     
    /* initialize lwip tcp_poll callback function for newpcb */
    tcp_poll(newpcb, tcp_server_poll, 1);
#endif

// RYU
netAS_PCB = newpcb;

netConnection();

    ret_err = ERR_OK;
  }
  else
  {
    /* return memory error */
    ret_err = ERR_MEM;
  }
  return ret_err;  
}
#endif

#if KCJ_TCP_SERVER
/**
  * @brief  This function is the implementation for tcp_recv LwIP callback
  * @param  arg: pointer on a argument for the tcp_pcb connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  pbuf: pointer on the received pbuf
  * @param  err: error information regarding the received pbuf
  * @retval err_t: error code
  */

#ifdef USED_RECV_QUEUE		//(+)210715 CJKIM, Received data process changing
void chrozen_recv()
{
#define	 MAX_BUFFER 1024

	int ret = 0;
	int packetLength = 0;

	char pBuffer[MAX_BUFFER];
	LCAS_PACKET* pPacket;

	do {
		
		if (get_available() >= PACKET_SIZE_MIN) {
			memset(pBuffer, 0 , MAX_BUFFER);
			ret = peek(pBuffer, 0, PACKET_SIZE_MIN);
			pPacket = (LCAS_PACKET*)(pBuffer); 

			packetLength = pPacket->nPacketLength;
			if (get_available() >= packetLength ) {		// Packet Data

				ret = dequeue(pBuffer, 0, packetLength);
				
//					DEBUG_I(DBG_LEVEL_INFO, "() RECV[%d]\r\n", ret);	//(+)210604 CJKIM, For Debug
//					ShowMemory(0, pBuffer, ret);
				
				if(PacketParse(pPacket->nPacketCode, pBuffer) == RE_ERR) 
				{
					DEBUG_E("() Failed to parse data[%d]\r\n", packetLength);
//					ShowMemory(0, pBuffer, packetLength);
				};
			} else {
				break;
			}
			
		} else {
			break;
		}
	}while(get_available() >= PACKET_SIZE_MIN);

}

#else
static bool chrozen_recv(struct tcp_pcb *tpcb, struct pbuf *p)
{
	bool retValue = true;
	UVD_PACKET* pPacket;
	int iPacketLength = 0;			// 읽을 패킷 길이
	static char acData_in[PACKET_BUFF_SIZE];
	static int iSaveDatainPos = 0;
	struct pbuf *pp = p;
	
	if(tpcb == NULL)
	{
		iSaveDatainPos = 0;
		return false;
	}

	//DEBUG_I(DBG_LEVEL_INFO, "() RECV[%d] : %s\r\n", p->len, dump_data(p->payload, 0, p->len));	//(+)210604 CJKIM, For Debug

	//DEBUG_I(DBG_LEVEL_INFO, "() RECV[%d]\r\n", p->len);	//(+)210604 CJKIM, For Debug
	//ShowMemory(0, p->payload, p->len);

	
	while(p != NULL)
	{
		if(p->len <= PACKET_BUFF_SIZE - iSaveDatainPos)
		{
			memcpy(acData_in + iSaveDatainPos, p->payload, p->len);
			iSaveDatainPos += p->len;
			p = p->next;
		}	

		while(iSaveDatainPos >= PACKET_SIZE_MIN) 
		{
			pPacket = (LCAS_PACKET*)(acData_in);	// + iPacketStartPos);
			iPacketLength = pPacket->nPacketLength;
			if(iPacketLength > 1024 ||
				(iPacketLength != PACKET_SIZE_MIN && iPacketLength != PACKET_SIZE_MIN + pPacket->nSlotSize))
			{
				iSaveDatainPos--;
				memcpy(acData_in, acData_in+1, iSaveDatainPos);
				continue;
			}
			if(iSaveDatainPos < iPacketLength) {	// 데이터를 더 받아야 한다.
				break;
			}

			if(PacketParse(pPacket->nPacketCode, acData_in) == RE_ERR) 
			{
				retValue = false;
			};

			iSaveDatainPos = iSaveDatainPos - iPacketLength;

			if(iSaveDatainPos != 0) 
			{ 
				memcpy(acData_in, acData_in + iPacketLength, iSaveDatainPos);		// 데이터를 맨 앞으로 이동
			}
			iPacketLength = 0;
		}
	}
	/* Update tcp window size to be advertized : should be called when received
	data (with the amount plen) has been processed by the application layer */
	tcp_recved(tpcb, pp->tot_len);
	
	pbuf_free(pp);

	DEBUG_I(DBG_LEVEL_INFO, "() [%d]\n", retValue);	//(+)210604 CJKIM, For Debug
	return retValue;
}
#endif

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	struct tcp_server_struct *es;
	err_t ret_err;
	
	LWIP_ASSERT("arg != NULL",arg != NULL);

	es = (struct tcp_server_struct *)arg;
	g_es = es;
	g_pcb = tpcb;

	/* if we receive an empty tcp frame from client => close connection */
	if (p == NULL)
	{
		DEBUG_E("() receive an empty tcp frame from client => close connection\r\n");
		/* remote host closed connection */
		es->state = ES_CLOSING;
		tcp_server_connection_close(tpcb, es);	//210624 CJKIM 

// 		if(es->p == NULL)
// 		{
// 			/* we're done sending, close connection */
// 			tcp_server_connection_close(tpcb, es);
// 		}
// 		else
// 		{
// #if 1 // RYU_TCP
// 			/* we're not done yet */
// 			/* acknowledge received packet */
// 			tcp_sent(tpcb, tcp_server_sent);

// 			/* send remaining data */
// 			tcp_server_send(tpcb, es);
// #endif			
//		}
		ret_err = ERR_OK;
	}
	/* else : a non empty frame was received from client but for some reason err != ERR_OK */
	else if(err != ERR_OK)
	{
		/* free received pbuf */
		if (p != NULL)
		{
			tcp_recved(tpcb, p->tot_len);
//			es->p = NULL;
			pbuf_free(p);
		}
		ret_err = err;
	}
	else if(es->state == ES_ACCEPTED)
	{
		/* first data chunk in p->payload */
		es->state = ES_RECEIVED;
		/* store reference to incoming pbuf (chain) */
//		es->p = p;
		/* initialize LwIP tcp_sent callback function */
#if 0
		tcp_sent(tpcb, tcp_server_sent);
#endif
		/* send back the received data (echo) */
//		tcp_server_send(tpcb, es);

		ret_err = ERR_OK;

		gcsvc.TCP_LineState=1;
		g_pcb 	= tpcb;
		g_es 	= es;
//		g_p 	= p;

#ifdef USED_RECV_QUEUE		//(+)210715 CJKIM, Received data process changing
		enqueue(p->payload, 0, p->len);

		/* Update tcp window size to be advertized : should be called when received
		data (with the amount plen) has been processed by the application layer */
		tcp_recved(tpcb, p->tot_len);

		pbuf_free(p);
#else
		chrozen_recv(NULL, NULL);
	
		if(!chrozen_recv(tpcb, p))
		{
			DEBUG_E("() Failed to parse received data.\r\n");
			//tcp_server_connection_close(tpcb, es);	//210624 CJKIM 
		}
#endif

		
	}
	else if (es->state == ES_RECEIVED)
	{
#ifdef USED_RECV_QUEUE		//(+)210715 CJKIM, Received data process changing
		enqueue(p->payload, 0, p->len);

		/* Update tcp window size to be advertized : should be called when received
		data (with the amount plen) has been processed by the application layer */
		tcp_recved(tpcb, p->tot_len);

		pbuf_free(p);
#else	
		chrozen_recv(tpcb, p);
#endif
		ret_err = ERR_OK;
	}

  /* data received when connection already closed */
	else
	{
		/* Acknowledge data reception */
		tcp_recved(tpcb, p->tot_len);

		/* free pbuf and do nothing */
		//es->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
	}
	return ret_err;
}

#else
/**
  * @brief  This function is the implementation for tcp_recv LwIP callback
  * @param  arg: pointer on a argument for the tcp_pcb connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  pbuf: pointer on the received pbuf
  * @param  err: error information regarding the received pbuf
  * @retval err_t: error code
  */
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	struct tcp_server_struct *es;
	err_t ret_err;

	static char acData_in[TCP_RECV_PACKET_BUFF_SIZE];

	static int iSaveDatainPos = 0;	// 저장해야할 acData_in[]의 어레이 위치 	
	LCAS_PACKET* pPacket;
	int iPacketLength = 0;	// 읽을 패킷 길이

	int find_data_cnt = 0; 

	LWIP_ASSERT("arg != NULL",arg != NULL);

	es = (struct tcp_server_struct *)arg;
	g_es = es;
	g_pcb = tpcb;

	/* if we receive an empty tcp frame from client => close connection */
	if (p == NULL)
	{
		/* remote host closed connection */
		es->state = ES_CLOSING;
		if(es->p == NULL)
		{
			/* we're done sending, close connection */
			tcp_server_connection_close(tpcb, es);
		}
		else
		{
// RYU_TCP
			/* we're not done yet */
			/* acknowledge received packet */
			tcp_sent(tpcb, tcp_server_sent);
			/* send remaining data */
			tcp_server_send(tpcb, es);
		}
		ret_err = ERR_OK;
	}
	/* else : a non empty frame was received from client but for some reason err != ERR_OK */
	else if(err != ERR_OK)
	{
		/* free received pbuf */
		if (p != NULL)
		{
			es->p = NULL;
			pbuf_free(p);
		}
		ret_err = err;
	}
	else if(es->state == ES_ACCEPTED)
	{
		/* first data chunk in p->payload */
		es->state = ES_RECEIVED;
		/* store reference to incoming pbuf (chain) */
		es->p = p;
		/* initialize LwIP tcp_sent callback function */
		tcp_sent(tpcb, tcp_server_sent);
		/* send back the received data (echo) */

//#ifndef SYSEM	
		tcp_server_send(tpcb, es);

		ret_err = ERR_OK;

		gcsvc.TCP_LineState=1;
		g_pcb 	= tpcb;
		g_es 	= es;

//==================================================================
// 처음 연결
//start==================================================================
		memcpy(acData_in + iSaveDatainPos, p->payload, p->len);
		iSaveDatainPos += p->len;

dp("iSaveDatainPos=%d len=%d\n",iSaveDatainPos,p->len);
		while(iSaveDatainPos >= PACKET_SIZE_MIN) 
		{
			pPacket = (LCAS_PACKET*)(acData_in);// + iPacketStartPos);
			iPacketLength = pPacket->nPacketLength;

			if(iSaveDatainPos < iPacketLength) {	// 데이터를 더 받아야 한다.
				break;
			}
//iprintf("1-PacketParse iSaveDatainPos=%d len=%d\n",iSaveDatainPos,p->len);
			if(PacketParse(pPacket->nPacketCode, acData_in) == RE_ERR) {
#if 1
// 종료 test--------------------------------------------
tcp_close(tpcb);
//tcp_server_connection_close(tpcb, es);
dp("test tcp_close\n");
#endif				
			};

			iSaveDatainPos = iSaveDatainPos - iPacketLength;
//dp("--iSaveDatainPos=%d\n",iSaveDatainPos);
			if(iSaveDatainPos != 0) {
				memcpy(acData_in, acData_in + iPacketLength, iSaveDatainPos);		// 데이터를 맨 앞으로 이동
			}
			iPacketLength = 0;
		}
//end==================================================================
//==================================================================
dp("First data received\r\n");

	}
	else if (es->state == ES_RECEIVED)
//	else if (es->state == ES_RECEIVED || es->state == ES_ACCEPTED)
	{
		/* more data received from client and previous data has been already sent*/
		if(es->p == NULL)
		{
			es->p = p;	//#ifndef SYSEM	

//==================================================================
//start==================================================================
#if TCP_SERVER_RECV_MODIFY
			if(p->len > TCP_RECV_PACKET_BUFF_SIZE) {
				return ERR_ABRT;
			}

			memcpy(acData_in + iSaveDatainPos, p->payload, p->len);
			iSaveDatainPos += p->len;

			while(iSaveDatainPos >= PACKET_SIZE_MIN)
			{
				pPacket = (LCAS_PACKET*)(acData_in + find_data_cnt);
				iPacketLength = pPacket->nPacketLength;

				if(iSaveDatainPos < iPacketLength) {	// 데이터를 더 받아야 한다. 
//iprintf(" err if(iSaveDatainPos(%d) < iPacketLength)=%d\n",iSaveDatainPos, iPacketLength);
					break;
				}

//				iprintf(" parse Length=%d nSlotSize=%d\n",iPacketLength, pPacket->nSlotSize);
				if(iPacketLength > TCP_PACKET_LENGTH_MAX ||	
					(iPacketLength != PACKET_SIZE_MIN && iPacketLength != PACKET_SIZE_MIN + pPacket->nSlotSize))
				{
					iSaveDatainPos--;
					find_data_cnt++;
					continue;
				}
				
//iprintf(" 2-Parse Pos=%d\n",iSaveDatainPos);
				if(PacketParse(pPacket->nPacketCode, acData_in) == RE_ERR) {
// 종료 test--------------------------------------------
//	tcp_close(tpcb);		// dp("test tcp_close\n");
// tcp_server_connection_close(tpcb, es);
				};

				iSaveDatainPos = iSaveDatainPos - iPacketLength;
				if(iSaveDatainPos != 0) { 
					memcpy(acData_in, acData_in + iPacketLength + find_data_cnt, iSaveDatainPos); 	// 데이터를 맨 앞으로 이동
				}
				
				find_data_cnt = 0;
			}
#else
			memcpy(acData_in + iSaveDatainPos, p->payload, p->len);
			iSaveDatainPos += len;
dp("iSaveDatainPos=%d len=%d\n",iSaveDatainPos,len);

			while(iSaveDatainPos >= PACKET_SIZE_MIN) 
			{
				pPacket = (LCAS_PACKET*)(acData_in);// + iPacketStartPos);
				iPacketLength = pPacket->nPacketLength;

				if(iSaveDatainPos < iPacketLength) {	// 데이터를 더 받아야 한다.
					break;
				}
//iprintf("2-PacketParse iSaveDatainPos=%d len=%d\n",iSaveDatainPos,len);
				if(PacketParse(pPacket->nPacketCode, acData_in) == RE_ERR) {

	// 종료 test--------------------------------------------
//	tcp_close(tpcb);
// tcp_server_connection_close(tpcb, es);
dp("test tcp_close\n");
				};

				iSaveDatainPos = iSaveDatainPos - iPacketLength;
				if(iSaveDatainPos != 0) { 
					memcpy(acData_in, acData_in + iPacketLength, iSaveDatainPos);		// 데이터를 맨 앞으로 이동
				}
				iPacketLength = 0;

			}
#endif			
//end==================================================================
//==================================================================
		}
		else
		{
			struct pbuf *ptr;
			/* chain pbufs to the end of what we recv'ed previously  */
			ptr = es->p;
			pbuf_chain(ptr,p);
		}
		ret_err = ERR_OK;
  }
  /* data received when connection already closed */
  else
  {
		/* Acknowledge data reception */
		tcp_recved(tpcb, p->tot_len);
dp("test tcp_recved =%d \n",p->tot_len);
		/* free pbuf and do nothing */
		es->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
  }
  return ret_err;
}
#endif

/**
  * @brief  This function implements the tcp_err callback function (called
  *         when a fatal tcp_connection error occurs. 
  * @param  arg: pointer on argument parameter 
  * @param  err: not used
  * @retval None
  */
static void tcp_server_error(void *arg, err_t err)
{
	struct tcp_server_struct *es;

#ifdef LWIP_DEBUG
	DEBUG_E("() Errror Code[%d, %s]\r\n", err, lwip_strerr(err));
#else
	DEBUG_E("() Errror Code[%d]\r\n", err);
#endif

	LWIP_UNUSED_ARG(err);
	es = (struct tcp_server_struct *)arg;

	//210615 CJKIM , 비정상으로 연결 해제 시 close
	tcp_server_connection_close(netAS_PCB, es);	//210624 CJKIM

	if (es != NULL)
	{
		/*  free es structure */
		es->transmitBuf_length = 0;
		//mem_free(es);	//210624 CJKIM , enable
						//210701 CJKIM , tcp_server_accept() 함수에서 pointer로 사용 하지 않음. 메모리 해제 금지
	}
}

/**
  * @brief  This function implements the tcp_poll LwIP callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: pointer on the tcp_pcb for the current tcp connection
  * @retval err_t: error code
  */
static err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_server_struct *es;

	es = (struct tcp_server_struct *)arg;
	if (es != NULL)
	{
		if (es->transmitBuf_length)
		{
			tcp_server_send(tpcb, es);
		}
		else
		{
			/* no remaining pbuf (chain)  */
			if(es->state == ES_CLOSING)
			{
				DEBUG_E("() no remaining pbuf (chain)  => close connection\r\n");
				/*  close tcp connection */
				tcp_server_connection_close(tpcb, es);
			}
		}
		ret_err = ERR_OK;
	}
	else
	{
		/* nothing to be done */
		tcp_abort(tpcb);
		ret_err = ERR_ABRT;
	}
	return ret_err;
}

/**
  * @brief  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data) 
  * @param  None
  * @retval None
  */
static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_server_struct *es;

	LWIP_UNUSED_ARG(len);

	es = (struct tcp_server_struct *)arg;

	if(es->transmitBuf_length > 0)
	{
		/* still got pbufs to send */
		//    tcp_server_send(tpcb, es);
	}
	else
	{
		/* if no more data to send and client closed connection*/
		if(es->state == ES_CLOSING) {
			DEBUG_E("() no more data to send and client closed connection  => close connection\n", __FUNCTION__);
			tcp_server_connection_close(tpcb, es);
		}
	}
	return ERR_OK;
}


void chrozen_send(struct tcp_pcb *tpcb, struct tcp_server_struct *es, char * p, int size)
{
	
	LCAS_PACKET *pPkt = p; 
	DEBUG_I(DBG_LEVEL_INFO, "() PKT Code[x%X, %s]\r\n", pPkt->nPacketCode, pkt_code_str(pPkt->nPacketCode));
	
	//(+)210615 CJKIM, Clarity 와 연결 되었을 시에만 전송 
	//(+)210722 CJKIM, TCP Zero Window 아닌 경우 에만 전송
	if ( STATE_COMM_CONNECTED == Clarity_Connect && STATE_TCP_NORMAL_WND == g_TcpWindowState ) {

		if(es != NULL)
		{
			int remain = (TCP_SEND_PACKET_BUFF_SIZE) - es->transmitBuf_length;
			if(size <= remain)
			{
				memcpy(es->transmitBuf + es->transmitBuf_length, p, size);
				es->transmitBuf_length += size;
		
				tcp_server_send(tpcb, es);
		
//					DEBUG_I(DBG_LEVEL_INFO, "() SEND[%d]\n", size);
//					ShowMemory(0, p, size );
			}
			else
			{
				DEBUG_E("() Transmit buffer has been fulled![%d, %d]\n", size, remain);
				tcp_server_connection_close(tpcb, es);	//210624 CJKIM 
				//tcp_server_connection_abort(tpcb, es);	//(+)210722 CJKIM, added
			}
		}

	}
	else {
		DEBUG_E("() SEND[%d] Failed to Send [%d, %d] \n", size, Clarity_Connect, g_TcpWindowState);
	}

}

/**
  * @brief  This function is used to send data for tcp connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void tcp_server_send(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{

   	err_t wr_err = ERR_OK;
	int len =0 ;
	int available = tcp_sndbuf(tpcb);	//(-/+) 210611 CJKIM, Debug 용으로 변수 변경.

	
	MSS_MAC_disable_intr();		//(+) 210621 CJKIM, disable mac interrupt before sending.

	if(es != NULL && available > 0 && es->transmitBuf_length > 0)
	{
		len = available < es->transmitBuf_length? available : es->transmitBuf_length;

		//(-/+) 210528 CJKIM, 1 -> TCP_WRITE_FLAG_COPY 변경, 값은 동일
     	wr_err = tcp_write(tpcb, es->transmitBuf, len, TCP_WRITE_FLAG_COPY);		
		if(wr_err == ERR_OK)
		{
			es->transmitBuf_length -= len;
			if(es->transmitBuf_length>0) {
				memcpy(es->transmitBuf, es->transmitBuf + len, es->transmitBuf_length);
				DEBUG_I(DBG_LEVEL_INFO, "() remian transmitBuf[%d]\n", es->transmitBuf_length);
			}
		}
	}

	// 210616 CJKIM , Send data when No Error
	if (ERR_OK == wr_err) {

		tcp_output(tpcb);	
		
		MSS_MAC_enable_intr();	//(+) 210621 CJKIM, enable mac interrupt after sending.
	}
	else {
		DEBUG_E("() Failed to write tcp data [%d, %d] [%d]\n", available, es->transmitBuf_length, wr_err);

		MSS_MAC_enable_intr();	//(+) 210621 CJKIM, enable mac interrupt after sending.
		
		// 210615 CJKIM , TCP Closing added
		if (ERR_MEM == wr_err) {
			tcp_server_connection_close(tpcb, es);	//210624 CJKIM 
			//tcp_server_connection_abort(tpcb, es);	//(+)210722 CJKIM, added
		}
	}

}

/**
  * @brief  This functions closes the tcp connection
  * @param  tcp_pcb: pointer on the tcp connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	DEBUG_I(DBG_LEVEL_INFO, "()\r\n");

#if SERVER_CLOSE_RUN_STOP_CMD
	comm_disconnected_uvd_init(); 	//(+)210716 CJKIM, re-init uvd state when network disconnecting
#endif

	gcsvc.TCP_LineState = 0;

	Clarity_Connect = STATE_COMM_NOT_CONNECTED;
	g_TcpWindowState = STATE_TCP_NORMAL_WND;		//(+)210722 CJKIM, TCP Zero Window

	if(devState.connected == YES) {
		devState.connected = NO;
		devState.disconnected = YES;
		netConnectAction();	
	}		

	/* remove all callbacks */
	tcp_arg(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, 0);

	/* delete es structure */
	if (es != NULL)
	{
		es->transmitBuf_length = 0;
		g_es = NULL;
		//mem_free(es);
	}

#if 1	// 210625 CJKIM Memory free added,  clarity에서 데이터 수신 안되어 close 반복 시 memory leak 발생.
	tcp_pcb_purge(tpcb);


#endif // 210625 CJKIM

	/* close tcp connection */
	tcp_close(tpcb);

	//LEDStateChange = 0;

#ifdef USED_RECV_QUEUE		//(+)210715 CJKIM, Received data process changing
	clear_queue();
#endif
}


static void tcp_server_connection_abort(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	DEBUG_I(DBG_LEVEL_INFO, "()\r\n");

#if SERVER_CLOSE_RUN_STOP_CMD
	comm_disconnected_uvd_init(); 	//(+)210716 CJKIM, re-init uvd state when network disconnecting
#endif

	gcsvc.TCP_LineState = 0;

	Clarity_Connect = STATE_COMM_NOT_CONNECTED;
	g_TcpWindowState = STATE_TCP_NORMAL_WND;		//(+)210722 CJKIM, TCP Zero Window

	if(devState.connected == YES) {
		devState.connected = NO;
		devState.disconnected = YES;
		netConnectAction();	
	}		
	
	/* remove all callbacks */
	tcp_arg(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, 0);

	/* delete es structure */
	if (es != NULL)
	{
		es->transmitBuf_length = 0;
		g_es = NULL;
		//mem_free(es);
	}

	tcp_pcb_purge(tpcb); 	 
	
	tcp_rst(tpcb->snd_nxt, tpcb->rcv_nxt, &tpcb->local_ip, &tpcb->remote_ip,
		tpcb->local_port, tpcb->remote_port);

	memp_free(MEMP_TCP_PCB, tpcb);

	//LEDStateChange = 0;

#ifdef USED_RECV_QUEUE		//(+)210715 CJKIM, Received data process changing
	clear_queue();
#endif
}



/*
 * GCDB(GC Data Block) send
 */
#if 0 
err_t tcp_GCDB_Send(struct tcp_pcb *tpcb, struct tcp_server_struct *es, struct pbuf *gcd_pbuf)
{
	err_t wr_err = ERR_OK;
	u16_t len;
	uint16_t i;
//xxx
	if(gcsvc.GCDB_SendEnable == 0) return ERR_OK;

	for(i=0;i<600;i++)
		data_buff[i] = hex2char_conv(i%10);
	data_buff[598] = '\r';
	data_buff[599] = '\n';

	len = 600;

	wr_err = tcp_write(tpcb, (void *)data_buff, len, 1);

	if (wr_err == ERR_OK)
	{
		tcp_output(g_pcb);
		gcsvc.fail_cnt = 0;
	}
	else
	{
		//dprintf("send fail\r\n");
		gcsvc.fail_cnt++;
		if(gcsvc.fail_cnt > 20)
		{
			gcsvc.GCDB_SendEnable = 0;
			dprintf("Auto send stop\r\n");
		}
	}

	return wr_err;
}
#endif

void tcp_esstate_print(void)
{
	if(GCC_MASTER != gcsvc.MasterSlave)
		return;

	if(esstate_save != g_es->state)
	{
		switch(g_es->state)
		{
			case ES_NONE : 		dprintf("NONE\r\n"); break;
			case ES_ACCEPTED : 	dprintf("ACCEPTED\r\n"); break;
			case ES_RECEIVED : 	dprintf("RECEIVED\r\n"); break;
			case ES_CLOSING : 	dprintf("CLOSING\r\n"); break;
		}

		esstate_save = g_es->state;
	}

	if(g_es->state == ES_CLOSING)
	{
		ConnTime++;
		if(ConnTime > 5)
		{
			ConnTime = 0;
			g_es->state = ES_NONE;
		}
	}
	else
		ConnTime = 0;
}

#endif /* LWIP_TCP */
