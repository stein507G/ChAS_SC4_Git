/**
 * rs422_if.h
 */

#ifndef RS422_IF_H_
#define RS422_IF_H_

#define RS422_CHROZEN_LC_SLAVE_COUNT    		3
#define RS422_CONFIG_STASUS_SIZE    	64 // 256 bytes
#define RS422_CONFIG_STASUS_SIZE_SHIFT    	6

#define RS422_RUN_CPUMODE    		0
#define RS422_RUN_AUTOMODE    	1
#define RS422_STATE_IDLE   0
#define RS422_STATE_RUN    1


//=================================================================
// RS422
// ----------------------------------
//#define RS422_MASTER			0
//#define RS422_SLAVE				1

	#define RS422_ID_MASTER			0x0E
	#define RS422_ID_GLOBAL			0x0A

	#define RS422_ID_UVD			0x01
	#define RS422_ID_AS				0x02
	#define RS422_ID_OVEN			0x03
//	#define RS422_ID_PDA				0x04
//	#define RS422_ID_AUX				0x05
//-----------------------------------------------------------------

#define RS422_TX_STATE_ADDR			0x2200
#define RS422_MY_ID_ADDR			0x220F
#define RS422_SEND_TRIG_ADDR 		0x2210
#define RS422_TX_LENGTH_ADDR 		0x2211
#define RS422_TX_CRC_ADDR 			0x2212
#define RS422_MASTER_MODE_ADDR			0x2213			// 0:cpuode,1:Auto mode
#define RS422_TX_FLAG_RESET_ADDR		0x2220
// 20210713
#define RS422_TX_DATA_SAVED_FLAG_ADDR		0x2222

#define RS422_RX_STATE_ADDR		0x2600
#define RS422_RCV_GALLOW_ADDR		0x2602				// 0:Off(자신의 ID만 응답), 1:ON(ID에 상관없이 응답)
#define RS422_RX_INFO_ADDR			0x2610	
#define RS422_RX_AUTO_RESULT_ADDR		0x2613	
#define RS422_RX_FLAG_RESET_ADDR		0x2620

#define FPGA_WR_RS422_SEND_TRIG(X)		*(volatile uint32_t *) (FPGA_BASE_ADDR + (RS422_SEND_TRIG_ADDR<<2)) = (X)
#define FPGA_WR_RS422_TX_LENGTH(X)		*(volatile uint32_t *) (FPGA_BASE_ADDR + (RS422_TX_LENGTH_ADDR<<2)) = (X)

#define FPGA_WR_RS422_MODE_AUTO				FPGA_WRITE_WORD(RS422_MASTER_MODE_ADDR,1)
#define FPGA_WR_RS422_MODE_CPU				FPGA_WRITE_WORD(RS422_MASTER_MODE_ADDR,0)
#define FPGA_WR_RS422_TX_FLAG_CLEAR			FPGA_WRITE_WORD(RS422_TX_FLAG_RESET_ADDR,1)
//#define FPGA_WR_RS422_RX_FLAG_CLEAR		*(volatile uint32_t *) (FPGA_BASE_ADDR + (RS422_RX_FLAG_RESET_ADDR<<2)) = 1
#define FPGA_WR_RS422_RX_FLAG_CLEAR			FPGA_WRITE_WORD(RS422_RX_FLAG_RESET_ADDR,1)

// 20210713
#define FPGA_WR_RS422_TX_DATA_SAVED			FPGA_WRITE_WORD(RS422_TX_DATA_SAVED_FLAG_ADDR,1)

#define RS422_RX_CMD_ADDR						0x2400
	#define AUTO_MODE_CMD    				0x000000C0		// Auto Mode Master -> Slave
	//#define CPU_MODE_SOC_CMD    	0x000000A0		// Auto Mode Master -> Slave
	//#define CPU_MODE_APC_CMD    	0x000000B0		// Auto Mode Slave -> Master
	#define STATE_REQUEST_CMD    		0x000000B0		// Auto Mode Slave -> Master (자동응답 버퍼에 쌓인것을 읽는다.)
	//#define STATE_SEND_RET_LEN   	0x00000072		// 112 + 2
	#define RS422_STATE_SEND_CMD    0x000000B0		// Auto Mode Slave -> Master 
	#define RS422_CMD_PACKET_SEND		0x00000009		// Master -> Slave ( CMD Packet - Ethernet통신에서 사용하던 패킷과 동일하게 전송 )

#define RS422_RX_PACKET_LENGTH_ADDR		0x2401
#define RS422_RX_PACKET_CODE_ADDR			0x2403
#define RS422_RX_SLOT_OFFSET_ADDR			0x2405
#define RS422_RX_SLOT_SIZE_ADDR				0x2406









// TCP_WRITE_PACKET
#define RS422_ENQUEUE_PACKET { \
		r_enqueue(pPacketBuff, ((LCAS_PACKET *)pPacketBuff)->nPacketLength); \
	}

#define RS422_SEND_LCAS_PACKET(Code,full_data,entry_data)	{ \
	MAKE_LCAS_PACKET(Code,full_data,entry_data); \
	RS422_ENQUEUE_PACKET; \
}

#define RS422_SEND_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data)	{ \
	MAKE_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data); \
	RS422_ENQUEUE_PACKET; \
}

#define RS422_SEND_REQUEST_LCAS_PACKET(Code,full_data,entry_data)	{ \
	REQUEST_LCAS_PACKET(Code,full_data,entry_data); \
	RS422_ENQUEUE_PACKET; \
}

#define RS422_SEND_REQUEST_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data)	{ \
	REQUEST_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data); \
	RS422_ENQUEUE_PACKET; \
}

#define RS422_SEND_RESPONSE_LCAS_PACKET(full_data,ReqHeader)	{ \
	RESPONSE_LCAS_PACKET(full_data,ReqHeader); \
	RS422_ENQUEUE_PACKET; \
}

#define RS422_SEND_RESPONSE_LCAS_TABLE_PACKET(TableNo,full_data,ReqHeader)	{ \
	RESPONSE_LCAS_TABLE_PACKET(TableNo,full_data,ReqHeader); \
	RS422_ENQUEUE_PACKET; \
}

#define RS422_SEND_ACK_LCAS_PACKET(ReqHeader)	{ \
	ACK_LCAS_PACKET(ReqHeader); \
	RS422_ENQUEUE_PACKET; \
}


typedef struct
{
	uint8_t		RunMode; // 0:CPU Mode, 1:else Auto Mode
	uint8_t		State;
	uint8_t		slvch; 	// slave channel number
	
	uint8_t		RSTAT;
	uint8_t		rx_slvid;	

//	uint8_t		firstSend;	// 맨처음에는 
	uint8_t		TSTAT;			// 송신완료 면 1 , 아니면 0  // (1)송신버퍼에저장 , (0)송신버퍼에저장안됨 - 전송할 데이터가 있다.
	uint8_t 	busyTxHead; // 저장할 펑션 번지
	uint8_t 	busyTxTail; // 전송할 펑션 번지 마지막
	uint8_t 	busyFn[8];	// 전송할 펑션번호 
	
	uint16_t	TxTime;
	
	uint8_t		AutoEnd;
uint8_t		rx_intr;		// ryu 추가 
	uint8_t		RxAllAllow; // when this bit is set, receive the frame no matter what the destination id

	uint8_t		sendCmd[5];
	uint8_t		sendStart;
	uint8_t		sendSop;
	
//	uint32_t	Config[RS422_CHROZEN_LC_SLAVE_COUNT][RS422_CONFIG_STASUS_SIZE];
	uint32_t	MainStatus[RS422_CONFIG_STASUS_SIZE];
//	uint32_t	SlaveStatus[RS422_CHROZEN_LC_SLAVE_COUNT][RS422_CONFIG_STASUS_SIZE];

	uint8_t		TST_patternA;

	uint32_t	reSendData[5];

	uint8_t		enable; 	//  alive;

	uint32_t	connected;

} RS422_ST; // GC Service struct

#define RS422_TX_DATA_BUF_SIZE		1024
#define RS422_TX_DATA_BUF_SIZE4		256

typedef struct
{
	int			Head;		// 읽을 데이터 위치
	int			Tail;

//	char*		pBuf;
	char		Data[RS422_TX_DATA_BUF_SIZE];

#if 	RS422_BUFFER_4BYTE
	int 		Head4; 	// 4byte 단위 인덱싱
	int 		Tail4;

	int			iData[RS422_TX_DATA_BUF_SIZE4];
#endif	
} RS422_BUF;


void rs422_SetMyBoardId(uint8_t Id);
void InterruptEnable_rs422(void);
void InterruptDisable_rs422(void);

void rs422_init(void);

#if RS422_MASTER
int rs422_PacketParse_Master();
int rs422_REQUEST_Send(uint32_t slvid);
int rs422_SendCmd();	// Master --> Slave
#else
int rs422_PacketParse_Slave();
#endif


int rs422_SendData();

//==============================================================================
void rs422_CPUModeSetting(void);
//==============================================================================

int r_get_buffer_size();
int r_get_buffer_size4();

int r_get_available();
int r_enqueue(const char* pBuffer, int size);

int r_wite_TxMem(int size);
int r_peek(char* pBuffer, int size);


int rs422_PacketParse(unsigned int nPacketCode, char *pData);
void rs422_Test();

//void rs422_State_Send_buffer(void);
//void rs422_State_Send_buffer_blank(void);
#endif // #ifndef RS422_IF_H_
