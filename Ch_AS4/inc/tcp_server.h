/*
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
 * This file is part of the lwIP TCP/IP stack.
 * 
 */
#ifndef __TCP_ECHOSERVER_H__
#define __TCP_ECHOSERVER_H__

#define GCC_MASTER   	0
#define GCC_SLAVE   	1
#define GCC_SLAVE_422   	3

#define STATE_COMM_NOT_CONNECTED 	0x00	// CJKIM, Client Connection state
#define STATE_COMM_CONNECTED 		0x01

#define STATE_TCP_NORMAL_WND 	0x00	//(+)210722 CJKIM, TCP Zero Window
#define STATE_TCP_ZERO_WND 		0x01	//(+)210722 CJKIM, TCP Zero Window

/* ECHO protocol states */
enum tcp_server_states
{
  ES_NONE = 0,
  ES_ACCEPTED,
  ES_RECEIVED,
  ES_CLOSING
};

typedef struct
{
	uint8_t		MasterSlave; //.Slave function is used for test.
	uint8_t		ApcDestId;
	uint8_t		ApcMyId;
	uint8_t		GCDB_SendEnable;
	uint16_t	GCDB_SendInterval; // unit (MS)
	uint16_t 	SendPcTime;
	uint8_t 	TCP_LineState; // 1:connected, else disconnected
	uint32_t	fail_cnt;
	uint32_t	pass_cnt;
} GCSVC_ST; // GC Service struct


/* structure for maintaing connection infos to be passed as argument 
   to LwIP callbacks*/
#if KCJ_TCP_SERVER		//(+)210715 CJKIM, Received data process changing
struct tcp_server_struct
{
	uint8_t state;             /* current connection state */
	struct tcp_pcb *pcb;    /* pointer on the current tcp_pcb */
	int transmitBuf_length;
//	char transmitBuf[PACKET_SIZE_MAX<<1];
	char transmitBuf[TCP_SEND_PACKET_BUFF_SIZE];// PACKET_SIZE_MAX];
} TCP_SERVER_STRICT;
#else
struct tcp_server_struct
{
  u8_t state;             /* current connection state */
  struct tcp_pcb *pcb;    /* pointer on the current tcp_pcb */
  struct pbuf *p;         /* pointer on the received/to be transmitted pbuf */
};
#endif

void chrozen_send(struct tcp_pcb *tpcb, struct tcp_server_struct *es, char * p, int size);

void tcp_server_init(void);

void tcp_send(void);
void tcp_esstate_print(void);


#if 0
int netConnectAction();

int net_SystemInform(char *pData);
int net_SystemConfig(char *pData);

void setAutoReadyRun();
int check_Oven(char *rcvData);
int net_Oven(char *pData);
//id TotalRunTimeCalc();

void applyOvenPacket4Slv();
void applyInletPacket4Slv();
void applyDetPacket4Slv();
void applyAuxPacket4Slv();
void applyValvePacket4Slv();
void applyStartStopPacket4Slv();

int check_Valve(char *rcvData);
int net_Valve(char *pData);

int check_Inlet(uint32_t portNo, char *rcvData);
int net_Inlet(uint32_t portNo, char *pData);
int apply_Inlet(uint32_t portNo);
void InletTempRcv_parainit(uint32_t portNo);
void applyInletPacket4Slv(uint32_t portNo);


int check_Det(uint32_t portNo, char *rcvData);
int net_Det(uint32_t portNo, char *pData);
int apply_Det(uint32_t portNo);
void DetTempRcv_parainit(uint32_t portNo);
void applyDetPacket4Slv(uint32_t portNo);
void DetSignalSetting(uint32_t portNo);

int Ignition_Cmd(uint32_t portNo);

int check_Signal(uint32_t portNo, char *rcvData);
int net_Signal(uint32_t portNo, char *pData);
int applySignal(uint32_t portNo);

int check_AuxTemp(char *rcvData);
int net_AuxTemp(char *pData);
int apply_AuxTemp();
void AuxTempRcv_parainit();
void applyAuxPacket4Slv();

int check_AuxAPC(uint32_t portNo, char *rcvData);
int net_AuxApc(uint32_t portNo, char *pData);
int apply_AuxApc(uint32_t portNo);


int check_Special(char *rcvData);
int net_Special(char *pData);

int check_Command(char *rcvData);
int net_Command(char *pData);


int (*pSend_State_Fn) (); // 함수포인터 선언
int send_State_Blank();
int send_State();

int (*pSend_SignalData_Fn)(uint32_t TestN);
int send_SignalData_Blank(uint32_t TestN);
int send_SignalData(uint32_t TestN);

int sendSignal_tcp();

int (*pLCD_Send_Data_Fn)();
//LCD_Send_DET_SIGNAL_DATA


int send_SelfMessage(unsigned char ucMessage, unsigned char ucNewValue);
int	errorFunction(unsigned char	err);
#endif
#endif /* __TCP_ECHOSERVER */
