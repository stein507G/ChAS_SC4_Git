
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

typedef struct _LCAS_PACKET
{
	unsigned int nPacketLength;			// 전체 길이

	unsigned int nDeviceID;				// #define PACKET_DEVICE_ID_YLAS				(0x97000)
	unsigned int nPacketCode;
	unsigned int nEventIndex;			// Event(Table 데이터)일 때의 Index	// 0 부터 보낸다. --> 어레이 1번에 저장하라
										// 제어기로 보낼때는 1번 어레이 부터 보내고 ,nEventIndex는 0부터 시작하라
	unsigned int nSlotOffset;
	unsigned int nSlotSize;				// slot's size
	unsigned char Slot[0];
} LCAS_PACKET;

typedef struct _LCAS_UPDATE_PACKET
{
	char Type[32];
	char Name[32];
	char Version[32];
	char Date[32];

	unsigned int nSize;
	unsigned int nCheckSum;
} LCAS_UPDATE_PACKET;

#define PACKET_HEADER_SIZE		(24)	//(28)

#define PACKET_SIZE_MIN			PACKET_HEADER_SIZE

#if EEPROM_ADJUST_DATA_SAVE_FIX
//#define PACKET_SIZE_MAX			1024	// 512
//#define PACKET_BUFF_SIZE		1024
#define TCP_RECV_PACKET_BUFF_SIZE		1024
#define TCP_SEND_PACKET_BUFF_SIZE		1024
#define TCP_PACKET_LENGTH_MAX			488 // 1024		// ChroZenAS
//#define PACKET_SIZE_MAX			768	// 512
//#define PACKET_BUFF_SIZE		768
#else
//#define PACKET_SIZE_MAX			1024
//#define PACKET_BUFF_SIZE		1024
#define TCP_RECV_PACKET_BUFF_SIZE		1024
#define TCP_SEND_PACKET_BUFF_SIZE		1024
#define TCP_PACKET_LENGTH_MAX			488 // 1024
#endif
//#define PACKET_BUFF_SIZE		4096

#define PA_RESPONSE			(0)
#define PA_REQUEST			(1)

#define PA_TABLE_ZERO		(0)
#define PA_TABLE			(1)

#define PACKET_DEVICE_ID_YLAS					(0x97000)

#define PACKCODE_LCAS_SYSTEM_INFORM				(PACKET_DEVICE_ID_YLAS + 0x400)
#define PACKCODE_LCAS_SYSTEM_CONFIG				(PACKET_DEVICE_ID_YLAS + 0x410)
#define PACKCODE_LCAS_RUN_SETUP						(PACKET_DEVICE_ID_YLAS + 0x420)
#define PACKCODE_LCAS_TIME_EVENT					(PACKET_DEVICE_ID_YLAS + 0x430)
#define PACKCODE_LCAS_MIX_SETUP						(PACKET_DEVICE_ID_YLAS + 0x440)

#define PACKCODE_LCAS_SEQUENCE						(PACKET_DEVICE_ID_YLAS + 0x460)
#define PACKCODE_LCAS_COMMAND							(PACKET_DEVICE_ID_YLAS + 0x470)
#define PACKCODE_LCAS_STATE								(PACKET_DEVICE_ID_YLAS + 0x480)
#define PACKCODE_LCAS_SLFEMSG							(PACKET_DEVICE_ID_YLAS + 0x490)
#define PACKCODE_LCAS_SERVICE							(PACKET_DEVICE_ID_YLAS + 0x500)

#define PACKCODE_LCAS_ADJUST_DATA					(PACKET_DEVICE_ID_YLAS + 0x510)
#define PACKCODE_LCAS_DIAG_DATA						(PACKET_DEVICE_ID_YLAS + 0x520)
#define PACKCODE_LCAS_TEMPCAL_DATA				(PACKET_DEVICE_ID_YLAS + 0x530)
#define PACKCODE_LCAS_USED_TIME						(PACKET_DEVICE_ID_YLAS + 0x540)

#define PACKCODE_LCAS_UPDATE							(PACKET_DEVICE_ID_YLAS + 0x700)

#define PACKCODE_LCAS_USER_DEFINE_CONTROL		(PACKET_DEVICE_ID_YLAS + 0x610)

#define PACKCODE_LCAS_SPECIAL 						(PACKET_DEVICE_ID_YLAS + 0x620)	// - 추가(20170914)




#define PACKCODE_LCAS_MOTOR_RUN					(PACKET_DEVICE_ID_YLAS + 0x110)
#define PACKCODE_LCAS_MOTOR_SET					(PACKET_DEVICE_ID_YLAS + 0x120)

#define PACKCODE_LCAS_BOARD_STATE				(PACKET_DEVICE_ID_YLAS + 0x121)


#define PACKCODE_LCAS_BOARD_CMD					(PACKET_DEVICE_ID_YLAS + 0x122)

#define PACKCODE_LCAS_MOTOR_RESET				(PACKET_DEVICE_ID_YLAS + 0x130)
#define PACKCODE_LCAS_MOTOR_STOP				(PACKET_DEVICE_ID_YLAS + 0x140)
#define PACKCODE_LCAS_MOTOR_BREAK				(PACKET_DEVICE_ID_YLAS + 0x150)
#define PACKCODE_LCAS_MOTOR_SLEEP				(PACKET_DEVICE_ID_YLAS + 0x160)

#define PACKCODE_LCAS_SETUP							(PACKET_DEVICE_ID_YLAS + 0x200)
#define PACKCODE_LCAS_CON								(PACKET_DEVICE_ID_YLAS + 0x210)
#define PACKCODE_LCAS_INJ_START					(PACKET_DEVICE_ID_YLAS + 0x220)
#define PACKCODE_LCAS_WASH_START				(PACKET_DEVICE_ID_YLAS + 0x230)
#define PACKCODE_LCAS_INITILIZE					(PACKET_DEVICE_ID_YLAS + 0x240)


// 20191228 - ehernet config전송을 위해서 추가
#define PACKCODE_LCAS_ETHERNET_CONFIG		(PACKET_DEVICE_ID_YLAS + 0x300)


#if RS422_ENABLE
#if KCJ_TCP_INTR
#define TCP_WRITE_PACKET	{ \
	if(rs422if.connected) { RS422_ENQUEUE_PACKET; } \
	else { \
		signed char retERR; \
		NVIC_DisableIRQ(EthernetMAC_IRQn); /*MSS_MAC_disable_intr();*/ /* (+) 210621 CJKIM, disable mac interrupt before sending. */ \
		retERR = tcp_write(netAS_PCB, pPacketBuff, ((LCAS_PACKET *)pPacketBuff)->nPacketLength, 1); \
		if(retERR == ERR_OK) { \
			tcp_output(netAS_PCB); \
			NVIC_EnableIRQ(EthernetMAC_IRQn); /*MSS_MAC_enable_intr();*/ /* (+) 210621 CJKIM, enable mac interrupt after sending.	 */ \
		} \
		else { \
			NVIC_EnableIRQ(EthernetMAC_IRQn); /*MSS_MAC_enable_intr();*/ /* (+) 210621 CJKIM, enable mac interrupt after sending.	 */ \
		} \
	} \
}
#else
#define TCP_WRITE_PACKET	{ \
	if(rs422if.connected) { RS422_ENQUEUE_PACKET; } \
	else { \
		tcp_write(netAS_PCB, pPacketBuff, ((LCAS_PACKET *)pPacketBuff)->nPacketLength, 1); \
		tcp_output(netAS_PCB); \
	} \
}
#endif
#else
#if KCJ_TCP_INTR
#define TCP_WRITE_PACKET	{ \
	signed char retERR; \
	NVIC_DisableIRQ(EthernetMAC_IRQn); /*MSS_MAC_disable_intr();*/ /* (+) 210621 CJKIM, disable mac interrupt before sending. */ \
	retERR = tcp_write(netAS_PCB, pPacketBuff, ((LCAS_PACKET *)pPacketBuff)->nPacketLength, 1); \
	if(retERR == ERR_OK) { \
		tcp_output(netAS_PCB); \
		NVIC_EnableIRQ(EthernetMAC_IRQn); /*MSS_MAC_enable_intr();*/ /* (+) 210621 CJKIM, enable mac interrupt after sending.	 */ \
	} \
	else { \
		NVIC_EnableIRQ(EthernetMAC_IRQn); /*MSS_MAC_enable_intr();*/ /* (+) 210621 CJKIM, enable mac interrupt after sending.	 */ \
	} \
}
#else
#define TCP_WRITE_PACKET	{ \
	tcp_write(netAS_PCB, pPacketBuff, ((LCAS_PACKET *)pPacketBuff)->nPacketLength, 1); \
	tcp_output(netAS_PCB); \
}
#endif
#endif



// Packet 조립
//
//		buffer:			조립한 packet이 저장될 pointer
//		full_data:		packet code에 해당하는 data 변수
//		entry_data:		full_data 변수 자체 혹은 그 멤버 변수
//		request:		명령 packet이면 0, 응답 요구 packet이면 1
//
// 패킷 만들기
#define MAKE_LCAS_PACKET(Code,full_data,entry_data) { \
	char *buffer = pPacketBuff; \
	((LCAS_PACKET *)buffer)->nPacketLength = sizeof(LCAS_PACKET) + sizeof(entry_data); \
	((LCAS_PACKET *)buffer)->nDeviceID = (PACKET_DEVICE_ID_YLAS); \
	((LCAS_PACKET *)buffer)->nPacketCode = (Code); \
	((LCAS_PACKET *)buffer)->nEventIndex = 0; \
	((LCAS_PACKET *)buffer)->nSlotOffset = ((int)&entry_data) - ((int)&full_data);	\
	((LCAS_PACKET *)buffer)->nSlotSize = sizeof(entry_data); \
	memcpy(((LCAS_PACKET*)buffer)->Slot, &entry_data, sizeof(entry_data)); \
}

#define SEND_LCAS_PACKET(Code,full_data,entry_data)	{ \
	MAKE_LCAS_PACKET(Code,full_data,entry_data); \
	TCP_WRITE_PACKET; \
}

#define MAKE_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data)	{ \
	char *buffer = pPacketBuff; \
	((LCAS_PACKET *)buffer)->nPacketLength = sizeof(LCAS_PACKET) + sizeof(entry_data); \
	((LCAS_PACKET *)buffer)->nDeviceID = (PACKET_DEVICE_ID_YLAS); \
	((LCAS_PACKET *)buffer)->nPacketCode = (Code); \
	((LCAS_PACKET *)buffer)->nEventIndex = TableNo;	\
	((LCAS_PACKET *)buffer)->nSlotOffset = ((int)&entry_data) - ((int)&full_data);	\
	((LCAS_PACKET *)buffer)->nSlotSize = sizeof(entry_data); \
	memcpy(((LCAS_PACKET*)buffer)->Slot, &entry_data, sizeof(entry_data)); \
}

#define SEND_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data)	{ \
	MAKE_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data); \
	TCP_WRITE_PACKET; \
}

// 요청
#define REQUEST_LCAS_PACKET(Code,full_data,entry_data)	{ \
	char *buffer = pPacketBuff; \
	((LCAS_PACKET *)buffer)->nPacketLength = sizeof(LCAS_PACKET); \
	((LCAS_PACKET *)buffer)->nDeviceID = (PACKET_DEVICE_ID_YLAS); \
	((LCAS_PACKET *)buffer)->nPacketCode = (Code); \
	((LCAS_PACKET *)buffer)->nEventIndex = 0;	\
	((LCAS_PACKET *)buffer)->nSlotOffset = ((int)&entry_data) - ((int)&full_data);	\
	((LCAS_PACKET *)buffer)->nSlotSize = sizeof(entry_data); \
}

#define SEND_REQUEST_LCAS_PACKET(Code,full_data,entry_data)	{ \
	REQUEST_LCAS_PACKET(Code,full_data,entry_data); \
	TCP_WRITE_PACKET; \
}

#define REQUEST_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data)	{ \
	char *buffer = pPacketBuff; \
	((LCAS_PACKET *)buffer)->nPacketLength = sizeof(LCAS_PACKET); \
    ((LCAS_PACKET *)buffer)->nDeviceID = (PACKET_DEVICE_ID_YLAS); \
	((LCAS_PACKET *)buffer)->nPacketCode = (Code); \
	((LCAS_PACKET *)buffer)->nEventIndex = TableNo;	\
	((LCAS_PACKET *)buffer)->nSlotOffset = ((int)&entry_data) - ((int)&full_data);	\
	((LCAS_PACKET *)buffer)->nSlotSize = sizeof(entry_data); \
}

#define SEND_REQUEST_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data)	{ \
	REQUEST_LCAS_TABLE_PACKET(Code,TableNo,full_data,entry_data); \
	TCP_WRITE_PACKET; \
}

// 데이터를 요청하는 Packet인지 확인
#define IS_REQUEST_PACKET(buffer) \
	( ((LCAS_PACKET*)buffer)->nPacketLength < sizeof(LCAS_PACKET) + ((LCAS_PACKET*)buffer)->nSlotSize )

// 받은 Packet을 처리
//		buffer:			받은 packet, 응답 요구 packet이면 응답 packet으로 재조립됨
//		full_data:		packet_code에 해당하는 data 변수
//		response:		응답 packet으로 재조립되면 1로 셋팅됨
#define USE_LCAS_PACKET(buffer,full_data)	\
		memcpy(((unsigned char *)&full_data) + ((LCAS_PACKET*)buffer)->nSlotOffset, \
				((LCAS_PACKET*)buffer)->Slot, \
				((LCAS_PACKET*)buffer)->nSlotSize);

// [ACK](Acknowledge) Packet인지 확인
#define IS_ACK_PACKET(buffer) \
	( ((LCAS_PACKET*)buffer)->nPacketLength == sizeof(LCAS_PACKET) && ((LCAS_PACKET*)buffer)->nSlotSize == 0)

#define RESPONSE_LCAS_PACKET(full_data,ReqHeader)	{ \
	char *buffer = pPacketBuff; \
	((LCAS_PACKET *)buffer)->nPacketLength = sizeof(LCAS_PACKET) + ((LCAS_PACKET*)ReqHeader)->nSlotSize; \
	((LCAS_PACKET *)buffer)->nDeviceID = (PACKET_DEVICE_ID_YLAS); \
	((LCAS_PACKET *)buffer)->nPacketCode = ((LCAS_PACKET*)ReqHeader)->nPacketCode; \
	((LCAS_PACKET *)buffer)->nEventIndex = 0;	\
	((LCAS_PACKET *)buffer)->nSlotOffset = ((LCAS_PACKET*)ReqHeader)->nSlotOffset; \
	((LCAS_PACKET *)buffer)->nSlotSize = ((LCAS_PACKET*)ReqHeader)->nSlotSize; \
	memcpy( &((LCAS_PACKET*)buffer)->Slot, \
		((unsigned char *)&full_data) + ((LCAS_PACKET*)buffer)->nSlotOffset, \
		((LCAS_PACKET*)ReqHeader)->nSlotSize ); \
}

#define SEND_RESPONSE_LCAS_PACKET(full_data,ReqHeader)	{ \
	RESPONSE_LCAS_PACKET(full_data,ReqHeader); \
	TCP_WRITE_PACKET; \
}

#define RESPONSE_LCAS_TABLE_PACKET(TableNo,full_data,ReqHeader)	{ \
	char *buffer = pPacketBuff; \
	((LCAS_PACKET *)buffer)->nPacketLength = sizeof(LCAS_PACKET) + ((LCAS_PACKET*)ReqHeader)->nSlotSize; \
    ((LCAS_PACKET *)buffer)->nDeviceID = (PACKET_DEVICE_ID_YLAS); \
	((LCAS_PACKET *)buffer)->nPacketCode = ((LCAS_PACKET*)ReqHeader)->nPacketCode; \
	((LCAS_PACKET *)buffer)->nEventIndex = TableNo;	\
	((LCAS_PACKET *)buffer)->nSlotOffset = ((LCAS_PACKET*)ReqHeader)->nSlotOffset; \
	((LCAS_PACKET *)buffer)->nSlotSize = ((LCAS_PACKET*)ReqHeader)->nSlotSize; \
	memcpy( &((LCAS_PACKET*)buffer)->Slot, \
		((unsigned char *)&full_data) + ((LCAS_PACKET*)buffer)->nSlotOffset, \
		((LCAS_PACKET*)ReqHeader)->nSlotSize ); \
}

#define SEND_RESPONSE_LCAS_TABLE_PACKET(TableNo,full_data,ReqHeader)	{ \
	RESPONSE_LCAS_TABLE_PACKET(TableNo,full_data,ReqHeader); \
	TCP_WRITE_PACKET; \
}
		
#if 0
#define ACK_LCAS_TABLE_PACKET(ReqHeader)	{ \
	char *buffer = pPacketBuff; \
	((LCAS_PACKET *)buffer)->nPacketLength = sizeof(LCAS_PACKET); \
    ((LCAS_PACKET *)buffer)->nDeviceID = (PACKET_DEVICE_ID_YLAS); \
	((LCAS_PACKET *)buffer)->nPacketCode = ((LCAS_PACKET*)ReqHeader)->nPacketCode; \
	((LCAS_PACKET *)buffer)->nEventIndex = ((LCAS_PACKET*)ReqHeader)->nEventIndex; \
	((LCAS_PACKET *)buffer)->nSlotOffset = ((LCAS_PACKET*)ReqHeader)->nSlotOffset; \
	((LCAS_PACKET *)buffer)->nSlotSize = 0; \
	send(ghClientSocket, buffer, ((LCAS_PACKET *)buffer)->nPacketLength, PA_RESPONSE);\
}
#endif

#if 0
#define ACK_LCAS_TABLE_PACKET(ReqHeader)	{ \
	char *buffer = pPacketBuff; \
	((LCAS_PACKET *)buffer)->nPacketLength = sizeof(LCAS_PACKET); \
    ((LCAS_PACKET *)buffer)->nDeviceID = (PACKET_DEVICE_ID_YLAS); \
	((LCAS_PACKET *)buffer)->nPacketCode = ((LCAS_PACKET*)ReqHeader)->nPacketCode; \
	((LCAS_PACKET *)buffer)->nEventIndex = ((LCAS_PACKET*)ReqHeader)->nEventIndex; \
	((LCAS_PACKET *)buffer)->nSlotOffset = ((LCAS_PACKET*)ReqHeader)->nSlotOffset; \
	((LCAS_PACKET *)buffer)->nSlotSize = 0; \
}
#endif

#define ACK_LCAS_PACKET(ReqHeader)	{ \
	char *buffer = pPacketBuff; \
	((LCAS_PACKET *)buffer)->nPacketLength = sizeof(LCAS_PACKET); \
	((LCAS_PACKET *)buffer)->nDeviceID = (PACKET_DEVICE_ID_YLAS); \
	((LCAS_PACKET *)buffer)->nPacketCode = ((LCAS_PACKET*)ReqHeader)->nPacketCode; \
	((LCAS_PACKET *)buffer)->nEventIndex = ((LCAS_PACKET*)ReqHeader)->nEventIndex; \
	((LCAS_PACKET *)buffer)->nSlotOffset = ((LCAS_PACKET*)ReqHeader)->nSlotOffset; \
	((LCAS_PACKET *)buffer)->nSlotSize = 0; \
}

#define SEND_ACK_LCAS_PACKET(ReqHeader)	{ \
	ACK_LCAS_PACKET(ReqHeader); \
	TCP_WRITE_PACKET; \
}

#define MEMCOPY_LCAS_STRUCT(STRUCT)	memcpy(& ## STRUCT, & ## tmp ## STRUCT, sizeof(tmp ## STRUCT))

#pragma pack(push,1)

#pragma pack(pop)

#endif
