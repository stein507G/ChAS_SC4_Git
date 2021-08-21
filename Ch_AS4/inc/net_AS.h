 /**
  ******************************************************************************
  * @file    net_AS.h
  * @date    01/20/2016
  * @brief   This file contains all the functions prototypes for the net_AS.c
  *          file.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NET_AS_H
#define __NET_AS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
// PC에서 전송될때 사용 - 제어기등
#define PC_CMD_STOP			0
#define PC_CMD_BREAK		1
#define PC_CMD_SLEEP		2
#define PC_CMD_RESET		3
#define PC_CMD_RUN			4


void net_AS_init(void);

int netConnectAction();
int	netConnection();

int net_CheckModel(char *pData);

int net_SystemInform(char *pData);

void saveEthernetConfig(int who);
void loadEthernetConfig(int who);
int net_EthernetConfig(char *pData);

int net_SystemConfig(char *pData);

int save_EEPROM_sysConfig_data();

void initTempCalData();
void tempConstCalc(unsigned char act);
void tempSetup();
int net_runSetup(char *pData);
int net_TimeEvent(char *pData);
int net_MixSetup(char *pData);
//int net_MixTable();

int vialPos2Cnt(VIAL_POS_t *pVial);
int cnt2vialPos(int cnt);
int net_Sequence(char *pData);
void commandStopFn();
int net_Command(char *pData);
int net_State();
int net_SelfMessage();

int net_AdjustData();
int net_ServiceAdjust();
int adjustInitPosition(unsigned char part);
int adjustQuit();
int adjustDataSaveEeprom(uint8_t *value , uint16_t size);
int adjustDataSaveEeprom_2nd(uint8_t *value , uint16_t size);

int PacketParse(unsigned int nPacketCode, char *pData);

int net_ServiceDiagnostics(char *pData);
int net_ServiceTempCal(char *pData);

int net_ServiceSyringeReplace();
int net_ServiceNeedleReplace();
int net_ServiceWashWaste();
//int net_ServiceUsedTime();
int net_ServiceUsedTime(char *pData);
int net_UsedTime(char *pData);
int net_Special(char *pData);

int save_EEPROM_special_data();

#if 0
int send_SystemInform();
int send_SystemConfig();
int send_runSetup();
int send_TimeEvent();
int send_MixSetup();
int send_MixTable();
int send_Sequence();
int send_Command();
#endif

int (*pSend_State_Fn) (); // 함수포인터 선언
int send_State();
int send_State_Blank();

int (*pSend_DiagData_Fn) (); // 함수포인터 선언
int send_DiagData_Blank();
int send_DiagData();

int send_SelfMessage(unsigned char ucMessage, unsigned char ucNewValue);
int send_AdjustData();
int send_Service();
//void receiveServicePacket();



#ifdef __cplusplus
}
#endif

#endif /* __NET_AS_H */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

