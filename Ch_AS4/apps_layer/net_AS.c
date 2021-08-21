/**
  ******************************************************************************
  * @file    net_AS
  * @author  Younglin Instrument Co., Ltd
  * @version V1.0.0
  * @date    01/20/2016
  * @brief   AutoSampler Control
  *          The application works as a server which wait for the client request
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/

#include "config.h"

#include "MotorCtrl_task.h"
#include "net_AS.h"
#include "LcasAction_task.h"
#include "PID_Control_AS.h"

#include "lwip/tcp.h"
#include "conf_eth.h"
//#include <string.h>

//#include "./drivers/mss_nvm/drivers/F2DSS_NVM/mss_nvm.h"
//#include "mss_nvm.h"

#include "EEPROM.h"
#include "Led_task.h"

#include "rs422_if.h"

#include "testPrgm.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define GREETING "Escape Character is Ctrl+]\r\nHello. What is your name?\r\n"
#define HELLO "Hello "
#define MAX_NAME_SIZE 32

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

struct name 
{
  int length;
  char bytes[MAX_NAME_SIZE];
};

//===============================================  
// 20160429  - tcp_write에 대한 것
// net_AS_accept()에서 생성
// static struct tcp_pcb *TcpPCB;  
//static 
struct tcp_pcb *netAS_PCB;
//===============================================

/* Private function prototypes -----------------------------------------------*/
static err_t net_AS_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t net_AS_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static void net_AS_conn_err(void *arg, err_t err);
//void PacketParse(unsigned int nPacketCode, char *pData_in);//, void *lpArg)
int PacketParse(unsigned int nPacketCode, char *pData);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Called when a data is received on the telnet connection
  * @param  arg	the user argument
  * @param  pcb	the tcp_pcb that has received the data
  * @param  p	the packet buffer
  * @param  err	the error value linked with the received data
  * @retval error value
  */
//int gbIsPcConnect = YES;
//TEST_LCAS_t Lcas,tmpLcas;
extern LCAS_CONTROL_t LcasCon;//,tmpLcasCon;
extern LCAS_DEV_STATE_t devState;
extern LCAS_DEV_SET_t devSet;

extern uint32_t watchdogSystemReset;

#if TEMP_CAL_DATA_FORMAT_DOUBLE
extern double GfTempConstA;
extern double GfTempConstB;
extern double GfTempConstC;
#else
extern float GfTempConstA;
extern float GfTempConstB;
extern float GfTempConstC;
#endif

extern void MSS_MAC_enable_intr();				//(+) 210621 CJKIM, For atomic tx .
extern void MSS_MAC_disable_intr();				//(+) 210621 CJKIM, For atomic tx .

//===============================================================================
// 통신프로토콜 
// Protocol관련
//===============================================================================
SYSTEM_INFORM_t sysInform;
SYSTEM_CONFIG_t sysConfig;
LCAS_RUN_SETUP_t runSetup;
LCAS_TIME_EVENT_t timeEvent;
LCAS_MIX_t mixSet;
//LCAS_MIX_TABLE_t mixTable[MIX_TABLE_COUNT];
LCAS_SEQUENCE_t sequence;
LCAS_COMMAND_t command;
LCAS_SELFMSG_t selfMsg;

LCAS_SERVICE_t service;
LCAS_STATE_t LcasState;

LCAS_DIAG_DATA_t diagData;
LCAS_USED_TIME_t usedTime;
LCAS_TEMP_CAL_t tempCalData;
LCAS_SPECIAL_t special;

// 20191228
ETHERNET_CONFIG_t ethernetConfig;

char pPacketBuff[TCP_SEND_PACKET_BUFF_SIZE];

//#if BOARD_TEST_PRGM
//#if BOARD_TEST_PRGM || MACHINE_TEST_PRGM

extern LCAS_BOARD_STATE_t	LcasBoardState;
extern LCAS_BOARD_STATE_t	LcasBoardCmd;

//extern 
	MOTOR_CTRL_t MotorCtrlSet;//,tmpMotorCtrlSet;
//#endif

#if RS422_ENABLE
extern RS422_ST rs422if;
extern RS422_BUF tx;
#endif


int netConnectAction()
{
//#if BOARD_TEST_PRGM
#if BOARD_TEST_PRGM || MACHINE_TEST_PRGM
	if(devState.connected) pSend_State_Fn = send_BoardState;
	else pSend_State_Fn = send_State_Blank;
#else
	if(devState.connected) pSend_State_Fn = send_State;
	else pSend_State_Fn = send_State_Blank;
#endif	
}

int	netConnection()
{
// 여기서 접속된 기기에 정보을 요청한다.
SEND_REQUEST_LCAS_PACKET(PACKCODE_LCAS_SYSTEM_INFORM, sysInform, sysInform);
}	

// 맨처음 연결시만 동작한다.
int net_CheckModel(char *pData)
{
	SYSTEM_INFORM_t tmp;
	char cModel[32];
	
	strcpy(cModel, CONNECT_CHECK_MODEL);

	USE_LCAS_PACKET(pData,tmp);

// 처음으로 연결 - 정당한 연결인지 확인한다,
	if(strcmp(tmp.cModel, cModel)) {			// 같지 않으면 
dfp("check--ERROR--PACKCODE_LCAS_SYSTEM_INFORM (%s)\n",tmp.cModel);
		return RE_ERR;				
	}

		SEND_ACK_LCAS_PACKET(pData);
dfp("check--EQUAL--PACKCODE_LCAS_SYSTEM_INFORM (%s)\n",tmp.cModel);

		devState.connected = YES;
		devState.disconnected = NO;
		
		netConnectAction();	

#if RS422_DISABLE_INTR
		InterruptDisable_rs422();
#endif


		if(devSet.adjustState	== NO) {
			errorFunction(SELF_ERROR_NO_ADJUSTMENT_DATA);
		}

		return RE_OK;
}

int net_SystemInform(char *pData)
{
	SYSTEM_INFORM_t tmp;
	int retSerialNo = RE_ERR;
	int retIpAddr = RE_ERR;
	
	if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
		send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
		return RE_ERR;
	}

	USE_LCAS_PACKET(pData,tmp);

	SEND_ACK_LCAS_PACKET(pData);
// 임의로 변경하지 못하도록	
	strcpy(sysInform.cModel, SYSTEM_MODEL);
	strcpy(sysInform.cVersion, SYSTEM_VERSION);

// 시리얼은 특정 문자열 입력후 적용되록 // 맨앞에 F 맨뒤에 F
// FAS-3456-170927F   
//strcpy(tmp.cSerialNo, "FAS-3456-170927F");
	if(tmp.cSerialNo[0] == 'F' && tmp.cSerialNo[13] == 'F') {
//		strcpy(sysInform.cSerialNo, &tmp.cSerialNo[1]);
		strncpy(sysInform.cSerialNo, &tmp.cSerialNo[1],12);	
		sysInform.cSerialNo[12] = '\0';
dfp("sysInform.cSerialNo = %s \n",sysInform.cSerialNo);
dfp("tmp.cSerialNo = %s \n",tmp.cSerialNo);
		sysInform.cInstallDate.year = tmp.cInstallDate.year;
		sysInform.cInstallDate.month = tmp.cInstallDate.month;
		sysInform.cInstallDate.date = tmp.cInstallDate.date;
dfp("sysInform.cInstallDate.year = %d month = %d date = %d\n",sysInform.cInstallDate.year,sysInform.cInstallDate.month,sysInform.cInstallDate.date);

		make_MACADDR(MAKE_MAC);

		retSerialNo = RE_OK;		
	}

	if(strcmp(tmp.cSerialNo, "SYSTIME")) {			// 같지 않으면 
	}
	else {
		sysInform.cSysDate.year = tmp.cSysDate.year;
		sysInform.cSysDate.month = tmp.cSysDate.month;
		sysInform.cSysDate.date = tmp.cSysDate.date;

		sysInform.cSysTime.hour = tmp.cSysTime.hour;
		sysInform.cSysTime.minute = tmp.cSysTime.minute;
		sysInform.cSysTime.second = tmp.cSysTime.second;

#if RTC_WRITE_SF2
		uint8_t rtc_time[10];
		uint8_t sec,sec1,sec2;

		rtc_time[0] = RTC_SECONDS_REG_ADDR;		// add

		sec = sysInform.cSysTime.second;	// sec
		sec2 = sec / 10 * 16 + sec % 10;;
		iprintf("sec2 = [%x]\n",sec2);
		rtc_time[1] = sec2;

		rtc_time[2] = sysInform.cSysTime.minute;
		sec = sysInform.cSysTime.minute;	// sec
		sec2 = sec / 10 * 16 + sec % 10;;
		iprintf("minute = [%x]\n",sec2);
		rtc_time[2] = sec2;

		rtc_time[3] = sysInform.cSysTime.hour;	// hour
		sec = sysInform.cSysTime.hour;	// sec
		sec2 = sec / 10 * 16 + sec % 10;;
		iprintf("hour = [%x]\n",sec2);
		rtc_time[3] = sec2;

				rtc_time[4] = 0x04;	// day
				
		rtc_time[5] = sysInform.cSysDate.date;	// date
		sec = sysInform.cSysDate.date;	// sec
		sec2 = sec / 10 * 16 + sec % 10;;
		iprintf("date = [%x]\n",sec2);
		rtc_time[5] = sec2;

		rtc_time[6] = sysInform.cSysDate.month;	// month
		sec = sysInform.cSysDate.month;	// sec
		sec2 = sec / 10 * 16 + sec % 10;;
		iprintf("month = [%x]\n",sec2);
		rtc_time[6] = sec2;

		rtc_time[7] = sysInform.cSysDate.year;	// year
		sec = sysInform.cSysDate.year;	// sec
		sec2 = sec / 10 * 16 + sec % 10;;
		iprintf("year = [%x]\n",sec2);
		rtc_time[7] = sec2;
		
		rtcWrite_time(rtc_time);

/*
int i;
for(i=0;i<10;i++) {
iprintf("rtc = [%x]\n",rtc_time[i]);
}
*/

#else
	set_RtcTime((sysInform.cSysDate.year + FIRST_YEAR - 1),sysInform.cSysDate.month,sysInform.cSysDate.date,
		sysInform.cSysTime.hour,sysInform.cSysTime.minute,sysInform.cSysTime.second);
//	memcpy(&sysInform, &tmp, sizeof(tmp));
#endif
	}	

// cSerialNo == "AAA"일경우 cIPAddress , cPortNo 변경가능 
	if(strcmp(tmp.cSerialNo, "IPADDR")) {			// 같지 않으면 
		// 변경안됨.
	}
	else {
		strcpy(sysInform.cIPAddress, tmp.cIPAddress);
		strcpy(sysInform.cPortNo, tmp.cPortNo);
retIpAddr = RE_OK;
	}

	if(retSerialNo == RE_OK || retIpAddr == RE_OK) {
		saveSystemInform(0);
	}
	return RE_OK;
}

// 미완성
extern unsigned char my_ip[];		
extern unsigned char my_netMask[];	
extern unsigned char my_gateway[];
extern unsigned char my_mac[];

int net_EthernetConfig(char *pData)
{
	ETHERNET_CONFIG_t tmp,tmp1;

	unsigned char saveFlag;	

	int retIpAddr = RE_OK;

	char* tok;
	int ip[4];
	
	
	if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
		send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
		return RE_ERR;
	}

	USE_LCAS_PACKET(pData,tmp);

	SEND_ACK_LCAS_PACKET(pData);
// 임의로 변경하지 못하도록	
//ethernetConfig
//	strcpy(ethernetConfig.cFirmwareVersion, FIRMWARE_VERSION);
//	strcpy(ethernetConfig.cMachineVersion, MACHINE_VER);

	if(strcmp(tmp.cModifyChar, "IPADDR")) {			// 같지 않으면 
diprintf("cModifyChar = %s\n",tmp.cModifyChar);				
	}
	else {

diprintf("cIPAddress = %s ",tmp.cIPAddress);			
		//strcpy(&tmp1.cIPAddress, tmp.cIPAddress);
		strcpy(tmp1.cIPAddress, tmp.cIPAddress);
		tok = strtok(tmp.cIPAddress," .-");
		if(tok != NULL) {
			my_ip[0] = atoi(tok);
			if(my_ip[0] > 255 || my_ip[0] < 0) retIpAddr = RE_ERR;
		}
		else retIpAddr = RE_ERR;
		tok = strtok(NULL," .-");	
		if(tok != NULL) {
			my_ip[1] = atoi(tok);		
			if(my_ip[1] > 255 || my_ip[1] < 0) retIpAddr = RE_ERR;		
		}
		else retIpAddr = RE_ERR;
		tok = strtok(NULL," .-");	
		if(tok != NULL) {
			my_ip[2] = atoi(tok);		
			if(my_ip[2] > 255 || my_ip[2] < 0) retIpAddr = RE_ERR;				
		}
		else retIpAddr = RE_ERR;
		tok = strtok(NULL," .-");	
		if(tok != NULL) {
			my_ip[3] = atoi(tok);		
			if(my_ip[3] > 255 || my_ip[3] < 0) retIpAddr = RE_ERR;				
		}		
		else retIpAddr = RE_ERR;
diprintf("ip[0]=[%d:%d:%d:%d]\n",my_ip[0],my_ip[1],my_ip[2],my_ip[3]);

diprintf("cGateway = %s ",tmp.cGateway);		
//		strcpy(&tmp1.cGateway, tmp.cGateway);
		strcpy(tmp1.cGateway, tmp.cGateway);tok = strtok(tmp.cGateway," .-");
		if(tok != NULL) {
			my_gateway[0] = atoi(tok);
			if(my_gateway[0] > 255 || my_gateway[0] < 0) retIpAddr = RE_ERR;
		}
		else retIpAddr = RE_ERR;
		tok = strtok(NULL," .-");	
		if(tok != NULL) {
			my_gateway[1] = atoi(tok);		
			if(my_gateway[1] > 255 || my_gateway[1] < 0) retIpAddr = RE_ERR;		
		}
		else retIpAddr = RE_ERR;
		tok = strtok(NULL," .-");	
		if(tok != NULL) {
			my_gateway[2] = atoi(tok);		
			if(my_gateway[2] > 255 || my_gateway[2] < 0) retIpAddr = RE_ERR;				
		}
		else retIpAddr = RE_ERR;
		tok = strtok(NULL," .-");	
		if(tok != NULL) {
			my_gateway[3] = atoi(tok);		
			if(my_gateway[3] > 255 || my_gateway[3] < 0) retIpAddr = RE_ERR;				
		}		
		else retIpAddr = RE_ERR;
diprintf("ip[0]=[%d:%d:%d:%d]\n",my_gateway[0],my_gateway[1],my_gateway[2],my_gateway[3]);

diprintf("cNetmask = %s ",tmp.cNetmask);		
//		strcpy(&tmp1.cNetmask, tmp.cNetmask);
		strcpy(tmp1.cNetmask, tmp.cNetmask);
		tok = strtok(tmp.cNetmask," .-");
		if(tok != NULL) {
			my_netMask[0] = atoi(tok);
			if(my_netMask[0] > 255 || my_netMask[0] < 0) retIpAddr = RE_ERR;
		}
		else retIpAddr = RE_ERR;
		tok = strtok(NULL," .-");	
		if(tok != NULL) {
			my_netMask[1] = atoi(tok);		
			if(my_netMask[1] > 255 || my_netMask[1] < 0) retIpAddr = RE_ERR;		
		}
		else retIpAddr = RE_ERR;
		tok = strtok(NULL," .-");	
		if(tok != NULL) {
			my_netMask[2] = atoi(tok);		
			if(my_netMask[2] > 255 || my_netMask[2] < 0) retIpAddr = RE_ERR;				
		}
		else retIpAddr = RE_ERR;
		tok = strtok(NULL," .-");	
		if(tok != NULL) {
			my_netMask[3] = atoi(tok);		
			if(my_netMask[3] > 255 || my_netMask[3] < 0) retIpAddr = RE_ERR;				
		}		
		else retIpAddr = RE_ERR;
diprintf("ip[0]=[%d:%d:%d:%d]\n",my_netMask[0],my_netMask[1],my_netMask[2],my_netMask[3]);

//		strcpy(&tmp1.cPortNo, tmp.cPortNo);
//		strcpy(&tmp1.cMacAddress, tmp.cMacAddress);
		strcpy(tmp1.cPortNo, tmp.cPortNo);
		strcpy(tmp1.cMacAddress, tmp.cMacAddress);
		if(retIpAddr == RE_OK) {
				strcpy(ethernetConfig.cIPAddress, tmp1.cIPAddress);
				strcpy(ethernetConfig.cGateway, tmp1.cGateway);
				strcpy(ethernetConfig.cNetmask, tmp1.cNetmask);		
				
				strcpy(ethernetConfig.cPortNo, tmp1.cPortNo);
				strcpy(ethernetConfig.cMacAddress, tmp1.cMacAddress);		

//#if MAKE_MAC_ADDR_SERIAL
//	make_MACADDR(APPLY_MAC);
//#endif		
/*
			sprintf(ethernetConfig.cIPAddress,"%d.%d.%d.%d",my_ip[0], my_ip[1], my_ip[2], my_ip[3]);
			dprintf("%s %3d.%3d.%3d.%3d\n",ethernetConfig.cIPAddress, my_ip[0], my_ip[1], my_ip[2], my_ip[3]);

			sprintf(ethernetConfig.cGateway,"%d.%d.%d.%d",my_gateway[0], my_gateway[1], my_gateway[2], my_gateway[3]);
			dprintf("%s %3d.%3d.%3d.%3d\n",ethernetConfig.cGateway, my_gateway[0], my_gateway[1], my_gateway[2], my_gateway[3]);

			sprintf(ethernetConfig.cNetmask,"%d.%d.%d.%d",my_netMask[0], my_netMask[1], my_netMask[2], my_netMask[3]);
			dprintf("%s %3d.%3d.%3d.%3d\n",ethernetConfig.cNetmask, my_netMask[0], my_netMask[1], my_netMask[2], my_netMask[3]);
*/				
			saveEthernetConfig(0);
			
diprintf("net_EthernetConfig retIpAddr == RE_OK\n");
diprintf("cancelEthernetConfigValue %\n cIPAddress = %s %s %s %s", ethernetConfig.cIPAddress, ethernetConfig.cPortNo, ethernetConfig.cGateway, ethernetConfig.cNetmask);


		}
	}

	return RE_OK;

}			

int net_SystemConfig(char *pData)
{
	SYSTEM_CONFIG_t tmp;
	unsigned char saveFlag;
	
	if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
		send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
		return RE_ERR;
	}

	USE_LCAS_PACKET(pData,tmp);

	if(	tmp.trayCooling > TRAY_COOLING_INSTALL 
			|| tmp.sampleLoopVol > SAMPLE_LOOP_VOLUME_MAX 
			|| tmp.syringeVol > SYRINGE_VOL_2500UL  
			|| tmp.needleTubeVol > NEEDLE_TUBING_VOL_MAX 
//			|| tmp.trayLeft > WELLPALTE_96
//			|| tmp.trayRight > WELLPALTE_96 
			|| tmp.trayLeft > TRAY_NONE
			|| tmp.trayRight > TRAY_NONE 
			|| tmp.processType > PROCESS_TYPE_COLUMN 
//			|| tmp.usePrepMode > USE_PREP_MODE_YES
			|| tmp.syringeUpSpdWash < SYRINGE_SPEED_MIN || tmp.syringeUpSpdWash > SYRINGE_SPEED_MAX
			|| tmp.syringeDnSpdWash < SYRINGE_SPEED_MIN || tmp.syringeDnSpdWash > SYRINGE_SPEED_MAX	

			|| tmp.injectMarkerTime > INJECTOR_MARKER_TIME_2sec
			|| tmp.startInSigTime > START_IN_SIGNAL_TIME_2sec

			|| tmp.useMicroPump > YES	
		) {
		errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
		SEND_LCAS_PACKET(PACKCODE_LCAS_SYSTEM_CONFIG, sysConfig, sysConfig);
		
dfp("SELF_ERROR_COMMAND_UNKNOWN\n");
		return RE_ERR;
	}

// ==============================================================================================	
// ==============================================================================================	
	memcpy(&sysConfig, &tmp, sizeof(tmp));

	SEND_ACK_LCAS_PACKET(pData);

//	sysConfig.syringeUpSpdWash = tmp.syringeUpSpdWash - 1;
//	sysConfig.syringeDnSpdWash = tmp.syringeDnSpdWash - 1;
	sysConfig.syringeUpSpdWash = tmp.syringeUpSpdWash;
	sysConfig.syringeDnSpdWash = tmp.syringeDnSpdWash;

	applySystemConfig();
// 20210602	
//	checkAdjustData();
	initCoordinate();

//	SEND_ACK_LCAS_PACKET(pData);

#if 1
	devState.save_EEPROM_sysConfig = YES;
#else
	unsigned char data[50];
	data[0] = DATA_SAVE_FLAG;
	memcpy(&data[1], &(sysConfig) , sizeof(sysConfig));
	eepromWrite_Data((SYSTEM_CONFIG_DATA_SAVE), sizeof(sysConfig) + 1, &(data[0]));
#endif

 // 동작전 설정이 바뀌었으므로 새로운 값으로 설정할 항목들이 많다.
//		devSet.injectionVolumeMax = 

//initSystemConfig();

	return RE_OK;
}

int save_EEPROM_sysConfig_data()
{
	unsigned char data[50];
	
	data[0] = DATA_SAVE_FLAG;
	memcpy(&data[1], &(sysConfig) , sizeof(sysConfig));
	eepromWrite_Data((SYSTEM_CONFIG_DATA_SAVE), sizeof(sysConfig) + 1, &(data[0]));

	devState.save_EEPROM_sysConfig = NO;

	return OK;
}

void initTempCalData()
{
	unsigned char saveFlag;

	GfTempConstA = 0;
	GfTempConstB = MAIN_TEMP_CONST_B;
	GfTempConstC =	MAIN_TEMP_CONST_C;

	tempCalData.setTemp1 = TEMP_CAL_SET_1ST;
	tempCalData.setTemp2 = TEMP_CAL_SET_2ND;
	tempCalData.meaTemp1 = TEMP_CAL_SET_1ST;
	tempCalData.meaTemp2 = TEMP_CAL_SET_2ND;

	tempConstCalc(TEMP_CAL_LOAD);	// load
}

void tempConstCalc(unsigned char act)
{
	float consta,constb;
	float fSet[2],fMea[2],fOffset[2];
	unsigned char saveFlag;
	float offset1,offset2;
	
	switch(act) {
//		case 0:	// load
		case TEMP_CAL_LOAD:
			eepromRead_Data(TEMP_CAL_DATA_SAVE,1,&saveFlag);

			tempCalData.setTemp1 = TEMP_CAL_SET_1ST;
			tempCalData.setTemp2 = TEMP_CAL_SET_2ND;
			tempCalData.meaTemp1 = TEMP_CAL_SET_1ST;
			tempCalData.meaTemp2 = TEMP_CAL_SET_2ND;

			if(saveFlag	== DATA_SAVE_FLAG) {
//				eepromRead_Data(TEMP_CAL_DATA_ADDR,sizeof(tempCalData),&tempCalData);
				eepromRead_Data(TEMP_CAL_DATA_ADDR,20,&tempCalData.meaTemp1);	
				offset1 = tempCalData.setTemp1 - tempCalData.meaTemp1;
				offset2 = tempCalData.setTemp2 - tempCalData.meaTemp2;
dfp("Cal Temp Load !!\n");					
				if(offset1 < TEMP_CAL_ERROR_MIN || offset1 > TEMP_CAL_ERROR_MAX || offset2 < TEMP_CAL_ERROR_MIN || offset2 > TEMP_CAL_ERROR_MAX) {
					tempCalData.meaTemp1 = TEMP_CAL_SET_1ST;
					tempCalData.meaTemp2 = TEMP_CAL_SET_2ND;
dfp("Cal Temp Load Error!!!\n");					
				}
			}
		
			fSet[0] = tempCalData.setTemp1;
			fSet[1] = tempCalData.setTemp2;
			fMea[0] = tempCalData.meaTemp1;
			fMea[1] = tempCalData.meaTemp2;

			fOffset[0] = fMea[0] - fSet[0];
			fOffset[1] = fMea[1] - fSet[1];

//			consta = ((fMea[1] - fSet[1]) - (fMea[0] - fSet[0])) / (fMea[1] - fMea[0]);
//			constb = (fMea[1] - fSet[1]) - consta * (fMea[1]);
			consta = (fOffset[1] - fOffset[0]) / (fMea[1] - fMea[0]);
			constb = fOffset[1] - consta * fMea[1];

			GfTempConstA = (consta + 1) * GfTempConstA;
			GfTempConstB = (consta + 1) * GfTempConstB;
			GfTempConstC = (consta + 1) * GfTempConstC + constb;

dfp("GfTempConstA = %d GfTempConstB = %d GfTempConstC = %d\n", (int)(GfTempConstA*100), (int)(GfTempConstB*100), (int)(GfTempConstC*100));
dfp("tempCalData.setTemp1 = %d setTemp2 = %d meaTemp1 = %d meaTemp2 = %d\n", (int)(tempCalData.setTemp1*100), (int)(tempCalData.setTemp2*100), (int)(tempCalData.meaTemp1*100),(int)(tempCalData.meaTemp2*100));

			break;
		case TEMP_CAL_SAVE:
	//	consta = ((gOVEN_SERVICE.btCalibSetTemp[1] - gOVEN_SERVICE.fCalibMeasureTemp[1]) - (gOVEN_SERVICE.btCalibSetTemp[0] - gOVEN_SERVICE.fCalibMeasureTemp[0])) / (gOVEN_SERVICE.fCalibMeasureTemp[1] - gOVEN_SERVICE.fCalibMeasureTemp[0]);
	//	constb = (gOVEN_SERVICE.btCalibSetTemp[1] - gOVEN_SERVICE.fCalibMeasureTemp[1]) - consta * (gOVEN_SERVICE.fCalibMeasureTemp[1]);
			fSet[0] = tempCalData.setTemp1;
			fSet[1] = tempCalData.setTemp2;
			fMea[0] = tempCalData.meaTemp1;
			fMea[1] = tempCalData.meaTemp2;

//			fOffset[0] = tempCalData.meaTemp1 - tempCalData.setTemp1;
//			fOffset[1] = tempCalData.meaTemp2 - tempCalData.setTemp2;
			fOffset[0] = fMea[0] - fSet[0];
			fOffset[1] = fMea[1] - fSet[1];

//			consta = ((fMea[1] - fSet[1]) - (fMea[0] - fSet[0])) / (fMea[1] - fMea[0]);
//			constb = (fMea[1] - fSet[1]) - consta * (fMea[1]);
			consta = (fOffset[1] - fOffset[0]) / (fMea[1] - fMea[0]);
			constb = fOffset[1] - consta * fMea[1];

			GfTempConstA = (consta + 1) * GfTempConstA;
			GfTempConstB = (consta + 1) * GfTempConstB;
			GfTempConstC = (consta + 1) * GfTempConstC + constb;

	// Constant 저장
//			TempConst(DATA_SAVE);

			unsigned char data[21];
			data[0] = DATA_SAVE_FLAG;
			memcpy(&data[1], &(tempCalData.meaTemp1) , 20);
			eepromWrite_Data((TEMP_CAL_DATA_SAVE), 21, &(data[0]));
 			break;
		case TEMP_CAL_START:
			tempCalData.setTemp1 = TEMP_CAL_SET_1ST;
			tempCalData.setTemp2 = TEMP_CAL_SET_2ND;
			
			tempCalData.meaTemp1 = TEMP_CAL_SET_1ST;
			tempCalData.meaTemp2 = TEMP_CAL_SET_2ND;

/*
			tempCalData.constA = 0;
			tempCalData.constB = MAIN_TEMP_CONST_B;
			tempCalData.constC = MAIN_TEMP_CONST_C;
*/		
			GfTempConstA = 0;
			GfTempConstB = MAIN_TEMP_CONST_B;
			GfTempConstC =	MAIN_TEMP_CONST_C;
			break;
		case TEMP_CAL_RESET:
			tempCalData.setTemp1 = TEMP_CAL_SET_1ST;
			tempCalData.setTemp2 = TEMP_CAL_SET_2ND;
			
			tempCalData.meaTemp1 = TEMP_CAL_SET_1ST;
			tempCalData.meaTemp2 = TEMP_CAL_SET_2ND;

/*
			tempCalData.constA = 0;
			tempCalData.constB = MAIN_TEMP_CONST_B;
			tempCalData.constC = MAIN_TEMP_CONST_C;
*/		
			GfTempConstA = 0;
			GfTempConstB = MAIN_TEMP_CONST_B;
			GfTempConstC =	MAIN_TEMP_CONST_C;

			saveFlag = 0x00;
			eepromWrite_Data((TEMP_CAL_DATA_SAVE), 1, &saveFlag);
//			eepromWrite_Data((TEMP_CAL_DATA_ADDR), sizeof(tempCalData), &tempCalData);
			break;
	}
}

void tempSetup()
{
	if(runSetup.tempOnoff == ON) {

// COOLING_CONTROL - 20191119
#if AUTO_TUNNING_CONTROL
#if	AUTO_TUNNING_TARGET == HEATING_SYSTEM
		pPID_Control_Fn = pidAutoTunning;
//		freeSettemp=100.0;	//35.0;
#else
		pPID_Control_Fn = pidAutoTunnCooling;
//		freeSettemp=0.0;	//35.0;
#endif
#else
		pPID_Control_Fn = PID_Control;
#endif
/*
		if(runSetup.temp != devState.oldTemp) {			
			devState.setTempReset = TEMP_NEW_VALUE;
		}
		else {
			devState.setTempReset = TEMP_NOT_RESET;
		}
*/		
	}
	else {
		PELTIER_DATA(0);
	//	PELTIER_SET_HEATING;	
		PELTIER_CON_OFF;
		FanConOff();
		pPID_Control_Fn = PID_Function_Blank;
	}

dp("tempSetup ===%d  %d  ========",runSetup.tempOnoff,devState.oldTempOnoff);					
// COOLING_CONTROL - 20191119
	if(runSetup.temp != devState.oldTemp) {			
		devState.oldTemp = runSetup.temp;
		devState.setTempReset = TEMP_NEW_VALUE;		
dp("diff\n");			
	}
	else {
		if(runSetup.tempOnoff != devState.oldTempOnoff) {
//			devState.oldTempOnoff = runSetup.tempOnoff;
			devState.setTempReset = TEMP_NEW_VALUE;
dp("diff onoff\n");							
		}		
		else devState.setTempReset = TEMP_NOT_RESET;
	}

	devState.tempReadyUnder = runSetup.temp - TEMP_READY_RANGE;
	devState.tempReadyOver = runSetup.temp + TEMP_READY_RANGE;
}

int net_runSetup(char *pData)
{
	LCAS_RUN_SETUP_t tmp;

	if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
		send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
		return RE_ERR;
	}

	USE_LCAS_PACKET(pData,tmp);

	if(	tmp.tempOnoff > ON 
			|| tmp.temp > TEMP_MAX || tmp.temp < TEMP_MIN
			|| tmp.injectionStart > INJ_START_ALL_READY
			|| tmp.injectionMode > INJ_MODE_MICOR_PICK
			|| tmp.analysisTime < 0.0
			|| tmp.flushEnable > YES
			|| tmp.flushVolume < 0.0 || tmp.flushVolume > FLUSH_VOLUME_MAX 
|| tmp.injectionVolume > devSet.injectionVolumeMax
			|| tmp.washBetween > WASH_BETWEEN_ANALYSYS_END
			
			|| tmp.washVolume > devSet.washVolumeMax
			
			|| tmp.syringeUpSpdInj < SYRINGE_SPEED_MIN || tmp.syringeUpSpdInj > SYRINGE_SPEED_MAX
			|| tmp.syringeDnSpdInj < SYRINGE_SPEED_MIN || tmp.syringeDnSpdInj > SYRINGE_SPEED_MAX
			
			|| tmp.needleHeight < NEEDLE_HEIGHT_MIN || tmp.needleHeight > NEEDLE_HEIGHT_MAX
			|| tmp.waitTime_AfterAspirate > WAIT_TIME_MAX
			|| tmp.waitTime_AfterDispense > WAIT_TIME_MAX			
//			|| tmp.useMicroPump > YES
			|| tmp.skipMissingSample > YES
			|| tmp.airSegment > YES
			|| tmp.airSegmentVol > AIR_SEGMENT_VOL_MAX
		) {
		errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
		SEND_LCAS_PACKET(PACKCODE_LCAS_RUN_SETUP, runSetup, runSetup);

/*
dp("Temp %d %d\n",tmp.tempOnoff,(int)(tmp.temp*100));
dp("injectionStart %d \n",tmp.injectionStart);
dp("injectionMode %d \n",tmp.injectionMode);
dp("analysisTime %d \n",(int)(tmp.analysisTime*100));
dp("injectionVolume %d \n",tmp.injectionVolume);
dp("washBetween %d \n",tmp.washBetween);
dp("washVolume %d \n",tmp.washVolume);
dp("syringeVol %d \n",sysConfig.syringeVol);

dp("syringeUpSpdInj %d \n",tmp.syringeUpSpdInj);
dp("syringeDnSpdInj %d \n",tmp.syringeDnSpdInj);
dp("needleHeight %d \n",(int)(tmp.needleHeight*100));
dp("skipMissingSample %d \n",tmp.skipMissingSample);
dp("airSegment %d \n",tmp.airSegment);
dp("airSegmentVol %d \n",tmp.airSegmentVol);
*/
dfp("SELF_ERROR_COMMAND_UNKNOWN\n");
		return RE_ERR;
	}

//dprintf("PACKCODE_LCAS_RUN_SETUP === injectionVolume %f tmp.washNeedleBeforeInj %d\n",tmp.injectionVolume,tmp.washNeedleBeforeInj);

//	if(tmp.injectionVolume > devSet.injectionVolumeMax) 
	// 파라미터 값 검사해라 
	switch(tmp.injectionMode) {
		case INJ_MODE_PARTIAL_LOOP:
//			if(((sysConfig.sampleLoopVol) / 2) < tmp.injectionVolume) {
			if(sysConfig.sampleLoopVol  < (tmp.injectionVolume * 2.0f)) {
				errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
				SEND_LCAS_PACKET(PACKCODE_LCAS_RUN_SETUP, runSetup, runSetup);
dprintf("error INJ_MODE_PARTIAL_LOOP -injectionVolume %f \n",tmp.injectionVolume);				
		return RE_ERR;
			}
			break;				
		case INJ_MODE_MICOR_PICK:
			if(tmp.airSegment == YES) {
//				if(((sysConfig.sampleLoopVol - tmp.airSegmentVol) / 2) < tmp.injectionVolume) {
				if((sysConfig.sampleLoopVol - tmp.airSegmentVol) < (tmp.injectionVolume * 2.0f)) {
					errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
					SEND_LCAS_PACKET(PACKCODE_LCAS_RUN_SETUP, runSetup, runSetup);
dprintf("error INJ_MODE_PARTIAL_LOOP -injectionVolume %f \n",tmp.injectionVolume);			
		return RE_ERR;
				}
			}
			else {
//				if(((sysConfig.sampleLoopVol) / 2) < tmp.injectionVolume) {
				if(sysConfig.sampleLoopVol < ((tmp.injectionVolume) * 2.0f)) {
					errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
					SEND_LCAS_PACKET(PACKCODE_LCAS_RUN_SETUP, runSetup, runSetup);
dprintf("error INJ_MODE_PARTIAL_LOOP -injectionVolume %f \n",tmp.injectionVolume);			
		return RE_ERR;
				}
			}				
			break;
	}

// ==============================================================================================	
	memcpy(&runSetup, &tmp, sizeof(tmp));
	SEND_ACK_LCAS_PACKET(pData);

//	runSetup.syringeUpSpdInj = tmp.syringeUpSpdInj - 1;
//	runSetup.syringeDnSpdInj = tmp.syringeDnSpdInj - 1;
	runSetup.syringeUpSpdInj = tmp.syringeUpSpdInj;
	runSetup.syringeDnSpdInj = tmp.syringeDnSpdInj;

#if FPGA_VERSION_TRIM_PUMP_TIME
//devSet.microPumpTimeCount = runSetup.microPumpTime * 10000;
devSet.microPumpTimeCount = runSetup.microPumpTime * MICRO_PUMP_TIME_CONVERSION;
#endif

// COOLING_CONTROL - 20191119
//	devState.oldTemp = tmp.temp;
	tempSetup();

	switch(runSetup.injectionMode) {
		case INJ_MODE_FULL_LOOP:
			pLoop_Injection_Fn = full_LoopInjection;
			break;
		case INJ_MODE_PARTIAL_LOOP:
			pLoop_Injection_Fn = partial_LoopInjection;			
			break;
		case INJ_MODE_MICOR_PICK:
			pLoop_Injection_Fn = micro_pick_Injection;
			break;
	}

	devState.doNextSample = YES;
	
 	return RE_OK;
}

int net_TimeEvent(char *pData)
{
	LCAS_TIME_EVENT_t tmp;

	if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
		send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
		return RE_ERR;
	}

	USE_LCAS_PACKET(pData,tmp);

	if(	tmp.useAuxTimeEvent > ON || tmp.initAux > ON
		|| tmp.endTime < 0.0 || tmp.endTime > AUX_EVENT_TIME_MAX
		|| tmp.auxOutSigTime > AUX_OUT_SIGNAL_TIME_2sec
		) {
//SEND_LCAS_PACKET(PACKCODE_LCAS_RUN_SETUP, timeEvent, runSetup);
		errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
		SEND_LCAS_PACKET(PACKCODE_LCAS_TIME_EVENT, timeEvent, timeEvent);
dfp("tmp.useAuxTimeEvent = %d 	tmp.initAux = %d tmp.auxOutSigTim = %d\n",tmp.useAuxTimeEvent, tmp.initAux,tmp.auxOutSigTime);
dfp("SELF_ERROR_COMMAND_UNKNOWN\n");

		return RE_ERR;
	}
	else {
		for(int i=0; i<AUX_EVENT_COUNT; i++) {
			if(tmp.auxEvent[i].auxOnoff > ON || /*tmp.auxEvent[i].fTime < 0.0 || */tmp.auxEvent[i].fTime > AUX_EVENT_TIME_MAX) {
				errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
				SEND_LCAS_PACKET(PACKCODE_LCAS_TIME_EVENT, timeEvent, timeEvent);
				return RE_ERR;
			}
		}
	}		

// ==============================================================================================	
	memcpy(&timeEvent, &tmp, sizeof(tmp));
	SEND_ACK_LCAS_PACKET(pData);

	switch(timeEvent.auxOutSigTime) {
		case AUX_OUT_SIGNAL_TIME_disable:
			devState.auxOutSigTime = 0;
			break;
		case AUX_OUT_SIGNAL_TIME_100ms:
			devState.auxOutSigTime = 1;
			break;
		case AUX_OUT_SIGNAL_TIME_200ms:
			devState.auxOutSigTime = 2;
			break;
		case AUX_OUT_SIGNAL_TIME_500ms:
			devState.auxOutSigTime = 5;
			break;
		case AUX_OUT_SIGNAL_TIME_1sec:
			devState.auxOutSigTime = 10;
			break;
		case AUX_OUT_SIGNAL_TIME_2sec:
			devState.auxOutSigTime = 20;
			break;
	}

	for(int i=0 ; i<AUX_EVENT_COUNT ; i++) {
// Float형을 int로 변환시에 소수점 첫째자리에서 반올림.
// 0.5가 없으면 0.99999999999의 경우 잘림???????????? 
		devState.auxTime100msCnt[i] = (unsigned int)(timeEvent.auxEvent[i].fTime * 60. * 10. + 0.5);
//dfp("============Time Event :  devState.auxTime100msCnt[%d]=%d\n\r",i,devState.auxTime100msCnt[i]);		
	}
dfp("============Time Event :  devState.auxTime100msCnt[%d]=%d\n\r",0,devState.auxTime100msCnt[0]);		

	return RE_OK;
}

int net_MixSetup(char *pData)
{
	LCAS_MIX_t tmp;
	uint32_t action;
	unsigned int ret = 0;
	unsigned char err = 0;
	int totalVialCount = 0;

	if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
		send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
		return RE_ERR;
	}

	USE_LCAS_PACKET(pData,tmp);

	totalVialCount = devSet.trayLeft_rowNo * devSet.trayLeft_colNo + devSet.trayRight_rowNo * devSet.trayRight_colNo;
	
	if(tmp.relativeDestination == 0 
			&& ( (tmp.firstDestPos.uPos.tray == TRAY_LEFT && (tmp.firstDestPos.uPos.etc > POS_XY_HOME	|| tmp.firstDestPos.uPos.x >= devSet.trayLeft_rowNo || tmp.firstDestPos.uPos.y >= devSet.trayLeft_colNo) )
			|| (tmp.firstDestPos.uPos.tray == TRAY_RIGHT && (tmp.firstDestPos.uPos.etc > POS_XY_HOME	|| tmp.firstDestPos.uPos.x >= devSet.trayRight_rowNo || tmp.firstDestPos.uPos.y >= devSet.trayRight_colNo) )
			|| tmp.firstDestPos.uPos.tray > TRAY_RIGHT ))
	{
		err = SELF_ERROR_COMMAND_UNKNOWN;
		ret = RE_ERR;
dfp("ERROR1 : Mix Setup\n");		
dfp("firstDestPos.uPos rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeDestination,tmp.firstDestPos.uPos.tray,tmp.firstDestPos.uPos.etc, tmp.firstDestPos.uPos.x, tmp.firstDestPos.uPos.y);
dfp("reagentPosA.uPos  rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeReagentA,tmp.reagentPosA.uPos.tray,tmp.reagentPosA.uPos.etc, tmp.reagentPosA.uPos.x, tmp.reagentPosA.uPos.y);
dfp("reagentPosB.uPos  rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeReagentB,tmp.reagentPosB.uPos.tray,tmp.reagentPosB.uPos.etc, tmp.reagentPosB.uPos.x, tmp.reagentPosB.uPos.y);
dfp("reagentPosC.uPos  rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeReagentC,tmp.reagentPosC.uPos.tray,tmp.reagentPosC.uPos.etc, tmp.reagentPosC.uPos.x, tmp.reagentPosC.uPos.y);
dfp("reagentPosD.uPos  rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeReagentD,tmp.reagentPosD.uPos.tray,tmp.reagentPosD.uPos.etc, tmp.reagentPosD.uPos.x, tmp.reagentPosD.uPos.y);
	}
	else if(tmp.relativeReagentA == 0 
			&& ( (tmp.reagentPosA.uPos.tray == TRAY_LEFT && (tmp.reagentPosA.uPos.etc > POS_XY_HOME	|| tmp.reagentPosA.uPos.x >= devSet.trayLeft_rowNo || tmp.reagentPosA.uPos.y >= devSet.trayLeft_colNo) )
			|| (tmp.reagentPosA.uPos.tray == TRAY_RIGHT && (tmp.reagentPosA.uPos.etc > POS_XY_HOME	|| tmp.reagentPosA.uPos.x >= devSet.trayRight_rowNo || tmp.reagentPosA.uPos.y >= devSet.trayRight_colNo) )
			|| tmp.reagentPosA.uPos.tray > TRAY_RIGHT ))
	{
		err = SELF_ERROR_COMMAND_UNKNOWN;
		ret = RE_ERR;
	}
	else if(tmp.relativeReagentB == 0 
			&& ( (tmp.reagentPosB.uPos.tray == TRAY_LEFT && (tmp.reagentPosB.uPos.etc > POS_XY_HOME	|| tmp.reagentPosB.uPos.x >= devSet.trayLeft_rowNo || tmp.reagentPosB.uPos.y >= devSet.trayLeft_colNo) )
			|| (tmp.reagentPosB.uPos.tray == TRAY_RIGHT && (tmp.reagentPosB.uPos.etc > POS_XY_HOME	|| tmp.reagentPosB.uPos.x >= devSet.trayRight_rowNo || tmp.reagentPosB.uPos.y >= devSet.trayRight_colNo) )
			|| tmp.reagentPosB.uPos.tray > TRAY_RIGHT))
	{
		err = SELF_ERROR_COMMAND_UNKNOWN;
		ret = RE_ERR;
	}
	else if(tmp.relativeReagentC == 0 
			&& ( (tmp.reagentPosC.uPos.tray == TRAY_LEFT && (tmp.reagentPosC.uPos.etc > POS_XY_HOME	|| tmp.reagentPosC.uPos.x >= devSet.trayLeft_rowNo || tmp.reagentPosC.uPos.y >= devSet.trayLeft_colNo) )
			|| (tmp.reagentPosC.uPos.tray == TRAY_RIGHT && (tmp.reagentPosC.uPos.etc > POS_XY_HOME	|| tmp.reagentPosC.uPos.x >= devSet.trayRight_rowNo || tmp.reagentPosC.uPos.y >= devSet.trayRight_colNo) )
			|| tmp.reagentPosC.uPos.tray > TRAY_RIGHT))
	{
		err = SELF_ERROR_COMMAND_UNKNOWN;
		ret = RE_ERR;
	}
	else if(tmp.relativeReagentD == 0 
			&& ( (tmp.reagentPosD.uPos.tray == TRAY_LEFT && (tmp.reagentPosD.uPos.etc > POS_XY_HOME	|| tmp.reagentPosD.uPos.x >= devSet.trayLeft_rowNo || tmp.reagentPosD.uPos.y >= devSet.trayLeft_colNo) )
			|| (tmp.reagentPosD.uPos.tray == TRAY_RIGHT && (tmp.reagentPosD.uPos.etc > POS_XY_HOME	|| tmp.reagentPosD.uPos.x >= devSet.trayRight_rowNo || tmp.reagentPosD.uPos.y >= devSet.trayRight_colNo) )
			|| tmp.reagentPosD.uPos.tray > TRAY_RIGHT))
	{	// error
			err = SELF_ERROR_COMMAND_UNKNOWN;
			ret = RE_ERR;
	}
	else {
		if(	tmp.useMixPrgm > ON 
			|| (tmp.relativeDestination == 1 && (tmp.firstDestPos.pos > totalVialCount || tmp.firstDestPos.pos < -totalVialCount))
			|| (tmp.relativeReagentA == 1 && (tmp.reagentPosA.pos > totalVialCount || tmp.reagentPosA.pos < -totalVialCount))
			|| (tmp.relativeReagentB == 1 && (tmp.reagentPosB.pos > totalVialCount || tmp.reagentPosB.pos < -totalVialCount))
			|| (tmp.relativeReagentC == 1 && (tmp.reagentPosC.pos > totalVialCount || tmp.reagentPosC.pos < -totalVialCount))
			|| (tmp.relativeReagentD == 1 && (tmp.reagentPosD.pos > totalVialCount || tmp.reagentPosD.pos < -totalVialCount))
			) {
			err = SELF_ERROR_COMMAND_UNKNOWN;
			ret = RE_ERR;			
	dfp("ERROR : Mix Setup = %d %d \n",tmp.firstDestPos.uPos.tray,tmp.firstDestPos.pos);
dfp("firstDestPos.uPos rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeDestination,tmp.firstDestPos.uPos.tray,tmp.firstDestPos.uPos.etc, tmp.firstDestPos.uPos.x, tmp.firstDestPos.uPos.y);
dfp("reagentPosA.uPos  rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeReagentA,tmp.reagentPosA.uPos.tray,tmp.reagentPosA.uPos.etc, tmp.reagentPosA.uPos.x, tmp.reagentPosA.uPos.y);
dfp("reagentPosB.uPos  rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeReagentB,tmp.reagentPosB.uPos.tray,tmp.reagentPosB.uPos.etc, tmp.reagentPosB.uPos.x, tmp.reagentPosB.uPos.y);
dfp("reagentPosC.uPos  rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeReagentC,tmp.reagentPosC.uPos.tray,tmp.reagentPosC.uPos.etc, tmp.reagentPosC.uPos.x, tmp.reagentPosC.uPos.y);
dfp("reagentPosD.uPos  rel=%d tray=%d etc=%d (%d,%d)\n",tmp.relativeReagentD,tmp.reagentPosD.uPos.tray,tmp.reagentPosD.uPos.etc, tmp.reagentPosD.uPos.x, tmp.reagentPosD.uPos.y);
		}
	}
	
	dfp(" Mix Setup = %d %d \n",tmp.firstDestPos.uPos.tray,tmp.firstDestPos.pos);

	for(int i=0; i<MIX_TABLE_COUNT; i++) {
		action = tmp.mixTable[i].action;
//dfp("tmp.mixTable[%d].action = %d err = %d ret = %d\n",i,action,err,ret);			
		if(action == MIX_ACTION_NONE) break;	// for
		else if(/*action >= MIX_ACTION_NONE && */action <= MIX_ACTION_WAIT) {
			switch(action) {
				case MIX_ACTION_NONE:
//dprintf("MIX_ACTION_NONE == mixTable[%d].action = %d addVol=%f  sysConfig.syringeVol = %d \n" ,i,tmp.mixTable[i].action,tmp.mixTable[i].addVol,sysConfig.syringeVol);					
					break;
				case MIX_ACTION_ADD:	
					if(tmp.mixTable[i].addFrom > MIX_FROM_WASH || tmp.mixTable[i].addTo > MIX_TO_DESTINATION || tmp.mixTable[i].addVol > devSet.injectionVolumeMax) {	// sysConfig.syringeVol
//dprintf("MIX_ACTION_ADD == tmp.mixTable[i].addVol=%f  sysConfig.syringeVol = %d \n" ,tmp.mixTable[i].addVol,sysConfig.syringeVol);
						err = SELF_ERROR_COMMAND_UNKNOWN;
					}
//dprintf("MIX_ACTION_ADD == mixTable[%d].action = %d addVol=%f  sysConfig.syringeVol = %d \n" ,i,tmp.mixTable[i].action,tmp.mixTable[i].addVol,sysConfig.syringeVol);					
					break;
				case MIX_ACTION_MIX:
					if(tmp.mixTable[i].mixVial > MIX_TO_DESTINATION || tmp.mixTable[i].mixTimes > MIX_TIMES_MAX || tmp.mixTable[i].mixVol > devSet.injectionVolumeMax) {	// sysConfig.syringeVol
//dprintf("MIX_ACTION_MIX == tmp.firstDestPos.uPos.etc=%d\n",tmp.firstDestPos.uPos.etc);															
						err = SELF_ERROR_COMMAND_UNKNOWN;						
					}
//dprintf("MIX_ACTION_MIX == mixTable[%d].action = %d addVol=%f  sysConfig.syringeVol = %d \n" ,i,tmp.mixTable[i].action,tmp.mixTable[i].addVol,sysConfig.syringeVol);					
					break;
				case MIX_ACTION_WAIT:
					if(tmp.mixTable[i].waitTime > MIX_WAIT_TIME_MAX) {
//dprintf("MIX_ACTION_WAIT == tmp.mixTable[i].waitTime=%d\n",tmp.mixTable[i].waitTime);																					
						err = SELF_ERROR_COMMAND_UNKNOWN;
					}
//dprintf("MIX_ACTION_WAIT == mixTable[%d].action = %d addVol=%f  sysConfig.syringeVol = %d \n" ,i,tmp.mixTable[i].action,tmp.mixTable[i].addVol,sysConfig.syringeVol);					
					break;
			}
		}
		else {
dprintf("--error tmp.mixTable[i].action = %d\n",action);			
						err = SELF_ERROR_COMMAND_UNKNOWN;
		}
	}

if(err > SELF_NO_ERROR) {
		errorFunction(err);
		SEND_LCAS_PACKET(PACKCODE_LCAS_MIX_SETUP, mixSet, mixSet);				
		return ret;
}

// ==============================================================================================	
// ==============================================================================================	
	memcpy(&mixSet, &tmp, sizeof(tmp));
	SEND_ACK_LCAS_PACKET(pData);
/*
dfp("firstDestVial.pos (%x)\n",mixSet.firstDestPos.pos);
dfp("reagentVialA.pos (%x)\n",mixSet.reagentPosA.pos);
dfp("reagentVialB.pos (%x)\n",mixSet.reagentPosB.pos);
dfp("reagentVialC.pos (%x)\n",mixSet.reagentPosC.pos);
dfp("reagentVialD.pos (%x)\n",mixSet.reagentPosD.pos);
dfp("firstDestVial.uPos (%d,%d)\n",mixSet.firstDestPos.uPos.x, mixSet.firstDestPos.uPos.y);
dfp("reagentVialA.uPos (%d,%d)\n",mixSet.reagentPosA.uPos.x, mixSet.reagentPosA.uPos.y);
dfp("reagentVialB.uPos (%d,%d)\n",mixSet.reagentPosB.uPos.x, mixSet.reagentPosB.uPos.y);
dfp("reagentVialC.uPos (%d,%d)\n",mixSet.reagentPosC.uPos.x, mixSet.reagentPosC.uPos.y);
dfp("reagentVialD.uPos (%d,%d)\n",mixSet.reagentPosD.uPos.x, mixSet.reagentPosD.uPos.y);
*/
	int cnt, firstSamplePosCnt;
/*
firstSamplePosCnt = vialPos2Cnt(&sequence.firstSamplePos);
dfp("sequence.firstSamplePos (%d,%d)\n",sequence.firstSamplePos.uPos.x, sequence.firstSamplePos.uPos.y);
dfp("firstSamplePosCnt =%d\n",firstSamplePosCnt);
*/
	firstSamplePosCnt = 0;
#if 0
	if(mixSet.relativeDestination == 0) {
		devState.firstDestVial.pos = mixSet.firstDestPos.pos;
	}
	else {
		devState.firstDestVial.pos = mixSet.firstDestPos.pos;
	}
	if(mixSet.relativeReagentA == 0) {
		devState.reagentVialA.pos = mixSet.reagentPosA.pos;
	}
	else {
		devState.reagentVialA.pos = mixSet.reagentPosA.pos;
	}
	if(mixSet.relativeReagentB == 0) {
		devState.reagentVialB.pos = mixSet.reagentPosB.pos;
	}
	else {
		devState.reagentVialB.pos = mixSet.reagentPosB.pos;
	}
	if(mixSet.relativeReagentC == 0) {
		devState.reagentVialC.pos = mixSet.reagentPosC.pos;
	}
	else {
		devState.reagentVialC.pos = mixSet.reagentPosC.pos;
	}
	if(mixSet.relativeReagentD == 0) {
		devState.reagentVialD.pos = mixSet.reagentPosD.pos;
	}
	else {
		devState.reagentVialD.pos = mixSet.reagentPosD.pos;
	}
#else
	if(mixSet.relativeDestination == 0) {
		devState.firstDestVial.pos = mixSet.firstDestPos.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.firstDestPos.pos;
		devState.firstDestVial.pos = cnt2vialPos(cnt);
//			mixSet.firstDestPos.pos;
	}
	
	if(mixSet.relativeReagentA == 0) {
		devState.reagentVialA.pos = mixSet.reagentPosA.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.reagentPosA.pos;
		devState.reagentVialA.pos = cnt2vialPos(cnt);
	}

	if(mixSet.relativeReagentB == 0) {
		devState.reagentVialB.pos = mixSet.reagentPosB.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.reagentPosB.pos;
		devState.reagentVialB.pos = cnt2vialPos(cnt);
dfp("cnt = %d  devState.reagentVialB.pos = %d\n",cnt,devState.reagentVialB.pos);
	}

	if(mixSet.relativeReagentC == 0) {
		devState.reagentVialC.pos = mixSet.reagentPosC.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.reagentPosC.pos;
		devState.reagentVialC.pos = cnt2vialPos(cnt);
	}

	if(mixSet.relativeReagentD == 0) {
		devState.reagentVialD.pos = mixSet.reagentPosD.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.reagentPosD.pos;
		devState.reagentVialD.pos = cnt2vialPos(cnt);
	}
#endif

/*
dfp("==============firstDestVial.pos (%x)\n",devState.firstDestVial.pos);
dfp("reagentVialA.pos (%x)\n",devState.reagentVialA.pos);
dfp("reagentVialB.pos (%x)\n",devState.reagentVialB.pos);
dfp("reagentVialC.pos (%x)\n",devState.reagentVialC.pos);
dfp("reagentVialD.pos (%x)\n",devState.reagentVialD.pos);
dfp("firstDestVial.uPos (%d,%d)\n",devState.firstDestVial.uPos.x, devState.firstDestVial.uPos.y);
dfp("reagentVialA.uPos (%d,%d)\n",devState.reagentVialA.uPos.x, devState.reagentVialA.uPos.y);
dfp("reagentVialB.uPos (%d,%d)\n",devState.reagentVialB.uPos.x, devState.reagentVialB.uPos.y);
dfp("reagentVialC.uPos (%d,%d)\n",devState.reagentVialC.uPos.x, devState.reagentVialC.uPos.y);
dfp("reagentVialD.uPos (%d,%d)\n",devState.reagentVialD.uPos.x, devState.reagentVialD.uPos.y);
*/
	return RE_OK;
}

int vialPos2Cnt(VIAL_POS_t *pVial)
{
	int	rowNo	=	pVial->uPos.x;	// 현재	주입이 끝난	바이알 좌표
	int	colNo	=	pVial->uPos.y;
	int	trayNo = pVial->uPos.tray;

	int cnt;

	if(sysConfig.processType ==	PROCESS_TYPE_ROW)	{	// 가로
		if(trayNo	== TRAY_LEFT)	{
			cnt = rowNo + devSet.trayLeft_rowNo * colNo;	// 0부터 (0,0) => 0
		}
		else {
			cnt = rowNo + devSet.trayRight_rowNo * colNo;	// 0부터 (0,0) => 0
		}
	}
	else {
		if(trayNo	== TRAY_LEFT)	{
			cnt = colNo + devSet.trayLeft_colNo * rowNo;	// 0부터 (0,0) => 0
		}
		else {
			cnt = colNo + devSet.trayRight_colNo * rowNo;	// 0부터 (0,0) => 0
		}
	}
	return cnt;

}

int cnt2vialPos(int cnt)
{
	VIAL_POS_t vPos;
	int trayVialCnt;
	
	trayVialCnt = devSet.trayLeft_rowNo * devSet.trayLeft_colNo;
	
	if(sysConfig.processType ==	PROCESS_TYPE_ROW)	{	// 가로
		if(cnt >= trayVialCnt) { // tray Right
			vPos.uPos.tray = TRAY_RIGHT;
			cnt = cnt - trayVialCnt;
			vPos.uPos.y = cnt / devSet.trayRight_rowNo;
			vPos.uPos.x = cnt - devSet.trayRight_rowNo * vPos.uPos.y;
			vPos.uPos.etc = 0;
		}
		else {
			vPos.uPos.tray = TRAY_LEFT;
			vPos.uPos.y = cnt / devSet.trayLeft_rowNo;
			vPos.uPos.x = cnt - devSet.trayLeft_rowNo * vPos.uPos.y;
			vPos.uPos.etc = 0;
		}
	}
	else {
		if(cnt >= trayVialCnt) { // tray Right
			vPos.uPos.tray = TRAY_RIGHT;
			cnt = cnt - trayVialCnt;
			vPos.uPos.x = cnt / devSet.trayRight_colNo;
			vPos.uPos.y = cnt - devSet.trayRight_colNo * vPos.uPos.x;
			vPos.uPos.etc = 0;
		}
		else {
			vPos.uPos.tray = TRAY_LEFT;
			vPos.uPos.x = cnt / devSet.trayLeft_colNo;
			vPos.uPos.y = cnt - devSet.trayLeft_colNo * vPos.uPos.x;
			vPos.uPos.etc = 0;
		}
	}

	return vPos.pos;

}

int net_Sequence(char *pData)
{
	LCAS_SEQUENCE_t tmp;

/*
	if(devState.actionMenu == AS_ACTION_NONE) {
dp("system is running.... PACKCODE_LCAS_SEQUENCE cmd=%d sub1=%d sub2=%d\n",service.command, service.subCommand1, service.subCommand2);					
		send_SelfMessage(SELF_COMMAND_REJECT,0);
		return RE_ERR;
	}
*/
	if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
		send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
		return RE_ERR;
	}

	USE_LCAS_PACKET(pData,tmp);
dfp("injectionTimes = %d %x\n", tmp.injectionTimes, tmp.injectionTimes);
	if( (tmp.firstSamplePos.uPos.tray == TRAY_LEFT && (sysConfig.trayLeft == TRAY_NONE || tmp.firstSamplePos.uPos.etc > POS_XY_HOME	|| tmp.firstSamplePos.uPos.x > devSet.trayLeft_rowNo || tmp.firstSamplePos.uPos.y > devSet.trayLeft_colNo) )
		|| (tmp.firstSamplePos.uPos.tray == TRAY_RIGHT && (sysConfig.trayRight == TRAY_NONE || tmp.firstSamplePos.uPos.etc > POS_XY_HOME	|| tmp.firstSamplePos.uPos.x > devSet.trayRight_rowNo || tmp.firstSamplePos.uPos.y > devSet.trayRight_colNo) )
		|| tmp.firstSamplePos.uPos.tray > TRAY_RIGHT // TRAY_ETC
		||(tmp.lastSamplePos.uPos.tray == TRAY_LEFT && (sysConfig.trayLeft == TRAY_NONE || tmp.lastSamplePos.uPos.etc > POS_XY_HOME	|| tmp.lastSamplePos.uPos.x > devSet.trayLeft_rowNo || tmp.lastSamplePos.uPos.y > devSet.trayLeft_colNo) )
		|| (tmp.lastSamplePos.uPos.tray == TRAY_RIGHT && (sysConfig.trayRight == TRAY_NONE || tmp.lastSamplePos.uPos.etc > POS_XY_HOME	|| tmp.lastSamplePos.uPos.x > devSet.trayRight_rowNo || tmp.lastSamplePos.uPos.y > devSet.trayRight_colNo) )
		|| tmp.lastSamplePos.uPos.tray > TRAY_RIGHT // TRAY_ETC 
		|| tmp.injectionTimes > INJECTION_TIMES_MAX
//		|| tmp.waitTime
		) {	// error
		errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
		SEND_LCAS_PACKET(PACKCODE_LCAS_SEQUENCE, sequence, sequence);
dfp("SELF_ERROR_COMMAND_UNKNOWN\n");
dfp("tmp.firstSamplePos.pos = %x\n", tmp.firstSamplePos.pos);
dfp("tmp.lastSamplePos.pos = %x\n", tmp.lastSamplePos.pos);

dfp("tmp.firstSamplePos.uPos (%d,%d)\n",tmp.firstSamplePos.uPos.x, tmp.firstSamplePos.uPos.y);
dfp("tmp.lastSamplePos.uPos (%d,%d)\n",tmp.lastSamplePos.uPos.x, tmp.lastSamplePos.uPos.y);

dfp("injectionTimes = %d %x\n", tmp.injectionTimes, tmp.injectionTimes);
		return RE_ERR;		
	}

	memcpy(&sequence, &tmp, sizeof(tmp));
	SEND_ACK_LCAS_PACKET(pData);

dfp("injectionTimes = %d %x\n", tmp.injectionTimes, tmp.injectionTimes);

dfp("--PACKCODE_LCAS_SEQUENCE - net_Sequence\n");					

// 분석해야할	바이알 위치	-	initSequence()
  devState.sample.pos = sequence.firstSamplePos.pos;
// 분석중인 샘플 - 인젝션후 
	devState.curSample.pos = sequence.firstSamplePos.pos;
	devState.injectVial.pos = sequence.firstSamplePos.pos;
	devState.lastSample.pos = sequence.lastSamplePos.pos;

	devState.injectionTimes = sequence.injectionTimes;

//    devState.waitTime = sequence.waitTime;

	devState.seqAction = YES;

// MIx prgm 
	int firstSamplePosCnt,cnt;

	firstSamplePosCnt = vialPos2Cnt(&sequence.firstSamplePos);

	if(mixSet.relativeDestination == 0) {
		devState.firstDestVial.pos = mixSet.firstDestPos.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.firstDestPos.pos;
		devState.firstDestVial.pos = cnt2vialPos(cnt);
	}

#if 1
	if(mixSet.relativeReagentA == 0) {
		devState.reagentVialA.pos = mixSet.reagentPosA.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.reagentPosA.pos;
		devState.reagentVialA.pos = cnt2vialPos(cnt);
	}

	if(mixSet.relativeReagentB == 0) {
		devState.reagentVialB.pos = mixSet.reagentPosB.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.reagentPosB.pos;
		devState.reagentVialB.pos = cnt2vialPos(cnt);
	}

	if(mixSet.relativeReagentC == 0) {
		devState.reagentVialC.pos = mixSet.reagentPosC.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.reagentPosC.pos;
		devState.reagentVialC.pos = cnt2vialPos(cnt);
	}

	if(mixSet.relativeReagentD == 0) {
		devState.reagentVialD.pos = mixSet.reagentPosD.pos;
	}
	else {
		cnt = firstSamplePosCnt + mixSet.reagentPosD.pos;
		devState.reagentVialD.pos = cnt2vialPos(cnt);
	}
#else
	devState.reagentVialA.pos = mixSet.reagentPosA.pos;
	devState.reagentVialB.pos = mixSet.reagentPosB.pos;
	devState.reagentVialC.pos = mixSet.reagentPosC.pos;
	devState.reagentVialD.pos = mixSet.reagentPosD.pos;
#endif

dfp("==============firstDestVial.pos (%x)\n",devState.firstDestVial.pos);
dfp("reagentVialA.pos (%x)\n",devState.reagentVialA.pos);
dfp("reagentVialB.pos (%x)\n",devState.reagentVialB.pos);
dfp("reagentVialC.pos (%x)\n",devState.reagentVialC.pos);
dfp("reagentVialD.pos (%x)\n",devState.reagentVialD.pos);
dfp("firstDestVial.uPos (%d,%d)\n",devState.firstDestVial.uPos.x, devState.firstDestVial.uPos.y);
dfp("reagentVialA.uPos (%d,%d)\n",devState.reagentVialA.uPos.x, devState.reagentVialA.uPos.y);
dfp("reagentVialB.uPos (%d,%d)\n",devState.reagentVialB.uPos.x, devState.reagentVialB.uPos.y);
dfp("reagentVialC.uPos (%d,%d)\n",devState.reagentVialC.uPos.x, devState.reagentVialC.uPos.y);
dfp("reagentVialD.uPos (%d,%d)\n",devState.reagentVialD.uPos.x, devState.reagentVialD.uPos.y);
 	return RE_OK;
}

void commandStopFn()
{
	devState.command = COMMAND_STOP;

	devState.doNextSample = NO;
	devState.isNextInjection = NO;

	// 하던 동작을 멈추고 ,
	// sequence남은 동작 삭제
	// 다시 주입가능한 STATE_READY상태로 

// 20200526
stepCtrlCmd(MOTOR_X, HOME_DIR_X | 	CMD_BREAK);
stepCtrlCmd(MOTOR_Y, HOME_DIR_Y | 	CMD_BREAK);
stepCtrlCmd(MOTOR_Z, HOME_DIR_Z | 	CMD_BREAK);
stepCtrlCmd(MOTOR_SYR, HOME_DIR_SYR | 	CMD_BREAK);
stepCtrlCmd(MOTOR_VAL, HOME_DIR_VAL | 	CMD_BREAK);
stepCtrlCmd(MOTOR_INJ, HOME_DIR_INJ | 	CMD_BREAK);

// RyuSF2-20210601
//delay_btw_transfers (10000000);
delay_btw_transfers (1000000);


	devState.homeFindEnd[MOTOR_X] = NO;
	devState.homeFindEnd[MOTOR_Y] = NO;
	devState.homeFindEnd[MOTOR_Z] = NO;
	devState.homeFindEnd[MOTOR_SYR] = NO;
	devState.homeFindEnd[MOTOR_VAL] = NO;
	devState.homeFindEnd[MOTOR_INJ] = NO;

// break 상태이므로 reset명령으로  
	motorResetAll();

	initValue();
PumpConOff();	
	initSpecial(99);
	devState.initCommand = YES;
	devState.command = COMMAND_INIT;//COMMAND_RESET;

	devState.actionMenuOld = devState.actionMenu;
	devState.actionMenu = AS_ACTION_INITILIZE;
	devState.actionSubmenuOld = devState.actionSubmenu;
	devState.actionSubmenu = AS_INIT_SUB_FIND_HOME_ALL;
//		devState.actionSubmenu = AS_INIT_SUB_NONE;

//				devState.seqAction = NO;
//				stateChange(STATE_INITILIZE);

}

void commandAbortFn()
{
// readMotorState()에서 처리한다.				
	devState.command = COMMAND_ABORT;

	devState.doNextSample = NO;
	devState.isNextInjection = NO;

//	stateChange(STATE_FAULT);
PumpConOff();
}

int net_Command(char *pData)
{
	uint32_t ret = YES;
	
	USE_LCAS_PACKET(pData,command);
dp("PACKCODE_LCAS_COMMAND cmd=%d sub1=%d sub2=%d\n LcasState.state=%d devState.command=%d\n"
	,command.btCommand, command.btSubCommand1, command.btSubCommand2,LcasState.state,devState.command);

	switch(command.btCommand) {
		case COMMAND_NONE:
			devState.command = COMMAND_NONE;
			errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			ret = NO;			
			break;
		case COMMAND_LOADING_INJECTION:
			if(LcasState.state == STATE_READY) {
				SEND_ACK_LCAS_PACKET(pData);				
				
				devState.command = COMMAND_LOADING_INJECTION;
				startLoadingCommand();
send_SelfMessage(SELF_START_LOADING	,SELF_START_LOADING_PCIN);
			}
			else {
				send_SelfMessage(SELF_COMMAND_REJECT,devState.command);	// 실행할 수 없음.
//				devState.command = COMMAND_NONE;
				ret = NO;
			}
			break;
		case COMMAND_LOADING:
				SEND_ACK_LCAS_PACKET(pData);
			
			break;
		case COMMAND_INJECTION:
			// 실행후 전송하라.
				SEND_ACK_LCAS_PACKET(pData);
			break;
		case COMMAND_END:
			if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
				SEND_ACK_LCAS_PACKET(pData);				
				devState.command = COMMAND_END;	// chkNextInjection()함수 참조

devState.doNextSample = NO;
devState.isNextInjection = NO;
			}
			else {
//				devState.command = COMMAND_NONE;
				send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
				ret = NO;
			}
PumpConOff();			
			break;
		case COMMAND_RESUME:
			if(devState.command == COMMAND_PAUSE) {
				SEND_ACK_LCAS_PACKET(pData);
				
				devState.command = COMMAND_RESUME;
				devState.actionMenu = devState.actionMenuOld;
				devState.actionSubmenu = devState.actionSubmenuOld;

				stateChange(devState.stateOld);
			}
			else {
//				devState.command = COMMAND_NONE;
				send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
				ret = NO;
			}
			break;
		case COMMAND_STOP:
// 멈추고 - Ready상태로 (initialize후 Ready상태로)
// 
#if RECEIVE_COMMAND_STOP_ANY
// 20200526
//==================================================================================
// 초기화를 마친후 sleepModeAll() 에서 devState.initCommand = NO;로 만든다.
// COMMAND_ABORT명령으로도 해제 할 수 있다. devState.initCommand = NO;로 만든다.
//==================================================================================
if(devState.initCommand == NO) {
		devState.initCommand = YES;
				SEND_ACK_LCAS_PACKET(pData);			
				commandStopFn();
}
else {
				send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
				ret = NO;
}

				
#else
			if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
				SEND_ACK_LCAS_PACKET(pData);			
				commandStopFn();
			}
			else {
				send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
				ret = NO;
			}
#endif			
			break;
		case COMMAND_ABORT:	// 즉시멈춤 - 
// 20200526
//==================================================================================
// 초기화를 마친후 sleepModeAll() 에서 devState.initCommand = NO;로 만든다.
// COMMAND_ABORT명령으로도 해제 할 수 있다. devState.initCommand = NO;로 만든다.
//==================================================================================
			SEND_ACK_LCAS_PACKET(pData);
			commandAbortFn();
			
			break;
		case COMMAND_PAUSE:
			if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
				SEND_ACK_LCAS_PACKET(pData);				
				devState.command = COMMAND_PAUSE;
				devState.actionMenuOld = devState.actionMenu;
				devState.actionMenu = AS_ACTION_STANDBY;
				devState.actionSubmenuOld = devState.actionSubmenu;

				stateChange(STATE_PAUSE);
			}
			else {
//				devState.command = COMMAND_NONE;
				send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
				ret = NO;
			}
			break;
		case COMMAND_INIT:
#if RECEIVE_COMMAND_INIT_FAULT			
			if(LcasState.state == STATE_FAULT) {
#endif				

// 20200526
//==================================================================================
// 초기화를 마친후 sleepModeAll() 에서 devState.initCommand = NO;로 만든다.
// COMMAND_ABORT명령으로도 해제 할 수 있다. devState.initCommand = NO;로 만든다.
//==================================================================================
#if RECEIVE_COMMAND_INIT_ANY
if(devState.initCommand == NO || LcasState.state == STATE_FAULT) {
#else
if(devState.initCommand == NO) {
#endif	

SEND_ACK_LCAS_PACKET(pData);

				devState.homeFindEnd[MOTOR_X] = NO;
				devState.homeFindEnd[MOTOR_Y] = NO;
				devState.homeFindEnd[MOTOR_Z] = NO;
				devState.homeFindEnd[MOTOR_SYR] = NO;
				devState.homeFindEnd[MOTOR_VAL] = NO;
				devState.homeFindEnd[MOTOR_INJ] = NO;


// 20200526
stepCtrlCmd(MOTOR_X, HOME_DIR_X | 	CMD_BREAK);
stepCtrlCmd(MOTOR_Y, HOME_DIR_Y | 	CMD_BREAK);
stepCtrlCmd(MOTOR_Z, HOME_DIR_Z | 	CMD_BREAK);
stepCtrlCmd(MOTOR_SYR, HOME_DIR_SYR | 	CMD_BREAK);
stepCtrlCmd(MOTOR_VAL, HOME_DIR_VAL | 	CMD_BREAK);
stepCtrlCmd(MOTOR_INJ, HOME_DIR_INJ | 	CMD_BREAK);

// RyuSF2-20210601
//delay_btw_transfers (10000000);
delay_btw_transfers (1000000);

// break 상태이므로 reset명령으로  
				motorResetAll();

				initValue();
PumpConOff();
				initSpecial(99);

				devState.initCommand = YES;
				devState.command = COMMAND_INIT;//COMMAND_RESET;


				devState.actionMenuOld = devState.actionMenu;
				devState.actionMenu = AS_ACTION_INITILIZE;
				devState.actionSubmenuOld = devState.actionSubmenu;
				devState.actionSubmenu = AS_INIT_SUB_FIND_HOME_ALL;
				
}
else {
				send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
				ret = NO;				
}

#if RECEIVE_COMMAND_INIT_FAULT
			}
			else {
//				devState.command = COMMAND_NONE;
				send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
				ret = NO;
			}
#endif			
			break;
		default:
			devState.command = COMMAND_NONE;
			errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			ret = NO;
			break;
	}

	return 0;

}

int net_State()
{
	return 0;
}

int net_SelfMessage()
{
	return 0;
}

int net_AdjustData()
{
	return 0;
}

#if	EEPROM_ADJUST_DATA_SAVE_FIX
#define ADJUST_DATA_SAVE_FIX			1
int net_ServiceAdjust()
{
	unsigned char part = service.subCommand1;
	unsigned char act = service.subCommand2;
	unsigned char ackPacket = YES;	// adjust 패킷을 응답해야한다.
	static int initPos = NO;
	unsigned char checkSum;
	int i,j;

unsigned char *da;
unsigned char *da2;

unsigned char *tmp;
unsigned char *old;

 tmp = (unsigned char*)(&devSet.adjustTmp.saveFlag);
 old = (unsigned char*)(&devSet.adjustOld.saveFlag);
 
dp("net_ServiceAdjust cmd=%d sub1=%d sub2=%d\n",service.command, service.subCommand1, service.subCommand2);				
//	devSet.adjustCurMotor = service.subCommand1;
//	ackPacket = net_AdjustFunc(devSet.adjustCurMotor, act);

		switch(act) {
			case ADJUST_LEFT:	// CCW
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X];
						break;
					case ADJUST_NEEDLE_HEIGHT:						
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
//						++devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X];
						break;
					case ADJUST_SYRINGE:						break;
					case ADJUST_VALVE_POS:
						if(devSet.adjustTmp.value[ADJUST_VALVE_X] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goVal(ADJUST_STEP_UNIT_VAL, 0,ADJUST_SPEED_VAL, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.value[ADJUST_VALVE_X];
						break;
					case ADJUST_INJECTOR_POS:
						if(devSet.adjustTmp.value[ADJUST_INJECTOR_X] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goInj(ADJUST_STEP_UNIT_INJ, 0,ADJUST_SPEED_INJ, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.value[ADJUST_INJECTOR_X];
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjustTmp.wash_x >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.wash_x;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjustTmp.waste_x >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.waste_x;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjustTmp.leftVial_x[sysConfig.trayLeft] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.leftVial_x[sysConfig.trayLeft];
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjustTmp.rightVial_x[sysConfig.trayRight] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.rightVial_x[sysConfig.trayRight];
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjustTmp.vialHeight_x >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.vialHeight_x;
						break;
#endif						
				}
	      break;
	    case ADJUST_LEFT_X2:	// CCW X2
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X , 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X] += ADJUST_STEP_UNIT_SCALE_X;			
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X , 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_SYRINGE:						break;						
					case ADJUST_VALVE_POS:
						if(devSet.adjustTmp.value[ADJUST_VALVE_X] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goVal(ADJUST_STEP_UNIT_VAL * ADJUST_STEP_UNIT_SCALE_VAL, 0,ADJUST_SPEED_VAL, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_VALVE_X] += ADJUST_STEP_UNIT_SCALE_VAL;
						break;
					case ADJUST_INJECTOR_POS:
						if(devSet.adjustTmp.value[ADJUST_INJECTOR_X] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goInj(ADJUST_STEP_UNIT_INJ * ADJUST_STEP_UNIT_SCALE_INJ, 0,ADJUST_SPEED_INJ, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_INJECTOR_X] += ADJUST_STEP_UNIT_SCALE_INJ;
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjustTmp.wash_x >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.wash_x += ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjustTmp.waste_x >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.waste_x += ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjustTmp.leftVial_x[sysConfig.trayLeft] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.leftVial_x[sysConfig.trayLeft] += ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjustTmp.rightVial_x[sysConfig.trayRight] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.rightVial_x[sysConfig.trayRight] += ADJUST_STEP_UNIT_SCALE_X;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjustTmp.vialHeight_x >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.vialHeight_x += ADJUST_STEP_UNIT_SCALE_X;
						break;
#endif						
				}
	      break;
	    case ADJUST_RIGHT:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);			
						--devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X];
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);			
						break;
					case ADJUST_SYRINGE:						break;
					case ADJUST_VALVE_POS:
						if(devSet.adjustTmp.value[ADJUST_VALVE_X] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goVal(-ADJUST_STEP_UNIT_VAL, 0,ADJUST_SPEED_VAL, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.value[ADJUST_VALVE_X];
						break;
					case ADJUST_INJECTOR_POS:
						if(devSet.adjustTmp.value[ADJUST_INJECTOR_X] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goInj(-ADJUST_STEP_UNIT_INJ, 0,ADJUST_SPEED_INJ, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.value[ADJUST_INJECTOR_X];
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjustTmp.wash_x <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);			
						--devSet.adjustTmp.wash_x;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjustTmp.waste_x <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);			
						--devSet.adjustTmp.waste_x;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjustTmp.leftVial_x[sysConfig.trayLeft] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.leftVial_x[sysConfig.trayLeft];
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjustTmp.rightVial_x[sysConfig.trayRight] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.rightVial_x[sysConfig.trayRight];
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goX(-ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjustTmp.vialHeight_x <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX(-ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.vialHeight_x;
						break;
#endif						
				}
	      break;				
	    case ADJUST_RIGHT_X2:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X , 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X] -= ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X , 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_SYRINGE:						break;						
					case ADJUST_VALVE_POS:
						if(devSet.adjustTmp.value[ADJUST_VALVE_X] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goVal((-ADJUST_STEP_UNIT_VAL) * ADJUST_STEP_UNIT_SCALE_VAL, 0,ADJUST_SPEED_VAL, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_VALVE_X] -= ADJUST_STEP_UNIT_SCALE_VAL;
						break;
					case ADJUST_INJECTOR_POS:
						if(devSet.adjustTmp.value[ADJUST_INJECTOR_X] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goInj((-ADJUST_STEP_UNIT_INJ) * ADJUST_STEP_UNIT_SCALE_INJ, 0,ADJUST_SPEED_INJ, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_INJECTOR_X] -= ADJUST_STEP_UNIT_SCALE_INJ;
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjustTmp.wash_x <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);			
						devSet.adjustTmp.wash_x -= ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjustTmp.waste_x <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);			
						devSet.adjustTmp.waste_x -= ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjustTmp.leftVial_x[sysConfig.trayLeft] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.leftVial_x[sysConfig.trayLeft] -= ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjustTmp.rightVial_x[sysConfig.trayRight] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.rightVial_x[sysConfig.trayRight] -= ADJUST_STEP_UNIT_SCALE_X;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjustTmp.vialHeight_x <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.vialHeight_x -= ADJUST_STEP_UNIT_SCALE_X;						
						break;
#endif							
				}
	      break;
	    case ADJUST_UP:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						break;
					case ADJUST_NEEDLE_HEIGHT:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Z] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						--devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Z];
						break;
					case ADJUST_SYRINGE:
						if(devSet.adjustTmp.value[ADJUST_SYRINGE_Z] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goSyr(-ADJUST_STEP_UNIT_SYR, 0, ADJUST_SPEED_SYL, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.value[ADJUST_SYRINGE_Z];
						break;						
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_VIAL_HEIGHT:
						if(devSet.adjustTmp.vialHeight <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						--devSet.adjustTmp.vialHeight;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_WASHING_PORT:
					case ADJUST_WASTE_PORT:
					case ADJUST_VIAL_LEFT:
					case ADJUST_VIAL_RIGHT:
					case ADJUST_VIAL_HEIGHT_XY:	// 값은 변화 없고 움직임 만 
						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						break;
#endif
					case ADJUST_MICRO_PUMP_VOLTAGE:
						if(devSet.adjustTmp.microPumpVolt >= ADJUST_VALUE_MICRO_PUMP_MAX) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_1);
							break;
						}
//						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						++devSet.adjustTmp.microPumpVolt;
						break;
				}	    
				break;
	    case ADJUST_UP_X2:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						goZ((-ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_NEEDLE_HEIGHT:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Z] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goZ((-ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Z] -= ADJUST_STEP_UNIT_SCALE_Z;
						break;
					case ADJUST_SYRINGE:
						if(devSet.adjustTmp.value[ADJUST_SYRINGE_Z] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goSyr((-ADJUST_STEP_UNIT_SYR) * ADJUST_STEP_UNIT_SCALE_SYR, 0, ADJUST_SPEED_SYL, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_SYRINGE_Z] -= ADJUST_STEP_UNIT_SCALE_SYR;
						break;						
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_VIAL_HEIGHT:
						if(devSet.adjustTmp.vialHeight <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goZ((-ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.vialHeight -= ADJUST_STEP_UNIT_SCALE_Z;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_WASHING_PORT:
					case ADJUST_WASTE_PORT:
					case ADJUST_VIAL_LEFT:
					case ADJUST_VIAL_RIGHT:
					case ADJUST_VIAL_HEIGHT_XY:	// 값은 변화 없고 움직임 만 
						goZ((-ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);	
						break;
#endif					
					case ADJUST_MICRO_PUMP_VOLTAGE:
						if(devSet.adjustTmp.microPumpVolt >= ADJUST_VALUE_MICRO_PUMP_MAX) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_1);
							break;
						}
						else if(devSet.adjustTmp.microPumpVolt == (ADJUST_VALUE_MICRO_PUMP_MAX-1)) {	// 최대 10 volt ==> 25  최소 5volt ==> 0						
							devSet.adjustTmp.microPumpVolt = ADJUST_VALUE_MICRO_PUMP_MAX;
						}
						else {
//						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);										
							devSet.adjustTmp.microPumpVolt += 2;
						}
						break;
				}	    
	      break;
	    case ADJUST_DOWN:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						goZ((ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);									
						break;
					case ADJUST_NEEDLE_HEIGHT:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Z] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goZ((ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						++devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Z];
						break;
					case ADJUST_SYRINGE:
						if(devSet.adjustTmp.value[ADJUST_SYRINGE_Z] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}						
						goSyr((ADJUST_STEP_UNIT_SYR), 0, ADJUST_SPEED_SYL, ACT_MOVE_STEP_RELATE);			
						++devSet.adjustTmp.value[ADJUST_SYRINGE_Z];
						break;						
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_VIAL_HEIGHT:
						if(devSet.adjustTmp.vialHeight >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goZ((ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						++devSet.adjustTmp.vialHeight;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_WASHING_PORT:
					case ADJUST_WASTE_PORT:
					case ADJUST_VIAL_LEFT:
					case ADJUST_VIAL_RIGHT:
					case ADJUST_VIAL_HEIGHT_XY:	// 값은 변화 없고 움직임 만 
						goZ((ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						break;
#endif					
					case ADJUST_MICRO_PUMP_VOLTAGE:
						if(devSet.adjustTmp.microPumpVolt <= ADJUST_VALUE_MICRO_PUMP_MIN) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_1);
							break;
						}
						else {
//						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);										
							--devSet.adjustTmp.microPumpVolt;
						}						
						break;

				}	  
	      break;
	    case ADJUST_DOWN_X2:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						goZ((ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_NEEDLE_HEIGHT:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Z] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goZ((ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Z] += ADJUST_STEP_UNIT_SCALE_Z;
						break;
					case ADJUST_SYRINGE:
						if(devSet.adjustTmp.value[ADJUST_SYRINGE_Z] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goSyr((ADJUST_STEP_UNIT_SYR) * ADJUST_STEP_UNIT_SCALE_SYR, 0, ADJUST_SPEED_SYL, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_SYRINGE_Z] += ADJUST_STEP_UNIT_SCALE_SYR;
						break;						
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_VIAL_HEIGHT:
						if(devSet.adjustTmp.vialHeight >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goZ((ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.vialHeight += ADJUST_STEP_UNIT_SCALE_Z;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_WASHING_PORT:
					case ADJUST_WASTE_PORT:
					case ADJUST_VIAL_LEFT:
					case ADJUST_VIAL_RIGHT:
					case ADJUST_VIAL_HEIGHT_XY:	// 값은 변화 없고 움직임 만 
						goZ((ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);	
						break;
#endif							
					case ADJUST_MICRO_PUMP_VOLTAGE:
						if(devSet.adjustTmp.microPumpVolt <= ADJUST_VALUE_MICRO_PUMP_MIN) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_1);
							break;
						}
						else if(devSet.adjustTmp.microPumpVolt == (ADJUST_VALUE_MICRO_PUMP_MIN+1)) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							devSet.adjustTmp.microPumpVolt = ADJUST_VALUE_MICRO_PUMP_MIN;
						}
						else {
//						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);										
							devSet.adjustTmp.microPumpVolt -= 2;
						}						
						break;
				}	  
	      break;
	    case ADJUST_BACK:		// Back
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);			
						--devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y];
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);			
						break;
					case ADJUST_SYRINGE:
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjustTmp.wash_y <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.wash_y;
						break;						
					case ADJUST_WASTE_PORT:
						if(devSet.adjustTmp.waste_y <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.waste_y;
						break;						
					case ADJUST_VIAL_LEFT:
						if(devSet.adjustTmp.leftVial_y[sysConfig.trayLeft] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.leftVial_y[sysConfig.trayLeft];
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjustTmp.rightVial_y[sysConfig.trayRight] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.rightVial_y[sysConfig.trayRight];
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjustTmp.vialHeight_y <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjustTmp.vialHeight_y;
						break;
#endif							
				}	    
				break;
	    case ADJUST_BACK_X2:	// Back X2
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y , 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y] -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y , 0, ACT_MOVE_STEP_RELATE);
#if TEST_EEPROM
 da = (unsigned char*)(&devSet.adjust.saveFlag);
 da2 = (unsigned char*)(&devSet.adjust_2nd.saveFlag); 

		eepromRead_Data(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust),devSet.adjust.saveFlag);	
		eepromRead_Data(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust_2nd),devSet.adjust_2nd.saveFlag);	

		for(i=0;i<45;i++) {
			if(da[i] != da2[i])  {
dp("[%d:%x:%x]",i,da[i],da2[i]);
			}
		}
#endif				
						break;
					case ADJUST_SYRINGE:
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjustTmp.wash_y <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.wash_y -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjustTmp.waste_y <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.waste_y -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjustTmp.leftVial_y[sysConfig.trayLeft] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.leftVial_y[sysConfig.trayLeft] -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjustTmp.rightVial_y[sysConfig.trayRight] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.rightVial_y[sysConfig.trayRight] -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjustTmp.vialHeight_y <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.vialHeight_y -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
#endif							
				}	    
	      break;
	    case ADJUST_FRONT:	// Front
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);			
						++devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y];
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goY((ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);			
						break;
					case ADJUST_SYRINGE:
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjustTmp.wash_y >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.wash_y;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjustTmp.waste_y >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.waste_y;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjustTmp.leftVial_y[sysConfig.trayLeft] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.leftVial_y[sysConfig.trayLeft];
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjustTmp.rightVial_y[sysConfig.trayRight] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.rightVial_y[sysConfig.trayRight];
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goY((ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjustTmp.vialHeight_y >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjustTmp.vialHeight_y;
						break;
#endif							
				}	  
	      break;
	    case ADJUST_FRONT_X2:	// Front X2
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y] += ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goY((ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);

#if TEST_EEPROM
 da = (unsigned char*)(&devSet.adjust.saveFlag);
 da2 = (unsigned char*)(&devSet.adjust_2nd.saveFlag); 
 		for(i=0;i<45;i++) {
da[i] = i;
da2[i] = i;
dp("[%d:%x:%x]",i,da[i],da2[i]);
		}
 
		eepromWrite_Data(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) ,devSet.adjust.saveFlag);
		eepromWrite_Data(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust_2nd) ,devSet.adjust_2nd.saveFlag);
#endif						
						break;
					case ADJUST_SYRINGE:
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjustTmp.wash_y >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.wash_y += ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjustTmp.waste_y >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.waste_y += ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjustTmp.leftVial_y[sysConfig.trayLeft] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.leftVial_y[sysConfig.trayLeft] += ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjustTmp.rightVial_y[sysConfig.trayRight] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.rightVial_y[sysConfig.trayRight] += ADJUST_STEP_UNIT_SCALE_Y;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goY((ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjustTmp.vialHeight_y >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,ADJUST_VALUE_ERROR_0);
							break;
						}
						goY((ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjustTmp.vialHeight_y += ADJUST_STEP_UNIT_SCALE_Y;
						break;
#endif							
				}	  
	      break;
	    case ADJUST_SAVE:
iprintf("	case ADJUST_SAVE:\n");
for(i=0;i<43;i++) {
	if(old[i] != tmp[i]) {
		iprintf("old[%d] = %d  tmp[%d] = %d\n",i,old[i],i,tmp[i]);
	}
}
 				switch(part) {
					case ADJUST_NEEDLE_XY:
			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_X] = DATA_SAVE_FLAG;
			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_Y] = DATA_SAVE_FLAG;

				    devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_X] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] = devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X];
				    devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_Y] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] = devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y];

for(i=0;i<6;i++) {
dp("=============\n devSet.adjust.saveFlag = %d devSet.adjust.value = %d\n",devSet.adjust.saveFlag[i],devSet.adjust.value[i]);
dp("===========================\n devSet.adjust_2nd.saveFlag = %d devSet.adjust_2nd.value = %d\n",devSet.adjust_2nd.saveFlag[i],devSet.adjust_2nd.value[i]);
}

//						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
#if 1
eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);	
#else
		eepromWrite_Data(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);	

#endif

						
#if	ADJUST_DATA_SAVE_FIX
						devSet.adjust_2nd.value[ADJUST_INJ_PORT_VALUE_X] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X];
						devSet.adjust_2nd.value[ADJUST_INJ_PORT_VALUE_Y] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y];
						checkSum = 0;
						for(i=0 ; i<6 ; i++) {
							checkSum = checkSum ^ devSet.adjust.value[i];
						}
						devSet.adjust_2nd.saveFlag[0] = checkSum;
	
//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveFlag[0], 12);
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);		
		ackPacket = NO;		
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);																		
		
#endif
for(i=0;i<6;i++) {
dp("===========================\n devSet.adjust.saveFlag = %d devSet.adjust.value = %d\n",devSet.adjust.saveFlag[i],devSet.adjust.value[i]);
dp("===========================\n devSet.adjust_2nd.saveFlag = %d devSet.adjust_2nd.value = %d\n",devSet.adjust_2nd.saveFlag[i],devSet.adjust_2nd.value[i]);
}

						initCoordinate();
						break;
					case ADJUST_NEEDLE_HEIGHT:
//			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_X] = DATA_SAVE_FLAG;
//			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_Y] = DATA_SAVE_FLAG;
			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_Z] = DATA_SAVE_FLAG;

//				    devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_X] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] = devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_X];
//				    devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_Y] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] = devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Y];
						devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_Z] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] = devSet.adjustTmp.value[ADJUST_INJ_PORT_VALUE_Z];

for(i=0;i<6;i++) {
dp("===========================\n devSet.adjust.saveFlag = %d devSet.adjust.value = %d\n",devSet.adjust.saveFlag[i],devSet.adjust.value[i]);
}

//						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);	
						
//						devSet.adjust_2nd.value[ADJUST_INJ_PORT_VALUE_X] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X];
//						devSet.adjust_2nd.value[ADJUST_INJ_PORT_VALUE_Y] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y];
						devSet.adjust_2nd.value[ADJUST_INJ_PORT_VALUE_Z] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z];						
						checkSum = 0;
						for(i=0 ; i<6 ; i++) {
							checkSum = checkSum ^ devSet.adjust.value[i];
						}
						devSet.adjust_2nd.saveFlag[0] = checkSum;
						
//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveFlag[0], 12);
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);	
		ackPacket = NO;
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);																		
		
						initCoordinate();
						break;
					case ADJUST_SYRINGE:
			    	devSet.adjust.saveFlag[ADJUST_SYRINGE_Z] = DATA_SAVE_FLAG;
						
						devSet.adjustOld.value[ADJUST_SYRINGE_Z] = devSet.adjust.value[ADJUST_SYRINGE_Z] = devSet.adjustTmp.value[ADJUST_SYRINGE_Z];

	//					adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);	
						devSet.adjust_2nd.value[ADJUST_SYRINGE_Z] = devSet.adjust.value[ADJUST_SYRINGE_Z];
						checkSum = 0;
						for(i=0 ; i<6 ; i++) {
							checkSum = checkSum ^ devSet.adjust.value[i];
						}
						devSet.adjust_2nd.saveFlag[0] = checkSum;

for(i=0;i<6;i++) {
dp("===========================\n devSet.adjust.saveFlag = %d devSet.adjust.value = %d\n",devSet.adjust.saveFlag[i],devSet.adjust.value[i]);
}
						
//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveFlag[0], 12);
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);
		ackPacket = NO;
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);																		
						initCoordinate();		
						break;
					case ADJUST_VALVE_POS:					
						devSet.adjust.saveFlag[ADJUST_VALVE_X] = DATA_SAVE_FLAG;
						
						devSet.adjustOld.value[ADJUST_VALVE_X] = devSet.adjust.value[ADJUST_VALVE_X] = devSet.adjustTmp.value[ADJUST_VALVE_X];

//						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);								

						devSet.adjust_2nd.value[ADJUST_VALVE_X] = devSet.adjust.value[ADJUST_VALVE_X];
						checkSum = 0;
						for(i=0 ; i<6 ; i++) {
							checkSum = checkSum ^ devSet.adjust.value[i];
						}
						devSet.adjust_2nd.saveFlag[0] = checkSum;
						
//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveFlag[0], 12);
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);
		ackPacket = NO;
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);																
						initCoordinate();				
						break;
					case ADJUST_INJECTOR_POS:				
						devSet.adjust.saveFlag[ADJUST_INJECTOR_X] = DATA_SAVE_FLAG;
						
						devSet.adjustOld.value[ADJUST_INJECTOR_X] = devSet.adjust.value[ADJUST_INJECTOR_X] = devSet.adjustTmp.value[ADJUST_INJECTOR_X];;

//						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);		

						devSet.adjust_2nd.value[ADJUST_INJECTOR_X] = devSet.adjust.value[ADJUST_INJECTOR_X];						
						checkSum = 0;
						for(i=0 ; i<6 ; i++) {
							checkSum = checkSum ^ devSet.adjust.value[i];
						}
						devSet.adjust_2nd.saveFlag[0] = checkSum;						
						
//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveFlag[0], 12);						
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);	
		ackPacket = NO;
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);																
						initCoordinate();	
						break;
					case ADJUST_WASHING_PORT:				
			    	devSet.adjust.saveWash = DATA_SAVE_FLAG;

						devSet.adjustOld.wash_x = devSet.adjust.wash_x = devSet.adjustTmp.wash_x;
						devSet.adjustOld.wash_y = devSet.adjust.wash_y = devSet.adjustTmp.wash_y;

//						adjustDataSaveEeprom(&devSet.adjust.saveWash, 3);
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);		

						devSet.adjust_2nd.wash_x = devSet.adjust.wash_x;
						devSet.adjust_2nd.wash_y = devSet.adjust.wash_y;

						checkSum = devSet.adjust.wash_x ^ devSet.adjust.wash_y;
						devSet.adjust_2nd.saveWash = checkSum;						
						
//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveWash, 3);						
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);	
		ackPacket = NO;
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);														
						initCoordinate();
						break;
					case ADJUST_WASTE_PORT:					
			    	devSet.adjust.saveWaste = DATA_SAVE_FLAG;

						devSet.adjustOld.waste_x = devSet.adjust.waste_x = devSet.adjustTmp.waste_x;
						devSet.adjustOld.waste_y = devSet.adjust.waste_y = devSet.adjustTmp.waste_y;

//						adjustDataSaveEeprom(&devSet.adjust.saveWaste, 3);
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);		
						devSet.adjust_2nd.waste_x = devSet.adjust.waste_x;
						devSet.adjust_2nd.waste_y = devSet.adjust.waste_y;
						
						checkSum = devSet.adjust.waste_x ^ devSet.adjust.waste_y;
						devSet.adjust_2nd.saveWaste = checkSum;
			
//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveWaste, 3);
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);		
		ackPacket = NO;
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);														
						initCoordinate();						
						break;
						
					case ADJUST_VIAL_LEFT:
						devSet.adjust.saveVL[sysConfig.trayLeft] = DATA_SAVE_FLAG;

						devSet.adjustOld.leftVial_x[sysConfig.trayLeft] = devSet.adjust.leftVial_x[sysConfig.trayLeft] = devSet.adjustTmp.leftVial_x[sysConfig.trayLeft];
				    devSet.adjustOld.leftVial_y[sysConfig.trayLeft] = devSet.adjust.leftVial_y[sysConfig.trayLeft] = devSet.adjustTmp.leftVial_y[sysConfig.trayLeft];

//						adjustDataSaveEeprom(&devSet.adjust.saveVL, 9);
						devSet.adjust_2nd.leftVial_x[sysConfig.trayLeft] = devSet.adjust.leftVial_x[sysConfig.trayLeft];
				    devSet.adjust_2nd.leftVial_y[sysConfig.trayLeft] = devSet.adjust.leftVial_y[sysConfig.trayLeft];

						checkSum = devSet.adjust.leftVial_x[0] ^ devSet.adjust.leftVial_y[0] 
							^ devSet.adjust.leftVial_x[1] ^ devSet.adjust.leftVial_y[1]
							^devSet.adjust.leftVial_x[2] ^ devSet.adjust.leftVial_y[2];
						
//						devSet.adjust_2nd.saveVL[sysConfig.trayLeft] = checkSum;
						devSet.adjust_2nd.saveVL[0] = checkSum;

//						adjustDataSaveEeprom_2nd(&devSet.adjust.saveVL, 9);

// vial left를 설정하면 ADJUST_VIAL_HEIGHT_XY를 다시하도록 한다.
						devSet.adjust.saveVialHeight_xy = 0;
						devSet.adjust_2nd.saveVialHeight_xy = 0;
						
				    devSet.adjustOld.vialHeight_x = devSet.adjust.vialHeight_x = devSet.adjust_2nd.vialHeight_x = 0;
				    devSet.adjustOld.vialHeight_y = devSet.adjust.vialHeight_y = devSet.adjust_2nd.vialHeight_y = 0;

//						adjustDataSaveEeprom(&devSet.adjust.saveVialHeight_xy, 3);
//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveVialHeight_xy, 3);
					
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);			
		ackPacket = NO;
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);								
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);														
		
						initCoordinate();				
						break;

					case ADJUST_VIAL_RIGHT:
						devSet.adjust.saveVR[sysConfig.trayRight] = DATA_SAVE_FLAG;	
						
				    devSet.adjustOld.rightVial_x[sysConfig.trayRight] = devSet.adjust.rightVial_x[sysConfig.trayRight] = devSet.adjustTmp.rightVial_x[sysConfig.trayRight];
				    devSet.adjustOld.rightVial_y[sysConfig.trayRight] = devSet.adjust.rightVial_y[sysConfig.trayRight] = devSet.adjustTmp.rightVial_y[sysConfig.trayRight];
						
//						adjustDataSaveEeprom(&devSet.adjust.saveVR, 9);

				    devSet.adjust_2nd.rightVial_x[sysConfig.trayRight] = devSet.adjust.rightVial_x[sysConfig.trayRight];
				    devSet.adjust_2nd.rightVial_y[sysConfig.trayRight] = devSet.adjust.rightVial_y[sysConfig.trayRight];

						checkSum = devSet.adjust.rightVial_x[0] ^ devSet.adjust.rightVial_y[0] 
							^ devSet.adjust.rightVial_x[1] ^ devSet.adjust.rightVial_y[1]
							^devSet.adjust.rightVial_x[2] ^ devSet.adjust.rightVial_y[2];
						
						devSet.adjust_2nd.saveVR[0] = checkSum;

//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveVR, 9);
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);			
		ackPacket = NO;
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);								
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);														
		
						initCoordinate();						
						break;
					case ADJUST_VIAL_HEIGHT:
						devSet.adjust.saveVialHeight = DATA_SAVE_FLAG;							
				    devSet.adjustOld.vialHeight = devSet.adjust.vialHeight = devSet.adjustTmp.vialHeight;

//						adjustDataSaveEeprom(&devSet.adjust.saveVialHeight, 2);

				    devSet.adjust_2nd.vialHeight = devSet.adjust.vialHeight;
						devSet.adjust_2nd.saveVialHeight = devSet.adjust.vialHeight;

//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveVialHeight, 2);
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);			
		ackPacket = NO;
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);								
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);														
		
						initCoordinate();						
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						devSet.adjust.saveVialHeight_xy = DATA_SAVE_FLAG;
				    devSet.adjustOld.vialHeight_x = devSet.adjust.vialHeight_x = devSet.adjustTmp.vialHeight_x;
				    devSet.adjustOld.vialHeight_y = devSet.adjust.vialHeight_y = devSet.adjustTmp.vialHeight_y;
						
//						adjustDataSaveEeprom(&devSet.adjust.saveVialHeight_xy, 3);

				    devSet.adjust_2nd.vialHeight_x = devSet.adjustTmp.vialHeight_x;
				    devSet.adjust_2nd.vialHeight_y = devSet.adjustTmp.vialHeight_y;

						checkSum = devSet.adjust.vialHeight_x ^ devSet.adjust.vialHeight_y;
						devSet.adjust_2nd.saveVialHeight_xy = checkSum;

//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveVialHeight_xy, 3);			
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);			
		ackPacket = NO;
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);								
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);														
		
						initCoordinate();
						break;
 					case ADJUST_MICRO_PUMP_VOLTAGE:
dp("ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss-save\n"); 					
						devSet.adjust.saveMicroPump = DATA_SAVE_FLAG;

				    devSet.adjustOld.microPumpVolt = devSet.adjust.microPumpVolt = devSet.adjustTmp.microPumpVolt;
				    devSet.adjust_2nd.microPumpVolt = devSet.adjustTmp.microPumpVolt;
						
						checkSum = devSet.adjust.microPumpVolt;
						devSet.adjust_2nd.microPumpVolt = checkSum;

		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);			
		ackPacket = NO;
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);								
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);														

						devSet.microPumpPwmCount = devSet.adjust.microPumpVolt * MICRO_PUMP_UNIT_PWM + MICRO_PUMP_PWM_MIN;
	 					break;
/*
						
						adjustDataSaveEeprom(&devSet.adjust.saveMicroPump, 2);






						devSet.adjust.saveVialHeight_xy = DATA_SAVE_FLAG;
				    devSet.adjustOld.vialHeight_x = devSet.adjust.vialHeight_x = devSet.adjustTmp.vialHeight_x;
				    devSet.adjustOld.vialHeight_y = devSet.adjust.vialHeight_y = devSet.adjustTmp.vialHeight_y;
						
//						adjustDataSaveEeprom(&devSet.adjust.saveVialHeight_xy, 3);

				    devSet.adjust_2nd.vialHeight_x = devSet.adjustTmp.vialHeight_x;
				    devSet.adjust_2nd.vialHeight_y = devSet.adjustTmp.vialHeight_y;

						checkSum = devSet.adjust.vialHeight_x ^ devSet.adjust.vialHeight_y;
						devSet.adjust_2nd.saveVialHeight_xy = checkSum;

//						adjustDataSaveEeprom_2nd(&devSet.adjust_2nd.saveVialHeight_xy, 3);			
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);			
		ackPacket = NO;
		eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);								
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);														













						devSet.microPumpPwmCount = devSet.adjust.microPumpVolt * MICRO_PUMP_UNIT_PWM + MICRO_PUMP_PWM_MIN;
	 					break;						
*/						

						
				}
				
				// 저장후 원복
//	    	devSet.adjust.value[MOTOR_Z] += STEP_MORE_ADJUST_VALUE_Z;
	      break;
	    case ADJUST_INIT_VALUE:
				
				for(int i=0;i<6;i++) {
					devSet.adjust.saveFlag[i] = 0;
					devSet.adjust.value[i] = 0;
				}
				for(int i=0 ; i<3; i++) {
					devSet.adjust.saveVL[i] = 0;		
					devSet.adjust.leftVial_x[i] = 0;
					devSet.adjust.leftVial_y[i] = 0;

		    	devSet.adjust.saveVR[i] = 0;													
					devSet.adjust.rightVial_x[i] = 0;
					devSet.adjust.rightVial_y[i] = 0;				
				}

	    	devSet.adjust.saveWash = 0;
	    	devSet.adjust.saveWaste = 0;

				devSet.adjust.wash_x = 0;
				devSet.adjust.wash_y = 0;
				devSet.adjust.waste_x = 0;
				devSet.adjust.waste_y = 0;
//				eepromWrite_Data(HOME_ADJUST_ADDR_X_SAVE,12,devSet.adjust.saveFlag);
//				eepromWrite_Data(HOME_ADJUST_ADDR_X_SAVE,sizeof(devSet.adjust),devSet.adjust.saveFlag);

				devSet.adjust.saveVialHeight = 0;							
				devSet.adjust.vialHeight = 0;

#if VIAL_POS_MODIFY
				devSet.adjust.saveVialHeight_xy = 0;							
				devSet.adjust.vialHeight_x = 0;
				devSet.adjust.vialHeight_y = 0;				
#endif
				eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);

				memcpy(&devSet.adjust_2nd, &devSet.adjust, sizeof(devSet.adjust));

				eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust_2nd) + 1,devSet.adjust_2nd.saveFlag);

				ackPacket = adjustInitPosition(part);
diprintf("=====ADJUST_INIT_VALUE \n");	    				
	      break;

	    case ADJUST_INIT_POS:	// adjust 할 수 있도록 준비 및
dp("=====ADJUST_INIT_POS \n");	    
dp("LcasState.state = %d devSet.adjust.act=%d %d %d \n",LcasState.state,devSet.adjustAct,devSet.adjustCurMotor,devSet.adjustOldMotor);
memcpy(&devSet.adjustTmp, &devSet.adjust, sizeof(devSet.adjust));
//diprintf("memcpy(&devSet.adjustTmp=%d\n",devSet.adjustTmp.saveFlag[0]);

				ackPacket = adjustInitPosition(part);
#if TEST_EEPROM_VERIFY
 da = (unsigned char*)(&devSet.adjust.saveFlag);
 da2 = (unsigned char*)(&devSet.adjust_2nd.saveFlag); 
diprintf("case ADJUST_INIT_POS:\n");
		for(i=0;i<45;i++) {
			if(i%6 == 0) diprintf("\n");
diprintf("[%d :%x :%x]\t",i,da[i],da2[i]);
		}
#endif
	      break;
	    case ADJUST_QUIT:
				adjustQuit();
#if TEST_EEPROM_VERIFY
 da = (unsigned char*)(&devSet.adjust.saveFlag);
 da2 = (unsigned char*)(&devSet.adjust_2nd.saveFlag); 

diprintf("case ADJUST_QUIT:\n");
		for(i=0;i<45;i++) {
			if(i%6 == 0) diprintf("\n");
diprintf("[%d :%x :%x]\t",i,da[i],da2[i]);
		}
#endif

	      break;		
//	    case ADJUST_REQUEST:		// 현재 저장된 데이터 전송 - ADJUST_DEV_DATA_t 
//	      break;
			case ADJUST_INIT_POS_END:
//	[ADJUST_INIT_POS]명령을 받고 초기화가 완료되면 PC에 전송한다.
			default:
				errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
				ackPacket = NO;
				break;
		}

	if(ackPacket == YES) {
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjustTmp, devSet.adjustTmp);
dp("devSet.adjustTmp.value[0]=%d\n",devSet.adjustTmp.value[0]);
	}

	return 0;
}

#else
int net_ServiceAdjust()
{
	unsigned char part = service.subCommand1;
	unsigned char act = service.subCommand2;
	unsigned char ackPacket = YES;	// adjust 패킷을 응답해야한다.
	static int initPos = NO;

dp("net_ServiceAdjust cmd=%d sub1=%d sub2=%d\n",service.command, service.subCommand1, service.subCommand2);				
//	devSet.adjustCurMotor = service.subCommand1;
//	ackPacket = net_AdjustFunc(devSet.adjustCurMotor, act);

		switch(act) {
			case ADJUST_LEFT:	// CCW
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X];
						break;
					case ADJUST_NEEDLE_HEIGHT:						
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
//						++devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X];
						break;
					case ADJUST_SYRINGE:						break;
					case ADJUST_VALVE_POS:
						if(devSet.adjust.value[ADJUST_VALVE_X] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goVal(ADJUST_STEP_UNIT_VAL, 0,ADJUST_SPEED_VAL, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.value[ADJUST_VALVE_X];
						break;
					case ADJUST_INJECTOR_POS:
						if(devSet.adjust.value[ADJUST_INJECTOR_X] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goInj(ADJUST_STEP_UNIT_INJ, 0,ADJUST_SPEED_INJ, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.value[ADJUST_INJECTOR_X];
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjust.wash_x >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.wash_x;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjust.waste_x >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.waste_x;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjust.leftVial_x[sysConfig.trayLeft] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.leftVial_x[sysConfig.trayLeft];
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjust.rightVial_x[sysConfig.trayRight] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.rightVial_x[sysConfig.trayRight];
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjust.vialHeight_x >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.vialHeight_x;
						break;
#endif						
				}
	      break;
	    case ADJUST_LEFT_X2:	// CCW X2
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X , 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] += ADJUST_STEP_UNIT_SCALE_X;			
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X , 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_SYRINGE:						break;						
					case ADJUST_VALVE_POS:
						if(devSet.adjust.value[ADJUST_VALVE_X] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goVal(ADJUST_STEP_UNIT_VAL * ADJUST_STEP_UNIT_SCALE_VAL, 0,ADJUST_SPEED_VAL, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_VALVE_X] += ADJUST_STEP_UNIT_SCALE_VAL;
						break;
					case ADJUST_INJECTOR_POS:
						if(devSet.adjust.value[ADJUST_INJECTOR_X] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goInj(ADJUST_STEP_UNIT_INJ * ADJUST_STEP_UNIT_SCALE_INJ, 0,ADJUST_SPEED_INJ, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_INJECTOR_X] += ADJUST_STEP_UNIT_SCALE_INJ;
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjust.wash_x >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.wash_x += ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjust.waste_x >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.waste_x += ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjust.leftVial_x[sysConfig.trayLeft] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.leftVial_x[sysConfig.trayLeft] += ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjust.rightVial_x[sysConfig.trayRight] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.rightVial_x[sysConfig.trayRight] += ADJUST_STEP_UNIT_SCALE_X;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjust.vialHeight_x >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(ADJUST_STEP_UNIT_X * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.vialHeight_x += ADJUST_STEP_UNIT_SCALE_X;
						break;
#endif						
				}
	      break;
	    case ADJUST_RIGHT:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);			
						--devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X];
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);			
						break;
					case ADJUST_SYRINGE:						break;
					case ADJUST_VALVE_POS:
						if(devSet.adjust.value[ADJUST_VALVE_X] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goVal(-ADJUST_STEP_UNIT_VAL, 0,ADJUST_SPEED_VAL, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.value[ADJUST_VALVE_X];
						break;
					case ADJUST_INJECTOR_POS:
						if(devSet.adjust.value[ADJUST_INJECTOR_X] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goInj(-ADJUST_STEP_UNIT_INJ, 0,ADJUST_SPEED_INJ, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.value[ADJUST_INJECTOR_X];
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjust.wash_x <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);			
						--devSet.adjust.wash_x;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjust.waste_x <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);			
						--devSet.adjust.waste_x;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjust.leftVial_x[sysConfig.trayLeft] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.leftVial_x[sysConfig.trayLeft];
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjust.rightVial_x[sysConfig.trayRight] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.rightVial_x[sysConfig.trayRight];
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goX(-ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjust.vialHeight_x <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX(-ADJUST_STEP_UNIT_X, 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.vialHeight_x;
						break;
#endif						
				}
	      break;				
	    case ADJUST_RIGHT_X2:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X , 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] -= ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X , 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_SYRINGE:						break;						
					case ADJUST_VALVE_POS:
						if(devSet.adjust.value[ADJUST_VALVE_X] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goVal((-ADJUST_STEP_UNIT_VAL) * ADJUST_STEP_UNIT_SCALE_VAL, 0,ADJUST_SPEED_VAL, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_VALVE_X] -= ADJUST_STEP_UNIT_SCALE_VAL;
						break;
					case ADJUST_INJECTOR_POS:
						if(devSet.adjust.value[ADJUST_INJECTOR_X] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goInj((-ADJUST_STEP_UNIT_INJ) * ADJUST_STEP_UNIT_SCALE_INJ, 0,ADJUST_SPEED_INJ, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_INJECTOR_X] -= ADJUST_STEP_UNIT_SCALE_INJ;
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjust.wash_x <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);			
						devSet.adjust.wash_x -= ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjust.waste_x <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);			
						devSet.adjust.waste_x -= ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjust.leftVial_x[sysConfig.trayLeft] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.leftVial_x[sysConfig.trayLeft] -= ADJUST_STEP_UNIT_SCALE_X;
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjust.rightVial_x[sysConfig.trayRight] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.rightVial_x[sysConfig.trayRight] -= ADJUST_STEP_UNIT_SCALE_X;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjust.vialHeight_x <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goX((-ADJUST_STEP_UNIT_X) * ADJUST_STEP_UNIT_SCALE_X, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.vialHeight_x -= ADJUST_STEP_UNIT_SCALE_X;						
						break;
#endif							
				}
	      break;
	    case ADJUST_UP:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						break;
					case ADJUST_NEEDLE_HEIGHT:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						--devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z];
						break;
					case ADJUST_SYRINGE:
						if(devSet.adjust.value[ADJUST_SYRINGE_Z] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goSyr(-ADJUST_STEP_UNIT_SYR, 0, ADJUST_SPEED_SYL, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.value[ADJUST_SYRINGE_Z];
						break;						
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_VIAL_HEIGHT:
						if(devSet.adjust.vialHeight <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						--devSet.adjust.vialHeight;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_WASHING_PORT:
					case ADJUST_WASTE_PORT:
					case ADJUST_VIAL_LEFT:
					case ADJUST_VIAL_RIGHT:
					case ADJUST_VIAL_HEIGHT_XY:	// 값은 변화 없고 움직임 만 
						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						break;
#endif
					case ADJUST_MICRO_PUMP_VOLTAGE:
						if(devSet.adjust.microPumpVolt >= 25) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
//						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						++devSet.adjust.microPumpVolt;
						break;
				}	    
				break;
	    case ADJUST_UP_X2:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						goZ((-ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_NEEDLE_HEIGHT:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goZ((-ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] -= ADJUST_STEP_UNIT_SCALE_Z;
						break;
					case ADJUST_SYRINGE:
						if(devSet.adjust.value[ADJUST_SYRINGE_Z] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goSyr((-ADJUST_STEP_UNIT_SYR) * ADJUST_STEP_UNIT_SCALE_SYR, 0, ADJUST_SPEED_SYL, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_SYRINGE_Z] -= ADJUST_STEP_UNIT_SCALE_SYR;
						break;						
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_VIAL_HEIGHT:
						if(devSet.adjust.vialHeight <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goZ((-ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						devSet.adjust.vialHeight -= ADJUST_STEP_UNIT_SCALE_Z;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_WASHING_PORT:
					case ADJUST_WASTE_PORT:
					case ADJUST_VIAL_LEFT:
					case ADJUST_VIAL_RIGHT:
					case ADJUST_VIAL_HEIGHT_XY:	// 값은 변화 없고 움직임 만 
						goZ((-ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);	
						break;
#endif					
					case ADJUST_MICRO_PUMP_VOLTAGE:
						if(devSet.adjust.microPumpVolt >= 25) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						else if(devSet.adjust.microPumpVolt == 24) {	// 최대 10 volt ==> 25  최소 5volt ==> 0						
							devSet.adjust.microPumpVolt = 25;
						}
						else {
//						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);										
							devSet.adjust.microPumpVolt += 2;
						}
						break;
				}	    
	      break;
	    case ADJUST_DOWN:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						goZ((ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);									
						break;
					case ADJUST_NEEDLE_HEIGHT:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goZ((ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						++devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z];
						break;
					case ADJUST_SYRINGE:
						if(devSet.adjust.value[ADJUST_SYRINGE_Z] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}						
						goSyr((ADJUST_STEP_UNIT_SYR), 0, ADJUST_SPEED_SYL, ACT_MOVE_STEP_RELATE);			
						++devSet.adjust.value[ADJUST_SYRINGE_Z];
						break;						
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_VIAL_HEIGHT:
						if(devSet.adjust.vialHeight >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goZ((ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						++devSet.adjust.vialHeight;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_WASHING_PORT:
					case ADJUST_WASTE_PORT:
					case ADJUST_VIAL_LEFT:
					case ADJUST_VIAL_RIGHT:
					case ADJUST_VIAL_HEIGHT_XY:	// 값은 변화 없고 움직임 만 
						goZ((ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);			
						break;
#endif					
					case ADJUST_MICRO_PUMP_VOLTAGE:
						if(devSet.adjust.microPumpVolt <= 0) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						else {
//						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);										
							--devSet.adjust.microPumpVolt;
						}						
						break;

				}	  
	      break;
	    case ADJUST_DOWN_X2:
				switch(part) {
					case ADJUST_NEEDLE_XY:
						goZ((ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_NEEDLE_HEIGHT:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goZ((ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] += ADJUST_STEP_UNIT_SCALE_Z;
						break;
					case ADJUST_SYRINGE:
						if(devSet.adjust.value[ADJUST_SYRINGE_Z] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goSyr((ADJUST_STEP_UNIT_SYR) * ADJUST_STEP_UNIT_SCALE_SYR, 0, ADJUST_SPEED_SYL, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_SYRINGE_Z] += ADJUST_STEP_UNIT_SCALE_SYR;
						break;						
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_VIAL_HEIGHT:
						if(devSet.adjust.vialHeight >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goZ((ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);
						devSet.adjust.vialHeight += ADJUST_STEP_UNIT_SCALE_Z;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_WASHING_PORT:
					case ADJUST_WASTE_PORT:
					case ADJUST_VIAL_LEFT:
					case ADJUST_VIAL_RIGHT:
					case ADJUST_VIAL_HEIGHT_XY:	// 값은 변화 없고 움직임 만 
						goZ((ADJUST_STEP_UNIT_Z) * ADJUST_STEP_UNIT_SCALE_Z, ACT_MOVE_STEP_RELATE);	
						break;
#endif							
					case ADJUST_MICRO_PUMP_VOLTAGE:
						if(devSet.adjust.microPumpVolt <= 0) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						else if(devSet.adjust.microPumpVolt == 1) {	// 최대 10 volt ==> 25  최소 5volt ==> 0
							devSet.adjust.microPumpVolt = 0;
						}
						else {
//						goZ((-ADJUST_STEP_UNIT_Z), ACT_MOVE_STEP_RELATE);										
							devSet.adjust.microPumpVolt -= 2;
						}						
						break;
				}	  
	      break;
	    case ADJUST_BACK:		// Back
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);			
						--devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y];
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);			
						break;
					case ADJUST_SYRINGE:
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjust.wash_y <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.wash_y;
						break;						
					case ADJUST_WASTE_PORT:
						if(devSet.adjust.waste_y <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.waste_y;
						break;						
					case ADJUST_VIAL_LEFT:
						if(devSet.adjust.leftVial_y[sysConfig.trayLeft] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.leftVial_y[sysConfig.trayLeft];
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjust.rightVial_y[sysConfig.trayRight] <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.rightVial_y[sysConfig.trayRight];
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjust.vialHeight_y <= -99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						--devSet.adjust.vialHeight_y;
						break;
#endif							
				}	    
				break;
	    case ADJUST_BACK_X2:	// Back X2
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y , 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y , 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_SYRINGE:
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjust.wash_y <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.wash_y -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjust.waste_y <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.waste_y -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjust.leftVial_y[sysConfig.trayLeft] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.leftVial_y[sysConfig.trayLeft] -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjust.rightVial_y[sysConfig.trayRight] <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.rightVial_y[sysConfig.trayRight] -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjust.vialHeight_y <= -98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((-ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.vialHeight_y -= ADJUST_STEP_UNIT_SCALE_Y;
						break;
#endif							
				}	    
	      break;
	    case ADJUST_FRONT:	// Front
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);			
						++devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y];
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goY((ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);			
						break;
					case ADJUST_SYRINGE:
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjust.wash_y >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.wash_y;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjust.waste_y >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.waste_y;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjust.leftVial_y[sysConfig.trayLeft] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.leftVial_y[sysConfig.trayLeft];
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjust.rightVial_y[sysConfig.trayRight] >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y, 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.rightVial_y[sysConfig.trayRight];
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goY((ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjust.vialHeight_y >= 99) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((ADJUST_STEP_UNIT_Y), 0, ACT_MOVE_STEP_RELATE);
						++devSet.adjust.vialHeight_y;
						break;
#endif							
				}	  
	      break;
	    case ADJUST_FRONT_X2:	// Front X2
				switch(part) {
					case ADJUST_NEEDLE_XY:
						if(devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] += ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_NEEDLE_HEIGHT:
						goY((ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_SYRINGE:
					case ADJUST_VALVE_POS:
					case ADJUST_INJECTOR_POS:
						break;
					case ADJUST_WASHING_PORT:
						if(devSet.adjust.wash_y >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.wash_y += ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_WASTE_PORT:
						if(devSet.adjust.waste_y >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.waste_y += ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_VIAL_LEFT:
						if(devSet.adjust.leftVial_y[sysConfig.trayLeft] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.leftVial_y[sysConfig.trayLeft] += ADJUST_STEP_UNIT_SCALE_Y;
						break;
					case ADJUST_VIAL_RIGHT:
						if(devSet.adjust.rightVial_y[sysConfig.trayRight] >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY(ADJUST_STEP_UNIT_Y * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.rightVial_y[sysConfig.trayRight] += ADJUST_STEP_UNIT_SCALE_Y;
						break;
#if VIAL_POS_MODIFY
					case ADJUST_VIAL_HEIGHT:
						goY((ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						if(devSet.adjust.vialHeight_y >= 98) {
							send_SelfMessage(SELF_COMMAND_REJECT,0);
							break;
						}
						goY((ADJUST_STEP_UNIT_Y) * ADJUST_STEP_UNIT_SCALE_Y, 0, ACT_MOVE_STEP_RELATE);
						devSet.adjust.vialHeight_y += ADJUST_STEP_UNIT_SCALE_Y;
						break;
#endif							
				}	  
	      break;
	    case ADJUST_SAVE:
 				switch(part) {
					case ADJUST_NEEDLE_XY:
			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_X] = DATA_SAVE_FLAG;
			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_Y] = DATA_SAVE_FLAG;

				    devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_X] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X];
				    devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_Y] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y];

#if 0
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 2);
						adjustDataSaveEeprom(&devSet.adjust.value[0], 2);
#else					
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
//						adjustDataSaveEeprom(&devSet.adjust.value[0], 2);
#endif

						initCoordinate();
						break;
					case ADJUST_NEEDLE_HEIGHT:
			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_X] = DATA_SAVE_FLAG;
			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_Y] = DATA_SAVE_FLAG;
			    	devSet.adjust.saveFlag[ADJUST_INJ_PORT_VALUE_Z] = DATA_SAVE_FLAG;

				    devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_X] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X];
				    devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_Y] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y];
						devSet.adjustOld.value[ADJUST_INJ_PORT_VALUE_Z] = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z];
#if 0
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 3);
						adjustDataSaveEeprom(&devSet.adjust.value[0], 3);
#else					
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
#endif
						initCoordinate();
						break;
					case ADJUST_SYRINGE:
			    	devSet.adjust.saveFlag[ADJUST_SYRINGE_Z] = DATA_SAVE_FLAG;
						devSet.adjustOld.value[ADJUST_SYRINGE_Z] = devSet.adjust.value[ADJUST_SYRINGE_Z];
#if 0
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[ADJUST_SYRINGE_Z], 1);
						adjustDataSaveEeprom(&devSet.adjust.value[ADJUST_SYRINGE_Z], 1);
#else					
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
#endif
						initCoordinate();						
						break;
					case ADJUST_VALVE_POS:					
						devSet.adjust.saveFlag[ADJUST_VALVE_X] = DATA_SAVE_FLAG;
						devSet.adjustOld.value[ADJUST_VALVE_X] = devSet.adjust.value[ADJUST_VALVE_X];

#if 0
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[ADJUST_VALVE_X], 1);
						adjustDataSaveEeprom(&devSet.adjust.value[ADJUST_VALVE_X], 1);
#else					
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
#endif						
						initCoordinate();						
						break;
					case ADJUST_INJECTOR_POS:				
						devSet.adjust.saveFlag[ADJUST_INJECTOR_X] = DATA_SAVE_FLAG;
						devSet.adjustOld.value[ADJUST_INJECTOR_X] = devSet.adjust.value[ADJUST_INJECTOR_X];

#if 0
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[ADJUST_INJECTOR_X], 1);
						adjustDataSaveEeprom(&devSet.adjust.value[ADJUST_INJECTOR_X], 1);
#else					
						adjustDataSaveEeprom(&devSet.adjust.saveFlag[0], 12);
#endif
						initCoordinate();						
						break;
					case ADJUST_WASHING_PORT:				
			    	devSet.adjust.saveWash = DATA_SAVE_FLAG;
						devSet.adjustOld.wash_x = devSet.adjust.wash_x;
						devSet.adjustOld.wash_y = devSet.adjust.wash_y;

						adjustDataSaveEeprom(&devSet.adjust.saveWash, 3);
						initCoordinate();
						break;
					case ADJUST_WASTE_PORT:					
			    	devSet.adjust.saveWaste = DATA_SAVE_FLAG;
						devSet.adjustOld.waste_x = devSet.adjust.waste_x;
						devSet.adjustOld.waste_y = devSet.adjust.waste_y;
						
						adjustDataSaveEeprom(&devSet.adjust.saveWaste, 3);
						initCoordinate();						
						break;
					case ADJUST_VIAL_LEFT:
						devSet.adjust.saveVL[sysConfig.trayLeft] = DATA_SAVE_FLAG;
						devSet.adjustOld.leftVial_x[sysConfig.trayLeft] = devSet.adjust.leftVial_x[sysConfig.trayLeft];
				    devSet.adjustOld.leftVial_y[sysConfig.trayLeft] = devSet.adjust.leftVial_y[sysConfig.trayLeft];
						
						adjustDataSaveEeprom(&devSet.adjust.saveVL, 9);
// vial left를 설정하면 ADJUST_VIAL_HEIGHT_XY를 다시하도록 한다.
						devSet.adjust.saveVialHeight_xy = 0;
				    devSet.adjustOld.vialHeight_x = devSet.adjust.vialHeight_x;
				    devSet.adjustOld.vialHeight_y = devSet.adjust.vialHeight_y;

				    devSet.adjust.vialHeight_x = 0;
				    devSet.adjust.vialHeight_y = 0;
						
						adjustDataSaveEeprom(&devSet.adjust.saveVialHeight_xy, 3);

						initCoordinate();				
						break;
					case ADJUST_VIAL_RIGHT:
						devSet.adjust.saveVR[sysConfig.trayRight] = DATA_SAVE_FLAG;	
				    devSet.adjustOld.rightVial_x[sysConfig.trayRight] = devSet.adjust.rightVial_x[sysConfig.trayRight];
				    devSet.adjustOld.rightVial_y[sysConfig.trayRight] = devSet.adjust.rightVial_y[sysConfig.trayRight];
						
						adjustDataSaveEeprom(&devSet.adjust.saveVR, 9);
						initCoordinate();						
						break;
					case ADJUST_VIAL_HEIGHT:
						devSet.adjust.saveVialHeight = DATA_SAVE_FLAG;							
				    devSet.adjustOld.vialHeight = devSet.adjust.vialHeight;

						adjustDataSaveEeprom(&devSet.adjust.saveVialHeight, 2);
						initCoordinate();						
						break;
					case ADJUST_VIAL_HEIGHT_XY:
						devSet.adjust.saveVialHeight_xy = DATA_SAVE_FLAG;
				    devSet.adjustOld.vialHeight_x = devSet.adjust.vialHeight_x;
				    devSet.adjustOld.vialHeight_y = devSet.adjust.vialHeight_y;
						
						adjustDataSaveEeprom(&devSet.adjust.saveVialHeight_xy, 3);
						initCoordinate();						
						break;
 					case ADJUST_MICRO_PUMP_VOLTAGE:
dp("ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss-save\n"); 					
						devSet.adjust.saveMicroPump = DATA_SAVE_FLAG;
				    devSet.adjustOld.microPumpVolt = devSet.adjust.microPumpVolt;
						
						adjustDataSaveEeprom(&devSet.adjust.saveMicroPump, 2);

						devSet.microPumpPwmCount = devSet.adjust.microPumpVolt * MICRO_PUMP_UNIT_PWM + MICRO_PUMP_PWM_MIN;
	 					break;
				}
				
				// 저장후 원복
//	    	devSet.adjust.value[MOTOR_Z] += STEP_MORE_ADJUST_VALUE_Z;
	      break;
	    case ADJUST_INIT_VALUE:
				for(int i=0;i<6;i++) {
					devSet.adjust.saveFlag[i] = 0;
					devSet.adjust.value[i] = 0;
				}
				for(int i=0 ; i<3; i++) {
					devSet.adjust.saveVL[i] = 0;		
					
					devSet.adjust.leftVial_x[i] = 0;
					devSet.adjust.leftVial_y[i] = 0;

		    	devSet.adjust.saveVR[i] = 0;													
					devSet.adjust.rightVial_x[i] = 0;
					devSet.adjust.rightVial_y[i] = 0;				
				}

	    	devSet.adjust.saveWash = 0;
	    	devSet.adjust.saveWaste = 0;

				devSet.adjust.wash_x = 0;
				devSet.adjust.wash_y = 0;
				devSet.adjust.waste_x = 0;
				devSet.adjust.waste_y = 0;
//				eepromWrite_Data(HOME_ADJUST_ADDR_X_SAVE,12,devSet.adjust.saveFlag);
//				eepromWrite_Data(HOME_ADJUST_ADDR_X_SAVE,sizeof(devSet.adjust),devSet.adjust.saveFlag);

				devSet.adjust.saveVialHeight = 0;							
				devSet.adjust.vialHeight = 0;

#if VIAL_POS_MODIFY
				devSet.adjust.saveVialHeight_xy = 0;							
				devSet.adjust.vialHeight_x = 0;
				devSet.adjust.vialHeight_y = 0;				
#endif
				eepromWrite_Data_Verify(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);

				ackPacket = adjustInitPosition(part);
	      break;

	    case ADJUST_INIT_POS:	// adjust 할 수 있도록 준비 및
dp("LcasState.state = %d devSet.adjust.act=%d %d %d \n",LcasState.state,devSet.adjustAct,devSet.adjustCurMotor,devSet.adjustOldMotor);
				ackPacket = adjustInitPosition(part);
	      break;
	    case ADJUST_QUIT:
				adjustQuit();
	      break;		
//	    case ADJUST_REQUEST:		// 현재 저장된 데이터 전송 - ADJUST_DEV_DATA_t 
//	      break;
			case ADJUST_INIT_POS_END:
//	[ADJUST_INIT_POS]명령을 받고 초기화가 완료되면 PC에 전송한다.
			default:
				errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
				ackPacket = NO;
				break;
		}

	if(ackPacket == YES) {
		SEND_LCAS_PACKET(PACKCODE_LCAS_ADJUST_DATA, devSet.adjust, devSet.adjust);
dp("devSet.adjust.value[0]=%d\n",devSet.adjust.value[0]);
	}

	return 0;
}
#endif

int adjustInitPosition(unsigned char part)
{
	unsigned char ack = YES;

	if( devSet.adjustAct == ADJUST_ACT_NOT_READY) {	// 맨처음 Adjust init position 동작
#if	EEPROM_ADJUST_DATA_SAVE_FIX
	// 트레이가 없을
		if((part == ADJUST_VIAL_LEFT || part == ADJUST_VIAL_HEIGHT_XY) && sysConfig.trayLeft >= TRAY_NONE) {
			send_SelfMessage(SELF_COMMAND_REJECT,0);
			ack = NO;
			return ack;
		}

		if((part == ADJUST_VIAL_RIGHT) && sysConfig.trayRight >= TRAY_NONE) {
			send_SelfMessage(SELF_COMMAND_REJECT,0);
			ack = NO;
			return ack;
		}
#endif
dp("======================================================================adjustInitPosition  == ADJUST_ACT_NOT_READ\n");
		devState.homeFindEnd[MOTOR_X] = NO;
		devState.homeFindEnd[MOTOR_Y] = NO;
		devState.homeFindEnd[MOTOR_Z] = NO;
		devState.homeFindEnd[MOTOR_SYR] = NO;
		devState.homeFindEnd[MOTOR_VAL] = NO;
		devState.homeFindEnd[MOTOR_INJ] = NO;

    devState.actionMenu = AS_ACTION_ADJUST;
    devState.actionSubmenu = AS_ADJUST_HOME_POS;
		stateChange(STATE_DEVICE_ADJUST);

		devSet.adjustAct = ADJUST_ACT_INIT;
		devSet.adjustOldMotor = devSet.adjustCurMotor;
		devSet.adjustCurMotor = service.subCommand1;		//

		ack = YES;
//					ackPacket = NO;
//					SEND_ACK_LCAS_PACKET(pData);
	}
	else if( devSet.adjustAct == ADJUST_ACT_READY) {

#if	EEPROM_ADJUST_DATA_SAVE_FIX
	// 트레이가 없을
		if((part == ADJUST_VIAL_LEFT || part == ADJUST_VIAL_HEIGHT_XY) && sysConfig.trayLeft >= TRAY_NONE) {
			send_SelfMessage(SELF_COMMAND_REJECT,0);
			ack = NO;
			return ack;
		}

		if((part == ADJUST_VIAL_RIGHT) && sysConfig.trayRight >= TRAY_NONE) {
			send_SelfMessage(SELF_COMMAND_REJECT,0);
			ack = NO;
			return ack;
		}
#endif

		if(part == devSet.adjustCurMotor) {	// 같은 위치
dp("devSet.adjustAct = %d send_SelfMessage(SELF_COMMAND_REJECT,0)\n",devSet.adjustAct);
			send_SelfMessage(SELF_COMMAND_REJECT,0);
			ack = NO;
		}
		else {
	    devState.actionMenu = AS_ACTION_ADJUST;
	    devState.actionSubmenu = AS_ADJUST_MOTOR_INIT;
//						stateChange(STATE_DEVICE_ADJUST);ㅅ

			devSet.adjustAct = ADJUST_ACT_INIT;
			devSet.adjustOldMotor = devSet.adjustCurMotor;
			devSet.adjustCurMotor = service.subCommand1;		// part

			ack = YES;
dp("======devSet.adjustCurMotor = %d ======================ADJUST_ACT_READY========Y\n",devSet.adjustCurMotor);
		}
	}
	else {	// ADJUST_ACT_EXCHANGE or ADJUST_ACT_ADJUST  ???
dp("devSet.adjustAct = %d send_SelfMessage(SELF_COMMAND_REJECT,0)\n",devSet.adjustAct);
			send_SelfMessage(SELF_COMMAND_REJECT,0);
			ack = YES;
	}
	return ack;
}

int adjustQuit()
{
		// 저장 이전 값으로 되돌린다.
	for(int i=0 ; i < 6	; i++ ) {
    devSet.adjust.value[i] = devSet.adjustOld.value[i];
dp("devSet.adjust.save[%d]=0x%x\n",i,devSet.adjust.saveFlag[i]);
dp("xxdevSet.adjust.value[%d]=%d [old] %d\n",i,devSet.adjust.value[i],devSet.adjustOld.value[i]);
	}

  devSet.adjust.leftVial_x[sysConfig.trayLeft] = devSet.adjustOld.leftVial_x[sysConfig.trayLeft];
  devSet.adjust.leftVial_y[sysConfig.trayLeft] = devSet.adjustOld.leftVial_y[sysConfig.trayLeft];

  devSet.adjust.rightVial_x[sysConfig.trayRight] = devSet.adjustOld.rightVial_x[sysConfig.trayRight];
  devSet.adjust.rightVial_y[sysConfig.trayRight] = devSet.adjustOld.rightVial_y[sysConfig.trayRight];

	devSet.adjust.wash_x = devSet.adjustOld.wash_x;
	devSet.adjust.wash_y = devSet.adjustOld.wash_y;

	devSet.adjust.waste_x = devSet.adjustOld.waste_x;
	devSet.adjust.waste_y = devSet.adjustOld.waste_y;

	devSet.adjust.vialHeight = devSet.adjustOld.vialHeight;

#if VIAL_POS_MODIFY
	devSet.adjust.vialHeight_x = devSet.adjustOld.vialHeight_x;
	devSet.adjust.vialHeight_y = devSet.adjustOld.vialHeight_y;
#endif

  devState.actionMenu = AS_ACTION_ADJUST;
  devState.actionSubmenu = AS_ADJUST_MODE_END;

	devState.homeFindEnd[MOTOR_X] = NO;
	devState.homeFindEnd[MOTOR_Y] = NO;
	devState.homeFindEnd[MOTOR_Z] = NO;
	devState.homeFindEnd[MOTOR_SYR] = NO;
	devState.homeFindEnd[MOTOR_VAL] = NO;
	devState.homeFindEnd[MOTOR_INJ] = NO;

PumpConOff();

	stateChange(STATE_DEVICE_ADJUST);

	devSet.adjustCurMotor = 0xff;
	devSet.adjustOldMotor = 0xff;

//				stateChange(STATE_READY);
}

//size,start,
int adjustDataSaveEeprom(uint8_t *value , uint16_t size)
{
	uint16_t startAddr;

//	startAddr = HOME_ADJUST_DATA_STARTADDR + (uint16_t)value - (uint16_t)&devSet.adjust;
//		startAddr = HOME_ADJUST_DATA_STARTADDR + (uint16_t *)value - (uint16_t *)&devSet.adjust;
		startAddr = HOME_ADJUST_DATA_STARTADDR + ((uint16_t *)value - (uint16_t *)&devSet.adjust.saveFlag[0]);


	eepromWrite_Data(startAddr, size, value);
}

#if EEPROM_ADJUST_DATA_SAVE_FIX
int adjustDataSaveEeprom_2nd(uint8_t *value , uint16_t size)
{
	uint16_t startAddr;

/*
	dp("&devSet.adjust=0x%x ,&devSet.adjust.saveFlag=0x%x, &devSet.adjust.value=0x%x, &devSet.adjust.value[0]=0x%x\n",&devSet.adjust ,&devSet.adjust.saveFlag, &devSet.adjust.value, &devSet.adjust.value[0]);
a = (int)&devSet.adjust.value[0]	- (int)&devSet.adjust;
	dp("&devSet.adjust.value[0]	- &devSet.adjust = 0x%x\n",a);
*/
//	startAddr = HOME_ADJUST_DATA_STARTADDR + (uint16_t)value	- (uint16_t)&devSet.adjust;
//	eepromWrite_Data(startAddr, size, value);

	//startAddr = HOME_ADJUST_2ND_STARTADDR + (uint16_t)value	- (uint16_t)&devSet.adjust_2nd;
	startAddr = HOME_ADJUST_2ND_STARTADDR + ((uint16_t *)value	- (uint16_t *)&devSet.adjust_2nd.saveFlag[0]);
	eepromWrite_Data(startAddr, size, value);
}
#endif

extern uint32_t LocalTime;
int PacketParse(unsigned int nPacketCode, char *pData)
{
	int i;

//=======================================================================
// 처음으로 연결 - 정당한 연결인지 확인한다,
//=======================================================================
#if RS422_ENABLE
	if( devState.connected == NO && rs422if.connected == NO) {
#else
	if(	devState.connected == NO) {
#endif
		switch(nPacketCode) {
			case PACKCODE_LCAS_SYSTEM_INFORM:
				if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청
					SEND_RESPONSE_LCAS_PACKET(sysInform,pData);
				}
				else if(IS_ACK_PACKET(pData)){
	dfp("ack--PACKCODE_LCAS_SYSTEM_INFORM\n");
				}
				else {
	dfp("rcv--PACKCODE_LCAS_SYSTEM_INFORM\n");
	// 에러일 경우는 tcp_close()
					return net_CheckModel(pData);
				}
				break;
			default:
				return RE_ERR;
		}
	}

//=======================================================================
iprintf("<<<nPacketCode = %x>>>\n",nPacketCode);

	switch(nPacketCode)
	{
		case PACKCODE_LCAS_SYSTEM_INFORM:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청
				SEND_RESPONSE_LCAS_PACKET(sysInform,pData);
dfp("snd--PACKCODE_LCAS_SYSTEM_INFORM sysInform.cVersion = %s\n",sysInform.cVersion);
			}
			else if(IS_ACK_PACKET(pData)){
dfp("ack--PACKCODE_LCAS_SYSTEM_INFORM\n");
			}
			else {
//dfp("rcv--PACKCODE_LCAS_SYSTEM_INFORM\n");
#if FIX_TCP_CLOSE_EXCUTE_1
			net_SystemInform(pData);
#else
			return net_SystemInform(pData);
#endif
			}
			break;

		case PACKCODE_LCAS_ETHERNET_CONFIG:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청
				SEND_RESPONSE_LCAS_PACKET(ethernetConfig,pData);
dfp("snd--PACKCODE_LCAS_ETHERNET_CONFIG ethernetConfig.cGateway = %s\n",ethernetConfig.cGateway);
			}
			else if(IS_ACK_PACKET(pData)){
dfp("ack--PACKCODE_LCAS_ETHERNET_CONFIG\n");
			}
			else {
//dfp("rcv--PACKCODE_LCAS_ETHERNET_CONFIG\n");
#if FIX_TCP_CLOSE_EXCUTE_1
				net_EthernetConfig(pData);
#else
			return net_EthernetConfig(pData);
#endif
			}
			break;

		case PACKCODE_LCAS_SYSTEM_CONFIG:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청
				SEND_RESPONSE_LCAS_PACKET(sysConfig,pData);
dfp("snd--PACKCODE_LCAS_SYSTEM_CONFIG sysConfig.sampleLoopVol= %d\n",sysConfig.sampleLoopVol);
			}
			else if(IS_ACK_PACKET(pData)){
dfp("ack--PACKCODE_LCAS_SYSTEM_CONFIG\n");
			}
			else {
				net_SystemConfig(pData);
//dfp("rcv--PACKCODE_LCAS_SYSTEM_CONFIG sysConfig.sampleLoopVol= %d\n",sysConfig.sampleLoopVol);
#if 0
	unsigned char data[50];
	data[0] = DATA_SAVE_FLAG;
	memcpy(&data[1], &(sysConfig) , sizeof(sysConfig));
	eepromWrite_Data((SYSTEM_CONFIG_DATA_SAVE), sizeof(sysConfig) + 1, &(data[0]));
#endif
			}
			break;
		case PACKCODE_LCAS_RUN_SETUP:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청
				SEND_RESPONSE_LCAS_PACKET(runSetup,pData);
dfp("snd--PACKCODE_LCAS_RUN_SETUP\n");
			}
			else if(IS_ACK_PACKET(pData)){
dfp("ack--PACKCODE_LCAS_RUN_SETUP\n");
			}
			else net_runSetup(pData);
			break;
		case PACKCODE_LCAS_TIME_EVENT:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청
				SEND_RESPONSE_LCAS_PACKET(timeEvent,pData);
dfp("snd--PACKCODE_LCAS_TIME_EVENT\n");
			}
			else if(IS_ACK_PACKET(pData)){
dfp("ack--PACKCODE_LCAS_TIME_EVENT\n");
			}
			else net_TimeEvent(pData);
			break;
		case PACKCODE_LCAS_MIX_SETUP:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청
				SEND_RESPONSE_LCAS_PACKET(mixSet,pData);
dfp("snd--PACKCODE_LCAS_MIX_SETUP\n");
			}
			else if(IS_ACK_PACKET(pData)){
dfp("ack--PACKCODE_LCAS_MIX_SETUP\n");
			}
			else net_MixSetup(pData);
			break;
//		case PACKCODE_LCAS_MIX_TABLE:
//			break;
		case PACKCODE_LCAS_SEQUENCE:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청
				SEND_RESPONSE_LCAS_PACKET(sequence,pData);
dfp("snd--PACKCODE_LCAS_SEQUENCE\n");
			}
			else if(IS_ACK_PACKET(pData)){
dfp("snd--PACKCODE_LCAS_SEQUENCE\n");
			}
			else net_Sequence(pData);
			break;
		case PACKCODE_LCAS_COMMAND:
/*
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청 - 등어올 일이 없네
				SEND_RESPONSE_LCAS_PACKET(command,pData);
			}
			else if(IS_ACK_PACKET(pData)){}
			else
*/
			net_Command(pData);
			break;
		case PACKCODE_LCAS_STATE:
			// 데이터 요청일 경우만 응답 한다.
			if(IS_REQUEST_PACKET(pData)) {
				SEND_RESPONSE_LCAS_PACKET(LcasState,pData);
			}
 			break;
		case PACKCODE_LCAS_SLFEMSG:
			if(IS_REQUEST_PACKET(pData)) {}
			else if(IS_ACK_PACKET(pData)){	// 1초내 응답이 없을 경우 다시 전송
				}
			else {
				USE_LCAS_PACKET(pData, selfMsg);
				switch(selfMsg.ucMessage) {
					case SELF_STATE:
					case SELF_ERROR:
						/*
						switch(selfMsg.ucNewValue) {
							case SELF_ERROR_COMMAND_UNKNOWN:		break;
							case SELF_ERROR_NO_ADJUSTMENT_DATA:	break;
							case SELF_ERROR_MISSING_VIAL_SKIP:	break;
							case SELF_ERROR_MISSING_VIAL_WAIT:	break;
						}
						*/
						break;
				case SELF_EXTOUT:					break;
				case SELF_START_INJECTION:			break;
				case SELF_STOP:					break;
				case SELF_END_SEQUENCE:	//			9		// 시퀀스 동작 수행 완료
					break;
				case SELF_USER_RESPONSE:	//		10	// Missing Vial 사용자 선택에 대한 응답
dfp("========================missing %d	\n",devState.missVialResponse);
					switch(selfMsg.ucNewValue) {
						case MISSING_VIAL_NO_RESPONSE:
							devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
							break;
						case MISSING_VIAL_SKIP:
							devState.missVialResponse = MISSING_VIAL_SKIP;
							break;
						case MISSING_VIAL_RETRY:
							devState.missVialResponse = MISSING_VIAL_RETRY;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
							devState.missVialResponse = MISSING_VIAL_ABORT;
// COMMAND_STOP가 똑같은 동작 실행
commandStopFn();
							break;
					}
dfp("========================missing %d	\n",devState.missVialResponse);
					break;
				}
			}
			break;
		case PACKCODE_LCAS_ADJUST_DATA:
			if(IS_REQUEST_PACKET(pData)) {
				SEND_RESPONSE_LCAS_PACKET(devSet.adjust,pData);
			}
			else if(IS_ACK_PACKET(pData)){}
			else {
				send_SelfMessage(SELF_COMMAND_REJECT,0);
			}
			break;
		case PACKCODE_LCAS_SERVICE:
	dp("devState.actionMenu=%d\n",devState.actionMenu);
//			if(devState.actionMenu == AS_ACTION_READY || devState.actionMenu == AS_ACTION_ADJUST || devState.actionMenu == AS_ACTION_DIAGNOSTICS) {
			if(devState.actionMenu == AS_ACTION_READY || devState.actionMenu == AS_ACTION_ADJUST || devState.actionMenu == AS_ACTION_DIAGNOSTICS || devState.actionMenu == AS_ACTION_FAULT) {
				USE_LCAS_PACKET(pData, service);
	dp("PACKCODE_LCAS_SERVICE cmd=%d sub1=%d sub2=%d\n",service.command, service.subCommand1, service.subCommand2);
				switch(service.command) {
					case SERVICE_ADJUST:
						net_ServiceAdjust();
						break;
					case SERVICE_DIAGNOSTICS:
						net_ServiceDiagnostics(pData);
						break;
					case SERVICE_TEMP_CALIBRATION:
//						SEND_ACK_LCAS_PACKET(pData);
						net_ServiceTempCal(pData);
						break;
					case SERVICE_SYRINGE_REPLACE:
						SEND_ACK_LCAS_PACKET(pData);
						net_ServiceSyringeReplace();
						break;
					case SERVICE_NEEDLE_REPLACE:
						SEND_ACK_LCAS_PACKET(pData);
						net_ServiceNeedleReplace();
						break;
					case SERVICE_WASH_WASTE:
						SEND_ACK_LCAS_PACKET(pData);
						net_ServiceWashWaste();
						break;
					case SERVICE_USED_TIME:
//						SEND_ACK_LCAS_PACKET(pData);
//						net_ServiceUsedTime();
							net_ServiceUsedTime(pData);
						break;
					case SERVICE_MAC_INIT:
						break;
					case SERVICE_MIRO_PUMP_ON_OFF:
						switch(service.subCommand1) {
							case MIRO_PUMP_CMD_ON:
//								unsigned char microPumpPwmCount;
//								microPumpPwmCount = devSet.adjustTmp.microPumpVolt * MICRO_PUMP_UNIT_PWM + MICRO_PUMP_PWM_MIN;
//								PumpConOn(microPumpPwmCount);
								PumpConOn(devSet.adjustTmp.microPumpVolt * MICRO_PUMP_UNIT_PWM + MICRO_PUMP_PWM_MIN);
								break;
							case MIRO_PUMP_CMD_OFF:
								PumpConOff();
								break;
						}
						break;

					default:
	// 여기에 도달하면 에러 - 알수 없는 파라미터	// 오류 전송할 것.	// selfMessage
						errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
						break;
				}
			}
			else {
				// 서비스모드 실행 불가능 상태임.
				send_SelfMessage(SELF_COMMAND_REJECT,0);
			}
			break;
		case PACKCODE_LCAS_USED_TIME:
			// 데이터 요청일 경우만 응답 한다.
			if(IS_REQUEST_PACKET(pData)) {
				SEND_RESPONSE_LCAS_PACKET(usedTime,pData);
			}
			else if(IS_ACK_PACKET(pData)){
			}
			else {
// service 명령으로 변경하라.
//				send_SelfMessage(SELF_COMMAND_REJECT,0);
				net_UsedTime(pData);
			}
			break;

		case PACKCODE_LCAS_DIAG_DATA:
			// 데이터 요청일 경우만 응답 한다.
			if(IS_REQUEST_PACKET(pData)) {
				SEND_RESPONSE_LCAS_PACKET(diagData,pData);
			}
			else if(IS_ACK_PACKET(pData)){}
			else {
// service 명령으로 변경하라.
				send_SelfMessage(SELF_COMMAND_REJECT,0);
dlp("reject -- \n");
			}
			break;
		case PACKCODE_LCAS_SPECIAL:
			if(IS_REQUEST_PACKET(pData)) {
				SEND_RESPONSE_LCAS_PACKET(special,pData);
			}
			else if(IS_ACK_PACKET(pData)){}
			else {
				net_Special(pData);
			}
			break;










//===============================================================================================
// Motor Test용 프로토콜
//===============================================================================================
		case PACKCODE_LCAS_MOTOR_RUN :

			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청이므로 설정값을 전송해 주어야 한다.
				SEND_RESPONSE_LCAS_PACKET(MotorCtrlSet,pData);
			}
			else {
				USE_LCAS_PACKET(pData,MotorCtrlSet);
dp("PACKCODE_LCAS_MOTOR_RUN MotorCtrlSet=%d\n",MotorCtrlSet.bRun);
//for(i=0; i<((LCAS_PACKET *)pData)->nPacketLength ; i++) {
//	dp("[%3d] = 0x%02x \n",i,pData[i]);
//}
			}
			break;
//===============================================================================================
// Apply Button
//===============================================================================================
		case PACKCODE_LCAS_MOTOR_SET:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청이므로 설정값을 전송해 주어야 한다.
				SEND_RESPONSE_LCAS_PACKET(MotorCtrlSet,pData);
			}
			else {
				USE_LCAS_PACKET(pData,MotorCtrlSet);
dp("PACKCODE_LCAS_MOTOR_SET\n");
//for(i=0; i<((LCAS_PACKET *)pData)->nPacketLength ; i++) {
//	dp("[%3d] = 0x%02x \n",i,pData[i]);
//}

#if 1
for(i=0;i<6;i++) {
dp("[]================================[%d]\n",i);
	dp("bCtrlEnable[%d]  = 0x%02x \n",i,MotorCtrlSet.bCtrlEnable[i]);

	dp("controlCmd[%d] = 0x%02x \n",i,MotorCtrlSet.bDir[i] | MotorCtrlSet.bInfinite[i]);

	dp("bDir[%d]         = 0x%02x \n",i,MotorCtrlSet.bDir[i]);
	dp("bInfinite[%d]    = 0x%02x \n",i,MotorCtrlSet.bInfinite[i]);
	dp("nStep[%d]        = %d \n",i,MotorCtrlSet.nStep[i]);
	dp("nSpeedMin[%d]    = %d \n",i,MotorCtrlSet.nSpeedMin[i]);
	dp("nSpeedMax[%d]    = %d \n",i,MotorCtrlSet.nSpeedMax[i]);
	dp("bUnit[%d]        = 0x%02x \n",i,MotorCtrlSet.bUnit[i]);
	dp("bSensorCheck[%d] = 0x%02x \n",i,MotorCtrlSet.bSensorCheck[i]);
	dp("nStopBreak[%d]   = 0x%02x \n",i,MotorCtrlSet.nStopBreak[i]);
}
dp("[]================================[%d]\n",i);
	dp("bReset = 0x%02x \n",MotorCtrlSet.bReset);
	dp("bStop  = 0x%02x \n",MotorCtrlSet.bStop);
	dp("bBreak = 0x%02x \n",MotorCtrlSet.bBreak);
	dp("bRun   = 0x%02x \n",MotorCtrlSet.bRun);
dp("[]================================[%d]\n",i);
#endif
			}

motorControlFn();

    SEND_LCAS_PACKET(PACKCODE_LCAS_MOTOR_SET, MotorCtrlSet, MotorCtrlSet);
			break;
/*
#define CMD_DIR_CCW				0x01 // 1
#define CMD_START					0x02 // 2
#define CMD_RESET					0x04 // 4
#define CMD_SLEEP					0x08 // 8
#define CMD_STOP 					0x10 // 16
#define CMD_BREAK					0x20 // 32
#define CMD_INFINITE			0x40 // 64
#define CMD_CALIBRATE			0x80 // 128
*/
		case PACKCODE_LCAS_BOARD_STATE:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청이므로 설정값을 전송해 주어야 한다.
			}
			else {
				USE_LCAS_PACKET(pData,LcasBoardState);
			}
			break;
		case PACKCODE_LCAS_BOARD_CMD:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청이므로 설정값을 전송해 주어야 한다.
			}
			else {
				USE_LCAS_PACKET(pData,LcasBoardCmd);
				netBoardCmd(pData);
			}
			break;
		case PACKCODE_LCAS_MOTOR_RESET:
			motorResetAll();
	dp("PACKCODE_LCAS_MOTOR_RESET\n");
			break;
		case PACKCODE_LCAS_MOTOR_STOP:
			dp("PACKCODE_LCAS_MOTOR_STOP\n");
			stepCtrlCmd(MOTOR_X , MotorCtrlSet.bDir[5] | CMD_STOP | MotorCtrlSet.bInfinite[5]);
			break;
		case PACKCODE_LCAS_MOTOR_BREAK:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청이므로 설정값을 전송해 주어야 한다.
				SEND_RESPONSE_LCAS_PACKET(MotorCtrlSet,pData);
			}
			else {
				USE_LCAS_PACKET(pData,MotorCtrlSet);
			stepCtrlCmd(MOTOR_X , MotorCtrlSet.bDir[5] | CMD_BREAK | MotorCtrlSet.bInfinite[5]);
			}
			dp("PACKCODE_LCAS_MOTOR_BREAK\n");
			break;

		case PACKCODE_LCAS_MOTOR_SLEEP:
			/*
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청이므로 설정값을 전송해 주어야 한다.
				SEND_RESPONSE_LCAS_PACKET(MotorCtrlSet,pData);
			}
			else {
				USE_LCAS_PACKET(pData,MotorCtrlSet);
//			stepCtrlCmd(MOTOR_X , MotorCtrlSet.bDir[5] | CMD_BREAK | MotorCtrlSet.bInfinite[5]);
			}
*/

/*
//			gpio_in = (*((uint32_t volatile *)(COREGPIO_APBBIF_1_IN_REG)));
//			gpio_in = HAL_get_32bit_reg( COREGPIO_APBBIF_1_BASE_ADDR, GPIO_IN );
			gpio_in = GPIO_get_inputs(&g_gpio1);
			*((uint32_t volatile *)(COREGPIO_APBBIF_1_OUT_REG)) = 0x00000000;
			dp("PACKCODE_LCAS_MOTOR_SLEEP == %d\n",gpio_in);
			*((uint32_t volatile *)(COREGPIO_APBBIF_1_OUT_REG)) = 0x00020000;
*/
			break;
		case PACKCODE_LCAS_SETUP:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청이므로 설정값을 전송해 주어야 한다.
				SEND_RESPONSE_LCAS_PACKET(LcasCon,pData);
dfp("PACKCODE_LCAS_SETUP request -- \n");
			}
			else {
				USE_LCAS_PACKET(pData,LcasCon);
dfp("PACKCODE_LCAS_SETUP  packet \n");
			}
		break;
		case PACKCODE_LCAS_CON:
			if(IS_REQUEST_PACKET(pData)) {	// 데이터 요청이므로 설정값을 전송해 주어야 한다.
				SEND_RESPONSE_LCAS_PACKET(LcasCon,pData);
			}
			else {
				USE_LCAS_PACKET(pData,LcasCon);
//dp("PACKCODE_LCAS_CON\n");
			}
			break;
		case PACKCODE_LCAS_INJ_START:
			dp("PACKCODE_LCAS_INJ_START\n");
			stepCtrlCmd(MOTOR_X ,CMD_DIR_CCW);

			USE_LCAS_PACKET(pData,LcasCon);
			break;
		case PACKCODE_LCAS_WASH_START:
			USE_LCAS_PACKET(pData,LcasCon);
			break;

		case PACKCODE_LCAS_INITILIZE: break;
/*
		case PACKCODE_YL6200_EVC_CLOSE :
//			::PostMessage(ghwndYL6200, WM_ETH_EVENT_CLOSE, 0, 0 );
//			RETAILMSG(1, (TEXT("===PACKCODE_YL6200_EVC_CLOSE=== \r\n")));
			break;
*/
	}

	return RE_OK;
}

int net_ServiceDiagnostics(char *pData)
{
dfp("net_ServiceDiagnostics service.subCommand = %d \n",service.subCommand1);
	switch(service.subCommand1) {
		case DIAGNOSTICS_NONE:
//			SEND_ACK_LCAS_PACKET(pData);
//			devState.diagState = DIAGNOSTICS_NONE;
errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			break;
		case DIAGNOSTICS_STOP: //	(1)		//	현재 Diagnostics 동작 정지
			SEND_ACK_LCAS_PACKET(pData);
			devState.diagState = DIAGNOSTICS_NONE;

			runSetup.tempOnoff = OFF;
			tempSetup();

			devState.actionSubmenu = AS_READY_SUB_NONE;

			break;
		case DIAGNOSTICS_QUIT: //	(2)		//	STATE_DIAGNOSTICS상태를 빠져나간다.(STATE_READY상태로 전환)
			SEND_ACK_LCAS_PACKET(pData);
			devState.diagState = DIAGNOSTICS_NONE;

			runSetup.tempOnoff = OFF;
			tempSetup();

//			pPID_Control_Fn = PID_Function_Blank;

			devState.actionMenu	=	AS_ACTION_READY;
			devState.actionSubmenu = AS_READY_SUB_NONE;
			stateChange(STATE_READY);
			break;
		case DIAG_POWER_CHECK: //	(3)		//	전원검사 및 전압값 데이터 전송
			SEND_ACK_LCAS_PACKET(pData);
			devState.diagState = DIAG_POWER_CHECK;

			devState.actionMenu	=	AS_ACTION_DIAGNOSTICS;
			devState.actionSubmenu = AS_DIAG_SUB_NONE;
			stateChange(STATE_DIAGNOSTICS);

			pAction_Fn[AS_ACTION_DIAGNOSTICS] = actionDiagNone;
			diagData.Current = DIAG_POWER_CHECK;
dprintf("PACKCODE_LCAS_DIAG_DATA N5Dcheck = %d diagData.V5Dvalue =%f\n",diagData.N5Dcheck, diagData.V2_5Dvalue);
			SEND_LCAS_PACKET(PACKCODE_LCAS_DIAG_DATA, diagData, diagData);
dprintf("PACKCODE_LCAS_DIAG_DATA N5Dcheck = %d diagData.V5Dvalue =%f\n",diagData.N5Dcheck, diagData.V1_5Dvalue);
			devState.diagState = DIAGNOSTICS_NONE;
			break;
		case DIAG_HEATER_CHECK: // (4)		//	적정파워 인가하고 일정시간이후 측정된 온도 검사, 히팅속도 검사
			SEND_ACK_LCAS_PACKET(pData);
			devState.diagState = DIAG_HEATER_CHECK;

			devState.actionMenu	=	AS_ACTION_DIAGNOSTICS;
//			devState.actionSubmenu = AS_DIAG_SUB_NONE;
			devState.actionSubmenu = 1;//
			stateChange(STATE_DIAGNOSTICS);

			pAction_Fn[AS_ACTION_DIAGNOSTICS] = actionDiagHeaterCheck;

			break;
		case DIAG_COOLER_CHECK: // (5)		//	적정파워 인가하고 일정시간이후 측정된 온도 검사, 쿨링속도 검사
			SEND_ACK_LCAS_PACKET(pData);
			devState.diagState = DIAG_COOLER_CHECK;

			devState.actionMenu	=	AS_ACTION_DIAGNOSTICS;
			devState.actionSubmenu = 1;//
			stateChange(STATE_DIAGNOSTICS);

			pAction_Fn[AS_ACTION_DIAGNOSTICS] = actionDiagCoolerCheck;

			break;
		case DIAG_SENSOR_CHECK: // (6)		//	적정파워 인가하고 일정시간이후 측정된 온도 검사
			SEND_ACK_LCAS_PACKET(pData);
			devState.diagState = DIAG_SENSOR_CHECK;

			devState.actionMenu	=	AS_ACTION_DIAGNOSTICS;
			devState.actionSubmenu = 1;//
			stateChange(STATE_DIAGNOSTICS);

			pAction_Fn[AS_ACTION_DIAGNOSTICS] = actionDiagTempsenCheck;

			break;
		case DIAG_LEAK_CHECK:		// (7)		//  Leak - 사용안함.
/*
			devState.diagState = DIAG_LEAK_CHECK;

			devState.actionMenu	=	AS_ACTION_DIAGNOSTICS;
			devState.actionSubmenu = AS_DIAG_SUB_NONE;
			stateChange(STATE_DIAGNOSTICS);
*/
errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			break;
		default:
errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			break;
	}

	return RE_OK;
}

int net_ServiceTempCal(char *pData)
{
// [STATE_TEMP_CALIBRATION]상태로 된다.
	static unsigned char step = 0;
	static int oldTemp = 20;
	static unsigned char tempOnOff = OFF;

	switch(service.subCommand1) {
		case TEMP_CAL_NONE:
			errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			break;
		case TEMP_CAL_START:
			if(step != 0) {
				errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			}
			SEND_ACK_LCAS_PACKET(pData);
			step = 1;
// 저장된 Calibration 값을 무시하고 디폴트 상태에서 - 피팅함수의 계수를 다시 계산후 온도적용한다.
tempConstCalc(TEMP_CAL_START);
			tempOnOff = runSetup.tempOnoff;
			oldTemp = runSetup.temp;

			// 온도를 35도로 설정
			runSetup.temp = TEMP_CAL_SET_1ST;
			runSetup.tempOnoff = ON;
			tempSetup();
			stateChange(STATE_TEMP_CALIBRATION);
			break;
		case TEMP_CAL_TEMP_MEA1:
//tempReady(1초마다 전송되는 상태값)가 (1)이되면 제어프로그램에서 Measure값 입력받아 전송
//온도를 10도로 설정
			if(step != 1) {
				errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			}
			SEND_ACK_LCAS_PACKET(pData);
			step = 2;
			if((tempCalData.setTemp1 - service.value) > TEMP_CAL_ERROR_MIN && (tempCalData.setTemp1 - service.value) < TEMP_CAL_ERROR_MAX) {
				tempCalData.meaTemp1 = service.value;
dfp("Cal Temp - Measure temp 1 Receive!!!\n");
				runSetup.temp = TEMP_CAL_SET_2ND;
				tempSetup();
			}
			else {
				errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			}
			break;
		case TEMP_CAL_TEMP_MEA2:
			if(step != 2) {
				errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			}
			SEND_ACK_LCAS_PACKET(pData);
			step = 3;
			if((tempCalData.setTemp2 - service.value) > TEMP_CAL_ERROR_MIN && (tempCalData.setTemp2 - service.value) < TEMP_CAL_ERROR_MAX) {
				tempCalData.meaTemp2 = service.value;
dfp("Cal Temp - Measure temp 2 Receive!!!\n");
//				tempCalData.setTemp2 = TEMP_CAL_SET_2ND;
				tempSetup();
			}
			else {
				errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
			}
			break;
		case TEMP_CAL_SAVE:	// 저장
			if(step != 3) {
				send_SelfMessage(SELF_COMMAND_REJECT,0);
			}
			SEND_ACK_LCAS_PACKET(pData);
			step = 0;
dfp("Cal Temp - TEMP_CAL_SAVE  !!\n");
			tempConstCalc(TEMP_CAL_SAVE);
//			if(first == NO) {

//			}
//			else {
//				send_SelfMessage(SELF_COMMAND_REJECT,0);
//			}

			break;
		case TEMP_CAL_RESET:	// 디폴트
			if(step != 0) {
				send_SelfMessage(SELF_COMMAND_REJECT,0);
			}
			SEND_ACK_LCAS_PACKET(pData);
			step = 0;

			tempConstCalc(TEMP_CAL_RESET);
			break;
		case TEMP_CAL_QUIT:
			SEND_ACK_LCAS_PACKET(pData);
			step = 0;

// 온도를 이전 값으로 되돌린다.
			tempOnOff = runSetup.tempOnoff;
			oldTemp = runSetup.temp;
			tempSetup();

			stateChange(STATE_READY);
			break;
	}

	return RE_OK;
}

int net_ServiceSyringeReplace()
{
	switch(service.subCommand1) {
		case NDL_SYR_REPLACE_NONE:
			break;
		case NDL_SYR_REPLACE_START:
	    devState.actionMenu = AS_ACTION_ADJUST;
	    devState.actionSubmenu = AS_ADJUST_SYRINGE_REPLACE;
			stateChange(STATE_DEVICE_ADJUST);
			break;
		case NDL_SYR_REPLACE_QUIT:
			adjustQuit();
			break;
	}
	return RE_OK;
}

int net_ServiceNeedleReplace()
{
	switch(service.subCommand1) {
		case NDL_SYR_REPLACE_NONE:
			break;
		case NDL_SYR_REPLACE_START:
	    devState.actionMenu = AS_ACTION_ADJUST;
	    devState.actionSubmenu = AS_ADJUST_NEEDLE_REPLACE;
			stateChange(STATE_DEVICE_ADJUST);
			break;
		case NDL_SYR_REPLACE_QUIT:
			adjustQuit();
			break;
	}
	return RE_OK;
}

int net_ServiceWashWaste()
{
	switch(service.subCommand1) {
		case WASH_WASTE_NEEDLE_INT:
	    devState.actionMenu = AS_ACTION_SERVICE_WASH;
	    devState.actionSubmenu = AS_SERVICE_WASH_WAKEUP;
	    devState.serviceWashCnt = service.subCommand2;
			stateChange(STATE_SERVICE_WASH);
			break;
//		case WASH_WASTE_NEEDLE_EXT:
//	    devState.actionMenu = AS_ACTION_SERVICE_WASH;
//	    devState.actionSubmenu = AS_SERVICE_WASH_WAKEUP;
//	    devState.serviceWashCnt = service.subCommand2;
//			stateChange(STATE_SERVICE_WASH);
//			break;
		case WASH_WASTE_NEEDLE_ALL:
	    devState.actionMenu = AS_ACTION_SERVICE_WASH;
	    devState.actionSubmenu = AS_SERVICE_WASH_WAKEUP;
	    devState.serviceWashCnt = service.subCommand2;
			stateChange(STATE_SERVICE_WASH);
			break;
		case WASH_WASTE_INJECTOR:
	    devState.actionMenu = AS_ACTION_SERVICE_WASH;
	    devState.actionSubmenu = AS_SERVICE_WASH_WAKEUP;
	    devState.serviceWashCnt = service.subCommand2;
			stateChange(STATE_SERVICE_WASH);
			break;
//		case WASH_WASTE_WASTE:
//	    devState.actionMenu = AS_ACTION_SERVICE_WASH;
//	    devState.actionSubmenu = AS_SERVICE_WASH_WAKEUP;
//	    devState.serviceWashCnt = service.subCommand2;
//			stateChange(STATE_SERVICE_WASH);
//			break;

	}
	return RE_OK;
}

//int net_ServiceUsedTime()
int net_ServiceUsedTime(char *pData)
{
	switch(service.subCommand1) {
		case USED_TIME_NONE:
			break;
		case USED_TIME_REQUEST:
			usedTimeCheck(DATA_LOAD);
			SEND_LCAS_PACKET(PACKCODE_LCAS_USED_TIME, usedTime, usedTime);
			break;
		case USED_TIME_CLEAR:
			SEND_ACK_LCAS_PACKET(pData);
			break;
//		case USED_TIME_SET:
//			break;
	}
}

//==============================================================================
// 저장하고자하는 값에 끝에 0xff를 붙여서 전송한다.
// 저장하고자하는 값 : 73(10진수) --> 0x49
// 전송값 : 0x49에 0xff를 붙여서 0x49ff --> 18943(10진수)을 전송
//==============================================================================
int net_UsedTime(char *pData)
{
	LCAS_USED_TIME_t tmp;
	unsigned char ff;
	unsigned char save = NO;
	int iff,niff;

	if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
		send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
		return RE_ERR;
	}

	USE_LCAS_PACKET(pData,tmp);

//	ff = iff = tmp.time;					if(ff == 0xff) {usedTime.time = tmp.time >> 8;	save = YES;}
#if 1
//	ff = tmp.time;					if(ff == 0xff) {usedTime.time = tmp.time >> 8;	save = YES;}
	ff = iff = -tmp.time;	if(ff == 0xff) {usedTime.time = iff >> 8;	save = YES;}
	dfp("heat=%d %d",usedTime.heatTime,tmp.heatTime);
	dfp("ff=%d %d",ff,tmp.heatTime);
	dfp("iff=0x%x iff=%d %d",iff,tmp.time);

	ff = iff = -tmp.runTime;				if(ff == 0xff) {usedTime.runTime = iff >> 8;	save = YES;}
	ff = iff = -tmp.heatTime;			if(ff == 0xff) {usedTime.heatTime = iff >> 8;	save = YES;}
	ff = iff = -tmp.x;							if(ff == 0xff) {usedTime.x = iff >> 8;	save = YES;}
	ff = iff = -tmp.y;							if(ff == 0xff) {usedTime.y = iff >> 8;	save = YES;}
	ff = iff = -tmp.z;							if(ff == 0xff) {usedTime.z = iff >> 8;	save = YES;}
	ff = iff = -tmp.syringe;				if(ff == 0xff) {usedTime.syringe = iff >> 8;	save = YES;}
	ff = iff = -tmp.valve;					if(ff == 0xff) {usedTime.valve = iff >> 8;	save = YES;}
	ff = iff = -tmp.injector;			if(ff == 0xff) {usedTime.injector = iff >> 8;	save = YES;}
	ff = iff = -tmp.injectionCnt;	if(ff == 0xff) {usedTime.injectionCnt = iff >> 8;	save = YES;}
#else
	ff = tmp.time;					if(ff == 0xff) {usedTime.time = tmp.time >> 8;	save = YES;}
	ff = tmp.runTime;				if(ff == 0xff) {usedTime.runTime = tmp.runTime >> 8;	save = YES;}
	ff = tmp.heatTime;			if(ff == 0xff) {usedTime.heatTime = tmp.heatTime >> 8;	save = YES;}
	ff = tmp.x;							if(ff == 0xff) {usedTime.x = tmp.x >> 8;	save = YES;}
	ff = tmp.y;							if(ff == 0xff) {usedTime.y = tmp.y >> 8;	save = YES;}
	ff = tmp.z;							if(ff == 0xff) {usedTime.z = tmp.z >> 8;	save = YES;}
	ff = tmp.syringe;				if(ff == 0xff) {usedTime.syringe = tmp.syringe >> 8;	save = YES;}
	ff = tmp.valve;					if(ff == 0xff) {usedTime.valve = tmp.valve >> 8;	save = YES;}
	ff = tmp.injector;			if(ff == 0xff) {usedTime.injector = tmp.injector >> 8;	save = YES;}
	ff = tmp.injectionCnt;	if(ff == 0xff) {usedTime.injectionCnt = tmp.injectionCnt >> 8;	save = YES;}
#endif
	if(save == YES) {
		usedTimeCheck(DATA_SAVE);			// 사용시간 저장 - ready 상태 일때저장
		SEND_LCAS_PACKET(PACKCODE_LCAS_USED_TIME, usedTime, usedTime);
	}
	else {
		send_SelfMessage(SELF_COMMAND_REJECT,0);
	}
}

int net_Special(char *pData)
{
	LCAS_SPECIAL_t tmp;
	unsigned char saveFlag;
dp("net_Special111-- \n");
	if(LcasState.state == STATE_RUN || LcasState.state == STATE_ANALYSIS) {
		send_SelfMessage(SELF_COMMAND_REJECT,0);	// 실행할 수 없음.
		return RE_ERR;
	}

	USE_LCAS_PACKET(pData,tmp);

	if(	tmp.LED_onoff > ON || tmp.Door_check > ON || tmp.Leak_check > ON || tmp.Buzzer_onoff > ON ) {
		errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
		SEND_LCAS_PACKET(PACKCODE_LCAS_SPECIAL, special, special);

		return RE_ERR;
	}
dp("net_Special-- \n");
	SEND_ACK_LCAS_PACKET(pData);

	if(special.LED_onoff != tmp.LED_onoff || special.Door_check != tmp.Door_check
		|| special.Leak_check != tmp.Leak_check || special.Buzzer_onoff != tmp.Buzzer_onoff)
	{
		memcpy(&special, &tmp, sizeof(tmp));

		if(special.LED_onoff == ON) {		EXT_LED_CON_ON;		}
		else {													EXT_LED_CON_OFF;	}

		if(special.Door_check == YES) {
			devState.DoorErrorSend = YES;
			devState.DoorError = NO;
		}
		else {
			devState.DoorErrorSend = NO;
			devState.DoorError = NO;
		}

		if(special.Leak_check == YES) {
			devState.LeakErrorSend = YES;
			devState.LeakError = NO;
		}
		else {
			devState.LeakErrorSend = NO;
			devState.LeakError = NO;
		}
		if(special.Buzzer_onoff == ON) {}
		else {}

		devState.save_EEPROM_special = YES;
#if 0
		unsigned char data[9];
		data[0] = DATA_SAVE_FLAG;
		memcpy(&data[1], &(special) , sizeof(special));
		eepromWrite_Data((SPECIAL_DATA_SAVE), sizeof(special) + 1, &(data[0]));
#endif		
	}

dfp("led=%d door=%d leak=%d buz=%d = NO\n",special.LED_onoff,special.Door_check,special.Leak_check,special.Buzzer_onoff);
}

int save_EEPROM_special_data()
{
	unsigned char data[50];
	
	data[0] = DATA_SAVE_FLAG;
	memcpy(&data[1], &(special) , sizeof(special));
	eepromWrite_Data((SPECIAL_DATA_SAVE), sizeof(special) + 1, &(data[0]));

	devState.save_EEPROM_special = NO;

	return OK;
}

#if 0
int send_SystemInform()
{
	return 0;
}

int send_SystemConfig()
{
	return 0;
}

int send_runSetup()
{
	return 0;
}

int send_TimeEvent()
{
	return 0;
}

int send_MixSetup()
{
	return 0;
}

int send_MixTable()
{
	return 0;
}

int send_Sequence()
{
	return 0;
}

int send_Command()
{
	return 0;
}
#endif

int send_State_Blank()
{
#if RS422_ENABLE
	LcasState.sample.pos = devState.curSample.pos;
	LcasState.injectVial.pos = devState.injectVial.pos;
//iprintf("send_State_Blank\n");
	if(rs422if.connected) {
		RS422_SEND_LCAS_PACKET(PACKCODE_LCAS_STATE, LcasState, LcasState);
		pSend_DiagData_Fn();
	}
#endif	
}

int send_State()
{
//	LcasState.sample.pos = devState.sample.pos;
iprintf("send_State\n");
	LcasState.sample.pos = devState.curSample.pos;
	LcasState.injectVial.pos = devState.injectVial.pos;

	SEND_LCAS_PACKET(PACKCODE_LCAS_STATE, LcasState, LcasState);

	pSend_DiagData_Fn();
	return 0;
}


int send_DiagData_Blank(){}

#if RS422_ENABLE
int send_DiagData()
{
	if(devState.connected) {
//#if PHOTO_SENSOR_TEST
		diagData.btLeakTest1 = (
			((devState.btMotorState[MOTOR_X] >> 28) & 0x00000007)
			| ((devState.btMotorState[MOTOR_Y] >> 1) & 0x00000038)
			| ((devState.btMotorState[MOTOR_Z] << 2) & 0x000000C0)
		);
		diagData.btLeakTest2 = (
	//		((devState.btMotorState[MOTOR_X] >> 28) & 0x00000007)
	//		| ((devState.btMotorState[MOTOR_Y] >> 1) & 0x00000038)
			((devState.btMotorState[MOTOR_Z] >> 6) & 0x00000001)
			| ((devState.btMotorState[MOTOR_SYR] >> 27) & 0x0000000e)
			| ((devState.btMotorState[MOTOR_VAL])  & 0x00000030)
			| ((devState.btMotorState[MOTOR_INJ] >> 22) & 0x000000c0)
		);

		SEND_LCAS_PACKET(PACKCODE_LCAS_DIAG_DATA, diagData, diagData);
	}
	else if(rs422if.connected) {
//		r_enqueue((const char *)(&diagData), sizeof(diagData));//, &tx);
		RS422_SEND_LCAS_PACKET(PACKCODE_LCAS_DIAG_DATA, diagData, diagData);
	}

	return 0;
}
#else
int send_DiagData()
{
//#if PHOTO_SENSOR_TEST
	diagData.btLeakTest1 = (
		((devState.btMotorState[MOTOR_X] >> 28) & 0x00000007)
		| ((devState.btMotorState[MOTOR_Y] >> 1) & 0x00000038)
		| ((devState.btMotorState[MOTOR_Z] << 2) & 0x000000C0)
	);
	diagData.btLeakTest2 = (
//		((devState.btMotorState[MOTOR_X] >> 28) & 0x00000007)
//		| ((devState.btMotorState[MOTOR_Y] >> 1) & 0x00000038)
		((devState.btMotorState[MOTOR_Z] >> 6) & 0x00000001)
		| ((devState.btMotorState[MOTOR_SYR] >> 27) & 0x0000000e)
		| ((devState.btMotorState[MOTOR_VAL])  & 0x00000030)
		| ((devState.btMotorState[MOTOR_INJ] >> 22) & 0x000000c0)
	);

	SEND_LCAS_PACKET(PACKCODE_LCAS_DIAG_DATA, diagData, diagData);
	return 0;
}
#endif

#if RS422_ENABLE
int send_SelfMessage(unsigned char ucMessage, unsigned char ucNewValue)
{
	selfMsg.ucMessage = ucMessage;
	selfMsg.ucNewValue = ucNewValue;

	if(devState.connected) {
		SEND_LCAS_PACKET(PACKCODE_LCAS_SLFEMSG, selfMsg, selfMsg);
	}
	else if(rs422if.connected) {
iprintf("send_SelfMessage ucMessage=%d %d\n",ucMessage,ucNewValue);
		RS422_SEND_LCAS_PACKET(PACKCODE_LCAS_SLFEMSG, selfMsg, selfMsg);	
	}

	return 0;
}
#else
int send_SelfMessage(unsigned char ucMessage, unsigned char ucNewValue)
{
	selfMsg.ucMessage = ucMessage;
	selfMsg.ucNewValue = ucNewValue;
  SEND_LCAS_PACKET(PACKCODE_LCAS_SLFEMSG, selfMsg, selfMsg);

	return 0;
}
#endif


int send_AdjustData()
{
	return 0;
}

int send_Service()
{
	return 0;
}


