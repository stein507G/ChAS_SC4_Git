
/*************************************************************************
 * (c) Copyright 2009	Actel	Corporation.	All	rights reserved.
 *
 *	Application	demo for Smartfusion
 *
 *
 * Author	:	Actel	Application	Team
 * Rev		 : 1.0.0.4
 *
 **************************************************************************/

/**************************************************************************/
/* Standard	Includes */
/**************************************************************************/

#include "config.h"

//#include <stdio.h>
//#include <stdlib.h>

/**************************************************************************/
/* Driver	Includes */
/**************************************************************************/

//#include "../drivers/mss_uart/mss_uart.h"

// ryu
//#include "define_AS.h"
//#include "debug_inform.h"

//#include "type_def_AS.h"

//#include "Def_stepCon.h"
//#include "protocolType_AS.h"
//#include "comType_AS.h"
//#include "protocol_AS.h"

extern char pPacketBuff[TCP_SEND_PACKET_BUFF_SIZE];
extern struct tcp_pcb *netAS_PCB;

#include "net_AS.h"
//#include "ADS1259_Ctrl.h"
#include "LcasAction_task.h"
#include "MotorCtrl_task.h"

#include "EEPROM.h"
#include "led_task.h"

unsigned char	bInitTicks = NO;

//===============================================================
// subMenu
uint8_t subMenu_w	=	1;
uint8_t subMenu_wch	=	1;
uint8_t subMenu_es	=	1;
uint8_t subMenu_sma	=	1;
uint8_t subMenu_wxch	=	1;
uint8_t subMenu_ghx	=	1;
uint8_t subMenu_ghcx	=	1;
uint8_t subMenu_as	=	1;
uint8_t subMenu_ds	=	1;
uint8_t subMenu_fli	=	1;
uint8_t subMenu_pli	=	1;
uint8_t subMenu_mpi	=	1;
uint8_t subMenu_map	=	1;
uint8_t subMenu_mmp	=	1;
uint8_t subMenu_mwp	=	1;

uint8_t subMenu_gpvz	=	1;
uint8_t subMenu_gpez	=	1;
uint8_t subMenu_gpezc	=	1;
uint8_t actRetry_gpezc	=	1;

uint8_t subMenu_gpezci	=	1;
uint8_t subMenu_gpv = 1;

uint8_t subMenu_ihpv	=	1;
uint8_t subMenu_ihpi	=	1;
uint8_t subMenu_ihps	=	1;
uint8_t subMenu_ihpz	=	1;
uint8_t subMenu_ihpx	=	1;
uint8_t subMenu_ihpy	=	1;
uint8_t subMenu_ihpa	=	1;

uint8_t subMenu_ami	=	1;
uint8_t subMenu_ane	=	1;
uint8_t subMenu_ase	=	1;

uint8_t subMenu_grpx = 1;
uint8_t subMenu_grpy = 1;
uint8_t subMenu_grpz = 1;

uint8_t subMenu_rcrx = 1;
uint8_t subMenu_rcry = 1;
uint8_t subMenu_rcrz = 1;

// mix
uint8_t subMenu_mp = 1;

//===============================================================

LCAS_CONTROL_t LcasCon;//,tmpLcasCon;
LCAS_DEV_STATE_t devState;

uint32_t remoteReadyIn = 0;

extern LCAS_STATE_t	LcasState;
extern LCAS_DEV_SET_t	devSet;
extern LCAS_RUN_SETUP_t	runSetup;
extern LCAS_TIME_EVENT_t timeEvent;
extern LCAS_MIX_t	mixSet;
//extern LCAS_MIX_TABLE_t	mixTable[MIX_TABLE_COUNT];
extern LCAS_SEQUENCE_t sequence;
extern LCAS_SERVICE_t service;
extern SYSTEM_INFORM_t sysInform;
extern SYSTEM_CONFIG_t sysConfig;
 //ADJUST_DEV_DATA_t	adjustTmp;
extern LCAS_TEMP_CAL_t tempCalData;
extern LCAS_USED_TIME_t usedTime;

extern LCAS_DIAG_DATA_t diagData;

extern LCAS_SPECIAL_t special;

extern ETHERNET_CONFIG_t ethernetConfig;

extern XY_COORD_t xyzCoord;

extern uint32_t	nextCaseDelay;
extern uint32_t	sysTicks;
//extern uint32_t	gbTempReadTick;
//extern uint32_t	gbPIDCtrlTick;

extern uint32_t	gbFabricRemoteInt;

extern uint32_t	remoteStartInCnt;
extern uint32_t	remoteStartInTime;

extern uint32_t	remoteStartOutSig;// = NO;	// 리모트	단자로 출력해야	된다.
extern uint32_t	remoteStartOutCnt;// = 0;
extern uint32_t	remoteStartOutTime;//	=	5;


#if TEMP_CAL_DATA_FORMAT_DOUBLE
extern double GfTempConstA;
extern double GfTempConstB;
extern double GfTempConstC;
#else
extern float GfTempConstA;
extern float GfTempConstB;
extern float GfTempConstC;
#endif

extern volatile int TimeEvent_1ms;	// pAction_Fn() 딜레이를 주기위한...

#if RS422_ENABLE
extern RS422_ST rs422if;
//extern RS422_BUF tx;
#endif


void init_analysisTimetick()
{
	devState.analysisTime100msCnt	=	0;
dp("analysisTimeCount=0000000000000000000000\n");
	devState.analysisTime100ms = runSetup.analysisTime	*	60 * 10;
};

//========================================================================
// 1.	inj,val	-->	ready	확인
// 1.	XY		:	Waste	Port
// 2.	syringe	가 원점(0)에 있지	않으면
//			a.Valve	:	Needle port
//			c.Syringe	:	Top(0점)으로 이동	-	버림

// 3.	Valve	washing	port
// 4.	Syringe	:	bottom(wash volume)으로	이동
// 5.	Valve	Needle port
// 6.	Syringe	:	Top(0점)으로 이동	-	버림
//========================================================================
#define	WASH_NEEDLE_ALL		0		// 시린지	+	펌프 이용
#define	WASH_NEEDLE_INT		1		// 시린지
#define	WASH_NEEDLE_EXT		2		// 펌프	이용
#define	WASH_INJECTOR			3		// 시린지
#define	WASH_INJECTOR_NEEDLE_ALL					4		// 인젝터와	Needle

//uint8_t subMenu_w	=	1;
//#define	SKIP_ACT_XY
int	Washing(int	repeat,	int	*curRepeat,	int	port)
{
	int32_t	readCount=0;
	static int encoderCount;
	static int retVal;
	static float washVolume = 0.0f;

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_w !=	Menu)	{
		dfp("subMenu_w=%d  stateSYR=%x\n",subMenu_w,devState.btMotorState[MOTOR_SYR]);
		Menu = subMenu_w;
}
#endif
	switch(subMenu_w)	{
// 1.	inj/val	-->	ready
		case 1:
// 20200504		
//			if(sleepChkVal(MOTOR_VAL)) { motorReadyCmd(MOTOR_VAL);}
			if(sleepChkVal(MOTOR_VAL)) { motorWakeupCmd(MOTOR_VAL);}
			if(sleepChkVal(MOTOR_INJ)) { motorReadyCmd(MOTOR_INJ);}
			++subMenu_w;
			break;
		// 2.	syringe	가 원점(0)에 있지	않으면
		case 2:
			if(devState.curStep[MOTOR_SYR] ==	0) {	// 버릴것이	없음
				subMenu_w	=	8;
			}
			else {
				++subMenu_w;
			}
			break;
		case 3:
			if(goPosWastePort()) {	++subMenu_w;	}
			break;
		case 4:		// a.Valve : Needle	port
			if(readyChkVal(MOTOR_VAL))	{
				retVal = goVal_Encoder(POS_VAL_NEEDLE,	0,ACT_SPEED_MAX_VAL, ACT_XY_POS);
				++subMenu_w;
			}
			break;
		case 5:
			if(readyChk(MOTOR_X) &&	readyChk(MOTOR_Y)	&& readyChk(MOTOR_Z) &&	readyChkVal(MOTOR_VAL))	{
				readyChkEncoder_Val(); 
				++subMenu_w;				
			}
			break;
		case 6:	// 버림
			encoderCount = goSyr_Encoder(POS_SYR_ZERO, 0, 	WASH_SPEED_UP_SYR, ACT_XY_POS);
			++subMenu_w;
			break;
		case 7:
			if(readyChk(MOTOR_SYR))	{	
#if SYRINGE_ENCODER_REMOVE_WASHING
#else
				readyChkEncoder_Syr();		
#endif				
				++subMenu_w;
			}
			break;
		case 8:
			switch(port) {
				case WASH_INJECTOR:
					if(goPosInjection()) {
						usedTime.injectionCnt++;	// 인젝터포트 사용횟수 
						++subMenu_w;
					}
					break;
				case WASH_NEEDLE_ALL:
					if(goPosWashingPort()) { ++subMenu_w;}
					break;
				case WASH_NEEDLE_INT:
					if(goPosWashingPort()) { ++subMenu_w;}
					break;
				case WASH_NEEDLE_EXT:
					if(goPosWashingPort()) { ++subMenu_w;}
					break;
//				case WASH_INJECTOR_NEEDLE_ALL:
//					if(goPosWashingPort()) { ++subMenu_w;}
//					break;
			}
			break;
		case 9:
			// 3.	Valve	washing	port
			retVal = goVal_Encoder(POS_VAL_WASHING, 0,ACT_SPEED_MAX_VAL,	ACT_XY_POS);
			++subMenu_w;
			break;
		case 10:
			if(readyChk(MOTOR_X) &&	readyChk(MOTOR_Y)	&& readyChk(MOTOR_Z) &&	readyChkVal(MOTOR_VAL))	{
				readyChkEncoder_Val();
dp("xxxx   Valve readCount=[%d]\n", readCount);
				++subMenu_w;				
			}
readCount	= *((uint32_t volatile *)(BASEADDR_MOTOR_VAL_INJ)); 
//readCount = (readCount >> 8) & 0x3ff; 
readCount = (readCount >> 8);//  & 0x3ff; 
//dfpErr("======  Valve  readCount=[0x%0x]\n", readCount ); 
			break;

#if FPGA_VERSION_TRIM_PUMP_TIME
// 4.	Syringe	:	bottom(500ul)으로	이동
// 동시에 외부세척용 펌프동작
		case 11:
			washVolume = runSetup.washVolume;
			encoderCount = goSyr_Encoder(washVolume,	0, WASH_SPEED_DOWN_SYR, ACT_XY_POS);
			++subMenu_w;
			break;
		case 12:
			if(readyChk(MOTOR_SYR))	{
#if SYRINGE_ENCODER_REMOVE_WASHING
#else
				readyChkEncoder_Syr();		
dp("xxxxxxxxx   syr readCount=[%d] [%d]\n", readCount , encoderCount);
#endif
				++subMenu_w;
			}
			break;
// 5.	Valve	Needle port
		case 13:
			retVal = goVal_Encoder(POS_VAL_NEEDLE,0,	ACT_SPEED_MAX_VAL, ACT_XY_POS);
			++subMenu_w;
			break;
		case 14:
			if(readyChkVal(MOTOR_VAL))	{
//#if FPGA_VERSION_TRIM
				readyChkEncoder_Val();
dp("xxxx-   Valve readCount=[%d]\n", readCount);
				++subMenu_w;
			}
			break;
// 6.	Syringe	:	Top(0점)으로 이동	-	버림
		case 15:
			encoderCount = goSyr_Encoder(POS_SYR_ZERO , 0, WASH_SPEED_UP_SYR,	ACT_XY_POS);
			++subMenu_w;
			break;
		case 16:
			if(readyChk(MOTOR_SYR))	{
#if SYRINGE_ENCODER_REMOVE_WASHING
#else
				readyChkEncoder_Syr();				
dp("xxxxxxxxx   syr readCount=[%d] [%d]\n", readCount , encoderCount);
#endif
				++subMenu_w;
			}
			break;
		case 17:
			if(port	== WASH_NEEDLE_ALL ||	port ==	WASH_NEEDLE_EXT)	{
				if(sysConfig.useMicroPump == YES) {
					PumpConOn(MICRO_PUMP_POWER_START);
//					PumpConOn(devSet.microPumpPwmCount);
					init_ticks(&nextCaseDelay);	// 다음 동작전 딜레이를 위해서 초기화 				
					++subMenu_w;
				}
				else {
					subMenu_w = 20;
				}
			}
			else {
				subMenu_w = 20;
			}
			break;
		case 18:
			GO_NEXT_CASE_TIME(nextCaseDelay,WAIT_MICRO_PUMP_START, subMenu_w);
			break;
		case 19:
			PumpConOn(devSet.microPumpPwmCount);
			init_ticks(&nextCaseDelay);	// 다음 동작전 딜레이를 위해서 초기화 				
			++subMenu_w;
			break;
		case 20:
			GO_NEXT_CASE_TIME(nextCaseDelay,devSet.microPumpTimeCount, subMenu_w);
			break;
		case 21:
			PumpConOff();
			++subMenu_w;			
			break;
		case 22:
			if(++(*curRepeat)	<	repeat)	{
//				subMenu_w	=	1;
// 반복동작에러를 수정한다.
				subMenu_w	=	9; // 똑같은 동작이므로 앞부분 생략
			}
			else {
				encoderCount = goZ_Encoder(POS_Z_TOP,ACT_XY_POS);
				*curRepeat = 0;
				++subMenu_w;
			}
			break;
		case 23:
//			if(readyChk(MOTOR_Z))	{
				//readyChkEncoder_Z();
			if(readyChkRetry_Z(encoderCount)) {
				motorSleepCmd(MOTOR_VAL	,	HOME_DIR_VAL);
				motorSleepCmd(MOTOR_INJ	,	HOME_DIR_INJ);
				++subMenu_w;
			}
			break;			
		case 24:
			if(sleepChkVal(MOTOR_VAL)	&& sleepChkVal(MOTOR_INJ)) {
				subMenu_w	=	1;
				return ACTION_MENU_END;
			}
			break;
		}

		return ACTION_MENU_ING;
#else
// 4.	Syringe	:	bottom(500ul)으로	이동
// 동시에 외부세척용 펌프동작
		case 11:
			washVolume = runSetup.washVolume;
			encoderCount = goSyr_Encoder(washVolume,	0, WASH_SPEED_DOWN_SYR, ACT_XY_POS);

			if(port	== WASH_NEEDLE_ALL ||	port ==	WASH_NEEDLE_EXT)	{
				if(sysConfig.useMicroPump == YES) {
					PumpConOn(PUMP_POWER_MID);
				}
			}
			++subMenu_w;
			break;
		case 12:
			if(readyChk(MOTOR_SYR))	{
#if SYRINGE_ENCODER_REMOVE_WASHING
#else
				readyChkEncoder_Syr();		
dp("xxxxxxxxx   syr readCount=[%d] [%d]\n", readCount , encoderCount);
#endif
				++subMenu_w;
			}
			break;
// 5.	Valve	Needle port
		case 13:
			retVal = goVal_Encoder(POS_VAL_NEEDLE,0,	ACT_SPEED_MAX_VAL, ACT_XY_POS);
			++subMenu_w;
			break;
		case 14:
			if(readyChkVal(MOTOR_VAL))	{
//#if FPGA_VERSION_TRIM
				readyChkEncoder_Val();
dp("xxxx-   Valve readCount=[%d]\n", readCount);
				++subMenu_w;
			}
			break;
// 6.	Syringe	:	Top(0점)으로 이동	-	버림
		case 15:
			encoderCount = goSyr_Encoder(POS_SYR_ZERO , 0, WASH_SPEED_UP_SYR,	ACT_XY_POS);
			++subMenu_w;
			break;
		case 16:
			if(readyChk(MOTOR_SYR))	{
				PumpConOff();
#if SYRINGE_ENCODER_REMOVE_WASHING
#else
				readyChkEncoder_Syr();				
dp("xxxxxxxxx   syr readCount=[%d] [%d]\n", readCount , encoderCount);
#endif
				++subMenu_w;
			}
			break;
		case 17:
			if(++(*curRepeat)	<	repeat)	{
//				subMenu_w	=	1;
// 반복동작에러를 수정한다.
				subMenu_w	=	9; // 똑같은 동작이므로 앞부분 생략
			}
			else {
				encoderCount = goZ_Encoder(POS_Z_TOP,ACT_XY_POS);
				*curRepeat = 0;
				++subMenu_w;
			}
			break;
		case 18:
//			if(readyChk(MOTOR_Z))	{
				//readyChkEncoder_Z();
			if(readyChkRetry_Z(encoderCount)) {
				motorSleepCmd(MOTOR_VAL	,	HOME_DIR_VAL);
				motorSleepCmd(MOTOR_INJ	,	HOME_DIR_INJ);
				++subMenu_w;
			}
			break;			
		case 19:
			if(sleepChkVal(MOTOR_VAL)	&& sleepChkVal(MOTOR_INJ)) {
				subMenu_w	=	1;
				return ACTION_MENU_END;
			}
			break;
		}

		return ACTION_MENU_ING;
#endif		
}

#if	0
//========================================================================
// 1.	XY		:	Waste	Port
// 2.	syringe	가 원점(0)에 있지	않으면
//			a.Valve	:	Needle port
//			c.Syringe	:	Top(0점)으로 이동	-	버림

// 3.	Valve	washing	port
// 4.	Syringe	:	bottom(500ul)으로	이동
// 5.	Valve	Needle port
// 6.	Syringe	:	Top(0점)으로 이동	-	버림
//========================================================================

#endif

// =========================================================
// if	syring top --> go	waste	-->	valve	needle --> syringe top
// go	home
// val,	inj	sleep
// syr,x,y,z sleep 추가 
// =========================================================
//uint8_t subMenu_es	=	1;
int	endSequence()
{
	int32_t	readCount=0;
	static int encoderCount;
	static int retVal;

//	static uint32_t	subMenu	=	1;

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_es !=	Menu)	{
    dfp("subMenu_es=%d\n",subMenu_es);
		Menu = subMenu_es;
}
#endif

	switch(subMenu_es)	{
		case 1:
#if WASH_BETWEEN_VIALS_BEFORE_FIX
				subMenu_es	=	1;
				return ACTION_MENU_END;
#else
			if(devState.vialExist == NO) {
				++subMenu_es;
			}
			else {
				subMenu_es	=	1;
				return ACTION_MENU_END;
			}
#endif			
			break;
		case 2:
			if(devState.curStep[MOTOR_SYR] ==	0) {	// 버릴것이	없음
				subMenu_es	=	8;
			}
			else {
				++subMenu_es;
			}
			break;
		case 3:
			if(goPosWastePort()) {	++subMenu_es;	}
			break;
		case 4:
			if(sleepChkVal(MOTOR_VAL)) { motorReadyCmd(MOTOR_VAL);}
			++subMenu_es;
			break;
		case 5:
			if(readyChkVal(MOTOR_VAL))	{
		retVal =		goVal_Encoder(POS_VAL_NEEDLE,0,	ACT_SPEED_MAX_VAL, ACT_XY_POS);
				++subMenu_es;
			}
			break;
		case 6:	// 버림
			if(readyChkVal(MOTOR_VAL))	{		
				readyChkEncoder_Val();
				encoderCount =		goSyr_Encoder(POS_SYR_ZERO, 0, WASH_SPEED_UP_SYR, ACT_XY_POS);
				++subMenu_es;
			}			
			break;
		case 7:
			if(readyChk(MOTOR_SYR))	{
#if SYRINGE_ENCODER_REMOVE_END
#else
				readyChkEncoder_Syr();		
#endif				
				++subMenu_es;
			}
			break;
		case 8:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_es,	NEXT_CASE_DELAY_INTERVAL);
			break;
		case 9:
			if(goPosHome())	{	++subMenu_es; }
			break;
		case 10:
			if(sleepModeAll()) {
				subMenu_es	=	1;
				return ACTION_MENU_END;
			}
			break;
	}
	return ACTION_MENU_ING;

}

int sleepModeAll()
{
//=====================================================	
// 주의 : READY 상태이거나 BREAK상태에서 만 정상동작한다.		
//=====================================================	
	
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_sma !=	Menu)	{
		dfp("subMenu_sma=%d\n",subMenu_sma);
		Menu = subMenu_sma;
}
#endif

	switch(subMenu_sma)	{
		case 1:
			motorSleepCmd(MOTOR_X	,	HOME_DIR_X);
			motorSleepCmd(MOTOR_Y	,	HOME_DIR_Y);

#if POWER_DOWN_Z_MODIFY
			motorSleepCmd(MOTOR_Z	,	HOME_DIR_Z);
#endif
#if POWER_DOWN_SYR_MODIFY
			motorSleepCmd(MOTOR_SYR	,	HOME_DIR_SYR);
#endif
			motorSleepCmd(MOTOR_VAL	,	HOME_DIR_VAL);
			motorSleepCmd(MOTOR_INJ	,	HOME_DIR_INJ);
			++subMenu_sma;
			break;
		case 2:
			if(sleepChk(MOTOR_X)	
				&& sleepChk(MOTOR_Y) 

#if POWER_DOWN_Z_MODIFY
				&&	sleepChk(MOTOR_Z)	
#endif
#if POWER_DOWN_SYR_MODIFY
				&& sleepChk(MOTOR_SYR) 
#endif
				&& sleepChkVal(MOTOR_VAL)	
				&& sleepChkVal(MOTOR_INJ)	) 
			{
				subMenu_sma	=	1;
				devState.bSleepMode = YES;
				devState.bCheckHome = NO;

// 20200526
if(devState.initCommand == YES) {
	devState.initCommand = NO;
	devState.command = COMMAND_INIT;
}
				return ACTION_MENU_END;
			}
			break;
	}

	return ACTION_MENU_ING;
}

int sleepModeError()
{
//=====================================================	
// 주의 : READY 상태이거나 BREAK상태에서 만 정상동작한다.		
//=====================================================	
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_sma !=	Menu)	{
		dfp("subMenu_sma=%d\n",subMenu_sma);
		Menu = subMenu_sma;
}
#endif

	switch(subMenu_sma)	{
		case 1:
			motorSleepCmd(MOTOR_X	,	HOME_DIR_X);
			motorSleepCmd(MOTOR_Y	,	HOME_DIR_Y);
			motorSleepCmd(MOTOR_Z	,	HOME_DIR_Z);

			motorSleepCmd(MOTOR_SYR	,	HOME_DIR_SYR);

			motorSleepCmd(MOTOR_VAL	,	HOME_DIR_VAL);
			motorSleepCmd(MOTOR_INJ	,	HOME_DIR_INJ);
			++subMenu_sma;
			break;
		case 2:
			if(sleepChk(MOTOR_X)	
				&& sleepChk(MOTOR_Y) 
				&&	sleepChk(MOTOR_Z)	
				&& sleepChk(MOTOR_SYR) 
				&& sleepChkVal(MOTOR_VAL)	
				&& sleepChkVal(MOTOR_INJ)	) 
			{
				subMenu_sma	=	1;
				devState.bSleepMode = YES;
				devState.bCheckHome = NO;
				
				return ACTION_MENU_END;
			}
			break;
	}

	return ACTION_MENU_ING;
}

//#define WAKEUP_DELAY_TIME			5000		// 0.2초
int wakeup_XYZ_checkHome()
{

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_wxch !=	Menu)	{
		dfp("subMenu_wxch=%d stateSYR=%x\n",subMenu_wxch,devState.btMotorState[MOTOR_SYR]);		
		Menu = subMenu_wxch;
}
#endif

	switch(subMenu_wxch)	{
		case 1:
			if(sleepChk(MOTOR_Y)) { motorReadyCmd(MOTOR_Y);}
			++subMenu_wxch;
			break;
		case 2:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_wxch, WAKEUP_DELAY_TIME);
			break;
		case 3:
			if(sleepChk(MOTOR_SYR)) { motorReadyCmd(MOTOR_SYR);}
			++subMenu_wxch;
			break;
		case 4:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_wxch, WAKEUP_DELAY_TIME);
			break;
		case 5:
			if(sleepChk(MOTOR_Z)) { motorReadyCmd(MOTOR_Z);}
			++subMenu_wxch;
			break;
		case 6:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_wxch, WAKEUP_DELAY_TIME);
			break;
		case 7:
			if(sleepChk(MOTOR_X)) { motorReadyCmd(MOTOR_X);}
			++subMenu_wxch;
			break;
		case 8:
			if(readyChk(MOTOR_X) &&	readyChk(MOTOR_Y)	&& readyChk(MOTOR_Z) && readyChk(MOTOR_SYR))	{
				++subMenu_wxch;
			}
			break;
// MOTRO_Z
// 현재는 영점에서 벗어나있는 점에있다. +160지점
// 0 점위치로 이동한다. : POS_Z_TOP
		case 9:
			subMenu_wxch = 11;
			break;
		case 10:
			
			if(devState.homeFindEnd[MOTOR_Z] == NO) {
				initHomePos_Z();
			}
			else {
				++subMenu_wxch;
			}
			break;

// MOTRO_Y
		case 11:
			devState.homeFindEnd[MOTOR_Y] = NO;
			++subMenu_wxch;
			break;
		case 12:
			if(devState.homeFindEnd[MOTOR_Y] == NO) {
				initHomePos_Y();
			}
			else {
				++subMenu_wxch;
			}
			break;
			
// MOTRO_X			
		case 13:
			devState.homeFindEnd[MOTOR_X] = NO;
			++subMenu_wxch;
			break;
		case 14:
			if(devState.homeFindEnd[MOTOR_X] == NO) {
				initHomePos_X();
			}
			else {
				++subMenu_wxch;
			}
			break;
		case 15:
			subMenu_wxch	=	1;
			devState.bSleepMode = NO;
			devState.bCheckHome = YES;
			return ACTION_MENU_END;
			break;
	}

	return ACTION_MENU_ING;
}

// injection Markout
int	startInjectionFunction()	// 인젝션	시작
{
#if HOME_FIND_TIME_CHECK_X
#else
	init_System_ticks(); 		// 없어도 되는 것 같은데...
#endif
	// 리모트	출력	
	if(sysConfig.injectMarkerTime	!= INJECTOR_MARKER_TIME_NONE)	{
		REMOTE_START_OUT_CONTACT_CLOSE;
		remoteStartOutSig	=	YES;	// 시그널	출력해야 하다.
		remoteStartOutCnt	=	0;
	}
	send_SelfMessage(SELF_START_INJECTION,0);

	stateChange(STATE_ANALYSIS);

	if(timeEvent.useAuxTimeEvent == YES) {
// 첫번째 Aux Event가 0.0분에 동작하여야 한다면 
		if(devState.auxTime100msCnt[0] == 0) {
dfp("if(devState.auxTime100msCnt[0] == 0) devState.curAuxEventNo = %d\n\r",devState.curAuxEventNo);
			switch(timeEvent.auxEvent[0].auxOnoff)	{
				case AUX_CONTACT_OPEN:
					devState.curAuxState = AUX_CONTACT_OPEN;
					devState.curAuxOutIng	=	NO;
//					devState.auxOnOff	=
//					devState.auxOnOffCnt =
					break;
				case AUX_CONTACT_CLOSE:
					devState.curAuxState = AUX_CONTACT_CLOSE;
					devState.curAuxOutIng	=	NO;
					break;
				case AUX_TOGGLE:
					if(devState.curAuxState	== AUX_CONTACT_OPEN) {
						AUX_SIG_OUT_CONTACT_CLOSE;
						devState.curAuxState = AUX_CONTACT_CLOSE;
					}
					else {
						AUX_SIG_OUT_CONTACT_OPEN;
						devState.curAuxState = AUX_CONTACT_CLOSE;
					}
					devState.curAuxOutIng	=	NO;
					break;
				case AUX_PULSE:
					devState.curAuxState = AUX_CONTACT_OPEN;
					devState.curAuxOutIng	=	YES;
					break;
			}

			devState.curAuxEventNo = 1;
		}
		else {
			devState.curAuxEventNo = 0;
		}
		pTime_Event_Fn = time_Event;
	}
	
	// LED
	// 다음	인젝션을 하기위해서	분석시간동안 대기하도록	devState.analysisTime100msCnt	초기화
	init_analysisTimetick();

//	devState.curSample.pos = devState.sample.pos;	

	return 0;
}

// 분석끝난후 동작
// auxEvent 
int endAnalysisFunction()
{
	pTime_Event_Fn = time_Event_Blank;
//	init_time_Event();

	initAuxEvntFn();
}

int	startLoadingCommand()	// 주입을	위한 준비
{
	dfp("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
	if(devState.seqAction	== YES)	{
		devState.actionMenu	=	AS_ACTION_MIX;		
		devState.actionSubmenu = AS_MIX_SUB_NONE;
//		devState.actionMenu	=	AS_ACTION_RUN;
//		devState.actionSubmenu = AS_RUN_SUB_MIX_PRGM;
//		devState.actionSubmenu = AS_RUN_SUB_WAKE_UP;
//		devState.actionSubmenu = AS_MIX_SUB_WAKE_UP;

		devState.curInjCnt = 0;
		LcasState.curInjCnt = 0;

		stateChange(STATE_RUN);
	}
}

int	stopFunction()
{
	return 0;
}

int	stateChange(int	state)
{

/*
	if(state ==	STATE_READY) {
		REMOTE_READY_OUT_CONTACT_CLOSE;
	}
	else {
		REMOTE_READY_OUT_CONTACT_OPEN;
	}
*/
	if(state !=	LcasState.state) {
		devState.stateOld	=	LcasState.state;
		LcasState.state	=	state;
		switch(state) {
			case STATE_READY:
				REMOTE_READY_OUT_CONTACT_CLOSE;
				pSend_DiagData_Fn = send_DiagData_Blank;		
				BUZZER_CON_OFF;
				break;
			case STATE_DIAGNOSTICS:
				REMOTE_READY_OUT_CONTACT_OPEN;
				pSend_DiagData_Fn = send_DiagData;
				BUZZER_CON_OFF;
				break;
			case STATE_FAULT:
				REMOTE_READY_OUT_CONTACT_OPEN;
				pSend_DiagData_Fn = send_DiagData_Blank;

devState.actionMenu = AS_ACTION_FAULT;
devState.actionSubmenu = AS_INIT_SUB_NONE;

motorBreakAll();
initSubmenuValue();
//sleepModeAll();
sleepModeError();
				break;
			default:
				REMOTE_READY_OUT_CONTACT_OPEN;
				pSend_DiagData_Fn = send_DiagData_Blank;
				BUZZER_CON_OFF;
				break;
		}
	}

}


//uint8_t subMenu_ghcx = 1;	// int	goHomeCheckXYZ()
// home check and go home
int	goHomeCheckXYZ()
{
	
#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ghcx != Menu) {
  dfp("subMenu_ghcx=%d\n",subMenu_ghcx);
  Menu = subMenu_ghcx;
}
#endif

	switch(subMenu_ghcx) {
		case 1:
//			if(goPosETC_Z(POS_XY_HOME,POS_Z_TOP))	{	++subMenu_ghcx; }
//			goZ(POS_Z_TOP,ACT_XY_POS);
			++subMenu_ghcx;
			break;
		case 2:
			if(readyChk(MOTOR_Z))	{
				devState.homeFindEnd[MOTOR_X] = NO;
				devState.homeFindEnd[MOTOR_Y] = NO;
				devState.homeFindEnd[MOTOR_Z] = NO;
				++subMenu_ghcx;			
			}
			break;
		case 3:
			if(devState.homeFindEnd[MOTOR_Z] == NO) {
				initHomePos_Z();
			}
			else {
				++subMenu_ghcx;			
			}
			break;
		case 4:
				if(devState.homeFindEnd[MOTOR_Y] == NO) {
					initHomePos_Y();
				}
				else {
					++subMenu_ghcx;			
				}
				break;
			case 5:
				if(devState.homeFindEnd[MOTOR_X] == NO) {
					initHomePos_X();
				}
				else {
					subMenu_ghcx++;
				}
				break;
		case 6:
			subMenu_ghcx	=	1;
			return ACTION_MENU_END;
			break;
	}

	return ACTION_MENU_ING;
			
}

int	chkNextInjectionMix()		 // next sample
{
	int	rowNo	=	devState.sample.uPos.x;	// 현재	주입이 끝난	바이알 좌표
	int	colNo	=	devState.sample.uPos.y;
	int	trayNo = devState.sample.uPos.tray;

	int	excute = NO;
	int	trayEnd	=	NO;

	if(devState.command	== COMMAND_END)	{
		dp("<<<+++++++++++++++Stop	Injection!!!!\n");
		devState.doNextSample	=	NO;
		devState.isNextInjection = NO;
		return devState.isNextInjection;
	}

	if(nextSample(&devState.sample) == RE_OK) {
		devState.isNextInjection = YES;
		devState.doNextSample	=	YES;
devState.curSample.pos = devState.sample.pos;			// 현재 진행중인 샘플 	
dp("excute!!\n");
dfp("sample.uPos (%d,%d)\n",devState.sample.uPos.x, devState.sample.uPos.y);
//chkNextVialMix();
		if(mixSet.relativeDestination == YES) {
			if(nextVial(&devState.firstDestVial)) {}
			else { /* error */}
dfp("relative firstDestVial.uPos (%d,%d)\n",devState.firstDestVial.uPos.x, devState.firstDestVial.uPos.y);			
		}
		if(mixSet.relativeReagentA == YES) {
			if(nextVial(&devState.reagentVialA)) {}
			else { /* error */}
dfp("relative reagentVialA.uPos (%d,%d)\n",devState.reagentVialA.uPos.x, devState.reagentVialA.uPos.y);			
		}
		if(mixSet.relativeReagentB == YES) {
			if(nextVial(&devState.reagentVialB)) {}
			else { /* error */}
dfp("relative reagentVialB.uPos (%d,%d)\n",devState.reagentVialB.uPos.x, devState.reagentVialB.uPos.y);			
		}
		if(mixSet.relativeReagentC == YES) {
			if(nextVial(&devState.reagentVialC)) {}
			else { /* error */}				
dfp("relative reagentVialC.uPos (%d,%d)\n",devState.reagentVialC.uPos.x, devState.reagentVialC.uPos.y);			
		}
		if(mixSet.relativeReagentD == YES) {
			if(nextVial(&devState.reagentVialD)) {}
			else { /* error */}		
dfp("relative reagentVialD.uPos (%d,%d)\n",devState.reagentVialD.uPos.x, devState.reagentVialD.uPos.y);			
		}

	}
	else {
			devState.doNextSample	=	NO;
			devState.isNextInjection = NO;
dfp("END sample.uPos (%d,%d)\n",devState.sample.uPos.x, devState.sample.uPos.y);
	}

return devState.isNextInjection;

}

int	chkNextInjectionRun()		 // next sample
{
	int	rowNo	=	devState.sample.uPos.x;	// 현재	주입이 끝난	바이알 좌표
	int	colNo	=	devState.sample.uPos.y;
	int	trayNo = devState.sample.uPos.tray;

	int	excute = NO;
	int	trayEnd	=	NO;

	if(devState.command	== COMMAND_END)	{
		dp("<<<+++++++++++++++Stop	Injection!!!!\n");
		devState.doNextSample	=	NO;
		devState.isNextInjection = NO;
		return devState.isNextInjection;
	}

		if(devState.curInjCnt	<	devState.injectionTimes) {
				devState.isNextInjection = YES;
				devState.doNextSample	=	NO;	// 아직은	다음 샘플차례가	아니다.
				return devState.isNextInjection;
		}
		else {
				devState.isNextInjection = NO;	// 아래에서	다시 검사한다.
				devState.doNextSample	=	NO;			// 아직은	다음 샘플차례가	아니다.
				devState.curInjCnt = 0;
//LcasState.curInjCnt = 0;				
		}

dp(">>>>>>>==================chkNextInjection!!\n");

	if(nextSample(&devState.sample) == RE_OK) {
		devState.isNextInjection = YES;
		devState.doNextSample	=	YES;
dp("excute!!\n");
dfp("sample.uPos (%d,%d)\n",devState.sample.uPos.x, devState.sample.uPos.y);
//chkNextVialMix();
		if(mixSet.relativeDestination == YES) {
			if(nextVial(&devState.firstDestVial)) {}
			else { /* error */}
dfp("relative firstDestVial.uPos (%d,%d)\n",devState.firstDestVial.uPos.x, devState.firstDestVial.uPos.y);			
		}
		if(mixSet.relativeReagentA == YES) {
			if(nextVial(&devState.reagentVialA)) {}
			else { /* error */}
dfp("relative reagentVialA.uPos (%d,%d)\n",devState.reagentVialA.uPos.x, devState.reagentVialA.uPos.y);			
		}
		if(mixSet.relativeReagentB == YES) {
			if(nextVial(&devState.reagentVialB)) {}
			else { /* error */}
dfp("relative reagentVialB.uPos (%d,%d)\n",devState.reagentVialB.uPos.x, devState.reagentVialB.uPos.y);			
		}
		if(mixSet.relativeReagentC == YES) {
			if(nextVial(&devState.reagentVialC)) {}
			else { /* error */}				
dfp("relative reagentVialC.uPos (%d,%d)\n",devState.reagentVialC.uPos.x, devState.reagentVialC.uPos.y);			
		}
		if(mixSet.relativeReagentD == YES) {
			if(nextVial(&devState.reagentVialD)) {}
			else { /* error */}		
dfp("relative reagentVialD.uPos (%d,%d)\n",devState.reagentVialD.uPos.x, devState.reagentVialD.uPos.y);			
		}

	}
	else {
			devState.doNextSample	=	NO;
			devState.isNextInjection = NO;
dfp("END sample.uPos (%d,%d)\n",devState.sample.uPos.x, devState.sample.uPos.y);			
	}

return devState.isNextInjection;

}

// =============================================================================
// 세척동작	여부
// 바이알	없음 세척	필요없음
// =============================================================================
int	checkWashBetween()
{
		switch(runSetup.washBetween) {
				case WASH_BETWEEN_NONE:
						devState.actionSubmenu = AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
/*
						if(devState.isNextInjection	== NO	&& devState.doNextSample ==	NO)	{	// 끝	-	세척동작을 안하고	끝
								devState.actionSubmenu = AS_RUN_SUB_WAIT_RUN_TIME;
						}	else {// 시간	대기 ...
							devState.actionSubmenu = AS_RUN_SUB_WAIT_RUN_TIME;
						}
*/
						break;
				case WASH_BETWEEN_INJECTIONS:	// 무조건	실행
						devState.actionSubmenu = AS_RUN_SUB_NEEDLE_WASH_AFTER;
						break;
				case WASH_BETWEEN_VIALS:
						if(devState.isNextInjection	== NO) { //	 &&	devState.doNextSample	== NO) {
							// 끝났으니	한 번	세척
								devState.actionSubmenu = AS_RUN_SUB_NEEDLE_WASH_AFTER;
						}
						else if(devState.doNextSample	== YES)	{
								devState.actionSubmenu = AS_RUN_SUB_NEEDLE_WASH_AFTER;
						}
						else { //	시간 대기	...
								devState.actionSubmenu = AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
						}
						break;
				case WASH_BETWEEN_ANALYSYS_END:	// 분석이	끝났을 때만	(한	시퀀스 끝나면	)
						if(devState.isNextInjection	== NO) { //	&& devState.doNextSample ==	NO)	{	// 끝
								devState.actionSubmenu = AS_RUN_SUB_NEEDLE_WASH_AFTER;
						}
						else { //	시간 대기	...
								devState.actionSubmenu = AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
						}
						break;
		}
		return 0;
}

// ================================================================================
// goPosVial -> goZ(vial detect) -> goSyr(uL,speed) -> goZ(top)
// 현재위치에서 추가로 흡입한다.
// MOTOR_VAL , MOTOR_INJ가 ready상태에 있다는 가정하에서 시작한다.
// ================================================================================
//uint8_t subMenu_as	=	1;
int aspirateSample(const VIAL_POS_t *pVial, float vol, uint32_t speed)
{
	int32_t	readCount=0;
	static int encoderCount = 0;

	static int error = NO;
	uint32_t needleDepth;	
	uint32_t needleDepthStep; 

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_as != Menu) {
    dfp("subMenu_as=%d\n",subMenu_as);
    Menu = subMenu_as;
}
#endif

	switch(subMenu_as) {
		case 1:
			if(goPosVial(pVial)) {
				++subMenu_as;
dfp("tray[%d] pos[%d][%d]\n",pVial->uPos.tray,pVial->uPos.x,pVial->uPos.y);			
			}
			break;
		case 2:	// Vial Detection upper go - 바이알 보다 높은 곳에서 검출되면 에러 - break,sleep
			if(readyChk(MOTOR_X) && readyChk(MOTOR_Y)) {
				if(pVial->uPos.tray == TRAY_LEFT) {
					encoderCount = goZ_Encoder(POS_Z_VIAL_LEFT_UPPER_DETECT, ACT_XY_POS);
				}
				else {
					encoderCount = goZ_Encoder(POS_Z_VIAL_RIGHT_UPPER_DETECT, ACT_XY_POS);
				}
				++subMenu_as;
				devSet.posCheckEnable = YES;
				devSet.posCheckMotor = MOTOR_Z; 
				devSet.posCheck1 = 0x30; // 검출 x - 이동전 
				devSet.posCheck2 = 0x20; // 니들 이동 검출 
			}
			break;			
		case 3:	// Vial Detection upper 
#if 1		
			if(readyChkRetry_Z(encoderCount)) {
				devSet.posCheckEnable = NO;

				if(pVial->uPos.tray == TRAY_LEFT) {
					encoderCount = 	goZ_Encoder(POS_Z_VIAL_LEFT_DETECT, ACT_XY_POS);
				}
				else {
					encoderCount = 	goZ_Encoder(POS_Z_VIAL_RIGHT_DETECT, ACT_XY_POS);
				}
				++subMenu_as;
			}
#else
			if(readyChk(MOTOR_Z))	{
				readyChkEncoder_Z();		
				devSet.posCheckEnable = NO;

				if(pVial->uPos.tray == TRAY_LEFT) {
				encoderCount = 	goZ_Encoder(POS_Z_VIAL_LEFT_DETECT, ACT_XY_POS);
				}
				else {
				encoderCount = 	goZ_Encoder(POS_Z_VIAL_RIGHT_DETECT, ACT_XY_POS);
				}
				++subMenu_as;
			}
#endif			
			break;
		case 4:
#if 1
			if(readyChkRetry_Z(encoderCount)) {
        if((devState.btMotorState[MOTOR_Z] & 0x30) ==  0x00) {
           ++subMenu_as;
        }
        else {	// skip vial - 바이알 없음.
					error = ACTION_MENU_ERR_VIAL;
subMenu_as = 11;
        }
			}
#else
      if(readyChk(MOTOR_Z))	{
				readyChkEncoder_Z();		      
        if((devState.btMotorState[MOTOR_Z] & 0x30) ==  0x00) {
           ++subMenu_as;
        }
        else {	// skip vial - 바이알 없음.
					error = ACTION_MENU_ERR_VIAL;
subMenu_as = 11;
        }
      }
#endif      
      break;			      
		case 5:
			if(pVial->uPos.tray == TRAY_LEFT) {
				needleDepth = devSet.left_z_bottom - runSetup.needleHeight;
			}
			else {
				needleDepth = devSet.right_z_bottom - runSetup.needleHeight;
			}
      needleDepthStep = (int)(needleDepth	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);

      encoderCount = goZ_Encoder(needleDepthStep, ACT_MOVE_STEP_ABSOLUTE);
      ++subMenu_as;
      break;
    case 6:
#if 1
			if(readyChkRetry_Z(encoderCount)) {
			  ++subMenu_as;				
			}
#else			
      if(readyChk(MOTOR_Z))	{
				readyChkEncoder_Z();    // WAIT_TIME_VALUE_CASE_STATE  잇으면 다음 case로 이 
        ++subMenu_as;				
      }
#endif      
      break;
		case 7:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_as,NEXT_CASE_DELAY_RUN);
			break;
    case 8:
			encoderCount =  goSyr_Encoder(vol, 0, speed, ACT_XY_POS_ADD);	// 현재위치에서 추가로 흡입하거나 배출
      ++subMenu_as;
      break;
    case 9:
			if(readyChk(MOTOR_SYR))	{
				readyChkEncoder_Syr();
      	++subMenu_as;			
			}
			break;
		case 10:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_as, runSetup.waitTime_AfterAspirate * CONVERT_WAIT_TIME_TO_COUNT);
			break;
			
		case 11:
// 바이알이 걸렸을 경우를 생각하라.	
dfp("<<<aspirateSample --- goZ_Encoder(POS_Z_TOP, ACT_XY_POS)>>>!!\n");															
	    encoderCount = goZ_Encoder(POS_Z_TOP, ACT_XY_POS);
      ++subMenu_as;			
      break;
		case 12:
#if 1
			if(readyChkRetry_Z(encoderCount)) {
      	subMenu_as = 1;
dfp("<<<aspirateSample --- error = %d \n",error);																			
      	if(error == ACTION_MENU_ERR_VIAL) {
					devState.vialExist = NO;
      		error = NO;
      		return ACTION_MENU_ERR_VIAL;
      	}
	      else {
					devState.vialExist = YES;
	      	return ACTION_MENU_END;
	      }
			}
#else			
      if(readyChk(MOTOR_Z))	{
				readyChkEncoder_Z();      
      	subMenu_as = 1;
dfp("<<<aspirateSample --- error = %d \n",error);																			
      	if(error == ACTION_MENU_ERR_VIAL) {
					devState.vialExist = NO;
      		error = NO;
      		return ACTION_MENU_ERR_VIAL;
      	}
	      else {
					devState.vialExist = YES;
	      	return ACTION_MENU_END;
	      }
	    }
#endif	    
			break;
	}

  return ACTION_MENU_ING;	
			
}

#if 0
//uint8_t subMenu_ds	=	1;
int dispenseSample(const VIAL_POS_t *pVial, int32_t vol, uint32_t speed)
{

}
#endif

// ============================================================================
// syr : top - 항상	시린지 Top(0점)위치에	있다고 가정하에	동작함.
// syringe 와	needle tubing에는	washing(혹은 이동상)이 차있는	상태로 가정
// ============================================================================
// injector	:	LOAD(POS_INJ_LOAD)
// valve - washing port	(POS_VAL_WASHING)
// needle	-	vial pos로 이동
// XXX needle	vial - Headspace pressure
// XXX syringe - ul	 : runSetup.flushVolume
// syringe - ul	:	runSetup.injectionVolume + ( )ul : 대기	 및	시간추가
		// 2배로 하자
		// 3 x loop	volume for loop	-	100uL이하
		// 2 x loop	volume for loops - 100uL ~500uL
		// 1.5 x loop	volume for loop	-	500uL이상
// waitTime	:	runSetup.waitTime	대기
// go	z	top
// needle	-	injector port로	이동
// go	z
// syringe - ul	:	runSetup.injectionVolume + (??)ul	:	주입
// injector	:	LOAD(POS_INJ_LOAD) ->	INJECTION(POS_INJ_INJECT)
// 대기
// go	z	top
// injector	:	수집완료후 INJECTION(POS_INJ_INJECT) ->	LOAD(POS_INJ_LOAD)
// washing
// valve - washing port
// syringe - runSetup.washVolume
// needle	-	waste	port
// valve - needle	port (POS_VAL_NEEDLE)
// syringe - runSetup.washVolume - (POS_SYR_ZERO)	(ACT_SPEED_HIGH_SYR)
// micro Pump	-	Needle 외부세정
// runSetup.washTimes
// 반복	주입 sequence.injectionTimes

//#define	GO_CASE_SAMPLE_SKIP_NO			18
//uint8_t subMenu_fli	=	1;

/* =========================================================================
2017.05.30
	1.??? air segment : 5ul - reduce the amount of flush volume.
	2.Sample needle : go sample vial
	3.Syringe dispenser : aspirates -> (flush volume)
	2.Sample needle : go top
	5.??? air segment : 5ul - reduce the amount of flush volume.
	6.Syringe dispenser : aspirates -> (Sample loop * 2)
		// sampleLoopVol 의 3x(100ul이하) or 2x(100~500ul) or 1.5x(500ul초과)	
	7.Sample needle : go top	
	8.??? air segment : 5ul - reduce the amount of flush volume.	
	9.Injector: LOAD position	
	10.Sample needle : go injector
	11.Syringe dispenser : Dispense --- (sampleLoopVol + air segment)
	12.Injector: INJECT position
	13.Analysisstart
		
	4.Injector : INJECT position
	5.Syringe dispenser : aspirates --- (flush volume)
	6.Injector: LOAD position
	7.Syringe dispenser : Dispense --- (Sample loop * 1.5)
	8.Injector: INJECT position
	9.Analysisstart
	
========================================================================== */
#define	FULL_LOOP_EXTRA_SAMPLE_VOL			20			// 여분
#define	FULL_LOOP_SAMPLE_VOL_100			3				// x3
#define	FULL_LOOP_SAMPLE_VOL_500			2				// x2
#define	FULL_LOOP_SAMPLE_VOL_OVER_500			1.5			// x2

#define FLUSH_MODIFY		1				// flush와 Sample 합체
#define AIR_SEGMENT_MODIFY		1			// 맨마지막에 air삭제
#define INJECTOR_LOADING_TIME_MODIFY		1			// moving -> wait 0.5 -> inj(load) -> wait 0.5 -> syr(sam load) -> wait 0.5 -> inj(inj)
#define INJECTOR_LOADING_TIME				0.5f
#define INJECTOR_INJECTION_TIME				0.2f

int	full_LoopInjection(const VIAL_POS_t	*pVial)
{
	int32_t	readCount=0;
	static int encoderCount;
	static int retVal;

	static uint32_t	syringeSpeed;
	static float	sampleVolume;
	static float injectionVolume;	
	int	aspirateReturn;

	static float airSegmentVol = 0.0f;
	static float flushVolume = 0.0f;

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_fli !=	Menu)	{
		dfp("subMenu_fli=%d  stateSYR=%x\n",subMenu_fli,devState.btMotorState[MOTOR_SYR]);
		Menu = subMenu_fli;
}
#endif

		switch(subMenu_fli)	{
				case 1:	// sleep 상태이면	ready상태로	만든다.
						if(sleepChkVal(MOTOR_VAL)) { motorReadyCmd(MOTOR_VAL);}
						if(sleepChkVal(MOTOR_INJ)) { motorReadyCmd(MOTOR_INJ);}
						++subMenu_fli;
						break;
				case 2:
						if(readyChkInj(MOTOR_INJ)	&& readyChkVal(MOTOR_VAL))	{
								++subMenu_fli;
						}
						break;
				case 3:
						retVal= goVal_Encoder(POS_VAL_NEEDLE,0,	ACT_SPEED_MAX_VAL, ACT_XY_POS);
						++subMenu_fli;
						break;
				case 4:
						if(readyChkVal(MOTOR_VAL))	{
				readyChkEncoder_Val(); 						
								++subMenu_fli;
						}
						break;
				case 5:
						airSegmentVol = runSetup.airSegmentVol;
#if 0 //FPGA_VERSION_TRIM							
						if(runSetup.airSegment ==	YES) {
							encoderCount =	goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
						}
						else {
							encoderCount =	goSyr_Encoder(0.0f,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
						}
#else
						if(runSetup.airSegment ==	YES) {
							encoderCount =	goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
						}
#endif						
						++subMenu_fli;
						break;
				case 6:
					if(readyChk(MOTOR_SYR))	{
if(runSetup.airSegment ==	YES) {					
				readyChkEncoder_Syr();
}				
// full loop 에서는 Flush기능 없어도 될 듯하다.											
						if(sysConfig.sampleLoopVol < 100) {
							sampleVolume = sysConfig.sampleLoopVol * FULL_LOOP_SAMPLE_VOL_100;// + runSetup.flushVolume;
							injectionVolume = sampleVolume - sysConfig.sampleLoopVol / 2;// - runSetup.flushVolume ;							
						}
						else if(sysConfig.sampleLoopVol <= 500) {
							sampleVolume = sysConfig.sampleLoopVol * FULL_LOOP_SAMPLE_VOL_500;// + runSetup.flushVolume;
							injectionVolume = sampleVolume - sysConfig.sampleLoopVol / 2;// - runSetup.flushVolume ;							
						}
						else {
							sampleVolume = sysConfig.sampleLoopVol * FULL_LOOP_SAMPLE_VOL_OVER_500;// + runSetup.flushVolume;
							injectionVolume = sampleVolume - sysConfig.sampleLoopVol / 4;// - runSetup.flushVolume ;							
						}

						syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj -1];							
						flushVolume = runSetup.flushVolume;
#if FLUSH_MODIFY
						if(runSetup.flushEnable == YES) {
							sampleVolume = flushVolume + sysConfig.sampleLoopVol * FULL_LOOP_SAMPLE_VOL_100;// + flushVolume;
							injectionVolume = sampleVolume - sysConfig.sampleLoopVol / 2;// - flushVolume ;	
subMenu_fli = 11;							
						}
						else {
subMenu_fli = 11;
						}
#else				
						if(runSetup.flushEnable == YES) {
							sampleVolume = sysConfig.sampleLoopVol * FULL_LOOP_SAMPLE_VOL_100;// + flushVolume;
							injectionVolume = sampleVolume - sysConfig.sampleLoopVol / 2;// - flushVolume ;	
							++subMenu_fli;
						}
						else {
subMenu_fli = 11;
						}
#endif						
					}
					break;
//=======================================================================
// Retry
//=======================================================================
				case 7:	// flush
					aspirateReturn = aspirateSample(pVial, flushVolume, syringeSpeed);

					if(aspirateReturn	== ACTION_MENU_END)	{
//=======================================================================
// Continue						
//=======================================================================
subMenu_fli = 9; 
					}
					else if(aspirateReturn ==	ACTION_MENU_ING) {		}
					else if(aspirateReturn ==	ACTION_MENU_ERR_VIAL)	{
// Vial	없음
						if(runSetup.skipMissingSample	== YES)	{
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로
//=======================================================================
// jump
//=======================================================================
subMenu_fli	=	25;
						}
						else {
							errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
							// 제어기 응답을 기다린다.							
							subMenu_fli++;
						}
					}
					break;
				case 8:
// 제어기 응답을 기다린다.
// skip  - 다음 바이알
// retry - 다시 시도
// abort - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로														
subMenu_fli	=	25;
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
subMenu_fli = 7;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("missing %d	\n",devState.missVialResponse);
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();

subMenu_fli = 1;
							break;						
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;
				case 9:
#if 0 // FPGA_VERSION_TRIM						
						if(runSetup.airSegment ==	YES) {
						encoderCount =	goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
						}
						else {
						encoderCount =	goSyr_Encoder(0.0f,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
						}
#else
						if(runSetup.airSegment ==	YES) {
dfp("if(runSetup.airSegment ==	YES) %d	\n",devState.missVialResponse);													
							encoderCount = goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
						}
#endif
						++subMenu_fli;
						break;
				case 10:
 					if(readyChk(MOTOR_SYR))	{
if(runSetup.airSegment ==	YES) {					 					
						readyChkEncoder_Syr();
}						
						++subMenu_fli;
 					}
					break;
				case 11:	// sample
					aspirateReturn = aspirateSample(pVial, sampleVolume, syringeSpeed);
					if(aspirateReturn	== ACTION_MENU_END)	{
//=======================================================================
// Continue						
//=======================================================================
LcasState.curInjCnt++;
subMenu_fli = 13; 
					}
					else if(aspirateReturn ==	ACTION_MENU_ING) {		}
					else if(aspirateReturn ==	ACTION_MENU_ERR_VIAL)	{
// Vial	없음
						if(runSetup.skipMissingSample	== YES)	{
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로							
//=======================================================================
// jump
//=======================================================================
//devState.missVialSkip = YES;
subMenu_fli	=	25;
						}
						else {
							errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
							// 제어기 응답을 기다린다.							
							subMenu_fli++;
						}
					}
					break;
				case 12:
// 제어기 응답을 기다린다.
// skip  - 다음 바이알
// retry - 다시 시도
// abort - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로														
//devState.missVialSkip = YES;							
subMenu_fli	=	25;		
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
subMenu_fli = 11;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("missing %d	\n",devState.missVialResponse);
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();
subMenu_fli = 1;
							break;						
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;			
//=======================================================================
// Continue
//=======================================================================
				case 13:	// full_loop - 마지막에 airSegment사용한다.
					if(runSetup.airSegment ==	YES) {
//#if AIR_SEGMENT_MODIFY
//						goSyr(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출

					}
					++subMenu_fli;
					break;
				case 14:
 					if(readyChk(MOTOR_SYR))	{
						if(goPosInjection()) {		
							usedTime.injectionCnt++;	// 인젝터포트 사용횟수 																
							++subMenu_fli;
						}
 					}
					break;
				case 15:
					WAIT_TIME_VALUE_CASE_STATE(subMenu_fli,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
					break;
				case 16:
				retVal = 	goInj_Encoder(POS_INJ_LOAD,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);
					++subMenu_fli;
					break;
				case 17:
 					if(readyChkInj(MOTOR_INJ))	{
readyChkEncoder_Inj();
++subMenu_fli;
//						WAIT_TIME_VALUE_CASE_STATE(subMenu_fli,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
					}
					break;
case 18:
WAIT_TIME_VALUE_CASE_STATE(subMenu_fli,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
break;

				case 19:
		encoderCount = 			goSyr_Encoder(-injectionVolume, 0, syringeSpeed, ACT_XY_POS_ADD);	// 현재위치에서	추가로 배출		
					++subMenu_fli;
					break;
				case 20:
					if(readyChk(MOTOR_SYR))	{			
readyChkEncoder_Syr();					
++subMenu_fli;
//				WAIT_TIME_VALUE_CASE_STATE(subMenu_fli,(INJECTOR_INJECTION_TIME * CONVERT_WAIT_TIME_TO_COUNT));					
					}
					break;
case 21:
WAIT_TIME_VALUE_CASE_STATE(subMenu_fli,(INJECTOR_INJECTION_TIME * CONVERT_WAIT_TIME_TO_COUNT));					
break;

//====================================================================================
// Start !!!!!!
//====================================================================================
				case 22:	// 인젝터	-	POS_INJ_INJECT
				retVal = goInj_Encoder(POS_INJ_INJECT,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);	// Start !!!!!!

						startInjectionFunction();

						++subMenu_fli;
						break;
				case 23:
						if(readyChkInj(MOTOR_INJ))	{
readyChkEncoder_Inj();
++subMenu_fli;
//							WAIT_TIME_VALUE_CASE_STATE(subMenu_fli,(runSetup.waitTime_AfterDispense * CONVERT_WAIT_TIME_TO_COUNT));		
						}
						break;
case 24:
WAIT_TIME_VALUE_CASE_STATE(subMenu_fli,(runSetup.waitTime_AfterDispense * CONVERT_WAIT_TIME_TO_COUNT));		
break;
//=======================================================================
// jump
//=======================================================================
				case 25:// 여기로	바이알 없음
					if(devState.curStep[MOTOR_SYR] ==	0) {	// 버릴것이	없음
						subMenu_fli = 28;					
					}
					else {
						subMenu_fli++;
					}
					break;
				case 26:
					if(goPosWastePort()) {					
//						goSyr(0, 0, syringeSpeed, ACT_XY_POS);
		encoderCount = goSyr_Encoder(POS_SYR_ZERO, 0, syringeSpeed, ACT_XY_POS);
						++subMenu_fli;
					}
					break;
				case 27:
					if(readyChk(MOTOR_SYR))	{
#if SYRINGE_ENCODER_REMOVE_END
#else
readyChkEncoder_Syr();						
#endif
						++subMenu_fli;
					}
					break;
				case 28:
// 동작	완료인가?
// INJ , VAL : Sleep mode로
						if(readyChk(MOTOR_Z))	{
								devState.curInjCnt++;
								subMenu_fli	=	1;
								return ACTION_MENU_END;
						}
						break;

		}
		return ACTION_MENU_ING;
}

#define INJECTOR_DEAD_VOLUME				0.6974
#define	PARTIAL_LOOP_EXTRA_SAMPLE_VOL_1ST			5.0f			// Loading 전 여분 - INJECTOR_DEAD_VOLUME 포함
#define	PARTIAL_LOOP_EXTRA_SAMPLE_VOL_2ND			5.0f			// Loading하고 남는 시료량 정도로 표현하면 될까?

/* =========================================================================
2017.05.26
	1.??? air segment : 5ul - reduce the amount of flush volume.
	2.Sample needle : go sample vial
	3.Syringe dispenser : aspirates -> (flush volume)
	4.Sample needle : go top
	5.??? air segment : 5ul - reduce the amount of flush volume.
	6.Syringe dispenser : aspirates --- (injectionVolume + PARTIAL_LOOP_EXTRA_SAMPLE_VOL )
	7.Sample needle : go top	
	8.??? air segment : 5ul - reduce the amount of flush volume.	
	9.Injector: LOAD position	
	10.Sample needle : go injector
	11.Syringe dispenser : Dispense --- (injectionVolume + air segment)
	12.Injector: INJECT position
	13.Analysisstart
========================================================================== */

int	partial_LoopInjection(const	VIAL_POS_t *pVial)
{
	int32_t	readCount=0;
	static int encoderCount;
	static int retVal;
	
	static uint32_t	syringeSpeed;
	static float	sampleVolume;	// 흡입량
	static float injectionVolume;	// 인젝터에 인젝션량
	int	aspirateReturn;

	static float airSegmentVol = 0.0f;
	static float flushVolume = 0.0f;

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_pli !=	Menu)	{
		dfp("subMenu_pli=%d\n",subMenu_pli);
		Menu = subMenu_pli;
}
#endif

		switch(subMenu_pli)	{
				case 1:	// sleep 상태이면	ready상태로	만든다.
						if(sleepChkVal(MOTOR_VAL)) { motorReadyCmd(MOTOR_VAL);}
						if(sleepChkVal(MOTOR_INJ)) { motorReadyCmd(MOTOR_INJ);}
						++subMenu_pli;
						break;
				case 2:
						if(readyChkInj(MOTOR_INJ)	&& readyChkVal(MOTOR_VAL))	{
							++subMenu_pli;
						}
						break;
				case 3:	// Valve : Needle	,	Inj	:	LOAD
						retVal = goVal_Encoder(POS_VAL_NEEDLE,	0,ACT_SPEED_MAX_VAL, ACT_XY_POS);
						++subMenu_pli;
						break;
				case 4:
						if(readyChkVal(MOTOR_VAL))	{
				readyChkEncoder_Val(); 												
								++subMenu_pli;
						}
						break;
				case 5:
						syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1];					
						airSegmentVol = runSetup.airSegmentVol;
#if 0 //FPGA_VERSION_TRIM												
						if(runSetup.airSegment ==	YES) {	// 첫번째 에어세그먼트
					encoderCount =		goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
//							injectionVolume = runSetup.injectionVolume;
						}
						else {
					// encoderCount = 0;
					encoderCount =		goSyr_Encoder(0.0f,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
//							injectionVolume = runSetup.injectionVolume;
						}
#else						
						if(runSetup.airSegment ==	YES) {	// 첫번째 에어세그먼트
							encoderCount =		goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
//							injectionVolume = runSetup.injectionVolume;
						}
#endif
						++subMenu_pli;
						break;
				case 6:
#if FLUSH_MODIFY
// air -> flush + injVol + extra
					if(readyChk(MOTOR_SYR))	{
						if(runSetup.airSegment ==	YES) {										
										readyChkEncoder_Syr();						
						}				
//						syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj];
						flushVolume = runSetup.flushVolume;
						if(runSetup.flushEnable == YES) {
							sampleVolume = flushVolume + runSetup.injectionVolume + PARTIAL_LOOP_EXTRA_SAMPLE_VOL_1ST + PARTIAL_LOOP_EXTRA_SAMPLE_VOL_2ND;
							subMenu_pli = 11;			
						}
						else {
							sampleVolume = runSetup.injectionVolume + PARTIAL_LOOP_EXTRA_SAMPLE_VOL_1ST + PARTIAL_LOOP_EXTRA_SAMPLE_VOL_2ND;
							subMenu_pli = 11;
						}
					}
#else
// air -> flush -> air -> injVol + extra
					if(readyChk(MOTOR_SYR))	{
				readyChkEncoder_Syr();												
//						syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj];
						flushVolume = runSetup.flushVolume;
						sampleVolume = runSetup.injectionVolume + PARTIAL_LOOP_EXTRA_SAMPLE_VOL_1ST + PARTIAL_LOOP_EXTRA_SAMPLE_VOL_2ND;
						if(runSetup.flushEnable == YES) {
							subMenu_pli++;
						}
						else {
							subMenu_pli = 11;
						}
					}
#endif
					break;
				case 7:
					aspirateReturn = aspirateSample(pVial, flushVolume, syringeSpeed);
					if(aspirateReturn	== ACTION_MENU_END)	{
						subMenu_pli = 9;
					}
					else if(aspirateReturn ==	ACTION_MENU_ING) {		}
					else if(aspirateReturn ==	ACTION_MENU_ERR_VIAL)	{
						// Vial	없음
						if(runSetup.skipMissingSample	== YES)	{
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로
//=======================================================================
// jump
//=======================================================================
subMenu_pli	=	28;
						}							
						else {
							errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
// 제어기 응답을 기다린다.														
							subMenu_pli++;
						}
					}
					break;
				case 8:
// 제어기 응답을 기다린다.// [skip]  - 다음 바이알 [retry] - 다시 시도  [abort] - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로														
subMenu_pli	=	28;
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
							subMenu_pli = 7;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("missing %d	\n",devState.missVialResponse);													
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();
subMenu_pli = 1;
							break;
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;
				case 9:
#if 0 //FPGA_VERSION_TRIM
						if(runSetup.airSegment ==	YES) {	// 두번째 에어세그먼트 - flush Enable
							encoderCount = goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
							++subMenu_pli;							
						}
						else {
							encoderCount = goSyr_Encoder(0.0f,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출						
							subMenu_pli++;
						}
#else
						if(runSetup.airSegment ==	YES) {	// 두번째 에어세그먼트 - flush Enable
							encoderCount = goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
						}
						++subMenu_pli;
#endif
						break;
				case 10:
 					if(readyChk(MOTOR_SYR))	{
if(runSetup.airSegment ==	YES) {		 					
						readyChkEncoder_Syr(); 	
}						
						++subMenu_pli;
 					}
					break;

 				case 11:
//					aspirateReturn = aspirateSample(pVial, runSetup.injectionVolume + PARTIAL_LOOP_EXTRA_SAMPLE_VOL, syringeSpeed);
					aspirateReturn = aspirateSample(pVial, sampleVolume, syringeSpeed);
					if(aspirateReturn	== ACTION_MENU_END)	{
LcasState.curInjCnt++;
						subMenu_pli = 13;
					}
					else if(aspirateReturn ==	ACTION_MENU_ING) {		}
					else if(aspirateReturn ==	ACTION_MENU_ERR_VIAL)	{
						// Vial	없음
						if(runSetup.skipMissingSample	== YES)	{
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로
//=======================================================================
// jump
//=======================================================================
subMenu_pli	=	28;
						}							
						else {
							errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
// 제어기 응답을 기다린다.
							subMenu_pli++;
						}
					}
					break;
				case 12:
// 제어기 응답을 기다린다.// [skip]  - 다음 바이알 [retry] - 다시 시도  [abort] - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로

subMenu_pli	=	28;
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
							subMenu_pli = 11;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("missing %d	\n",devState.missVialResponse);
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();
subMenu_pli = 1;
							break;						
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;
//=======================================================================
// Continue						
//=======================================================================
				case 13:	// partial_loop - 마지막 airSegment사용할 것인지?
					injectionVolume = runSetup.injectionVolume;
//#if AIR_SEGMENT_MODIFY
					++subMenu_pli;
					break;
				case 14:
 					if(readyChk(MOTOR_SYR))	{
							if(goPosInjection()) {		
								usedTime.injectionCnt++;	// 인젝터포트 사용횟수 																
								++subMenu_pli;
dfp(">>>>injectVial=(%d,%d) sample=(%d,%d)\n",LcasState.injectVial.uPos.x,LcasState.injectVial.uPos.y,LcasState.sample.uPos.x,LcasState.sample.uPos.y);													
							}
 					}
					break;
				case 15:
					WAIT_TIME_VALUE_CASE_STATE(subMenu_pli,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));					
					break;


					
				case 16:
			encoderCount = goSyr_Encoder(-(PARTIAL_LOOP_EXTRA_SAMPLE_VOL_1ST), 0, syringeSpeed, ACT_XY_POS_ADD);	// 현재위치에서	추가로 배출		
					++subMenu_pli;
					break;
case 17:
//*********** 
// 20200517
// 추가 
//***********
					if(readyChk(MOTOR_SYR))	{			
readyChkEncoder_Syr();					
++subMenu_pli;
}
break;

				case 18:
					WAIT_TIME_VALUE_CASE_STATE(subMenu_pli,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));					
					break;
				case 19:
			retVal =		goInj_Encoder(POS_INJ_LOAD,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);
					++subMenu_pli;
					break;
				case 20:
 					if(readyChkInj(MOTOR_INJ))	{
readyChkEncoder_Inj();					
++subMenu_pli;
//						WAIT_TIME_VALUE_CASE_STATE(subMenu_pli,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
					}
					break;

case 21:
WAIT_TIME_VALUE_CASE_STATE(subMenu_pli,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
break;

				case 22:
		encoderCount = goSyr_Encoder(-injectionVolume, 0, syringeSpeed, ACT_XY_POS_ADD);	// 현재위치에서	추가로 배출		
					++subMenu_pli;
					break;
				case 23:
					if(readyChk(MOTOR_SYR))	{	
readyChkEncoder_Syr();					
++subMenu_pli;
					}
					break;

case 24:
WAIT_TIME_VALUE_CASE_STATE(subMenu_pli,(INJECTOR_INJECTION_TIME * CONVERT_WAIT_TIME_TO_COUNT));					
break;


//====================================================================================
// Start !!!!!!
//====================================================================================
				case 25:// 인젝터	-	POS_INJ_INJECT
			retVal = goInj_Encoder(POS_INJ_INJECT,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);	// Start !!!!!!

						startInjectionFunction();

dfp(">>>>>injectVial=(%d,%d) sample=(%d,%d)\n",LcasState.injectVial.uPos.x,LcasState.injectVial.uPos.y,LcasState.sample.uPos.x,LcasState.sample.uPos.y);
						++subMenu_pli;
						break;
				case 26:
						if(readyChkInj(MOTOR_INJ))	{
readyChkEncoder_Inj();					
++subMenu_pli;
//							WAIT_TIME_VALUE_CASE_STATE(subMenu_pli,(runSetup.waitTime_AfterDispense * CONVERT_WAIT_TIME_TO_COUNT));								
						}
						break;
case 27:
WAIT_TIME_VALUE_CASE_STATE(subMenu_pli,(runSetup.waitTime_AfterDispense * CONVERT_WAIT_TIME_TO_COUNT));								
break;

//=======================================================================
// jump
//=======================================================================
				case 28:// 여기로	바이알 없음
					if(devState.curStep[MOTOR_SYR] ==	0) {	// 버릴것이	없음
subMenu_pli = 31;
					}
					else {
						subMenu_pli++;
					}
					break;
				case 29:
					if(goPosWastePort()) {					
			encoderCount = goSyr_Encoder(POS_SYR_ZERO, 0, syringeSpeed, ACT_XY_POS);
						++subMenu_pli;
					}
					break;
				case 30:
					if(readyChk(MOTOR_SYR))	{
#if SYRINGE_ENCODER_REMOVE_END
#else
readyChkEncoder_Syr();
#endif
						++subMenu_pli;
					}
					break;
				case 31:
					encoderCount = goZ_Encoder(POS_Z_TOP, ACT_XY_POS);
					++subMenu_pli;
					break;
				case 32:
// 동작	완료인가?
// INJ , VAL : Sleep mode로
					if(readyChkRetry_Z(encoderCount)) {
//						if(readyChk(MOTOR_Z))	{
//readyChkEncoder_Z();
						devState.curInjCnt++;
						subMenu_pli	=	1;
						return ACTION_MENU_END;
					}
					break;

		}
		return ACTION_MENU_ING;
}


// ==================================================================
// zero	sample loss
// ============================================================================
// syr : top - 항상	시린지 Top(0점)위치에	있다고 가정하에	동작함.
// syringe 와	needle tubing에는	washing(혹은 이동상)이 차있는	상태로 가정
// ============================================================================
// Valve : washing port
// Needle	,	Inj	:	LOAD
// ==================================================================
//uint8_t subMenu_mpi	=	1;

/* =========================================================================
2017.05.25
1)===============
	1.??? air segment : 5ul - reduce the amount of flush volume.
	2.Sample needle : go transport vial
	3.Syringe dispenser : aspirates --- (Transport Volume / 2)
		(Sample loop - Sample Volume + TRANSPORT_EXTRA_VOLUME)
	4.Sample needle : go sample vial
	5.Syringe dispenser : aspirates --- (Sample Volume)
	6.Sample needle : go transport vial	
	7.Syringe dispenser : aspirates --- (Transport Volume / 2)
		(Sample loop - Sample Volume + TRANSPORT_EXTRA_VOLUME)
	
//	4.Injector : INJECT position
//	5.Syringe dispenser : aspirates --- (flush volume)

	6.Injector: LOAD position
	7.Syringe dispenser : Dispense --- (Transport Volume - TRANSPORT_EXTRA_VOLUME / 2)
	8.Injector: INJECT position
	9.Analysisstart

2017.05.26
2)===================


	1.??? air segment : 5ul - reduce the amount of flush volume.
	2.Sample needle : go transport vial
	3.Syringe dispenser : aspirates --- (Transport Volume / 2)
		(Sample loop - Sample Volume + TRANSPORT_EXTRA_VOLUME)
	4.Sample needle : go sample vial
	5.Syringe dispenser : aspirates --- (Sample Volume)

	1.??? air segment : 5ul - reduce the amount of flush volume.
	6.Injector: LOAD position
	7.Syringe dispenser : Dispense --- (Transport Volume / 2 + Sample Volume - TRANSPORT_EXTRA_VOLUME / 2)
	8.Injector: INJECT position
	9.Analysisstart

2017.05.29
3)===================
	1. Valve : Washing Port
	2. Syringe dispenser : aspirates -> (needleTubeVol + transportVolume + TRANSPORT_EXTRA_VOLUME)
	3. Valve : Needle Port
	4. Syringe dispenser : Dispense -> (needleTubeVol)
	
	5. air segment : 5ul - reduce the amount of flush volume.
			aspirates -> airSegmentVol
	6. Sample needle : go sample vial
	7.Syringe dispenser : aspirates -> (Sample Volume)	: aspirateSample()
	8. air segment : 5ul - aspirates -> airSegmentVol
	
	9. Sample needle : go Injector
	
	10.Injector: LOAD position
	11.Syringe dispenser : Dispense -> injectionVolume = (sysConfig.sampleLoopVol) - transportVolume;
	8.Injector: INJECT position
	9.Analysisstart

========================================================================== */
#define	TRANSPORT_EXTRA_VOLUME			10.0f
//#define	TRANSPORT_EXTRA_VOLUME			2
#define MICRO_PICK_TRANSFORT_VOL		5.0f	// INJECTOR_DEAD_VOLUME 포함. TRANSPORT_EXTRA_VOLUME보다 작아야 
//#define MICRO_PICK_TRANSFORT_VOL		9.0f	// INJECTOR_DEAD_VOLUME 포함. TRANSPORT_EXTRA_VOLUME보다 작아야 
#define MICRO_INJECTION_MODIFY	1					// transportVolume 주입 차이

#if TRANSPORT_LIQUID_USE
#define GO_VIAL_NOT_EXIST_mpi_no		22
int	micro_pick_Injection(const VIAL_POS_t	*pVial)
{
	int32_t	readCount=0;
	static int encoderCount;
	static int retVal;

	static uint32_t	syringeSpeed;
	static float	sampleVolume;
	static float transportVolume;
	static float injectionVolume;
	int	aspirateReturn;

	static float airSegmentVol = 0.0f;
	static float flushVolume = 0.0f;

	static int transVialPos;

VIAL_POS_t transVial;

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_mpi !=	Menu)	{
		dfp("subMenu_mpi=%d\n",subMenu_mpi);
		Menu = subMenu_mpi;
}
#endif

		switch(subMenu_mpi)	{
				case 1:	// sleep 상태이면	ready상태로	만든다.
					if(sleepChkVal(MOTOR_VAL)) { motorReadyCmd(MOTOR_VAL);}
					if(sleepChkVal(MOTOR_INJ)) { motorReadyCmd(MOTOR_INJ);}
					++subMenu_mpi;
					break;
				case 2:
					if(readyChkInj(MOTOR_INJ)	&& readyChkVal(MOTOR_VAL))	{
						syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1];
						++subMenu_mpi;
					}
					break;
				case 3:
				retVal =	goVal_Encoder(POS_VAL_NEEDLE,	0,ACT_SPEED_MAX_VAL, ACT_XY_POS);
					++subMenu_mpi;
					break;
				case 4:
					if(readyChkVal(MOTOR_VAL))	{
				readyChkEncoder_Val();					
						++subMenu_mpi;
					}
					break;
				case 5:
					syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1];
					airSegmentVol = runSetup.airSegmentVol;
#if 0 //FPGA_VERSION_TRIM					
					if(runSetup.airSegment ==	YES) {	// 첫번째 에어세그먼트
						encoderCount = goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
					}
					else {
						encoderCount = goSyr_Encoder(0.0f,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출					
					}
#else
					if(runSetup.airSegment ==	YES) {	// 첫번째 에어세그먼트
				encoderCount = goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
					}
#endif
					++subMenu_mpi;
					break;
				case 6:
					if(readyChk(MOTOR_SYR))	{
if(runSetup.airSegment ==	YES) {							
readyChkEncoder_Syr();						
}
//					sampleVolume = runSetup.injectionVolume;
						transportVolume = TRANSPORT_EXTRA_VOLUME;
//					transportVolume = MICRO_PICK_TRANSFORT_VOL
//					injectionVolume	=	sampleVolume + MICRO_PICK_TRANSFORT_VOL;
						injectionVolume	=	MICRO_PICK_TRANSFORT_VOL;

						transVial.uPos.tray = pVial->uPos.tray;
						transVial.uPos.etc = pVial->uPos.etc;
						if(pVial->uPos.tray == TRAY_LEFT) {
							transVial.uPos.x = devSet.trayLeft_rowNo - 1;
							transVial.uPos.y = devSet.trayLeft_colNo - 1;
						}
						else {
							transVial.uPos.x = devSet.trayRight_rowNo - 1;
							transVial.uPos.y = devSet.trayRight_colNo - 1;
						}

						transVialPos = transVial.pos;  // 저장 
		
						++subMenu_mpi;

					}
					break;
				case 7: // Transport
					transVial.pos = transVialPos;	  // 불러오기
					aspirateReturn = aspirateSample(&transVial, transportVolume, syringeSpeed);
//					aspirateReturn = aspirateSample(&transVialPos, sampleVolume, syringeSpeed);
					
					if(aspirateReturn	== ACTION_MENU_END)	{
//						LcasState.curInjCnt++;
						subMenu_mpi = 9; 
					}
					else if(aspirateReturn ==	ACTION_MENU_ING) {		}
					else if(aspirateReturn ==	ACTION_MENU_ERR_VIAL)	{
						// Vial	없음
						if(runSetup.skipMissingSample	== YES)	{
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로
//=======================================================================
// jump
//=======================================================================
//devState.missVialSkip = YES;
							subMenu_mpi	=	GO_VIAL_NOT_EXIST_mpi_no;	// 배출후 다음바이알
						}
						else {
							errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
							subMenu_mpi++;
						}
					}
					break;
				case 8:
// 제어기 응답을 기다린다.// [skip]  - 다음 바이알 [retry] - 다시 시도  [abort] - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로														
							subMenu_mpi	=	GO_VIAL_NOT_EXIST_mpi_no;
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
							subMenu_mpi = 7;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("missing %d	\n",devState.missVialResponse);
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();
subMenu_mpi = 1;
							break;
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;
				case 9:
#if 0 //FPGA_VERSION_TRIM
					if(runSetup.airSegment ==	YES) {	// 첫번째 에어세그먼트
						encoderCount =				goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
						injectionVolume += airSegmentVol;
					}
					else {
						encoderCount =				goSyr_Encoder(0.0f,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
					}
#else
					if(runSetup.airSegment ==	YES) {	// 첫번째 에어세그먼트
						encoderCount =				goSyr_Encoder(airSegmentVol,	0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
						injectionVolume += airSegmentVol;
					}
#endif
					sampleVolume = runSetup.injectionVolume;
					injectionVolume += sampleVolume;
					++subMenu_mpi;
					break;
				case 10:
 					if(readyChk(MOTOR_SYR))	{
if(runSetup.airSegment ==	YES) {		 					
readyChkEncoder_Syr();  					
}
						++subMenu_mpi;
 					}
					break;
				case 11:	// Sample
					aspirateReturn = aspirateSample(pVial, sampleVolume, syringeSpeed);
					if(aspirateReturn	== ACTION_MENU_END)	{
LcasState.curInjCnt++;
						subMenu_mpi = 13;
					}
					else if(aspirateReturn ==	ACTION_MENU_ING) {		}
					else if(aspirateReturn ==	ACTION_MENU_ERR_VIAL)	{
						// Vial	없음
						if(runSetup.skipMissingSample	== YES)	{
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로
//=======================================================================
// jump
//=======================================================================
							subMenu_mpi	=	GO_VIAL_NOT_EXIST_mpi_no;
						}							
						else {
							errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
// 제어기 응답을 기다린다.
							subMenu_mpi++;
						}
					}
					break;
				case 12:
// 제어기 응답을 기다린다.// [skip]  - 다음 바이알 [retry] - 다시 시도  [abort] - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로

							subMenu_mpi	=	GO_VIAL_NOT_EXIST_mpi_no;
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
							subMenu_mpi = 11;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("missing %d	\n",devState.missVialResponse);
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();
subMenu_mpi = 1;
							break;						
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;
				case 13:	// air segment - needle(z)는 현재 TOP 
					if(readyChk(MOTOR_Z))	{
//#if AIR_SEGMENT_MODIFY					// 맨마지막에 air삭제			
						++subMenu_mpi;
					}
					break;
				case 14:	// 인젝터로	이동
					if(readyChk(MOTOR_SYR))	{
						if(goPosInjection()) {		
								usedTime.injectionCnt++;	// 인젝터포트 사용횟수 																
								++subMenu_mpi;
						}
					}
					break;
				case 15:
					if(readyChk(MOTOR_X) &&	readyChk(MOTOR_Y) && readyChk(MOTOR_Z))	{
							//WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
++subMenu_mpi;							
					}
					break;
case 16:
WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
break;

				case 17:
				retVal =	goInj_Encoder(POS_INJ_LOAD,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);
					++subMenu_mpi;
					break;
				case 18:
					if(readyChkInj(MOTOR_INJ)) {
readyChkEncoder_Inj();						
++subMenu_mpi;						
							//WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
					}
					break;
case 19:
WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
break;

				case 20:
					syringeSpeed = devSet.syrSpeedInj[runSetup.syringeUpSpdInj - 1];
			encoderCount =		goSyr_Encoder(-injectionVolume,0,	syringeSpeed,	ACT_XY_POS_ADD);
					++subMenu_mpi;
					break;
				case 21:
						if(readyChk(MOTOR_SYR))	{
readyChkEncoder_Syr();						
++subMenu_mpi;						
							//WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(INJECTOR_INJECTION_TIME * CONVERT_WAIT_TIME_TO_COUNT));					
						}
						break;
case 22:
WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(INJECTOR_INJECTION_TIME * CONVERT_WAIT_TIME_TO_COUNT));					
break;
//====================================================================================
// Start !!!!!!
//====================================================================================
				case 23:	// 인젝터	-	POS_INJ_INJECT
			retVal =		goInj_Encoder(POS_INJ_INJECT,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);	// Start !!!!!!
					startInjectionFunction();
					++subMenu_mpi;
					break;
					
				case 24:
						if(readyChkInj(MOTOR_INJ))	{
readyChkEncoder_Inj();							
++subMenu_mpi;						
							//WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(runSetup.waitTime_AfterDispense * CONVERT_WAIT_TIME_TO_COUNT));																
						}
						break;
case 25:
WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(runSetup.waitTime_AfterDispense * CONVERT_WAIT_TIME_TO_COUNT));																
break;

//=======================================================================
// jump
//=======================================================================
//#define GO_VIAL_NOT_EXIST_mpi_no		22
				case 26:// 여기로	바이알 없음
					if(devState.curStep[MOTOR_SYR] ==	0) {	// 버릴것이	없음
subMenu_mpi = 29;
					}
					else {
						subMenu_mpi++;
					}
					break;
				case 27:
					if(goPosWastePort()) {					
				encoderCount =			goSyr_Encoder(POS_SYR_ZERO, 0, syringeSpeed, ACT_XY_POS);						
						++subMenu_mpi;
					}
					break;
				case 28:
					if(readyChk(MOTOR_SYR))	{
#if SYRINGE_ENCODER_REMOVE_END
#else
readyChkEncoder_Syr();						
#endif
						++subMenu_mpi;
					}
					break;
				case 29:	// 바이알	없을 경우	여기로 - 현재	시린지가 TOP에 있지	않을 수도	있다.
			encoderCount = goZ_Encoder(POS_Z_TOP, ACT_XY_POS);
					++subMenu_mpi;
					break;
				case 30:
// 동작	완료인가 - 현재	시린지가 TOP에 있지	않을 수도	있다.
					if(readyChkRetry_Z(encoderCount)) {
//					if(readyChk(MOTOR_Z))	{
						//readyChkEncoder_Z();
//dfp("==========sampleVolume=%d  transportVolume=%d injectionVolume=%d\n",sampleVolume,(int)(transportVolume*10),(int)(injectionVolume*10));
							devState.curInjCnt++;
							subMenu_mpi	=	1;
							return ACTION_MENU_END;
					}
					break;
		}
		return ACTION_MENU_ING;
}
#else
int	micro_pick_Injection(const VIAL_POS_t	*pVial)
{
		static uint32_t	syringeSpeed;
		static float	sampleVolume;
//		static float transportVolume;
		static float injectionVolume;
		int	aspirateReturn;

	static float airSegmentVol = 0.0f;
	static float flushVolume = 0.0f;

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_mpi !=	Menu)	{
		dfp("subMenu_mpi=%d\n",subMenu_mpi);
		Menu = subMenu_mpi;
}
#endif

		switch(subMenu_mpi)	{
				case 1:	// sleep 상태이면	ready상태로	만든다.
						if(sleepChkVal(MOTOR_VAL)) { motorReadyCmd(MOTOR_VAL);}
						if(sleepChkVal(MOTOR_INJ)) { motorReadyCmd(MOTOR_INJ);}
						++subMenu_mpi;
						break;
				case 2:
						if(readyChkInj(MOTOR_INJ)	&& readyChkVal(MOTOR_VAL))	{
//							sampleVolume = runSetup.injectionVolume;
							syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1];
//							injectionVolume	=	sampleVolume + MICRO_PICK_TRANSFORT_VOL;
							++subMenu_mpi;
						}
						break;
				case 3:
						goVal(POS_VAL_WASHING, 0,ACT_SPEED_MAX_VAL,	ACT_XY_POS);
						++subMenu_mpi;
						break;
				case 4: // washing액 흡입 
						if(readyChkVal(MOTOR_VAL))	{
							goSyr((sysConfig.needleTubeVol + TRANSPORT_EXTRA_VOLUME * 2.0f) , 0, syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입							
							++subMenu_mpi;
						}
						break;
				case 5:
					if(readyChk(MOTOR_SYR))	{
						goVal(POS_VAL_NEEDLE,	0,ACT_SPEED_MAX_VAL, ACT_XY_POS);
						++subMenu_mpi;
					}
					break;
				case 6:
					if(readyChkInj(MOTOR_VAL))	{
						++subMenu_mpi;
					}
					break;

				case 7:
					if(goPosWastePort()) {
						// TRANSPORT_EXTRA_VOLUME만 남기고 배출
						goSyr((-sysConfig.needleTubeVol - TRANSPORT_EXTRA_VOLUME) , 0, syringeSpeed,	ACT_XY_POS_ADD);	
						++subMenu_mpi;
					}
					break;
				case 8:
					if(readyChk(MOTOR_SYR))	{
						++subMenu_mpi;
					}
					break;
				case 9:
						goZ(POS_Z_TOP, ACT_XY_POS);
						++subMenu_mpi;
						break;
				case 10:
					if(readyChk(MOTOR_Z))	{
#if MICRO_INJECTION_MODIFY
						airSegmentVol = runSetup.airSegmentVol;			
						sampleVolume = runSetup.injectionVolume;
						injectionVolume	=	sampleVolume + MICRO_PICK_TRANSFORT_VOL;
//						syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj];
						if(runSetup.airSegment ==	YES) {
							goSyr(airSegmentVol,	0,syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입
							injectionVolume += airSegmentVol;
						}
						++subMenu_mpi;
#else
						airSegmentVol = runSetup.airSegmentVol;
						sampleVolume = runSetup.injectionVolume;
						injectionVolume	=	sampleVolume + MICRO_PICK_TRANSFORT_VOL;
//						syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj];
						if(runSetup.airSegment ==	YES) {
							goSyr(airSegmentVol,	0,syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입
							injectionVolume += airSegmentVol;
						}
						++subMenu_mpi;
#endif
					}
					break;
				case 11:
					if(readyChk(MOTOR_SYR))	{
						++subMenu_mpi;
					}
					break;
				case 12:
					aspirateReturn = aspirateSample(pVial, sampleVolume, syringeSpeed);
					
//					injectionVolume = injectionVolume + sampleVolume;
					if(aspirateReturn	== ACTION_MENU_END)	{
LcasState.curInjCnt++;						
						subMenu_mpi = 14; 
					}
					else if(aspirateReturn ==	ACTION_MENU_ING) {		}
					else if(aspirateReturn ==	ACTION_MENU_ERR_VIAL)	{
						// Vial	없음
						if(runSetup.skipMissingSample	== YES)	{
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로
//=======================================================================
// jump
//=======================================================================
//devState.missVialSkip = YES;
							subMenu_mpi	=	23;	// 배출후 다음바이알
						}
						else {
							errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
							subMenu_mpi++;
						}
					}
					break;
case 13:
// 제어기 응답을 기다린다.// [skip]  - 다음 바이알 [retry] - 다시 시도  [abort] - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
							devState.curInjCnt = devState.injectionTimes;		// 남은	주입 모두	Cancel - 다음	Sample로														
//devState.missVialSkip = YES;							
							subMenu_mpi	=	23; // 배출후 다음바이알
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
							subMenu_mpi = 12;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("missing %d	\n",devState.missVialResponse);													
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();
subMenu_mpi = 1;
							break;						
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;
				case 14:	// air segment - needle(z)는 현재 TOP 
					if(readyChk(MOTOR_Z))	{
						if(runSetup.airSegment ==	YES) {
//#if AIR_SEGMENT_MODIFY					// 맨마지막에 air삭제			
						}
						++subMenu_mpi;
					}
					break;
				case 15:	// 인젝터로	이동
					if(readyChk(MOTOR_SYR))	{
						if(goPosInjection()) {		
								usedTime.injectionCnt++;	// 인젝터포트 사용횟수 																
								++subMenu_mpi;
						}
					}
					break;
				case 16:
					if(readyChk(MOTOR_X) &&	readyChk(MOTOR_Y) && readyChk(MOTOR_Z))	{
							WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
					}
					break;
				case 17:
					goInj(POS_INJ_LOAD,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);
					++subMenu_mpi;
					break;
				case 18:
					if(readyChkInj(MOTOR_INJ)) {
							WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(INJECTOR_LOADING_TIME * CONVERT_WAIT_TIME_TO_COUNT));
					}
					break;
				case 19:
					syringeSpeed = devSet.syrSpeedInj[runSetup.syringeUpSpdInj-1];
					goSyr(-injectionVolume,0,	syringeSpeed,	ACT_XY_POS_ADD);
					++subMenu_mpi;
					break;
				case 20:
						if(readyChk(MOTOR_SYR))	{					
							WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(INJECTOR_INJECTION_TIME * CONVERT_WAIT_TIME_TO_COUNT));					
						}
						break;
//====================================================================================
// Start !!!!!!
//====================================================================================
				case 21:	// 인젝터	-	POS_INJ_INJECT
//						if(readyChk(MOTOR_SYR))	{
								goInj(POS_INJ_INJECT,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);	// Start !!!!!!

								startInjectionFunction();

								++subMenu_mpi;
//						}
						break;
				case 22:
						if(readyChkInj(MOTOR_INJ))	{
							WAIT_TIME_VALUE_CASE_STATE(subMenu_mpi,(runSetup.waitTime_AfterDispense * CONVERT_WAIT_TIME_TO_COUNT));																
						}
						break;
						
//=======================================================================
// jump
//=======================================================================
				case 23:// 여기로	바이알 없음
					if(goPosWastePort()) {					
						goSyr(POS_SYR_ZERO, 0, syringeSpeed, ACT_XY_POS);						
						++subMenu_mpi;
					}
					break;
				case 24:
					if(readyChk(MOTOR_SYR))	{
						++subMenu_mpi;
					}
					break;
				case 25:	// 바이알	없을 경우	여기로 - 현재	시린지가 TOP에 있지	않을 수도	있다.
//					if(readyChk(MOTOR_SYR))	{	// 8번에서
						goZ(POS_Z_TOP, ACT_XY_POS);
						++subMenu_mpi;
//					}
					break;
				case 26:
// 동작	완료인가 - 현재	시린지가 TOP에 있지	않을 수도	있다.
						if(readyChk(MOTOR_Z))	{
//dfp("==========sampleVolume=%d  transportVolume=%d injectionVolume=%d\n",sampleVolume,(int)(transportVolume*10),(int)(injectionVolume*10));
								devState.curInjCnt++;
								subMenu_mpi	=	1;
								return ACTION_MENU_END;
						}
						break;
		}
		return ACTION_MENU_ING;
}
#endif

// x=	7*16
// y=	-19*16

/*
#if	EEPROM_ADJUST_DATA_SAVE_FIX
#define	ADJUST_VALUE_X_MIN		-120
#define	ADJUST_VALUE_X_MAX		120
#define	ADJUST_VALUE_Y_MIN		-120
#define	ADJUST_VALUE_Y_MAX		120

#define	ADJUST_VALUE_Z_MIN		-120
#define	ADJUST_VALUE_Z_MAX		120

#define	ADJUST_VALUE_SYR_MIN		-120
#define	ADJUST_VALUE_SYR_MAX		120
#define	ADJUST_VALUE_VAL_MIN		-120
#define	ADJUST_VALUE_VAL_MAX		120
#define	ADJUST_VALUE_INJ_MIN		-120
#define	ADJUST_VALUE_INJ_MAX		120

#define	ADJUST_VALUE_MICRO_PUMP_MIN			0
#define	ADJUST_VALUE_MICRO_PUMP_MAX			25


#else
#define	ADJUST_VALUE_X_MIN		-100
#define	ADJUST_VALUE_X_MAX		100
#define	ADJUST_VALUE_Y_MIN		-100
#define	ADJUST_VALUE_Y_MAX		100

#define	ADJUST_VALUE_Z_MIN		-100
#define	ADJUST_VALUE_Z_MAX		100

#define	ADJUST_VALUE_SYR_MIN		-100
#define	ADJUST_VALUE_SYR_MAX		100
#define	ADJUST_VALUE_VAL_MIN		-100
#define	ADJUST_VALUE_VAL_MAX		100
#define	ADJUST_VALUE_INJ_MIN		-100
#define	ADJUST_VALUE_INJ_MAX		100
#endif
*/

#define NET_TEST_EEPROM			1

#if	EEPROM_ADJUST_DATA_SAVE_FIX

int	checkAdjustData()
{
	int	test = NO;//YES;
	int	result = NO;
	int i=0;
	int j=0;
	int saveAct = NO;
	int newVersion;
	unsigned char checkSum = 0;
	unsigned char *da;
	unsigned char *da2;

// 20210602
//	eepromRead_Data(HOME_ADJUST_VERSION_CHECK_ADDR, HOME_ADJUST_VERSION_CHECK_SIZE,(uint8_t *)(&devSet.adjust_ver));
//	eepromRead_Data(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust),devSet.adjust.saveFlag);	

	if(devSet.adjust_ver != HOME_ADJUST_VERSION_CHECK_DATA) {	// 같지 않으면 
		// 구버전 현재 데이터를 백업 저장 및 checksum 저장 
		newVersion = NO;
		memcpy(&devSet.adjust_2nd, &devSet.adjust, sizeof(devSet.adjust));
	}
	else {
		newVersion = YES;
	}

	if(newVersion == YES) {
#if NET_TEST_EEPROM		
		eepromRead_Data(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust_2nd),devSet.adjust_2nd.saveFlag);	
#endif
	}

#if TEST_EEPROM_VERIFY
 da = (unsigned char*)(&devSet.adjust.saveFlag);
 da2 = (unsigned char*)(&devSet.adjust_2nd.saveFlag);
		for(i=0;i<43;i++) {
			if(da[i] != da2[i])  {
				iprintf("  ---");
			}
iprintf("[%d:%x:%x]\t",i,da[i],da2[i]);						
		}
iprintf("\n");
#endif

	devSet.adjustState = YES;
	result = RE_OK;	
	saveAct = NO;

/*	
dfp("===================newVersion = %d\n",newVersion);
for(i=0;i<6;i++) {
dp("devSet.adjust.saveFlag = %d devSet.adjust.value = %d\n",devSet.adjust.saveFlag[i],devSet.adjust.value[i]);
dp("   adjust_2nd.saveFlag = %d devSet.adjust.value = %d\n",devSet.adjust_2nd.saveFlag[i],devSet.adjust_2nd.value[i]);
}
*/
	for(i=0 ; i<6 ; i++) {
		if(devSet.adjust.saveFlag[i] ==	DATA_SAVE_FLAG) {
			if(devSet.adjust.value[i]	<	ADJUST_VALUE_X_MIN ||	devSet.adjust.value[i] > ADJUST_VALUE_X_MAX) {
diprintf("err adjust.value [%d]=%d  adjust_2nd=[%d]\n",i,devSet.adjust.value[i],devSet.adjust_2nd.value[i]);					
//				devSet.adjust.value[i] = 0;
				// 초기화 
				devSet.adjust.saveFlag[i] = 0;
				devSet.adjust_2nd.saveFlag[i] = 0x01;
				devSet.adjust.value[i] = devSet.adjust_2nd.value[i] = 0;
				
				result = RE_ERR;							// 에러
				devSet.adjustState = NO;	// 조정필요
				saveAct = YES;						// 저장필요
				checkSum = 0x01;

			}
			else {
				if(newVersion == NO) {
					// 올드 버전이고 데이터는 정상이다 , 새로운 버전에 맞춰서 새롭게 2nd 와 checksum를 저장한다.			
					// 올드값으로  2nd 에 복사해 넣는다. 
					// devSet.adjust_2nd.saveFlag[0] 에 checksum
					checkSum = checkSum ^ devSet.adjust.value[i];
				}
				else {
					if(devSet.adjust.value[i] != devSet.adjust_2nd.value[i]) { // 이상한 값이다. 초기화 
						// 초기화 
diprintf("diff adjust.value [%d]=%d  adjust_2nd=[%d]\n",i,devSet.adjust.value[i],devSet.adjust_2nd.value[i]);											
						devSet.adjust.saveFlag[i] = 0;
						devSet.adjust_2nd.saveFlag[i] = 0x01;	// 0 아닌 값으로 
						devSet.adjust.value[i] = devSet.adjust_2nd.value[i] = 0;
						
						result = RE_ERR;						
						devSet.adjustState = NO;
						saveAct = YES;
						checkSum = 0x01;
					}
				}
			}
		}
		else {	// 저장이 안되어 있으므로 초기화 한다.  -- 저저장안한다.
			devSet.adjust.value[i] = 0;		
dp("adjust.value not Saved [%d]=%d\n",i,devSet.adjust.saveFlag[i]);							
			devSet.adjust.saveFlag[i] = 0;
			devSet.adjust_2nd.saveFlag[i] = 0x01;	// 0 아닌 값으로 
			devSet.adjust.value[i] = devSet.adjust_2nd.value[i] = 0;		
			
			result = RE_ERR;
			devSet.adjustState = NO;
			checkSum = 0x01;
		}
	}

	if(newVersion == NO) {
		devSet.adjust_2nd.saveFlag[0] = checkSum;
	}

	checkSum = 0x00;

/*
dfp("===================newVersion = %d\n",newVersion);
for(i=0;i<6;i++) {
dfp("devSet.adjust.saveFlag = %d devSet.adjust.value = %d\n",devSet.adjust.saveFlag[i],devSet.adjust.value[i]);
dfp("   adjust_2nd.saveFlag = %d devSet.adjust.value = %d\n",devSet.adjust_2nd.saveFlag[i],devSet.adjust_2nd.value[i]);
}
*/

//===========================================================================================
	if(devSet.adjust.saveWash ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.wash_x <	ADJUST_VALUE_X_MIN ||	devSet.adjust.wash_x > ADJUST_VALUE_X_MAX || devSet.adjust.wash_y <	ADJUST_VALUE_Y_MIN ||	devSet.adjust.wash_y > ADJUST_VALUE_Y_MAX) {
diprintf("err adjust [wash] invalid [%d] [%d]\n",devSet.adjust.wash_x,devSet.adjust.wash_y);
			devSet.adjust.saveWash = 0;
			devSet.adjust_2nd.saveWash = 0x01;
			devSet.adjust.wash_x = devSet.adjust_2nd.wash_x = 0;
			devSet.adjust.wash_y = devSet.adjust_2nd.wash_y = 0;
			// 초기화 
			result = NO;
			devSet.adjustState = NO;
			saveAct = YES;
			checkSum = 0x01;
		}
		else {
			if(newVersion == NO) {	// 올드 버전이고 데이터는 정상이다 , 새로운 버전에 맞춰서 새롭게 2nd 와 checksum를 저장한다.
				checkSum = devSet.adjust.wash_x ^ devSet.adjust.wash_y;
			}
			else {
				if((devSet.adjust.wash_x != devSet.adjust_2nd.wash_x) || (devSet.adjust.wash_y != devSet.adjust_2nd.wash_y) ) { // 이상한 값이다. 초기화 
					// 초기화 
diprintf("diff adjust [wash] invalid [%d] [%d]\n",devSet.adjust.wash_x,devSet.adjust.wash_y);					
					devSet.adjust.saveWash = 0;
					devSet.adjust_2nd.saveWash = 0x01;
					devSet.adjust.wash_x = devSet.adjust_2nd.wash_x = 0;
					devSet.adjust.wash_y = devSet.adjust_2nd.wash_y = 0;

					result = NO;											
					devSet.adjustState = NO;
					saveAct = YES;
					checkSum = 0x01;
				}
			}
		}
	}
	else {
		devSet.adjust.saveWash = 0;
		devSet.adjust_2nd.saveWash = 0x01;
		devSet.adjust.wash_x = devSet.adjust_2nd.wash_x = 0;
		devSet.adjust.wash_y = devSet.adjust_2nd.wash_y = 0;
		
diprintf("adjust [saveWash] not Saved [%d]\n",devSet.adjust.saveWash);
			// 초기화 
		result = NO;
		devSet.adjustState = NO;		
		checkSum = 0x01;
	}	

	if(newVersion == NO) {
		devSet.adjust_2nd.saveWash = checkSum;
	}

	checkSum = 0x00;

//=================================================================================================
	if(devSet.adjust.saveWaste ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.waste_x <	ADJUST_VALUE_X_MIN ||	devSet.adjust.waste_x > ADJUST_VALUE_X_MAX || devSet.adjust.waste_y <	ADJUST_VALUE_Y_MIN ||	devSet.adjust.waste_y > ADJUST_VALUE_Y_MAX) {
diprintf("err adjust [waste] invalid [%d] [%d]\n",devSet.adjust.waste_x,devSet.adjust.waste_y);	
			devSet.adjust.saveWaste = 0;
			devSet.adjust_2nd.saveWaste = 0x01;			
			devSet.adjust.waste_x = devSet.adjust_2nd.waste_x = 0;
			devSet.adjust.waste_y = devSet.adjust_2nd.waste_y = 0;
			

			// 초기화 
			result = NO;
			devSet.adjustState = NO;
			saveAct = YES;
			checkSum = 0x01;
		}
		else {
			if(newVersion == NO) {	// 올드 버전이고 데이터는 정상이다 , 새로운 버전에 맞춰서 새롭게 2nd 와 checksum를 저장한다.
				checkSum = devSet.adjust.waste_x ^ devSet.adjust.waste_y;
			}
			else {
				if((devSet.adjust.waste_x != devSet.adjust_2nd.waste_x) || (devSet.adjust.waste_y != devSet.adjust_2nd.waste_y) ) { // 이상한 값이다. 초기화 
					// 초기화 
diprintf("diff adjust [waste] invalid [%d] [%d]\n",devSet.adjust.waste_x,devSet.adjust.waste_y);						
					devSet.adjust.saveWaste = 0;
					devSet.adjust_2nd.saveWaste = 0x01;
					devSet.adjust.waste_x = devSet.adjust_2nd.waste_x = 0;
					devSet.adjust.waste_y = devSet.adjust_2nd.waste_y = 0;

					result = NO;											
					devSet.adjustState = NO;
					saveAct = YES;
					checkSum = 0x01;
				}
			}
		}
	}
	else {
		devSet.adjust.saveWaste = 0;
		devSet.adjust_2nd.saveWaste = 0x01;
		devSet.adjust.waste_x = devSet.adjust_2nd.waste_x = 0;
		devSet.adjust.waste_y = devSet.adjust_2nd.waste_y = 0;
		
diprintf("adjust [saveWaste] not Saved [%d]\n",devSet.adjust.saveWaste);
			// 초기화 
		result = NO;
		devSet.adjustState = NO;		
		checkSum = 0x01;
	}	

	if(newVersion == NO) {
		devSet.adjust_2nd.saveWaste = checkSum;
	}

	checkSum = 0x00;


//=========================================================================================================
	if(devSet.adjust.saveVialHeight ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.vialHeight <	ADJUST_VALUE_X_MIN ||	devSet.adjust.vialHeight > ADJUST_VALUE_X_MAX) {
diprintf("err adjust [vialHeight] invalid [%d]\n",devSet.adjust.vialHeight);	
			devSet.adjust.saveVialHeight = 0;
			devSet.adjust_2nd.saveVialHeight = 0x01;			
			devSet.adjust.vialHeight = devSet.adjust_2nd.vialHeight = 0;
			
			// 초기화 
			result = NO;
			devSet.adjustState = NO;
			saveAct = YES;
			checkSum = 0x01;
		}
		else {
			if(newVersion == NO) {	// 올드 버전이고 데이터는 정상이다 , 새로운 버전에 맞춰서 새롭게 2nd 와 checksum를 저장한다.
				checkSum = devSet.adjust.vialHeight;
			}
			else {
				if((devSet.adjust.vialHeight != devSet.adjust_2nd.vialHeight) ) { // 이상한 값이다. 초기화 
					// 초기화 
diprintf("diff adjust [vialHeight] invalid [%d]\n",devSet.adjust.vialHeight);						
					devSet.adjust.saveVialHeight = 0;
					devSet.adjust_2nd.saveVialHeight = 0x01;
					devSet.adjust.vialHeight = devSet.adjust_2nd.vialHeight = 0;
		
					result = NO;											
					devSet.adjustState = NO;
					saveAct = YES;
					checkSum = 0x01;
				}
			}
		}
	}
	else {
		devSet.adjust.saveVialHeight = 0;
		devSet.adjust_2nd.saveVialHeight = 0x01;
		devSet.adjust.vialHeight = devSet.adjust_2nd.vialHeight = 0;
		
diprintf("adjust [saveVialHeight] not Saved [%d]\n",devSet.adjust.saveWaste);
			// 초기화 
		result = NO;
		devSet.adjustState = NO;		
		checkSum = 0x01;
	}	

	if(newVersion == NO) {
		devSet.adjust_2nd.saveVialHeight = checkSum;
	}

	checkSum = 0x00;


//=========================================================================================================
	if(devSet.adjust.saveVialHeight_xy ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.vialHeight_x <	ADJUST_VALUE_X_MIN ||	devSet.adjust.vialHeight_x > ADJUST_VALUE_X_MAX || devSet.adjust.vialHeight_y <	ADJUST_VALUE_X_MIN ||	devSet.adjust.vialHeight_y > ADJUST_VALUE_X_MAX) {
diprintf("diff adjust [saveVialHeight_xy] [%d] [%d]\n",devSet.adjust.vialHeight_x,devSet.adjust.vialHeight_y);									
			devSet.adjust.saveVialHeight_xy = 0;
			devSet.adjust_2nd.saveVialHeight_xy = 0x01;			
			devSet.adjust.vialHeight_x = devSet.adjust_2nd.vialHeight_x = 0;
			devSet.adjust.vialHeight_y = devSet.adjust_2nd.vialHeight_y = 0;

			// 초기화 
			result = NO;
			devSet.adjustState = NO;
			saveAct = YES;
			checkSum = 0x01;
		}
		else {
			if(newVersion == NO) {	// 올드 버전이고 데이터는 정상이다 , 새로운 버전에 맞춰서 새롭게 2nd 와 checksum를 저장한다.
				checkSum = devSet.adjust.vialHeight_x ^ devSet.adjust.vialHeight_y;
			}
			else {
				if((devSet.adjust.vialHeight_x != devSet.adjust_2nd.vialHeight_x) || (devSet.adjust.vialHeight_y != devSet.adjust_2nd.vialHeight_y) ) { // 이상한 값이다. 초기화 
					// 초기화 
diprintf("err adjust [saveVialHeight_xy] [%d] [%d]\n",devSet.adjust.vialHeight_x,devSet.adjust.vialHeight_y);														
					devSet.adjust.saveVialHeight_xy = 0;
					devSet.adjust_2nd.saveVialHeight_xy = 0x01;
					devSet.adjust.vialHeight_x = devSet.adjust_2nd.vialHeight_x = 0;
					devSet.adjust.vialHeight_y = devSet.adjust_2nd.vialHeight_y = 0;

					result = NO;											
					devSet.adjustState = NO;
					saveAct = YES;
					checkSum = 0x01;
				}
			}
		}
	}
	else {
		devSet.adjust.saveVialHeight_xy = 0;
		devSet.adjust_2nd.saveVialHeight_xy = 0x01;
		devSet.adjust.vialHeight_x = devSet.adjust_2nd.vialHeight_x = 0;
		devSet.adjust.vialHeight_y = devSet.adjust_2nd.vialHeight_y = 0;

diprintf("adjust [saveVialHeight_xy] not Saved [%d]\n",devSet.adjust.saveVialHeight_xy);		
			// 초기화 
		result = NO;
		devSet.adjustState = NO;		
		checkSum = 0x01;
	}	

	if(newVersion == NO) {
		devSet.adjust_2nd.saveVialHeight_xy = checkSum;
	}

	checkSum = 0x00;

//=========================================================================================================
	if(sysConfig.trayLeft < TRAY_NONE) {
		if(devSet.adjust.saveVL[sysConfig.trayLeft] ==	DATA_SAVE_FLAG) {
			if(devSet.adjust.leftVial_x[sysConfig.trayLeft] <	ADJUST_VALUE_X_MIN ||	devSet.adjust.leftVial_y[sysConfig.trayLeft] > ADJUST_VALUE_X_MAX) {
diprintf("err adjust [saveVL] [%d] [%d]\n",devSet.adjust.leftVial_x[sysConfig.trayLeft] ,devSet.adjust.leftVial_y[sysConfig.trayLeft] );													
				devSet.adjust.saveVL[sysConfig.trayLeft] = 0;		
				devSet.adjust_2nd.saveVL[sysConfig.trayLeft] = 0x01;		
				devSet.adjust.leftVial_x[sysConfig.trayLeft] = devSet.adjust_2nd.leftVial_x[sysConfig.trayLeft] = 0;
				devSet.adjust.leftVial_y[sysConfig.trayLeft] = devSet.adjust_2nd.leftVial_y[sysConfig.trayLeft] = 0;			
				
				// 초기화 
				result = NO;
				devSet.adjustState = NO;
				saveAct = YES;
				checkSum = 0x01;
			}
			else {
				if(newVersion == NO) {	// 올드 버전이고 데이터는 정상이다 , 새로운 버전에 맞춰서 새롭게 2nd 와 checksum를 저장한다.
					checkSum = devSet.adjust.leftVial_x[0] ^ devSet.adjust.leftVial_y[0] 
						^ devSet.adjust.leftVial_x[1] ^ devSet.adjust.leftVial_y[1]
						^devSet.adjust.leftVial_x[2] ^ devSet.adjust.leftVial_y[2];
				}
				else {
					if((devSet.adjust.leftVial_x[sysConfig.trayLeft] != devSet.adjust_2nd.leftVial_x[sysConfig.trayLeft]) 
						|| (devSet.adjust.leftVial_y[sysConfig.trayLeft] != devSet.adjust_2nd.leftVial_y[sysConfig.trayLeft]) ) { // 이상한 값이다. 초기화 
						// 초기화
diprintf("diff adjust [saveVL] [%d] [%d]\n",devSet.adjust.leftVial_x[sysConfig.trayLeft] ,devSet.adjust.leftVial_y[sysConfig.trayLeft] );																			
						devSet.adjust.saveVL[sysConfig.trayLeft] = 0;		
						devSet.adjust_2nd.saveVL[sysConfig.trayLeft] = 0x01;		
						devSet.adjust.leftVial_x[sysConfig.trayLeft] = devSet.adjust_2nd.leftVial_x[sysConfig.trayLeft] = 0;
						devSet.adjust.leftVial_y[sysConfig.trayLeft] = devSet.adjust_2nd.leftVial_y[sysConfig.trayLeft] = 0;			

						result = NO;
						devSet.adjustState = NO;
						saveAct = YES;
						checkSum = 0x01;
					}
				}
			}
		}
		else {
			devSet.adjust.saveVL[sysConfig.trayLeft] = 0;
			devSet.adjust_2nd.saveVL[sysConfig.trayLeft] = 0x01;
			devSet.adjust.leftVial_x[sysConfig.trayLeft];
			devSet.adjust.leftVial_y[sysConfig.trayLeft];

			for(i=0;i<3;i++) {
diprintf("adjust [saveVL] not Saved [%d]\n",devSet.adjust.saveVL[i]);						
				devSet.adjust_2nd.saveVL[i] = 0x01;
				devSet.adjust_2nd.leftVial_x[i] = 0;
				devSet.adjust_2nd.leftVial_y[i] = 0;
			}
			
				// 초기화 
			result = NO;
			devSet.adjustState = NO;
			checkSum = 0x01;
		}
	}

	if(newVersion == NO) {
		devSet.adjust_2nd.saveVL[0] = checkSum;
	}

	checkSum = 0x00;

//=========================================================================================================
if(sysConfig.trayRight < TRAY_NONE) {
	if(devSet.adjust.saveVR[sysConfig.trayRight] ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.rightVial_x[sysConfig.trayRight] <	ADJUST_VALUE_X_MIN ||	devSet.adjust.rightVial_y[sysConfig.trayRight] > ADJUST_VALUE_X_MAX) {
diprintf("err adjust [saveVL] [%d] [%d]\n",devSet.adjust.rightVial_x[sysConfig.trayRight] ,devSet.adjust.rightVial_y[sysConfig.trayRight] );																
			devSet.adjust.saveVR[sysConfig.trayRight] = 0;		
			devSet.adjust_2nd.saveVR[sysConfig.trayRight] = 0x01;		
			devSet.adjust.rightVial_x[sysConfig.trayRight] = devSet.adjust_2nd.rightVial_x[sysConfig.trayRight] = 0;
			devSet.adjust.rightVial_y[sysConfig.trayRight] = devSet.adjust_2nd.rightVial_y[sysConfig.trayRight] = 0;			
			
			result = NO;
			devSet.adjustState = NO;
			saveAct = YES;
			checkSum = 0x01;
		}
		else {
			if(newVersion == NO) {	// 올드 버전이고 데이터는 정상이다 , 새로운 버전에 맞춰서 새롭게 2nd 와 checksum를 저장한다.
				checkSum = devSet.adjust.rightVial_x[0] ^ devSet.adjust.rightVial_y[0] 
					^ devSet.adjust.rightVial_x[1] ^ devSet.adjust.rightVial_y[1]
					^devSet.adjust.rightVial_x[2] ^ devSet.adjust.rightVial_y[2];
			}
			else {
				if((devSet.adjust.rightVial_x[sysConfig.trayRight] != devSet.adjust_2nd.rightVial_x[sysConfig.trayRight]) 
					|| (devSet.adjust.rightVial_y[sysConfig.trayRight] != devSet.adjust_2nd.rightVial_y[sysConfig.trayRight]) ) { // 이상한 값이다. 초기화 
					// 초기화 
diprintf("diff adjust [saveVL] [%d] [%d]\n",devSet.adjust.rightVial_x[sysConfig.trayRight] ,devSet.adjust.rightVial_y[sysConfig.trayRight] );																					
					devSet.adjust.saveVR[sysConfig.trayRight] = 0;		
					devSet.adjust_2nd.saveVR[sysConfig.trayRight] = 0x01;		
					devSet.adjust.rightVial_x[sysConfig.trayRight] = devSet.adjust_2nd.rightVial_x[sysConfig.trayRight] = 0;
					devSet.adjust.rightVial_y[sysConfig.trayRight] = devSet.adjust_2nd.rightVial_y[sysConfig.trayRight] = 0;			

					result = NO;											
					devSet.adjustState = NO;
					saveAct = YES;
					checkSum = 0x01;
				}
			}
		}
	}
	else {
		devSet.adjust.saveVR[sysConfig.trayRight] = 0;		
		devSet.adjust_2nd.saveVR[sysConfig.trayRight] = 0x01;		
		devSet.adjust.rightVial_x[sysConfig.trayRight] = devSet.adjust_2nd.rightVial_x[sysConfig.trayRight] = 0;
		devSet.adjust.rightVial_y[sysConfig.trayRight] = devSet.adjust_2nd.rightVial_y[sysConfig.trayRight] = 0;			

			for(i=0;i<3;i++) {
diprintf("adjust [saveVR] not Saved [%d]\n",devSet.adjust.saveVR[i]);						
				devSet.adjust_2nd.saveVR[i] = 0x01;
				devSet.adjust_2nd.rightVial_x[i] = 0;
				devSet.adjust_2nd.rightVial_y[i] = 0;
			}
		
			// 초기화 
		result = NO;
		devSet.adjustState = NO;		
		checkSum = 0x01;
	}	
}

	if(newVersion == NO) {
		devSet.adjust_2nd.saveVR[0] = checkSum;
	}

// ================================================================================================================
	if(newVersion == NO) {	// 올드 버전이므로 무조건 저장한다.
//		eepromWrite_Data(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);
		eepromWrite_Data_Verify_2nd(HOME_ADJUST_2ND_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust_2nd.saveFlag);
		
diprintf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX newVersion eepromWrite_Data\n");								
	}

	if(saveAct == YES) {
#if NET_TEST_EEPROM				
		eepromWrite_Data(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust) + 1,devSet.adjust.saveFlag);		
#endif
diprintf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX saveAct eepromWrite_Data\n");										
		saveAct = NO;
		checkSum = 0x00;
		
	}
// ==================================================================================================================

dfp("===================sysConfig.trayRight = %d\n",sysConfig.trayRight);

dp("devSet.adjust.rightVial_x[sysConfig.trayRight] = %d devSet.adjust.value = %d\n",devSet.adjust.rightVial_x[sysConfig.trayRight]);
dp("devSet.adjust.rightVial_y[sysConfig.trayRight] = %d\n",devSet.adjust.rightVial_y[sysConfig.trayRight]);


		for(i=0;i<6;i++) {
			devSet.adjustOld.value[i]	=	devSet.adjust.value[i];
		}

	    devSet.adjustOld.leftVial_x[sysConfig.trayLeft] = devSet.adjust.leftVial_x[sysConfig.trayLeft];
	    devSet.adjustOld.leftVial_y[sysConfig.trayLeft] = devSet.adjust.leftVial_y[sysConfig.trayLeft];

	    devSet.adjustOld.rightVial_x[sysConfig.trayRight] = devSet.adjust.rightVial_x[sysConfig.trayRight];
	    devSet.adjustOld.rightVial_y[sysConfig.trayRight] = devSet.adjust.rightVial_y[sysConfig.trayRight];

			devSet.adjustOld.wash_x = devSet.adjust.wash_x;
			devSet.adjustOld.wash_y = devSet.adjust.wash_y;

			devSet.adjustOld.waste_x = devSet.adjust.waste_x;
			devSet.adjustOld.waste_y = devSet.adjust.waste_y;

			devSet.adjustOld.vialHeight = devSet.adjust.vialHeight;			

			devSet.adjustOld.vialHeight_x = devSet.adjust.vialHeight_x;			
			devSet.adjustOld.vialHeight_y = devSet.adjust.vialHeight_y;						


		if(devSet.adjustState == NO) {
			if(devState.connected) errorFunction(SELF_ERROR_NO_ADJUSTMENT_DATA);
		}

		if(newVersion == NO) {
			devSet.adjust_ver = HOME_ADJUST_VERSION_CHECK_DATA;
			eepromRead_Data(HOME_ADJUST_VERSION_CHECK_ADDR, HOME_ADJUST_VERSION_CHECK_SIZE,(uint8_t *)(&devSet.adjust_ver));
		}


#if MICRO_PUMP_VOLTAGE_ADJUSTMENT
//	unsigned char saveMicroPump;
//	unsigned char microPumpVolt;
//#define	ADJUST_VALUE_MICRO_PUMP_MIN		1
//#define	ADJUST_VALUE_MICRO_PUMP_MAX		10

//=========================================================================================================
	if(devSet.adjust.saveMicroPump ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.microPumpVolt <	ADJUST_VALUE_MICRO_PUMP_MIN ||	devSet.adjust.microPumpVolt > ADJUST_VALUE_MICRO_PUMP_MAX) {
diprintf("diff adjust [microPumpVolt] [%d] \n",devSet.adjust.microPumpVolt);
			devSet.adjust.saveMicroPump = 0;
			devSet.adjust_2nd.saveMicroPump = 0x01;			
			devSet.adjust.microPumpVolt = devSet.adjust_2nd.microPumpVolt = MICRO_PUMP_VOLT_DEFAULT; //(0~10)
//			devSet.microPumpPwmCount = devSet.adjust.microPumpVolt*MICRO_PUMP_UNIT_PWM + MICRO_PUMP_UNIT_PWM;

			// 초기화 
			result = NO;
			devSet.adjustState = NO;
			saveAct = YES;
			checkSum = 0x01;
		}
		else {
			if(newVersion == NO) {	// 올드 버전이고 데이터는 정상이다 , 새로운 버전에 맞춰서 새롭게 2nd 와 checksum를 저장한다.
				checkSum = devSet.adjust.microPumpVolt;
			}
			else {
				if((devSet.adjust.microPumpVolt != devSet.adjust_2nd.microPumpVolt)) { // 이상한 값이다. 초기화 
					// 초기화 
diprintf("err adjust [microPumpVolt] [%d]\n",devSet.adjust.microPumpVolt);
					devSet.adjust.saveMicroPump = 0;
					devSet.adjust_2nd.saveMicroPump = 0x01;
					devSet.adjust.microPumpVolt = devSet.adjust_2nd.microPumpVolt = MICRO_PUMP_VOLT_DEFAULT;

					result = NO;											
					devSet.adjustState = NO;
					saveAct = YES;
					checkSum = 0x01;
				}
			}
		}
	}
	else {
		devSet.adjust.saveMicroPump = 0;
		devSet.adjust_2nd.saveMicroPump = 0x01;
		devSet.adjust.microPumpVolt = devSet.adjust_2nd.microPumpVolt = MICRO_PUMP_VOLT_DEFAULT;

diprintf("adjust [microPumpVolt] not Saved[0x%x] [%d]\n",devSet.adjust.saveMicroPump, devSet.adjust.microPumpVolt);		
			// 초기화 
		result = NO;
		devSet.adjustState = NO;		
		checkSum = 0x01;
	}	

	if(newVersion == NO) {
		devSet.adjust_2nd.microPumpVolt = checkSum;
	}

	checkSum = 0x00;

	devSet.microPumpPwmCount = devSet.adjust.microPumpVolt*MICRO_PUMP_UNIT_PWM + MICRO_PUMP_PWM_MIN;
	
#endif		
		
}

#else

int	checkAdjustData()
{
	int	test = NO;//YES;
	int	result = NO;
	int i=0;

	eepromRead_Data(HOME_ADJUST_DATA_STARTADDR,sizeof(devSet.adjust),devSet.adjust.saveFlag);

	for(i=0; i<6 ; i++)	{
			dp("devSet.adjust.save[%d]=0x%x\n",i,devSet.adjust.saveFlag[i]);
			dp("xxdevSet.adjust.value[%d]=%d\n",i,devSet.adjust.value[i]);
	}

	dp("devSet.adjust.saveVL[%d]=0x%x\n",sysConfig.trayLeft,devSet.adjust.saveVL[sysConfig.trayLeft]);
	dp("devSet.adjust.saveVR[%d]=0x%x\n",sysConfig.trayRight,devSet.adjust.saveVR[sysConfig.trayRight]);			
for(i=0; i<3 ;i++) {	
dfp("	devSet.adjust.leftVial_x[%d] = [%d,%d]\n", i, devSet.adjust.leftVial_x[i] , devSet.adjust.leftVial_y[i]);
dfp("	devSet.adjust.rightVial_x[%d] = [%d,%d]\n", i, devSet.adjust.rightVial_x[i] ,  devSet.adjust.rightVial_y[i]);
}
	dp("devSet.adjust.saveW=0x%x\n",devSet.adjust.saveWash);
	dp("devSet.adjust.saveWaste=0x%x\n",devSet.adjust.saveWaste);

	devSet.adjustState = YES;
	result = YES;	

	for(i=0 ; i<6 ; i++) {	
		if(devSet.adjust.saveFlag[i] ==	DATA_SAVE_FLAG) {
			if(devSet.adjust.value[i]	<	ADJUST_VALUE_X_MIN ||	devSet.adjust.value[i] > ADJUST_VALUE_X_MAX) {
				devSet.adjust.value[i] = 0;
				result = NO;
				devSet.adjustState = NO;
dp("adjust.value invalid [%d]=%d\n",i,devSet.adjust.value[i]);
			}
		}
		else {
			devSet.adjust.value[i] = 0;		
			devSet.adjustState = NO;
			result = NO;
dp("adjust.value not Saved [%d]=%d\n",i,devSet.adjust.saveFlag[i]);							
		}	
	}		

	if(devSet.adjust.saveWash ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.wash_x <	ADJUST_VALUE_X_MIN ||	devSet.adjust.wash_x > ADJUST_VALUE_X_MAX || devSet.adjust.wash_y <	ADJUST_VALUE_Y_MIN ||	devSet.adjust.wash_y > ADJUST_VALUE_Y_MAX) {
			devSet.adjust.wash_x = 0;
			devSet.adjust.wash_y = 0;			
			result = NO;
			devSet.adjustState = NO;
dp("adjust [wash] invalid [%d] [%d]\n",devSet.adjust.wash_x,devSet.adjust.wash_y);
		}
	}
	else {
		devSet.adjust.wash_x = 0;
		devSet.adjust.wash_y = 0;			
		devSet.adjustState = NO;
		result = NO;
dp("adjust [wash] not Saved [%d]\n",devSet.adjust.saveWash);									
	}	

	if(devSet.adjust.saveWaste ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.waste_x <	ADJUST_VALUE_X_MIN ||	devSet.adjust.waste_x > ADJUST_VALUE_X_MAX || devSet.adjust.waste_y <	ADJUST_VALUE_Y_MIN ||	devSet.adjust.waste_y > ADJUST_VALUE_Y_MAX) {
			devSet.adjust.waste_x = 0;
			devSet.adjust.waste_y = 0;			
			result = NO;
			devSet.adjustState = NO;
dp("adjust [waste] invalid [%d] [%d]\n",devSet.adjust.waste_x,devSet.adjust.waste_y);			
		}
	}
	else {
		devSet.adjust.waste_x = 0;
		devSet.adjust.waste_y = 0;			
		devSet.adjustState = NO;
		result = NO;
dp("adjust [waste] not Saved [%d]\n",devSet.adjust.saveWaste);
	}				

	if(devSet.adjust.saveVialHeight ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.vialHeight <	ADJUST_VALUE_X_MIN ||	devSet.adjust.vialHeight > ADJUST_VALUE_X_MAX) {
			devSet.adjust.vialHeight = 0;
			result = NO;
			devSet.adjustState = NO;
dp("adjust [vialHeight] invalid [%d]\n",devSet.adjust.vialHeight);			
		}
	}
	else {
		devSet.adjust.vialHeight = 0;
		devSet.adjustState = NO;
		result = NO;
dp("adjust [vialHeight] not Saved [%d]\n",devSet.adjust.saveVialHeight);
	}				

	if(devSet.adjust.saveVialHeight_xy ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.vialHeight_x <	ADJUST_VALUE_X_MIN ||	devSet.adjust.vialHeight_x > ADJUST_VALUE_X_MAX || devSet.adjust.vialHeight_y <	ADJUST_VALUE_X_MIN ||	devSet.adjust.vialHeight_y > ADJUST_VALUE_X_MAX) {
			devSet.adjust.vialHeight_x = 0;
			devSet.adjust.vialHeight_y = 0;			
			result = NO;
			devSet.adjustState = NO;
dp("adjust [vialHeight] invalid [%d]\n",devSet.adjust.vialHeight);			
		}
	}
	else {
		devSet.adjust.vialHeight_x = 0;
		devSet.adjust.vialHeight_y = 0;			
		devSet.adjustState = NO;
		result = NO;
dp("adjust [vialHeight_xy] not Saved [%d]\n",devSet.adjust.saveVialHeight_xy);
	}				
			
if(sysConfig.trayLeft < TRAY_NONE) {
	if(devSet.adjust.saveVL[sysConfig.trayLeft] ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.leftVial_x[sysConfig.trayLeft] <	ADJUST_VALUE_X_MIN ||	devSet.adjust.leftVial_y[sysConfig.trayLeft] > ADJUST_VALUE_X_MAX) {
			devSet.adjust.leftVial_x[sysConfig.trayLeft] = 0;
			devSet.adjust.leftVial_y[sysConfig.trayLeft] = 0;			
			result = NO;
			devSet.adjustState = NO;
dp("adjust [vial Left] invalid [%d]:[%d] [%d]\n",sysConfig.trayLeft ,devSet.adjust.leftVial_x[sysConfig.trayLeft],devSet.adjust.leftVial_y[sysConfig.trayLeft]);						
		}
	}
	else {
		devSet.adjust.leftVial_x[sysConfig.trayLeft] = 0;
		devSet.adjust.leftVial_y[sysConfig.trayLeft] = 0;			
		devSet.adjustState = NO;
		result = NO;
dp("adjust [vial Left] not Saved [%d]=%d\n",sysConfig.trayLeft,devSet.adjust.saveVL[sysConfig.trayLeft]);
	}
}

if(sysConfig.trayRight < TRAY_NONE) {
	if(devSet.adjust.saveVR[sysConfig.trayRight] ==	DATA_SAVE_FLAG) {
		if(devSet.adjust.rightVial_x[sysConfig.trayRight] <	ADJUST_VALUE_X_MIN ||	devSet.adjust.rightVial_y[sysConfig.trayRight] > ADJUST_VALUE_X_MAX) {
			devSet.adjust.rightVial_x[sysConfig.trayRight] = 0;
			devSet.adjust.rightVial_y[sysConfig.trayRight] = 0;			
			result = NO;
			devSet.adjustState = NO;
dp("adjust [vial Right] invalid [%d]:[%d] [%d]\n",sysConfig.trayRight ,devSet.adjust.leftVial_x[sysConfig.trayRight],devSet.adjust.leftVial_y[sysConfig.trayRight]);								
		}
	}
	else {
		devSet.adjust.rightVial_x[sysConfig.trayRight] = 0;
		devSet.adjust.rightVial_y[sysConfig.trayRight] = 0;			
		devSet.adjustState = NO;
		result = NO;
dp("adjust [vial Right] not Saved [%d]=%d\n",sysConfig.trayRight,devSet.adjust.saveVL[sysConfig.trayRight]);		
	}		
}

			for(i=0;i<6;i++) {
				devSet.adjustOld.value[i]	=	devSet.adjust.value[i];
			}

	    devSet.adjustOld.leftVial_x[sysConfig.trayLeft] = devSet.adjust.leftVial_x[sysConfig.trayLeft];
	    devSet.adjustOld.leftVial_y[sysConfig.trayLeft] = devSet.adjust.leftVial_y[sysConfig.trayLeft];

	    devSet.adjustOld.rightVial_x[sysConfig.trayRight] = devSet.adjust.rightVial_x[sysConfig.trayRight];
	    devSet.adjustOld.rightVial_y[sysConfig.trayRight] = devSet.adjust.rightVial_y[sysConfig.trayRight];

			devSet.adjustOld.wash_x = devSet.adjust.wash_x;
			devSet.adjustOld.wash_y = devSet.adjust.wash_y;

			devSet.adjustOld.waste_x = devSet.adjust.waste_x;
			devSet.adjustOld.waste_y = devSet.adjust.waste_y;

			devSet.adjustOld.vialHeight = devSet.adjust.vialHeight;			

			devSet.adjustOld.vialHeight_x = devSet.adjust.vialHeight_x;			
			devSet.adjustOld.vialHeight_y = devSet.adjust.vialHeight_y;						

			if(devSet.adjustState == NO) {
				if(devState.connected) errorFunction(SELF_ERROR_NO_ADJUSTMENT_DATA);
			}

		return result;
}

#endif

//int nextVialNew(VIAL_POS_t *pVial)
int nextSample(VIAL_POS_t *pVial)
{
		int	rowNo	=	pVial->uPos.x;	// 현재	주입이 끝난	바이알 좌표
		int	colNo	=	pVial->uPos.y;
		int	trayNo = pVial->uPos.tray;
		int	excute = NO;
		int	trayEnd	=	NO;
	
		if(sysConfig.processType ==	PROCESS_TYPE_ROW)	{	// 가로
				if(trayNo	== TRAY_LEFT)	{
					if(++rowNo ==	devSet.trayLeft_rowNo) {
						rowNo	=	0;
						if(++colNo ==	devSet.trayLeft_colNo) {
							colNo	=	0;
							trayNo = TRAY_RIGHT;
						}
					}
				}
				else if(trayNo ==	TRAY_RIGHT)	{
					if(++rowNo ==	devSet.trayRight_rowNo)	{
						rowNo	=	0;
						if(++colNo ==	devSet.trayRight_colNo)	{
//							colNo	=0;
							trayEnd	=	YES;
						}
					}
				}
				else { }			// TRAY_ETC	영역
				
// 다음	샘플을 실행할	지 검사
				if(trayEnd ==	YES) excute	=	NO;
				else if(devState.lastSample.uPos.tray	<	trayNo)	excute = NO;
				else if(devState.lastSample.uPos.tray	>	trayNo)	excute = YES;//NO;
				else if(devState.lastSample.uPos.y > colNo)	excute = YES;//NO;
				else if(devState.lastSample.uPos.y ==	colNo	&& devState.lastSample.uPos.x	>= rowNo)	excute = YES;
				else excute	=	NO;
		}
		else if(sysConfig.processType	== PROCESS_TYPE_COLUMN)	{	// 세로부터
				if(trayNo	== TRAY_LEFT)	{
					if(++colNo ==	devSet.trayLeft_colNo) {
						colNo	=	0;
						if(++rowNo ==	devSet.trayLeft_rowNo) {
							rowNo	=	0;
							trayNo = TRAY_RIGHT;
						}
					}
				}
				else if(trayNo ==	TRAY_RIGHT)	{
					if(++colNo ==	devSet.trayRight_colNo)	{
						colNo	=	0;
						if(++rowNo ==	devSet.trayRight_rowNo)	{
							//rowNo	=	0;
							trayEnd	=	YES;
						}
					}
				}
				else { }			// TRAY_ETC	영역

// 다음	샘플을 실행할	지 검사
				if(trayEnd ==	YES) excute	=	NO;
				else if(devState.lastSample.uPos.tray	<	trayNo)	excute = NO;
				else if(devState.lastSample.uPos.tray	>	trayNo)	excute = YES;//NO;
				else if(devState.lastSample.uPos.x > rowNo)	excute = YES;//NO;
				else if(devState.lastSample.uPos.x ==	rowNo	&& devState.lastSample.uPos.y	>= colNo)	excute = YES;
				else excute	=	NO;
		}

#if 0
	if(trayEnd ==	YES) return RE_ERR;
	else {
		pVial->uPos.x = rowNo;
		pVial->uPos.y = colNo;
		pVial->uPos.tray	=	trayNo;

		if(excute) {		return RE_OK;		}
		else 				{ return RE_ERR; }

	}
#endif

	if(excute) {	// 다음	샘플 설정
		pVial->uPos.x = rowNo;
		pVial->uPos.y = colNo;
		pVial->uPos.tray	=	trayNo;
dfp("RE_OK============\n");			
dfp("pVial->uPos (%d,%d)\n",pVial->uPos.x, pVial->uPos.y);
		return RE_OK;
	}
	else {
dfp("RE_ERR===========\n");			
		return RE_ERR;
	}

}

int nextVial(VIAL_POS_t *pVial)
{
		int	rowNo	=	pVial->uPos.x;	// 현재	주입이 끝난	바이알 좌표
		int	colNo	=	pVial->uPos.y;
		int	trayNo = pVial->uPos.tray;
//		int	excute = NO;
		int	trayEnd	=	NO;
	
		if(sysConfig.processType ==	PROCESS_TYPE_ROW)	{	// 가로
				if(trayNo	== TRAY_LEFT)	{
					if(++rowNo ==	devSet.trayLeft_rowNo) {
						rowNo	=	0;
						if(++colNo ==	devSet.trayLeft_colNo) {
							colNo	=	0;
							trayNo = TRAY_RIGHT;
						}
					}
				}
				else if(trayNo ==	TRAY_RIGHT)	{
					if(++rowNo ==	devSet.trayRight_rowNo)	{
						rowNo	=	0;
						if(++colNo ==	devSet.trayRight_colNo)	{
//							colNo	=0;
							trayEnd	=	YES;
						}
					}
				}
				else { }			// TRAY_ETC	영역

#if 0				
// 다음	샘플을 실행할	지 검사
				if(trayEnd ==	YES) excute	=	NO;
				else if(devState.lastSample.uPos.tray	<	trayNo)	excute = NO;
				else if(devState.lastSample.uPos.tray	>	trayNo)	excute = YES;//NO;
				else if(devState.lastSample.uPos.y > colNo)	excute = YES;//NO;
				else if(devState.lastSample.uPos.y ==	colNo	&& devState.lastSample.uPos.x	>= rowNo)	excute = YES;
				else excute	=	NO;
#endif				
		}
		else if(sysConfig.processType	== PROCESS_TYPE_COLUMN)	{	// 세로부터
				if(trayNo	== TRAY_LEFT)	{
					if(++colNo ==	devSet.trayLeft_colNo) {
						colNo	=	0;
						if(++rowNo ==	devSet.trayLeft_rowNo) {
							rowNo	=	0;
							trayNo = TRAY_RIGHT;
						}
					}
				}
				else if(trayNo ==	TRAY_RIGHT)	{
					if(++colNo ==	devSet.trayRight_colNo)	{
						colNo	=	0;
						if(++rowNo ==	devSet.trayRight_rowNo)	{
							//rowNo	=	0;
							trayEnd	=	YES;
						}
					}
				}
				else { }			// TRAY_ETC	영역

#if 0
// 다음	샘플을 실행할	지 검사
				if(trayEnd ==	YES) excute	=	NO;
				else if(devState.lastSample.uPos.tray	<	trayNo)	excute = NO;
				else if(devState.lastSample.uPos.tray	>	trayNo)	excute = YES;//NO;
				else if(devState.lastSample.uPos.x > rowNo)	excute = YES;//NO;
				else if(devState.lastSample.uPos.x ==	rowNo	&& devState.lastSample.uPos.y	>= colNo)	excute = YES;
				else excute	=	NO;
#endif				
		}



	if(trayEnd ==	YES) return RE_ERR;
	else {
		pVial->uPos.x = rowNo;
		pVial->uPos.y = colNo;
		pVial->uPos.tray	=	trayNo;
		return RE_OK;
	}

}

int chkNextVialMix()
{
	if(mixSet.relativeDestination == YES) {
		if(nextVial(&devState.firstDestVial)) {						}
		else { /* error */}
dfp("relative firstDestVial.uPos (%d,%d)\n",devState.firstDestVial.uPos.x, devState.firstDestVial.uPos.y);
	}
	if(mixSet.relativeReagentA == YES) {
		if(nextVial(&devState.reagentVialA)) {						}
		else { /* error */}
dfp("relative reagentVialA.uPos (%d,%d)\n",devState.reagentVialA.uPos.x, devState.reagentVialA.uPos.y);
	}
	if(mixSet.relativeReagentB == YES) {
		if(nextVial(&devState.reagentVialB)) {}
		else { /* error */}
dfp("relative reagentVialB.uPos (%d,%d)\n",devState.reagentVialB.uPos.x, devState.reagentVialB.uPos.y);
	}
	if(mixSet.relativeReagentC == YES) {
		if(nextVial(&devState.reagentVialC)) {						}
		else { /* error */}						
dfp("relative reagentVialC.uPos (%d,%d)\n",devState.reagentVialC.uPos.x, devState.reagentVialC.uPos.y);
	}
	if(mixSet.relativeReagentD == YES) {
		if(nextVial(&devState.reagentVialD)) {						}
		else { /* error */}						
dfp("relative reagentVialD.uPos (%d,%d)\n",devState.reagentVialD.uPos.x, devState.reagentVialD.uPos.y);
	}
}

int	mix_ProgramNew()
{
		switch(mixSet.mixTable[devState.mixTableNo].action)	{
			case MIX_ACTION_NONE:	// 테이블의 끝이다 - 다음 바이알 설정
				return ACTION_MENU_END;
				break;
			case MIX_ACTION_ADD:
// 1.	air	segment	?uL
// 2.	go vial	=> mixTable[i].addFrom (sample,	reagentA or	B, wash)
//						=> wash일	경우는 밸브	작동
// 3.	syringe	=> mixTable[i].addVol
// 4.	go vial	=> mixTable[i].addTo (sample,	Destination)
// 5.	syringe	=> position	0	or (XXX	mixTable[i].addVol)

// 6.	Washing

// 1.세척은	mix_Program()진입해서	일단 세척한다.

// 2.from
// 3.to
// 4.세척
				switch(mix_addPrgmNew(devState.mixTableNo)) {
					case ACTION_MENU_ING:
						break;
					case ACTION_MENU_END:
						++devState.mixTableNo;
						break;
					case ACTION_MENU_ERR_VIAL:
//						++devState.mixTableNo;
						devState.mixTableNo = 0;
						// check next sample - 현재 샘플은 뛰어 넘고
//						devState.actionSubmenu = AS_MIX_SUB_SET_VIAL;
						return ACTION_MENU_ERR_VIAL;
						break;
					default:
						devState.mixTableNo = 0;
						return ACTION_MENU_ERR_VIAL;
				}

/*				
				if(	mix_addPrgmNew(devState.mixTableNo) ) {
// error
// end
// skip
					
					dfp("mixSet.mixTable[%d].action = %d\n",devState.mixTableNo,mixSet.mixTable[devState.mixTableNo].action);
					++devState.mixTableNo;
				}
*/				
				break;
			case MIX_ACTION_MIX:
// 1.	go vial	=> Destination

// 1.세척은	mix_Program()진입해서	일단 세척한다.

// 2.to에	해당하는 것을	mix
// 3.세척
#if 1
				switch(mix_mixPrgmNew(devState.mixTableNo)) {
					case ACTION_MENU_ING:
						break;
					case ACTION_MENU_END:
						++devState.mixTableNo;
						break;
					case ACTION_MENU_ERR_VIAL:
//						++devState.mixTableNo;
						devState.mixTableNo = 0;
						// check next sample - 현재 샘플은 뛰어 넘고
//						devState.actionSubmenu = AS_MIX_SUB_SET_VIAL;
						return ACTION_MENU_ERR_VIAL;
						break;
					default:
						devState.mixTableNo = 0;						
						return ACTION_MENU_ERR_VIAL;
						
				}
#else
				if(	mix_mixPrgmNew(devState.mixTableNo) ) ++devState.mixTableNo;
#endif				
				break;
			case MIX_ACTION_WAIT:
// 1.	add	나 mix 동작이	끝난이후 대기	시간
//				if(	mix_waitPrgmNew(devState.mixTableNo)	)	++devState.mixTableNo;
				switch(mix_waitPrgmNew(devState.mixTableNo)) {
					case ACTION_MENU_ING:
						break;
					case ACTION_MENU_END:
						++devState.mixTableNo;
						break;
					default:
						devState.mixTableNo = 0;						
						return ACTION_MENU_ERR_VIAL;
						
				}
				break;
		}

/*
			if(mixSet.firstDestPos.pos ==	devState.sample.pos
					|| mixTable[i].addFrom ==	MIX_FROM_SAMPLE
					|| mixTable[i].addTo ==	MIX_TO_SAMPLE	){}
*/
	return ACTION_MENU_ING;

}

int	mix_addPrgmNew(unsigned int order)
{
	int32_t	readCount=0;
	static int encoderCount;
	static int retVal;

	static uint32_t	syringeSpeed;
	static float addVolume;
	static int	aspirateReturn = ACTION_MENU_ING;

	static VIAL_POS_t* pVial;

	static float airSegmentVol = 0.0f;
	static float flushVolume = 0.0f;
//	static unsigned char curVial = 0;		// 0 : sample , 1 : reggent 나 기타 

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_map !=	Menu)	{
		dfp("===================================subMenu_map=%d\n",subMenu_map);
		Menu = subMenu_map;
}
#endif

		switch(subMenu_map)	{
				case 1:
					aspirateReturn = ACTION_MENU_ING;
					++subMenu_map;
					break;
				case 2:	// before washing
					if(Washing(runSetup.washNeedleBeforeInj,	&(devState.curNdlWashBeforeCnt), WASH_NEEDLE_ALL)) {
						++subMenu_map;
					}
					break;
				case 3:
					 if(sleepChkVal(MOTOR_VAL))	{	motorReadyCmd(MOTOR_VAL);}
					 
						switch(mixSet.mixTable[order].addFrom) {
							case MIX_FROM_SAMPLE:
								pVial	=	&devState.sample;
//								curVial = MIX_FROM_SAMPLE;
//							curVial = 0;																				
								break;
							case MIX_FROM_REAGENT_A:
								pVial	=	&devState.reagentVialA;
//								curVial = 1;								
								break;
							case MIX_FROM_REAGENT_B:
								pVial	=	&devState.reagentVialB;																
//								curVial = 1;								
								break;
							case MIX_FROM_REAGENT_C:
								pVial	=	&devState.reagentVialC;
//								curVial = 1;								
								break;								
							case MIX_FROM_REAGENT_D:
								pVial	=	&devState.reagentVialD;
//								curVial = 1;								
								break;								
							case MIX_FROM_WASH:		// 나중에	고려하자
//								pVial	=	&devState.sample;
//								goPosETC(POS_XY_WASTE);
//								curVial = 1;
								break;
						}
						++subMenu_map;
						break;
				case 4:
						if(readyChkVal(MOTOR_VAL))	{
					retVal =		goVal_Encoder(POS_VAL_NEEDLE,	0,ACT_SPEED_MAX_VAL, ACT_XY_POS);
dp("mix_addPrgm!!\n");
							++subMenu_map;
						}
						break;
				case 5:
						if(readyChkVal(MOTOR_VAL))	{
	readyChkEncoder_Val(); 								
							airSegmentVol = runSetup.airSegmentVol;
#if 0 //FPGA_VERSION_TRIM	
							if(runSetup.airSegment ==	YES) {	// 에어세그먼트
								encoderCount = goSyr_Encoder(airSegmentVol,	0,devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1], ACT_XY_POS);	// 처음이라	절대위치
							}
							else {
								encoderCount = goSyr_Encoder(0.0f,	0,devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1], ACT_XY_POS);	// 처음이라	절대위치
							}
#else
							if(runSetup.airSegment ==	YES) {	// 에어세그먼트
								encoderCount = goSyr_Encoder(airSegmentVol,	0,devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1], ACT_XY_POS);	// 처음이라	절대위치
							}
#endif							
							++subMenu_map;
						}
						break;
				case 6:
					if(readyChk(MOTOR_SYR))	{
if(runSetup.airSegment ==	YES) {							
readyChkEncoder_Syr();
}
						addVolume	=	mixSet.mixTable[order].addVol;
						syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1];
						++subMenu_map;
					}
					break;

				case 7:	// 흡입	addFrom  // 동작이 끝나면 Z위치는 Top 위치
					aspirateReturn = aspirateSample(pVial, addVolume,	syringeSpeed);
//					retErr = aspirateReturn;
//dfp("===================================aspirateReturn = %d	\n",aspirateReturn);
					if(aspirateReturn	== ACTION_MENU_END)	{
// 마지막에	에어세그먼트 뺄까	말까?
						subMenu_map	=	9;
					}
					else if(aspirateReturn ==	ACTION_MENU_ING) {	}
					else if(aspirateReturn ==	ACTION_MENU_ERR_VIAL)	{
							if(runSetup.skipMissingSample	== YES)	{
								errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
	dfp("===================================missing %d	\n",devState.missVialResponse);
if(devState.mixTableNo == 0) {
	// 다음 샘플로 바로 
//						chkNextInjectionMix();
	if(chkNextInjectionMix()) {}	// 다음 샘플로 바로 	
	else {
return aspirateReturn;	
		};


}
else {
	// Next Vial	// skip
	//devState.missVialSkip = YES;
	//pVial	=	&devState.sample;// 다음  mix를 실행하기 위해서 
								subMenu_map	=	12;
}	
							}
							else {
								errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
								// 제어기 응답을 기다린다.														
								subMenu_map++;
							}
						}							
//					}
					break;

				case 8:
// 제어기 응답을 기다린다.// [skip]  - 다음 바이알 [retry] - 다시 시도  [abort] - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
// skip							
//devState.missVialSkip = YES;
dfp("===================================missing %d	\n",devState.missVialResponse);
dfp("===================================missing %d	\n",devState.missVialResponse);
//pVial	=	&devState.sample;// 다음  mix를 실행하기 위해서 
							subMenu_map	=	12;	
//							return ACTION_MENU_END;
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
							subMenu_map = 7;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("===================================missing %d	\n",devState.missVialResponse);
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();
subMenu_map = 1;
							break;
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;				

// ==================================
// 배출
				case 9:
					if(readyChk(MOTOR_SYR))	{
						switch(mixSet.mixTable[order].addTo) {
							case MIX_TO_SAMPLE:
								pVial	=	&devState.sample;
devState.injectVial.pos = devState.sample.pos;
//								curVial = MIX_TO_SAMPLE;
//							curVial = 0;																												
								break;
							case MIX_TO_DESTINATION:
								pVial	=	&devState.firstDestVial;
devState.injectVial.pos = devState.firstDestVial.pos;
//								curVial = 1;
								break;
						}
						if(runSetup.airSegment ==	YES) {	// 에어세그먼트
//							addVolume	=	mixTable[order].addVol + airSegmentVol;
							addVolume	=	mixSet.mixTable[order].addVol	+	airSegmentVol * 2;
						}
						else {
							addVolume	=	mixSet.mixTable[order].addVol	+	airSegmentVol;
						}
						syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1];
						++subMenu_map;
					}
					break;

				case 10:	// 배출	 addTo // 동작이 끝나면 Z위치는 Top 위치 			
					aspirateReturn = aspirateSample(pVial, -addVolume, syringeSpeed);
					if(aspirateReturn	== ACTION_MENU_END)	{
						subMenu_map	=	12;
					}
					else if(aspirateReturn ==	ACTION_MENU_ING) {	 }
					else if(aspirateReturn ==	ACTION_MENU_ERR_VIAL)	{
#if 0						
						if(curVial != 0) {	// sample이 아니면 
// 시퀀스 끝내야 한다. 분석불가능 		
errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
aspirateReturn = ACTION_MENU_ERR_VIAL_END_MIX;
subMenu_map	=	12;
						}
						else {
#endif							
							if(runSetup.skipMissingSample	== YES)	{
								errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
	// skip				
	//devState.missVialSkip = YES;
	dfp("===================================missing %d	\n",devState.missVialResponse);
	//pVial	=	&devState.sample;// 다음  mix를 실행하기 위해서 
								subMenu_map	=	12;	// 세척부터 다시
//	return ACTION_MENU_END;
							}
							else {
								errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
								subMenu_map++;
							}
						}
//==================================					
// jump
//==================================					

//					}
					break;
				case 11:
// 제어기 응답을 기다린다.// [skip]  - 다음 바이알 [retry] - 다시 시도  [abort] - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
// skip				
//devState.missVialSkip = YES;
dfp("===================================missing %d	\n",devState.missVialResponse);
dfp("===================================missing %d	\n",devState.missVialResponse);
//pVial	=	&devState.sample;// 다음  mix를 실행하기 위해서 
							subMenu_map	=	12;// 세척부터 다시
//							return ACTION_MENU_END;
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
							subMenu_map = 10;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("MISSING_VIAL_STOP %d	\n",devState.missVialResponse);
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();
							subMenu_map = 1;
							break;
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;				

				case 12:	// after washing
					if(Washing(runSetup.washNeedleAfterInj, &(devState.curNdlWashAfterCnt), WASH_NEEDLE_ALL)) {
						++subMenu_map;
					}
					break;
				case 13:
					subMenu_map	=	1;
//					return ACTION_MENU_END;
					return aspirateReturn;
					break;
			
			}

	return ACTION_MENU_ING;

}
 
// Destination Vial이	사용되지 않으면	Sample Vial을	Mix	한다.	-- Target	Vial을 Mix
// 1.	에어 세그먼트	-	5~10uL
// 2.	흡입 - mixSet.mixVol
// 3.	배출 - mixSet.mixVol
// 4.	반복 횟수	만큼 반복
// 5.	에어 세그먼트(전체)	만큼 배출
//uint8_t subMenu_mmp	=	1;
int	mix_mixPrgmNew(unsigned int order)
{
	int32_t	readCount=0;
	static int encoderCount;
	static int retVal;

	static uint32_t	syringeSpeed;
	static float addVolume;
	static uint32_t	mixTimes = 0;
	uint32_t needleDepth;
	uint32_t needleDepthStep;
	static int	aspirateReturn;

	static VIAL_POS_t* pVial;

	static float airSegmentVol = 0.0f;
	static float flushVolume = 0.0f;
//	static unsigned char curVial = 0;		// 0 : sample , 1 : reggent 나 기타 	

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	Menu = 0;
if(subMenu_mmp !=	Menu)	{
		dfp("subMenu_mmp=%d\n",subMenu_mmp);
		Menu = subMenu_mmp;
}
#endif

	switch(subMenu_mmp)	{
		case 1:
			aspirateReturn = ACTION_MENU_ING;
			++subMenu_mmp;			
			break;
		case 2:	// before washing
			if(Washing(runSetup.washNeedleBeforeInj,	&(devState.curNdlWashBeforeCnt), WASH_NEEDLE_ALL)) {
				++subMenu_mmp;
			}
			break;
		case 3:
			if(sleepChkVal(MOTOR_VAL)) { motorReadyCmd(MOTOR_VAL);}
				switch(mixSet.mixTable[order].mixVial) {
					case MIX_TO_SAMPLE:
						pVial	=	&devState.sample;
						break;
					case MIX_TO_DESTINATION:
						pVial	=	&devState.firstDestVial;	
						break;
				}
				++subMenu_mmp;
			break;
		case 4:
			if(readyChkVal(MOTOR_VAL))	{
			retVal =	goVal_Encoder(POS_VAL_NEEDLE,	0,ACT_SPEED_MAX_VAL, ACT_XY_POS);
				++subMenu_mmp;
			}
			break;
		case 5:
			if(readyChkVal(MOTOR_VAL))	{
readyChkEncoder_Val(); 			
//				if(runSetup.airSegment ==	YES) {	// 에어세그먼트
					// 무조건	에어세그먼트 삽입	5	~	10uL
					airSegmentVol = runSetup.airSegmentVol;
				encoderCount = goSyr_Encoder(airSegmentVol,	0,devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1], ACT_XY_POS);	// 처음이라	절대위치
//				}
				++subMenu_mmp;
			}
			break;
		case 6:
			if(readyChk(MOTOR_SYR))	{
readyChkEncoder_Syr();
				++subMenu_mmp;
			}
			break;


		case 7:
			if(goPosVial(pVial)) {
				++subMenu_mmp;
			}
			break;
		case 8:
			if(readyChk(MOTOR_X) &&	readyChk(MOTOR_Y)) {
					if(pVial->uPos.tray	== TRAY_LEFT)	{
			encoderCount =				goZ_Encoder(POS_Z_VIAL_LEFT_UPPER_DETECT,	ACT_XY_POS);
					}
					else {
			encoderCount =				goZ_Encoder(POS_Z_VIAL_RIGHT_UPPER_DETECT, ACT_XY_POS);
					}
					++subMenu_mmp;
devSet.posCheckEnable = YES;
devSet.posCheckMotor = MOTOR_Z;
devSet.posCheck1 = 0x30; // 검출 x - 이동전 
devSet.posCheck2 = 0x20; // 니들 이동 검출
			}
			break;
    case 9:	// Vial Detection upper 
			if(readyChkRetry_Z(encoderCount)) {
      	devSet.posCheckEnable = NO;

				if(pVial->uPos.tray == TRAY_LEFT) {
	      	encoderCount =    goZ_Encoder(POS_Z_VIAL_LEFT_DETECT, ACT_XY_POS);
				}
				else {
					encoderCount =     goZ_Encoder(POS_Z_VIAL_RIGHT_DETECT, ACT_XY_POS);
				}
        ++subMenu_mmp;
      }
      break;	
		case 10:
			if(readyChkRetry_Z(encoderCount)) {
				if((devState.btMotorState[MOTOR_Z] & 0x30) ==	 0x00) {					
					subMenu_mmp = 12;
					aspirateReturn = ACTION_MENU_END;
				}
				else {	// skip	vial - 바이알	없음.
				
#if 0
					if(curVial != 0) {	// sample이 아니면 
// 시퀀스 끝내야 한다. 분석불가능 		
errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
aspirateReturn = ACTION_MENU_ERR_VIAL_END_MIX;
subMenu_map	=	12;
					}
					else {
						
#endif				
						if(runSetup.skipMissingSample	== YES)	{
							errorFunction(SELF_ERROR_MISSING_VIAL_SKIP);
#if 1
	devState.vialExist = NO;
	//pVial	=	&devState.sample;// 다음  mix를 실행하기 위해서 
#endif			
	dfp("===================================missing %d	\n",devState.missVialResponse);

							aspirateReturn = ACTION_MENU_ERR_VIAL;
							
subMenu_mmp	=	22;
							mixTimes = 0;
						}
						else {
							errorFunction(SELF_ERROR_MISSING_VIAL_WAIT);
							subMenu_mmp++;
	//						mixTimes = 0;
						}
					}
				}
//			}
			break;

		case 11:
// 제어기 응답을 기다린다.// [skip]  - 다음 바이알 [retry] - 다시 시도  [abort] - ( stop ,cancel , end)  - 현재 시퀀스 분석종료
					switch(devState.missVialResponse) {
						case MISSING_VIAL_NO_RESPONSE:
							break;
						case MISSING_VIAL_SKIP:
// skip				
//devState.missVialSkip = YES;
#if 1
devState.vialExist = NO;
//pVial	=	&devState.sample;// 다음  mix를 실행하기 위해서 
#endif
subMenu_mmp	=	23;
						mixTimes = 0;
							break;
						case MISSING_VIAL_RETRY:
//=======================================================================
// Retry
//=======================================================================
							subMenu_mmp = 6;
							break;
//						case MISSING_VIAL_ABORT:
						case MISSING_VIAL_STOP:
dfp("missing %d	\n",devState.missVialResponse);
// COMMAND_STOP가 똑같은 동작 실행
							commandStopFn();
							subMenu_mmp = 1;
							break;
					}
					devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
					break;		
			
		case 12:
			if(readyChk(MOTOR_Z))	{
if(pVial->uPos.tray == TRAY_LEFT) {
					needleDepth = devSet.left_z_bottom - runSetup.needleHeight;
}
else {
					needleDepth = devSet.right_z_bottom - runSetup.needleHeight;
}
					needleDepthStep	=	(int)(needleDepth	/	MM_PER_STEP_Z	+	0.5) * (int)(MICRO_STEP_Z);
		encoderCount =				goZ_Encoder(needleDepthStep, ACT_MOVE_STEP_ABSOLUTE);
					++subMenu_mmp;
			}
			break;
		case 13:
			if(readyChkRetry_Z(encoderCount)) {
				++subMenu_mmp;
			}
			break;

		case 14:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_mmp,NEXT_CASE_DELAY_RUN);
		break;

//==================================					
// repeat
//==================================					
		case 15:	// 흡입
			addVolume	=	mixSet.mixTable[order].mixVol;
// 속도	빠르게 조정
			syringeSpeed = devSet.syrSpeedInj[runSetup.syringeDnSpdInj-1];
		encoderCount =	goSyr_Encoder(addVolume, 0,syringeSpeed, ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
			++subMenu_mmp;
			break;
		case 16:
			if(readyChk(MOTOR_SYR))	{
			readyChkEncoder_Syr();
			++subMenu_mmp;
			}
			break;

		case 17:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_mmp,	WAIT_MIX_PROGRAM);
		break;

		case 18:
syringeSpeed = devSet.syrSpeedInj[runSetup.syringeUpSpdInj-1];
	encoderCount =	goSyr_Encoder(-addVolume,	0,syringeSpeed,	ACT_XY_POS_ADD);	// 현재위치에서	추가로 흡입하거나	배출
			++subMenu_mmp;
			break;
		case 19:
			if(readyChk(MOTOR_SYR))	{
			readyChkEncoder_Syr();
			++subMenu_mmp;
				//WAIT_TIME_VALUE_CASE_STATE(subMenu_mmp,	WAIT_MIX_PROGRAM);
			}
			break;
		case 20:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_mmp,	WAIT_MIX_PROGRAM);
		break;

		case 21:	// 반복	횟수
			if(++mixTimes	== mixSet.mixTable[order].mixTimes)	{	// 마지막에는	전부 배출
		encoderCount =		goSyr_Encoder(POS_SYR_ZERO, 0,syringeSpeed, ACT_XY_POS);	// 현재위치에서	추가로 흡입하거나	배출
				mixTimes = 0;
				++subMenu_mmp;
			}
			else {
//==================================					
// repeat
//==================================					
dfp("repeat\n");

subMenu_mmp	=	15;	// 반복
			}
			break;
		case 22:	// after washing
			if(Washing(runSetup.washNeedleAfterInj, &(devState.curNdlWashAfterCnt), WASH_NEEDLE_ALL)) {
//				subMenu_mmp = 19;
				subMenu_mmp	=	1;
				return ACTION_MENU_END;
			}
			break;
			
//==================================					
// jump
//==================================					
		case 23:
			if(readyChk(MOTOR_SYR))	{
			encoderCount =	 goZ_Encoder(POS_Z_TOP,	ACT_XY_POS);
				 ++subMenu_mmp;
			}
			break;
		case 24:
			if(readyChkRetry_Z(encoderCount)) {
				++subMenu_mmp;
			}
			break;

		case 25:
			subMenu_mmp	=	1;
			return aspirateReturn;
			break;
	}

	return ACTION_MENU_ING;
}

//uint8_t subMenu_mwp	=	1;
int	mix_waitPrgmNew(unsigned	int	order)
{
static uint32_t waitTime = 0;
static uint32_t waitTime1 = 0;

#if 0
	switch(subMenu_mwp)	{
		case 1:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_mwp,	mixSet.mixTable[order].waitTime	*	CONVERT_WAIT_TIME_TO_COUNT);
			break;
		case 2:
			subMenu_mwp = 1;
			return ACTION_MENU_END;
	}
	return ACTION_MENU_ING;
#endif

	switch(subMenu_mwp)	{
		case 1:
			waitTime = mixSet.mixTable[order].waitTime	*	10000;
			if(bInitTicks == NO) {
					init_ticks(&nextCaseDelay);
					bInitTicks = YES;
			}
			else {
				subMenu_mwp = 2;
			}
			break;
		case 2:
			if(mixSet.mixTable[order].waitTime > 60) {
				subMenu_mwp++;
			}
			else {
				subMenu_mwp = 6;
			}
			break;
		case 3:	// sleep
			if(sleepModeAll()) subMenu_mwp++;
			break;
		case 4:
			get_System_ticks();
			if((sysTicks - nextCaseDelay) > (waitTime1))	{
	        ++subMenu_mwp;
//	        bInitTicks = NO;
	    }
	    break;
		case 5:
			if(wakeup_XYZ_checkHome()) subMenu_mwp = 6;
			break;
		case 6:
			get_System_ticks();
			if((sysTicks - nextCaseDelay) > (waitTime))	{
	        ++subMenu_mwp;
	        bInitTicks = NO;
	    }
	    break;
		case 7:
			subMenu_mwp = 1;
			return ACTION_MENU_END;
	}
	
	return ACTION_MENU_ING;
	
}

//#define	AUX_OFF					0
//#define	AUX_ON					1
//#define	AUX_TOGGLE			2
int	time_Event()
{
//	devState.analysisTime100msCnt;
//	timeEvent.auxEvent[i].fTime;
//	timeEvent.auxEvent[i].auxOnoff
//	devState.curAuxEventNo = 0;
//	devState.curAuxState = AUX_ON;

	if(devState.curAuxOutIng ==	YES) {
		if(++devState.auxOutSigTimeCnt >=	devState.auxOutSigTime)	{	// 시그널	출력(on,off) 제어
			if(devState.curAuxState	== AUX_CONTACT_OPEN) {
				AUX_SIG_OUT_CONTACT_CLOSE;
dfp("AUX_SIG_OUT_CONTACT_CLOSE\n\r");				
			}
			else {
				AUX_SIG_OUT_CONTACT_OPEN;
dfp("AUX_SIG_OUT_CONTACT_OPEN\n\r");								
			}
			devState.curAuxOutIng	=	NO;
		}
	}

//	if(timeEvent.endTime <=	devState.analysisTime100msCnt) {
	if(timeEvent.auxEvent[devState.curAuxEventNo].fTime >= 0.0) {
//		if(devState.analysisTime100msCnt >=	timeEvent.auxEvent[devState.curAuxEventNo].fTime)	{
		if(devState.analysisTime100msCnt >=	devState.auxTime100msCnt[devState.curAuxEventNo])	{
			switch(timeEvent.auxEvent[devState.curAuxEventNo].auxOnoff)	{
				case AUX_CONTACT_OPEN:
dfp("AUX_CONTACT_OPEN devState.analysisTime100msCn=%d\n\r",devState.analysisTime100msCnt);
					devState.curAuxState = AUX_CONTACT_OPEN;
					devState.curAuxOutIng	=	NO;
//					devState.auxOnOff	=
//					devState.auxOnOffCnt =
					break;
				case AUX_CONTACT_CLOSE:
dfp("AUX_CONTACT_CLOSE devState.analysisTime100msCn=%d\n\r",devState.analysisTime100msCnt);					
					devState.curAuxState = AUX_CONTACT_CLOSE;
					devState.curAuxOutIng	=	NO;
					break;
				case AUX_TOGGLE:
					if(devState.curAuxState	== AUX_CONTACT_OPEN) {
dfp("AUX_TOGGLE---AUX_CONTACT_OPEN devState.analysisTime100msCn=%d\n\r",devState.analysisTime100msCnt);																
						AUX_SIG_OUT_CONTACT_CLOSE;
						devState.curAuxState = AUX_CONTACT_CLOSE;
					}
					else {
dfp("AUX_TOGGLE---AUX_SIG_OUT_CONTACT_CLOSE devState.analysisTime100msCn=%d\n\r",devState.analysisTime100msCnt);																						
						AUX_SIG_OUT_CONTACT_OPEN;
						devState.curAuxState = AUX_CONTACT_CLOSE;
					}
					devState.curAuxOutIng	=	NO;
					break;
				case AUX_PULSE:
dfp("AUX_PULSE devState.analysisTime100msCn=%d\n\r",devState.analysisTime100msCnt);
					devState.curAuxState = AUX_CONTACT_OPEN;
					devState.curAuxOutIng	=	YES;
					break;
			}

dfp("devState.curAuxEventNo = %d\n\r",devState.curAuxEventNo);																								
			++devState.curAuxEventNo;

		}
	}
	else {	// end
dfp("time_Event end \n\r");
pTime_Event_Fn = time_Event_Blank;
	}
}

int	time_Event_Blank()
{
}

//int	errorFunction(unsigned char	err)
int	errFunction(unsigned char	err)
{
dfpErr("errorFunction=%d\n",err);
	switch(err)	{
		case SELF_ERROR_COMMAND_UNKNOWN:
			send_SelfMessage(SELF_ERROR, SELF_ERROR_COMMAND_UNKNOWN);
//			LcasState.errorCode = err;
//			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_NO_ADJUSTMENT_DATA:
			send_SelfMessage(SELF_ERROR, SELF_ERROR_NO_ADJUSTMENT_DATA);
			break;
		case SELF_ERROR_MISSING_VIAL_SKIP:
			send_SelfMessage(SELF_ERROR, SELF_ERROR_MISSING_VIAL_SKIP);
			break;
		case SELF_ERROR_MISSING_VIAL_WAIT:
			send_SelfMessage(SELF_ERROR, SELF_ERROR_MISSING_VIAL_WAIT);
			break;
		case SELF_ERROR_OPERATION:
			send_SelfMessage(SELF_ERROR, SELF_ERROR_OPERATION);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_DOOR_OPEN: //	(6) // 도어열림
			send_SelfMessage(SELF_ERROR, SELF_ERROR_DOOR_OPEN);		
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_LEAK: //			(7) // Leak
			send_SelfMessage(SELF_ERROR, SELF_ERROR_LEAK);		
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_TEMPERATURE: //	(8) // 온도 이상
			send_SelfMessage(SELF_ERROR, SELF_ERROR_TEMPERATURE);		
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_MOTOR_X_AXIS: //					9 	
			send_SelfMessage(SELF_ERROR, err);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_MOTOR_Y_AXIS: //					10 	
			send_SelfMessage(SELF_ERROR, err);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_MOTOR_Z_AXIS: //					11 	
			send_SelfMessage(SELF_ERROR, err);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_MOTOR_SYRINGE: //				12 	
			send_SelfMessage(SELF_ERROR, err);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_MOTOR_VALVE: //					13 	
			send_SelfMessage(SELF_ERROR, err);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_MOTOR_INJECTOR: //				14 	
			send_SelfMessage(SELF_ERROR, err);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_POSITION_XY: //					15
			send_SelfMessage(SELF_ERROR, err);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		case SELF_ERROR_POSITION_Z: //						16
			send_SelfMessage(SELF_ERROR, err);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
		default:	//#define SELF_ERROR_CODE_MAX		(255)
			send_SelfMessage(SELF_ERROR, err);
			LcasState.errorCode = err;
			stateChange(STATE_FAULT);
			break;
			
	}
/*
	// 에러시	조치사항
	// PC로	Selfmessage전송
	if(runSetup.skipMissingSample	== YES)	{
		// selfMessage 전송
		// 계속진행
	}
	else {
		// selfMessage 전송
		// 에러후	멈춤
		// PC	명령 기다림	-	계속 / stop	/	skip ...
	}
*/
	return 0;
}

int actionNone(int value)
{
}

int actionInitilize(int value)	// AS_ACTION_INITILIZE
{

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
//iprintf("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif
	
			switch(devState.actionSubmenu) {
				case AS_INIT_SUB_NONE:
					if(devSet.adjustState	== YES)	devState.actionSubmenu = AS_INIT_SUB_FIND_HOME_ALL;
					else {	// adjust	data 저장안됨.
						devState.actionSubmenu = AS_ADJUST_HOME_POS;
						devState.actionMenu	=	AS_ACTION_ADJUST;
						errorFunction(SELF_ERROR_NO_ADJUSTMENT_DATA);
					}
					break;
				case AS_INIT_SUB_FIND_HOME_ALL:
					if(initHomePos_All())	{
						if(devSet.adjustState	== YES)	{
							devState.actionSubmenu = AS_INIT_SUB_WASH_SYRINGE;
						}
						else {
							devState.actionSubmenu = AS_INIT_SUB_NONE;
							devState.actionMenu	=	AS_ACTION_ADJUST;
						}
					}
					break;
				case AS_INIT_SUB_WASH_SYRINGE:
					if(Washing(WASH_TIMES_DEFAULT_INIT,	&(devState.curNdlWashAfterCnt),	WASH_NEEDLE_INT))	{
						devState.actionSubmenu = AS_INIT_SUB_WASH_NEEDLE;
					}
					break;
				case AS_INIT_SUB_WASH_NEEDLE:
					if(Washing(WASH_TIMES_DEFAULT_INIT,	&(devState.curNdlWashAfterCnt),	WASH_NEEDLE_ALL))	{
						devState.actionSubmenu = AS_INIT_SUB_WASH_INJECTOR;
					}
					break;
				case AS_INIT_SUB_WASH_INJECTOR:
					if(Washing(WASH_TIMES_DEFAULT_INIT,	&(devState.curNdlWashAfterCnt),	WASH_INJECTOR))	{
						devState.actionSubmenu = AS_INIT_SUB_WASH_END_GO_HOME;
					}
					break;
				case AS_INIT_SUB_WASH_END_GO_HOME:
					if(goPosHome())	{
						devState.actionSubmenu = AS_INIT_SUB_SLEEP;
					}
					break;
				case AS_INIT_SUB_SLEEP:
					if(sleepModeAll()) {
						devState.actionMenu	=	AS_ACTION_READY;
						devState.actionSubmenu = AS_READY_SUB_NONE;
						stateChange(STATE_READY);
					}
					break;
			}	
}

int actionReady(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif

			switch(devState.actionSubmenu) {
				case AS_READY_SUB_NONE:
					break;
/*					
				case AS_READY_SUB_REMOTE_OUT:
//					REMOTE_READY_OUT_CONTACT_CLOSE;
					stateChange(STATE_READY);
//					devState.seqAction = YES;
					devState.actionSubmenu = AS_READY_SUB_NONE;
					break;
*/					
			}

}

int actionMix(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif

	switch(devState.actionSubmenu) {
		case AS_MIX_SUB_NONE:
			if(mixSet.useMixPrgm ==	YES /* && devState.mixPrgmFinish	== NO && mixSet.mixTable[0].action != MIX_ACTION_NONE*/) {	// 첫테이블이 존재 
				devState.actionSubmenu = AS_RUN_SUB_WAKE_UP;
			}
			else {
				devState.actionMenu	=	AS_ACTION_RUN;
				devState.actionSubmenu = AS_RUN_SUB_NONE;
			}
			break;
		case AS_MIX_SUB_WAKE_UP:
	// wake up - home sensor check
			if(wakeup_XYZ_checkHome()) {
				devState.actionSubmenu = AS_MIX_SUB_PRGM;
			}
			break;
		case AS_MIX_SUB_PRGM:
			switch(mix_ProgramNew()) {
				case ACTION_MENU_ING:
					break;
				case ACTION_MENU_END:
					devState.actionMenu	=	AS_ACTION_RUN;
					devState.actionSubmenu = AS_RUN_SUB_WASH_BETWEEN_CHECK_BEFORE;
					break;
				case ACTION_MENU_ERR_VIAL:
// 현재 mix프로그램이 완벽하게 실행되지 못하여 다음 샘플로 이동한다.
//					devState.actionMenu	=	AS_ACTION_RUN;						
//					devState.actionSubmenu = AS_RUN_SUB_SET_NEXT_VIAL;
					devState.actionSubmenu = AS_MIX_SUB_SET_NEXT_VIAL;
					break;
				case ACTION_MENU_ERR_VIAL_END_MIX:
					devState.actionMenu	=	AS_ACTION_RUN;						
					devState.actionSubmenu = AS_RUN_SUB_END_SEQUENCE;
					break;
				default:
					break;							
			}
			break;
		case AS_MIX_SUB_SET_NEXT_VIAL:
			if(chkNextInjectionMix()) {					
					devState.actionSubmenu = AS_MIX_SUB_PRGM;			
//					devState.mixTableNo = 0;
			}
			else {
				devState.actionMenu	=	AS_ACTION_RUN;												
				devState.actionSubmenu = AS_RUN_SUB_END_SEQUENCE;

				devState.mixPrgmFinish = NO;
				devState.mixTableNo = 0;
				
				devState.seqAction = NO;
			}
					
/*
#if MIX_MISS_VIAL_FIX
#else
		devState.vialExist = YES;
#endif
		devState.mixTableNo	=	0;	// 끝	
		return ACTION_MENU_END;
	}					
*/					

					break;				
		}
				
}
		
//int actionRunNew(int value)
int actionRun(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif

			switch(devState.actionSubmenu) {
				case AS_RUN_SUB_NONE:
//					devState.actionSubmenu = AS_RUN_SUB_WASH_BEFORE_CHECK;	// AS_RUN_SUB_MIX_PRGM
//					devState.actionSubmenu = AS_RUN_SUB_MIX_PRGM;
					devState.actionSubmenu = AS_RUN_SUB_WAKE_UP;
					break;
				case AS_RUN_SUB_WAKE_UP:
// wake up - home sensor check
					if(wakeup_XYZ_checkHome()) {
//						if(devState.doNextSample ==	YES)	{
							devState.actionSubmenu = AS_RUN_SUB_WASH_BETWEEN_CHECK_BEFORE;
//							devState.mixPrgmFinish = NO;
					}
					break;
				
				case AS_RUN_SUB_WASH_BETWEEN_CHECK_BEFORE:
					if(runSetup.washBetween	== WASH_BETWEEN_NONE)	devState.actionSubmenu = AS_RUN_SUB_GO_VIAL_AND_INJECTION;
					else if(runSetup.washBetween ==	WASH_BETWEEN_INJECTIONS) devState.actionSubmenu	=	AS_RUN_SUB_NEEDLE_WASH_BEFORE;
					else if(runSetup.washBetween ==	WASH_BETWEEN_VIALS)	{
#if WASH_BETWEEN_VIALS_BEFORE_FIX					
						if(devState.doNextSample ==	YES) 
							devState.actionSubmenu	=	AS_RUN_SUB_NEEDLE_WASH_BEFORE;
						else	
							devState.actionSubmenu	=	AS_RUN_SUB_GO_VIAL_AND_INJECTION;
#else
//						if(devState.doNextSample ==	YES) 
						devState.actionSubmenu	=	AS_RUN_SUB_NEEDLE_WASH_BEFORE;
//						else														 devState.actionSubmenu	=	AS_RUN_SUB_GO_VIAL;
#endif
					}
					else if(runSetup.washBetween ==	WASH_BETWEEN_ANALYSYS_END) devState.actionSubmenu	=	AS_RUN_SUB_GO_VIAL_AND_INJECTION;
					break;
				case AS_RUN_SUB_NEEDLE_WASH_BEFORE:
					if(runSetup.washNeedleBeforeInj	== 0)	devState.actionSubmenu = AS_RUN_SUB_INJECTOR_WASH_BEFORE;
					else if(Washing(runSetup.washNeedleBeforeInj,	&(devState.curNdlWashBeforeCnt), WASH_NEEDLE_ALL)) {
							devState.actionSubmenu = AS_RUN_SUB_INJECTOR_WASH_BEFORE;
					}
					break;
				case AS_RUN_SUB_INJECTOR_WASH_BEFORE:
					if(runSetup.washInjportBeforeInj ==	0) devState.actionSubmenu	=	AS_RUN_SUB_GO_VIAL_AND_INJECTION;
					else if(Washing(runSetup.washInjportBeforeInj, &(devState.curInjWashBeforeCnt),	WASH_INJECTOR))	{
							devState.actionSubmenu = AS_RUN_SUB_GO_VIAL_AND_INJECTION;
					}
					break;
				 case	AS_RUN_SUB_GO_VIAL_AND_INJECTION:
					if(pLoop_Injection_Fn(&devState.injectVial)) {
						devState.actionSubmenu = AS_RUN_SUB_SET_NEXT_VIAL;
					}
					break;
				case AS_RUN_SUB_SET_NEXT_VIAL:
					chkNextInjectionRun();

					if(devState.doNextSample == YES) {	// 다음 샘플
						devState.injectVial.pos = devState.sample.pos;					
					}
					else
					{
					}
					
					if(devState.vialExist	== NO)	{	
#if  WASH_BETWEEN_VIALS_BEFORE_FIX				
dfp("was\n");
//						if(devState.isNextInjection	== NO)  devState.actionSubmenu	=	AS_RUN_SUB_NEEDLE_WASH_AFTER;					
						if(devState.isNextInjection	== NO)  devState.actionSubmenu	=	AS_RUN_SUB_WASH_BETWEEN_CHECK_AFTER;					
						else 						devState.actionSubmenu = AS_RUN_SUB_REPEAT_INJECTION;
#else
// 바이알	없음 세척	필요없음 - AS_RUN_SUB_GO_VIAL_AND_INJECTION 에서 missing vial 검사
						devState.actionSubmenu = AS_RUN_SUB_REPEAT_INJECTION;
#endif
					}
					else {
dfp("was\n");
						devState.actionSubmenu = AS_RUN_SUB_WASH_BETWEEN_CHECK_AFTER;
//						devState.actionSubmenu = AS_RUN_SUB_NEEDLE_WASH_AFTER;
					}
					break;
				case AS_RUN_SUB_WASH_BETWEEN_CHECK_AFTER:
					if(runSetup.washBetween	== WASH_BETWEEN_NONE)	devState.actionSubmenu = AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
					else if(runSetup.washBetween ==	WASH_BETWEEN_INJECTIONS) devState.actionSubmenu	=	AS_RUN_SUB_NEEDLE_WASH_AFTER;
//					else if(runSetup.washBetween ==	WASH_BETWEEN_VIALS &&	devState.doNextSample	== YES)	devState.actionSubmenu = AS_RUN_SUB_NEEDLE_WASH_BEFORE;
					else if(runSetup.washBetween ==	WASH_BETWEEN_VIALS)	{
#if WASH_BETWEEN_VIALS_BEFORE_FIX										
						if(devState.isNextInjection	== NO) devState.actionSubmenu	=	AS_RUN_SUB_NEEDLE_WASH_AFTER;
						else if(devState.doNextSample ==	YES) devState.actionSubmenu	=	AS_RUN_SUB_NEEDLE_WASH_AFTER;
						else														 devState.actionSubmenu	=	AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
#else
						if(devState.doNextSample ==	YES) devState.actionSubmenu	=	AS_RUN_SUB_NEEDLE_WASH_AFTER;
						else														 devState.actionSubmenu	=	AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
#endif
					}
					else if(runSetup.washBetween ==	WASH_BETWEEN_ANALYSYS_END) {
#if WASH_BETWEEN_VIALS_BEFORE_FIX										
						if(devState.isNextInjection	== NO)  devState.actionSubmenu	=	AS_RUN_SUB_NEEDLE_WASH_AFTER;
						else 																devState.actionSubmenu	=	AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
#else
						devState.actionSubmenu	=	AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
#endif						
					}
					break;
				case AS_RUN_SUB_NEEDLE_WASH_AFTER:
//					if(runSetup.washBetween	== WASH_BETWEEN_NONE)	devState.actionSubmenu = AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
//					else {
						if(runSetup.washNeedleAfterInj ==	0) devState.actionSubmenu	=	AS_RUN_SUB_INJECTOR_WASH_AFTER;
						else if(Washing(runSetup.washNeedleAfterInj, &(devState.curNdlWashAfterCnt), WASH_NEEDLE_ALL)) {
							devState.actionSubmenu = AS_RUN_SUB_INJECTOR_WASH_AFTER;
						}
//					}
					break;
				case AS_RUN_SUB_INJECTOR_WASH_AFTER:
//					if(runSetup.washBetween	== WASH_BETWEEN_NONE)	devState.actionSubmenu = AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
//					else {
						if(runSetup.washInjportAfterInj	== 0)	devState.actionSubmenu = AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
						else if(Washing(runSetup.washInjportAfterInj,	&(devState.curInjWashAfterCnt),	WASH_INJECTOR))	{
							devState.actionSubmenu = AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME;
						}
//					}
					break;
				case AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME:	// 세척이	끝났으면 홈위치에서	대기
#if MODIFY_GO_HOME_AND_CHECK				
					if(goHomeCheckXYZ()) {
#else						
					if(goPosHome())	{
#endif
						devState.actionSubmenu = AS_RUN_SUB_SLEEP;
					}
					break;
				case AS_RUN_SUB_SLEEP:
						if(sleepModeAll()) {
							devState.actionSubmenu = AS_RUN_SUB_REPEAT_INJECTION;
//						}
//						devState.actionSubmenu = AS_RUN_SUB_REPEAT_INJECTION;
//						analysisTime = (uint32_t)(runSetup.analysisTime	*	60 * 10);

//						motorSleepCmd(MOTOR_VAL	,	HOME_DIR_VAL);
//						motorSleepCmd(MOTOR_INJ	,	HOME_DIR_INJ);
						
// home위치	체크 및	다음 인젝션	시간 동안	홈위치에서 대기
// 소스추가
dfp("WAIT	<<<<<<<<<<<<<>!!\n <<<analysisTimeCount= %d	 %d	isNextInjection =	%d >>>!!\n",devState.analysisTime100msCnt,devState.analysisTime100ms,devState.isNextInjection);
						}
//===================================================
					break;
				case AS_RUN_SUB_REPEAT_INJECTION:	// 같은	시료 반복분석	,	다음 분석이	종료되길 기다림
					if(devState.analysisTime100msCnt > devState.analysisTime100ms)	{//	바이알이 없을	경우는 이전	분석시간을 가지고	있으므로 여기는	참
						// 분석종료 
endAnalysisFunction();
// Autochro2에서 - runSetup.analysisTime 에 10초정도 여유를 주고 다음 

						if(devState.doNextSample == YES) {	// 다음 샘플로 넘어간다.
dfp("if(devState.doNextSample == YES) <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");						
							if(devState.vialExist	== NO) {	// 바이알이	없을 경우	(다음	샘플)--> mixing부터	해야 AS_RUN_SUB_MIX_PRGM --> mixing중에 발생하면 mix_ProgramNew()에서 해결함.
dfp("if(devState.vialExist == NO) <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");													
								devState.actionSubmenu = AS_RUN_SUB_GO_VIAL_AND_INJECTION;
devState.curSample.pos = devState.sample.pos;								
							}
							else {
								if(mixSet.useMixPrgm ==	YES) {
									devState.actionMenu = AS_ACTION_MIX;
									devState.actionSubmenu = AS_MIX_SUB_NONE;
									devState.mixTableNo = 0;
send_SelfMessage(SELF_START_LOADING	,SELF_START_LOADING_AUTO);									
LcasState.curInjCnt = 0;									
//send_SelfMessage(SELF_START_LOADING	,SELF_START_LOADING_AUTO);
//dfp("<<<analysisTime= %d %d	isNextInjection == YES %d >>>!!\n",devState.analysisTime100msCnt,devState.analysisTime100ms,devState.isNextInjection);
								}
								else {
									devState.actionSubmenu = AS_RUN_SUB_WAKE_UP;
send_SelfMessage(SELF_START_LOADING	,SELF_START_LOADING_AUTO);
LcasState.curInjCnt = 0;
								}
devState.curSample.pos = devState.sample.pos;
							}								
						}
						else {
							if(devState.isNextInjection	== YES)	{
dfp("if(devState.isNextInjection == YES) <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");								
								if(devState.vialExist	== NO) {	// 바이알이	없을 경우	(다음	샘플)--> mixing부터	해야 AS_RUN_SUB_MIX_PRGM
									if(mixSet.useMixPrgm ==	YES) {
										devState.actionMenu = AS_ACTION_MIX;
										devState.actionSubmenu = AS_MIX_SUB_NONE;

devState.mixTableNo = 0;
										
									}
									else {
										devState.actionSubmenu = AS_RUN_SUB_GO_VIAL_AND_INJECTION;
									}
/*
								devState.mixPrgmFinish = NO;
dfp("devState.vialExist	=	NO \n");
								devState.vialExist = YES;
*/								
								}
								else {
										devState.actionSubmenu = AS_RUN_SUB_WAKE_UP;
								}
send_SelfMessage(SELF_START_LOADING	,SELF_START_LOADING_AUTO);
dfp("if(devState.isNextInjection	== YES)<<<analysisTime= %d %d	isNextInjection == YES %d >>>!!\n",devState.analysisTime100msCnt,devState.analysisTime100ms,devState.isNextInjection);
							}
							else {
								devState.actionSubmenu = AS_RUN_SUB_END_SEQUENCE;

								devState.mixPrgmFinish = NO;
								devState.mixTableNo = 0;
								
								devState.seqAction = NO;
							}
						}
					}
					break;
				case AS_RUN_SUB_END_SEQUENCE:
					if(endSequence())	{
						devState.actionMenu	=	AS_ACTION_READY;
						devState.actionSubmenu = AS_READY_SUB_NONE;
						stateChange(STATE_READY);
						send_SelfMessage(SELF_END_SEQUENCE,0);
dfp("<<<SELF_END_SEQUENCE>>>	devState.vialExist = %d\n",devState.vialExist);		
					}
					break;
//				case AS_RUN_SUB_WASH_EXTERNAL:
//					break;
			}
}

int actionFault(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif
}

int actionStandby(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif
}

int actionDiagnostics(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif

//	pDiag_Fn[devState.diagMenu](0);

//devState.diagState = DIAG_POWER_CHECK;
		switch(devState.actionSubmenu) {
			case AS_DIAG_SUB_NONE:
				break;
				
		}
		

}

int actionDiagNone(int value)
{
}

// -25도에서 40도 설정 : 경과 시간 체크
// 25도 설정 
// 40도 설정
// Temp Off
// devState.diagState = DIAGNOSTICS_NONE
#define DIAG_HEATER_CHECK_TIME_READY		40
#define DIAG_HEATER_CHECK_TIME_LIMIT		240
#define DIAG_HEATER_CHECK_TEMP_START_25		25.0
#define DIAG_HEATER_CHECK_TEMP_TARGET_40	40.0
#define DIAG_HEATER_CHECK_TEMP_SET			(DIAG_HEATER_CHECK_TEMP_TARGET_40 + 10)			// 설정값은 50으로 
int actionDiagHeaterCheck(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif

//	pDiag_Fn[devState.diagMenu](0);

//devState.diagState = DIAG_POWER_CHECK;
		switch(devState.actionSubmenu) {
			case 0:
				break;
			case 1:
				runSetup.tempOnoff = ON;
/*
				pPID_Control_Fn = PID_Control;
				runSetup.temp = DIAG_HEATER_CHECK_TEMP_START_25;	
				devState.tempReadyUnder = runSetup.temp - TEMP_READY_RANGE;
				devState.tempReadyOver = runSetup.temp + TEMP_READY_RANGE;
*/				
				runSetup.temp = DIAG_HEATER_CHECK_TEMP_START_25;	
				tempSetup();				
				devState.setTempReset = TEMP_NEW_VALUE;

				devState.diagTimeCount = 0;
				
				devState.actionSubmenu = 2;
				break;
			case 2:
				if(LcasState.tempReady == YES) { // && devState.diagTimeCount >= DIAG_HEATER_CHECK_TIME_READY) {				
//					runSetup.tempOnoff = ON;
					
					runSetup.temp = DIAG_HEATER_CHECK_TEMP_SET;	
					tempSetup();				
					devState.tempReadyUnder = DIAG_HEATER_CHECK_TEMP_TARGET_40 - TEMP_READY_RANGE;
					devState.tempReadyOver = DIAG_HEATER_CHECK_TEMP_TARGET_40 + TEMP_READY_RANGE;
					devState.setTempReset = TEMP_NEW_VALUE;
					
					devState.diagTimeCount = 0;
					
					devState.actionSubmenu = 3;
				}
				break;
//			case 3:
//				if(LcasState.tempReady == YES) {
//					devState.actionSubmenu = 3;
//				}
//				break;
			case 3:
				if(LcasState.fTemp > DIAG_HEATER_CHECK_TEMP_TARGET_40 && devState.diagTimeCount <= DIAG_HEATER_CHECK_TIME_LIMIT) {						
/*
					runSetup.tempOnoff = OFF;
					pPID_Control_Fn = PID_Function_Blank;				
					
					runSetup.temp = DIAG_HEATER_CHECK_TEMP_START_25;	
					devState.tempReadyUnder = runSetup.temp - TEMP_READY_RANGE;
					devState.tempReadyOver = runSetup.temp + TEMP_READY_RANGE;
					devState.setTempReset = TEMP_NEW_VALUE;
*/					
					devState.actionSubmenu = 4;
					
					diagData.Current = DIAG_HEATER_CHECK;
					diagData.btHeaterTest = DIAG_RESULT_PASS;
				}
				else if(devState.diagTimeCount > DIAG_HEATER_CHECK_TIME_LIMIT) {		
/*					
					runSetup.tempOnoff = OFF;
					pPID_Control_Fn = PID_Function_Blank;				

					runSetup.temp = DIAG_HEATER_CHECK_TEMP_START_25;	
					devState.tempReadyUnder = runSetup.temp - TEMP_READY_RANGE;
					devState.tempReadyOver = runSetup.temp + TEMP_READY_RANGE;
					devState.setTempReset = TEMP_NEW_VALUE;
*/					
					devState.actionSubmenu = 4;

					diagData.Current = DIAG_HEATER_CHECK;
					diagData.btHeaterTest = DIAG_RESULT_FAIL;
				}
				break;
			case 4:
				SEND_LCAS_PACKET(PACKCODE_LCAS_DIAG_DATA, diagData, diagData);
				pAction_Fn[AS_ACTION_DIAGNOSTICS] = actionDiagNone;	

				runSetup.tempOnoff = OFF;
				tempSetup();
/*
				pPID_Control_Fn = PID_Function_Blank;				

				runSetup.temp = DIAG_HEATER_CHECK_TEMP_START_25;	
				devState.tempReadyUnder = runSetup.temp - TEMP_READY_RANGE;
				devState.tempReadyOver = runSetup.temp + TEMP_READY_RANGE;
				devState.setTempReset = TEMP_NEW_VALUE;
*/
				devState.actionSubmenu = 0;
				
				devState.diagTimeCount = 0;
				
				diagData.Current = DIAGNOSTICS_NONE;
//				diagData.btHeaterTest = DIAG_RESULT_NONE;				

				break;
				
		}
			
}

#define DIAG_COOLER_CHECK_TIME_READY		40
#define DIAG_COOLER_CHECK_TIME_LIMIT		2400
#define DIAG_COOLER_CHECK_TEMP_START_25		25.0
#define DIAG_COOLER_CHECK_TEMP_TARGET_04	4.0
#define DIAG_COOLER_CHECK_TEMP_SET			(DIAG_COOLER_CHECK_TEMP_TARGET_04 - 2.0)			// 설정값은 2도 아래 

int actionDiagCoolerCheck(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif

		switch(devState.actionSubmenu) {
			case 0:
				break;
			case 1:
				runSetup.tempOnoff = ON;
/*
				pPID_Control_Fn = PID_Control;
				
				runSetup.temp = DIAG_COOLER_CHECK_TEMP_START_25;	
				devState.tempReadyUnder = runSetup.temp - TEMP_READY_RANGE;
				devState.tempReadyOver = runSetup.temp + TEMP_READY_RANGE;
*/
				runSetup.temp = DIAG_COOLER_CHECK_TEMP_START_25;	
				tempSetup();
				devState.setTempReset = TEMP_NEW_VALUE;

				devState.diagTimeCount = 0;
				
				devState.actionSubmenu = 2;
				break;
			case 2:
				if(LcasState.tempReady == YES) { // && devState.diagTimeCount >= DIAG_COOLER_CHECK_TIME_READY) {				
//					runSetup.tempOnoff = ON;
					
					runSetup.temp = DIAG_COOLER_CHECK_TEMP_SET;	
					tempSetup();
					devState.tempReadyUnder = DIAG_COOLER_CHECK_TEMP_TARGET_04 - TEMP_READY_RANGE;
					devState.tempReadyOver = DIAG_COOLER_CHECK_TEMP_TARGET_04 + TEMP_READY_RANGE;
					devState.setTempReset = TEMP_NEW_VALUE;
					
					devState.diagTimeCount = 0;
					
					devState.actionSubmenu = 3;
				}
				break;
			case 3:
				if(LcasState.fTemp < DIAG_COOLER_CHECK_TEMP_TARGET_04 && devState.diagTimeCount <= DIAG_COOLER_CHECK_TIME_LIMIT) {
			
					devState.actionSubmenu = 4;
					
					diagData.Current = DIAG_COOLER_CHECK;
					diagData.btHeaterTest = DIAG_RESULT_PASS;
				}
				else if(devState.diagTimeCount > DIAG_COOLER_CHECK_TIME_LIMIT) {		
				
					devState.actionSubmenu = 4;

					diagData.Current = DIAG_COOLER_CHECK;
					diagData.btHeaterTest = DIAG_RESULT_FAIL;
				}
				break;
			case 4:
				SEND_LCAS_PACKET(PACKCODE_LCAS_DIAG_DATA, diagData, diagData);
				pAction_Fn[AS_ACTION_DIAGNOSTICS] = actionDiagNone;	

				runSetup.tempOnoff = OFF;
				tempSetup();
/*				
				pPID_Control_Fn = PID_Function_Blank;				

				runSetup.temp = DIAG_COOLER_CHECK_TEMP_START_25;	
				devState.tempReadyUnder = runSetup.temp - TEMP_READY_RANGE;
				devState.tempReadyOver = runSetup.temp + TEMP_READY_RANGE;
				devState.setTempReset = TEMP_NEW_VALUE;
*/
				devState.actionSubmenu = 0;
				
				devState.diagTimeCount = 0;
				
				diagData.Current = DIAGNOSTICS_NONE;

				break;
				
		}	
}

#define DIAG_TEMPSEN_CHECK_TIME		120
//#define DIAG_COOLER_CHECK_TIME_LIMIT		2400
#define DIAG_TEMPSEN_CHECK_TEMP_START_25		25.0
#define DIAG_TEMPSEN_CHECK_SET_TEMP_GAP			5.0
#define DIAG_TEMPSEN_CHECK_CHECK_TEMP_GAP		1.0
#define TEMP_HIGH			1
#define TEMP_LOW			0
//#define DIAG_COOLER_CHECK_TEMP_TARGET_04	4.0
//#define DIAG_COOLER_CHECK_TEMP_SET			(DIAG_COOLER_CHECK_TEMP_TARGET_04 - 2.0)			// 설정값은 2도 아래 

int actionDiagTempsenCheck(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif

static float fCurTemp = 0.0;
//static float fCheckGap = 0.0;
static unsigned char bCurTemp = TEMP_LOW;

		switch(devState.actionSubmenu) {
			case 0:
				break;
			case 1:
				runSetup.tempOnoff = ON;
//				pPID_Control_Fn = PID_Control;
				fCurTemp = LcasState.fTemp;
				if(fCurTemp > DIAG_TEMPSEN_CHECK_TEMP_START_25) {
					runSetup.temp = fCurTemp - DIAG_TEMPSEN_CHECK_SET_TEMP_GAP;
//					fTempGap = DIAG_TEMPSEN_CHECK_CHECK_TEMP_GAP;
					bCurTemp = TEMP_HIGH;
//					fCheckTemp = fCurTemp - DIAG_TEMPSEN_CHECK_CHECK_TEMP_GAP;
				}
				else {
					runSetup.temp = fCurTemp + DIAG_TEMPSEN_CHECK_SET_TEMP_GAP;
//					fTempGap = -DIAG_TEMPSEN_CHECK_CHECK_TEMP_GAP;					
					bCurTemp = TEMP_LOW;					
//					fCheckTemp = fCurTemp + DIAG_TEMPSEN_CHECK_CHECK_TEMP_GAP;
				}

				tempSetup();
				
				devState.setTempReset = TEMP_NEW_VALUE;
				devState.diagTimeCount = 0;
				devState.actionSubmenu = 2;
				break;
			case 2:
				if(devState.diagTimeCount >= DIAG_TEMPSEN_CHECK_TIME) {
					devState.diagTimeCount = 0;
					devState.actionSubmenu = 3;
				}
				break;
			case 3:
				if(bCurTemp == TEMP_LOW) {
					if(fCurTemp > (LcasState.fTemp + DIAG_TEMPSEN_CHECK_CHECK_TEMP_GAP)) {
						diagData.btHeaterTest = DIAG_RESULT_PASS;
					}
					else {
						diagData.btHeaterTest = DIAG_RESULT_FAIL;
					}
				}
				else {
					if(fCurTemp < (LcasState.fTemp - DIAG_TEMPSEN_CHECK_CHECK_TEMP_GAP)) {
						diagData.btHeaterTest = DIAG_RESULT_PASS;
					}
					else {
						diagData.btHeaterTest = DIAG_RESULT_FAIL;
					}
				}
				diagData.Current = DIAG_SENSOR_CHECK;				
				devState.actionSubmenu = 4;
				break;
			case 4:
				SEND_LCAS_PACKET(PACKCODE_LCAS_DIAG_DATA, diagData, diagData);
				pAction_Fn[AS_ACTION_DIAGNOSTICS] = actionDiagNone;	

				runSetup.tempOnoff = OFF;
				tempSetup();

				devState.actionSubmenu = 0;
				
				devState.diagTimeCount = 0;
				
				diagData.Current = DIAGNOSTICS_NONE;

				break;
				
		}		
}

int actionAdjust(int value)
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif
	
	switch(devState.actionSubmenu) {
		case AS_ADJUST_HOME_POS:
			if(initHomePos_All())	{
				devState.actionSubmenu = AS_ADJUST_MOTOR_INIT;
			}
			break;
		case AS_ADJUST_MOTOR_INIT:
			if(adjustMotorInit())	{
				devState.actionSubmenu = AS_ADJUST_MOTOR_SET;
				devSet.adjustAct = ADJUST_ACT_READY;
#if	EEPROM_ADJUST_DATA_SAVE_FIX
memcpy(&devSet.adjustTmp, &devSet.adjust, sizeof(devSet.adjust));
iprintf("memcpy(&devSet.adjustTmp=%d\n",devSet.adjustTmp.saveFlag[0]);
#endif
			}
			break;
		case AS_ADJUST_MOTOR_SET:
			break;
		case AS_ADJUST_SYRINGE_REPLACE:
			if(adjustSyringeExchange()) {
				devState.actionSubmenu = AS_ADJUST_MOTOR_SET;
				devSet.adjustAct = ADJUST_ACT_EXCHANGE;
			}
			break;
		case AS_ADJUST_NEEDLE_REPLACE:
			if(adjustNeedleExchange()) {
				devState.actionSubmenu = AS_ADJUST_MOTOR_SET;
				devSet.adjustAct = ADJUST_ACT_EXCHANGE;
			}
			break;
#if FPGA_VERSION_TRIM_ADJUST_SLEEP
		case AS_ADJUST_MODE_SLEEP:
			if(sleepModeAll()) {
				devState.actionMenu	=	AS_ACTION_READY;
				devState.actionSubmenu = AS_READY_SUB_NONE;
				devSet.adjustAct = ADJUST_ACT_NOT_READY;
				stateChange(STATE_READY);
			}
			break;
		case AS_ADJUST_MODE_END:
					if(initHomePos_All())	{
						devState.actionMenu	=	AS_ACTION_ADJUST;
						devState.actionSubmenu = AS_ADJUST_MODE_SLEEP;
						devSet.adjustAct = ADJUST_ACT_NOT_READY;
//						stateChange(STATE_READY);
					}
					break;
#else
		case AS_ADJUST_MODE_END:
			if(initHomePos_All())	{
				devState.actionMenu	=	AS_ACTION_READY;
				devState.actionSubmenu = AS_READY_SUB_NONE;

				devSet.adjustAct = ADJUST_ACT_NOT_READY;

				stateChange(STATE_READY);
			}
			break;
#endif
	}
}

int actionServiceWash(int value) 
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif

			switch(devState.actionSubmenu) {
				case AS_SERVICE_WASH_WAKEUP:
					if(wakeup_XYZ_checkHome()) {
						switch(service.subCommand1) {
							case WASH_WASTE_NEEDLE_INT:
								devState.actionSubmenu = AS_SERVICE_WASH_NEEDLE_INT;
								break;
//							case WASH_WASTE_NEEDLE_EXT:
//								devState.actionSubmenu = AS_SERVICE_WASH_NEEDLE_EXT;
//								break;
							case WASH_WASTE_NEEDLE_ALL:
								devState.actionSubmenu = AS_SERVICE_WASH_NEEDLE_ALL;
								break;
							case WASH_WASTE_INJECTOR:
								devState.actionSubmenu = AS_SERVICE_WASH_INJECTOR;
								break;
//							case WASH_WASTE_WASTE:
//								devState.actionSubmenu = AS_SERVICE_WASH_WASTE;
//								break;
						}
					}
					break;
				case AS_SERVICE_WASH_NEEDLE_INT:
					if(Washing(devState.serviceWashCnt,	&(devState.curSvcWashCnt),	WASH_NEEDLE_INT))	{
						devState.actionSubmenu = AS_SERVICE_WASH_END;
					}
					break;
/*					
				case AS_SERVICE_WASH_NEEDLE_EXT:
					if(Washing(devState.serviceWashCnt,	&(devState.curSvcWashCnt),	WASH_NEEDLE_INT))	{
						devState.actionSubmenu = AS_SERVICE_WASH_END;
					}
					break;
*/					
				case AS_SERVICE_WASH_NEEDLE_ALL:
					if(Washing(devState.serviceWashCnt,	&(devState.curSvcWashCnt),	WASH_NEEDLE_ALL))	{
						devState.actionSubmenu = AS_SERVICE_WASH_END;
					}
					break;
				case AS_SERVICE_WASH_INJECTOR:
					if(Washing(devState.serviceWashCnt,	&(devState.curSvcWashCnt),	WASH_INJECTOR))	{
						devState.actionSubmenu = AS_SERVICE_WASH_END;
					}
					break;
/*					
				case AS_SERVICE_WASH_WASTE:
					if(Washing(devState.serviceWashCnt,	&(devState.curSvcWashCnt),	WASH_NEEDLE_INT))	{
						devState.actionSubmenu = AS_SERVICE_WASH_END;
					}
					break;
*/					
				case AS_SERVICE_WASH_END:
					if(goPosHome())	{
						devState.actionSubmenu = AS_SERVICE_WASH_SLEEP;
					}
					break;
				case AS_SERVICE_WASH_SLEEP:					
					if(sleepModeAll()) {
						devState.actionMenu	=	AS_ACTION_READY;
						devState.actionSubmenu = AS_READY_SUB_NONE;
						stateChange(STATE_READY);
//					send_SelfMessage(SELF_END_SEQUENCE,0);
dfp("<<<AS_SERVICE_WASH_END>>>	\n");
					}
					break;
					
			}	
}

int actionStopFunction(int value) 
{
#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif
}

// =================================================================================
// Auto	Sampler	==>	State
// 모든동작은	HOME_POSITION을	기준으로 한다.
// 모든동작은	현재위치를 검사후	동작한다.	-	불필요하게 이동하는	것을 금지한다.
//
// =================================================================================
#if 0
int Action()
{

}

void timer_100ms()
{
	static uint32_t	cnt	=	0;

	devState.analysisTime100msCnt++;

	pTime_Event_Fn();

	pLed_Control_Fn();

	if(++cnt ==	10)	{
		cnt	=	0;
		pSend_State_Fn();
	}

	gbTempReadTick = YES;

	remoteControl();

}
#endif

void initSystemInform()
{
	unsigned char saveFlag;
	
	devState.sysinfoSave = NO;

//	strcpy(sysInform.cModel, "YL9152 LC AutoSampler");
//	strcpy(sysInform.cVersion, "1.0.0");
	strcpy(sysInform.cModel, SYSTEM_MODEL);
	strcpy(sysInform.cVersion, SYSTEM_VERSION);

	eepromRead_Data(SYSTEM_INFORM_DATA_SAVE, 1 /*sizeof(saveFlag)*/, &saveFlag);
	if(saveFlag == DATA_SAVE_FLAG) {
//		eepromRead_Data((SYSTEM_INFORM_DATA_ADDR), 37 /*sizeof(sysInform)*/, &(sysInform.cVersion/*cSerialNo*/));
// serialNo , InstallDate 만 읽는다.
		eepromRead_Data((SYSTEM_INFORM_DATA_ADDR), SYSTEM_INFORM_DATA_READSIZE, &(sysInform.cSerialNo));
dfp("sysInform.cInstallDate.year = %d - %d - %d---- \n\r",sysInform.cInstallDate.year,sysInform.cInstallDate.month,sysInform.cInstallDate.date);

strcpy(sysInform.cIPAddress, "10.10.10.52");
//strcpy(sysInform.cIPAddress, "10.10.10.50");
strcpy(sysInform.cPortNo, "4242");

	}
	else {
//		strcpy(sysInform.cSerialNo, "AS23451610311DE6");
		strcpy(sysInform.cSerialNo, "AS2017171112");
//		strcpy(sysInform.cInstallDate, "20170306");
		sysInform.cInstallDate.year = 17;
		sysInform.cInstallDate.month = 11;
		sysInform.cInstallDate.date = 03;
		
		strcpy(sysInform.cIPAddress, "10.10.10.52");
//		strcpy(sysInform.cIPAddress, "10.10.10.50");
		strcpy(sysInform.cPortNo, "4242");
dfp("NO DATA---- \n\r");
	}
}

void saveSystemInform(int who)
{
	unsigned char saveFlag = DATA_SAVE_FLAG;
	unsigned char data[21];

	data[0] = DATA_SAVE_FLAG;

	switch(who) {
		case 0:
// serialNo , InstallDate 만 저장
			memcpy(&data[1], &(sysInform.cSerialNo) , SYSTEM_INFORM_DATA_READSIZE);
			eepromWrite_Data((SYSTEM_INFORM_DATA_SAVE), SYSTEM_INFORM_DATA_READSIZE + 1, &(data[0]));
			break;
		case 1:
// serialNo , InstallDate 만 저장
			memcpy(&data[1], &(sysInform.cSerialNo) , SYSTEM_INFORM_DATA_READ_SERIAL);
			eepromWrite_Data((SYSTEM_INFORM_DATA_SAVE), SYSTEM_INFORM_DATA_READ_SERIAL + 1, &(data[0]));
 			break;
		case 2:
			memcpy(&data[1], &(sysInform.cSerialNo) , SYSTEM_INFORM_DATA_READ_SERIAL);
			eepromWrite_Data((SYSTEM_INFORM_DATA_SAVE), SYSTEM_INFORM_DATA_READ_SERIAL + 1, &(data[0]));
 			break;
	}
	
}

void initSubmenuValue()
{
//===============================================================
// subMenu
	subMenu_w	=	1;
	subMenu_wch	=	1;
	subMenu_es	=	1;
	subMenu_sma	=	1;
	subMenu_wxch	=	1;
	subMenu_ghx	=	1;
	subMenu_ghcx	=	1;
	subMenu_as	=	1;
	subMenu_ds	=	1;
	subMenu_fli	=	1;
	subMenu_pli	=	1;
	subMenu_mpi	=	1;
	subMenu_map	=	1;
	subMenu_mmp	=	1;
	subMenu_mwp	=	1;

	subMenu_gpvz	=	1;
	subMenu_gpez	=	1;
	subMenu_gpezc	=	1;
	actRetry_gpezc	=	1;

	subMenu_gpezci = 1;
	subMenu_gpv = 1;

	subMenu_ihpv	=	1;
	subMenu_ihpi	=	1;
	subMenu_ihps	=	1;
	subMenu_ihpz	=	1;
	subMenu_ihpx	=	1;
	subMenu_ihpy	=	1;
	subMenu_ihpa	=	1;

	subMenu_ami	=	1;
	subMenu_ane	=	1;
	subMenu_ase  = 1;

	subMenu_grpx = 1;
	subMenu_grpy = 1;
	subMenu_grpz = 1;

	subMenu_rcrx = 1;
	subMenu_rcry = 1;
	subMenu_rcrz = 1;
	
// mix
	subMenu_mp = 1;
//===============================================================
}

void initValue()
{
	
	devState.stateOld	=	0;//STATE_INITILIZE;
//	LcasState.state = STATE_INITILIZE
	stateChange(STATE_INITILIZE);

	LcasState.tempReady = NO;
	LcasState.errorCode = SELF_NO_ERROR;

	devState.actionMenu	=	AS_ACTION_INITILIZE;
//	devState.actionSubmenu = AS_INIT_SUB_FIND_HOME;
//	devState.actionSubmenu = AS_INIT_SUB_FIND_HOME_VAL_INJ_Z;
	devState.actionSubmenu = AS_INIT_SUB_NONE;
	initSubmenuValue();

	devState.bSleepMode = NO;
	devState.bCheckHome = NO;

	devState.curSvcWashCnt	=	0;

	devState.curNdlWashBeforeCnt = 0;			// 몇번	샐행했는가?
	devState.curInjWashBeforeCnt = 0;			// 몇번	샐행했는가?
	devState.curNdlWashAfterCnt	=	0;			// 몇번	샐행했는가?
	devState.curInjWashAfterCnt	=	0;			// 몇번	샐행했는가?

	devState.mixPrgmFinish = NO;
//		static int tableNo = 0;
	devState.mixTableNo = 0;

	devState.seqAction = NO;	// 시퀀스	없음.

	devState.doNextSample	=	YES;
	devState.isNextInjection = NO;

	devState.vialExist = YES;
	devState.missVialResponse = MISSING_VIAL_NO_RESPONSE;
//	devState.missVialSkip = NO;

//	devSet.adjustCurMotor	=	0;	// adjust실행중이	아닐경우
	devSet.adjustCurMotor	=	0xff;	// adjust실행중이	아닐경우
	devSet.adjustOldMotor	=	0xff;
	devSet.adjustAct = ADJUST_ACT_NOT_READY;

	devState.command = COMMAND_NONE;
	devState.initCommand = NO;

	devState.setTempReset	=	TEMP_NEW_VALUE;

	devState.curAuxEventNo = 0;
	devState.curAuxState = AUX_CONTACT_OPEN;	// AUX_OFF;
	devState.curAuxOutIng	=	NO;

	devState.auxOutSigTime = 0;	// 시그널	출격시간
	devState.auxOutSigTimeCnt	=	0; //	시그널 출격시간	체크

	devState.analysisTime100msCnt	=	0;

	devState.usedTimeSave = NO;

//	devState.vialPlateCheck = NO;
	devState.adReadValid	=	NO;
	devState.powerCheck = NO;
	devState.powerState = NO;

	devState.powerOffLed = OFF;

	devState.save_EEPROM_sysConfig = NO;


}

void applySystemConfig()
{
	switch(sysConfig.syringeVol) {
		case SYRINGE_VOL_250UL:
			xyzCoord.step_1ul_SYR = SYR_PULSE_PER_VOL_250_FLOAT;
			devSet.injectionVolumeMax = 50;
			// washVolumeMax = ??
			devSet.washVolumeMax = 250;
//			devSet.syrExchangeHeight = 175; // devSet.washVolumeMax * 0.7			
			devSet.syrExchangeHeight = 125; // devSet.washVolumeMax * 0.5						
			if(runSetup.washVolume > devSet.washVolumeMax) runSetup.washVolume = devSet.washVolumeMax;
			break;
		case SYRINGE_VOL_500UL:
			xyzCoord.step_1ul_SYR = SYR_PULSE_PER_VOL_500_FLOAT;
			devSet.injectionVolumeMax = 100;
			devSet.washVolumeMax = 500;			
//			devSet.syrExchangeHeight = 350; // devSet.washVolumeMax * 0.7			
			devSet.syrExchangeHeight = 250; // devSet.washVolumeMax * 0.5						
			if(runSetup.washVolume > devSet.washVolumeMax) runSetup.washVolume = devSet.washVolumeMax;			
			break;
		case SYRINGE_VOL_1000UL:
			xyzCoord.step_1ul_SYR = SYR_PULSE_PER_VOL_1000_FLOAT;
			devSet.injectionVolumeMax = 200;
			devSet.washVolumeMax = 1000;
//			devSet.syrExchangeHeight = 700; // devSet.washVolumeMax * 0.7						
			devSet.syrExchangeHeight = 500; // devSet.washVolumeMax * 0.5									
			if(runSetup.washVolume > devSet.washVolumeMax) runSetup.washVolume = devSet.washVolumeMax;			
			break;
		case SYRINGE_VOL_2500UL:
			xyzCoord.step_1ul_SYR = SYR_PULSE_PER_VOL_2500_FLOAT;
			devSet.injectionVolumeMax = 500;
			devSet.washVolumeMax = 2500;	
//			devSet.syrExchangeHeight = 1750; // devSet.washVolumeMax * 0.7						
			devSet.syrExchangeHeight = 1250; // devSet.washVolumeMax * 0.5									
			if(runSetup.washVolume > devSet.washVolumeMax) runSetup.washVolume = devSet.washVolumeMax;			
			break;
		default:
			xyzCoord.step_1ul_SYR = SYR_PULSE_PER_VOL_500_FLOAT;
			devSet.injectionVolumeMax = 100;			
			devSet.washVolumeMax = 500;
//			devSet.syrExchangeHeight = 350; // devSet.washVolumeMax * 0.7						
			devSet.syrExchangeHeight = 250; // devSet.washVolumeMax * 0.5									
			if(runSetup.washVolume > devSet.washVolumeMax) runSetup.washVolume = devSet.washVolumeMax;			
			break;
	}

//	devSet.syrExchangeHeight = devSet.injectionVolumeMax; // 시린지 교체시 높이
dprintf("====================	sysConfig.trayLeft=%d xyzCoord.trayRight =%d\n", sysConfig.trayLeft, sysConfig.trayRight );
	switch(sysConfig.trayLeft) {	// TRAY_FRONT
		case TRAY_VIALS_40:
			devSet.trayLeft_rowNo	=	TRAY40_COUNT_X;
			devSet.trayLeft_colNo	=	TRAY40_COUNT_Y;
			break;
		case TRAY_VIALS_60:
			devSet.trayLeft_rowNo	=	TRAY60_COUNT_X;
			devSet.trayLeft_colNo	=	TRAY60_COUNT_Y;
			break;
		case WELLPALTE_96:
			devSet.trayLeft_rowNo	=	WELLPALTE_96_COUNT_X;	//TRAY96_COUNT_X;
			devSet.trayLeft_colNo	=	WELLPALTE_96_COUNT_Y;	//TRAY96_COUNT_Y;
			break;
		default:
			break;
	}

	switch(sysConfig.trayRight)	{	// TRAY_RIGHT
		case TRAY_VIALS_40:
			devSet.trayRight_rowNo = TRAY40_COUNT_X;
			devSet.trayRight_colNo = TRAY40_COUNT_Y;
			break;
		case TRAY_VIALS_60:
			devSet.trayRight_rowNo = TRAY60_COUNT_X;
			devSet.trayRight_colNo = TRAY60_COUNT_Y;
			break;
		case WELLPALTE_96:
			devSet.trayRight_rowNo = WELLPALTE_96_COUNT_X;	//TRAY96_COUNT_X;
			devSet.trayRight_colNo = WELLPALTE_96_COUNT_Y;	//TRAY96_COUNT_Y;
			break;
		default:
			break;
	}

	switch(sysConfig.injectMarkerTime) {
		case INJECTOR_MARKER_TIME_NONE:
			remoteStartOutTime = 0;
			break;
		case INJECTOR_MARKER_TIME_100ms:
			remoteStartOutTime = 1;
			break;
		case INJECTOR_MARKER_TIME_200ms:
			remoteStartOutTime = 2;
			break;
		case INJECTOR_MARKER_TIME_500ms:
			remoteStartOutTime = 5;
			break;
		case INJECTOR_MARKER_TIME_1sec:
			remoteStartOutTime = 10;
			break;
		case INJECTOR_MARKER_TIME_2sec:
			remoteStartOutTime = 20;
			break;
	}

	switch(sysConfig.startInSigTime) {
		case START_IN_SIGNAL_TIME_disable:
			remoteStartInTime	=	0;
			break;
		case START_IN_SIGNAL_TIME_100ms:
			remoteStartInTime	=	1;
			break;
		case START_IN_SIGNAL_TIME_200ms:
			remoteStartInTime	=	2;
			break;
		case START_IN_SIGNAL_TIME_500ms:
			remoteStartInTime	=	5;
			break;
		case START_IN_SIGNAL_TIME_1sec:
			remoteStartInTime	=	10;
			break;
		case START_IN_SIGNAL_TIME_2sec:
			remoteStartInTime	=	20;
			break;
	}

}

// 저장되어	있는 값을	읽거나 초기화
void initSystemConfig()
{
	unsigned char readSysCfgBuf[50];		// sizeof(sysConfig) + 1 ==> 45  ==> 대략 50정도설정
	unsigned char size;
	unsigned char dataError = YES;
	SYSTEM_CONFIG_t tmp;

	size = sizeof(sysConfig);

	eepromRead_Data(SYSTEM_CONFIG_DATA_SAVE,(size + 1),&readSysCfgBuf);

	if(readSysCfgBuf[0]	== DATA_SAVE_FLAG) {
//		memcpy(&sysConfig, &readSysCfgBuf[1], size);		
		memcpy(&tmp, &readSysCfgBuf[1], size);

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
			) 
		{	// 초기화 해야 한다.
			dataError = YES;
		}
		else {
			memcpy(&sysConfig, &readSysCfgBuf[1], size);		
			dataError = NO;
		}
dfp("sysConfig.trayLeft= [%d] sysConfig.trayRight= [%d]\n",sysConfig.trayLeft, sysConfig.trayRight);
//dfp("initSystemConfig sysConfig.sampleLoopVol= %d\n",sysConfig.sampleLoopVol);
	}

	if(dataError == YES) {	// 초기화

		sysConfig.trayCooling	=	NO;
		sysConfig.sampleLoopVol	=	100;
		// 0 : 250uL	1: 500uL(default)	 2:1000uL	3: 2500uL	in Prep	Mode
		sysConfig.syringeVol = SYRINGE_VOL_500UL;		// 60mm	=	500ul	,	1u = 0.12mm;
		sysConfig.needleTubeVol	=	NEEDLE_TUBING_VOL_DEFAULT;

//		sysConfig.trayLeft = TRAY_VIALS_40;
//	devSet.left_z_bottom = COORD_Z_VIAL_40_BOTTOM + (devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] * (ADJUST_STEP_UNIT_Z / MICRO_STEP_Z) * MM_PER_STEP_Z);
		sysConfig.trayLeft	=	TRAY_VIALS_60;
	devSet.left_z_bottom = COORD_Z_VIAL_60_BOTTOM + (devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] * (ADJUST_STEP_UNIT_Z / MICRO_STEP_Z) * MM_PER_STEP_Z);					

		sysConfig.trayRight	=	TRAY_VIALS_60;
	devSet.right_z_bottom = COORD_Z_VIAL_60_BOTTOM + (devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] * (ADJUST_STEP_UNIT_Z / MICRO_STEP_Z) * MM_PER_STEP_Z);					

		sysConfig.processType	=	PROCESS_TYPE_ROW;
//		sysConfig.usePrepMode	=	NO;

    sysConfig.syringeUpSpdWash = SPEED_LEVEL_10;

    sysConfig.syringeDnSpdWash = SPEED_LEVEL_3;
//		sysConfig.syringeDnSpdWash = SPEED_LEVEL_6;

		sysConfig.injectMarkerTime = INJECTOR_MARKER_TIME_default;
		sysConfig.startInSigTime = START_IN_SIGNAL_TIME_default;

		sysConfig.useMicroPump = NO;
		
dfp("initSystemConfig ======== not Save Data --- Init=================== %d\n");
	}

// Magnetic Sensor Input  - Tray Detection
#if	MECH_VER == 6	// 이전 버전은 기능없음.
	Core_gpioInput = (*((uint32_t	volatile *)(COREGPIO_APBBIF_0_IN_REG)));

	int magnetic;
	magnetic =  (Core_gpioInput >> 16) & 0x07;
	switch(magnetic) {
		case TRAY_DETECT_WELLPALTE_96:	sysConfig.trayLeft = WELLPALTE_96;	break;
		case TRAY_DETECT_VIALS_40:			sysConfig.trayLeft = TRAY_VIALS_40;	break;
		case TRAY_DETECT_VIALS_60: 			sysConfig.trayLeft = TRAY_VIALS_60;	break;
		case TRAY_DETECT_NONE:					sysConfig.trayLeft = TRAY_NONE;			break;
		default:												sysConfig.trayLeft = TRAY_NONE;			break;
	}

//	sysConfig.trayLeft = TRAY_VIALS_40;
//	devSet.left_z_bottom = COORD_Z_VIAL_40_BOTTOM + (devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] * (ADJUST_STEP_UNIT_Z / MICRO_STEP_Z) * MM_PER_STEP_Z);
//		sysConfig.trayRight	=	TRAY_VIALS_60;

	magnetic =	(Core_gpioInput >> 19) & 0x07;
	switch(magnetic) {
		case TRAY_DETECT_WELLPALTE_96:	sysConfig.trayRight = WELLPALTE_96;	break;
		case TRAY_DETECT_VIALS_40:			sysConfig.trayRight = TRAY_VIALS_40;	break;
		case TRAY_DETECT_VIALS_60: 			sysConfig.trayRight = TRAY_VIALS_60;	break;
		case TRAY_DETECT_NONE:					sysConfig.trayRight = TRAY_NONE;			break;
		default:												sysConfig.trayRight = TRAY_NONE;			break;
	}

dfp("Core_gpioInput Magnetyic = 0x%08x L=%d R=%d\n\r",Core_gpioInput,sysConfig.trayLeft,sysConfig.trayRight);
#elif	MECH_VER == 7
	
		int magneticL,magneticR ;
		
	#if NEW_BOARD_ENCODER_2020
		#if VIAL_TRAY_CONFIG_FIX	
			magneticL = (devSet.msen_switch >> 3) & 0x07;
			magneticR = devSet.msen_switch & 0x07;			
		#else
			magneticL = devSet.msen_switch & 0x07;	
			magneticR = (devSet.msen_switch >> 3) & 0x07;
		#endif
	#else		
		Core_gpioInput = (*((uint32_t	volatile *)(COREGPIO_APBBIF_0_IN_REG)));	
		#if VIAL_TRAY_CONFIG_FIX			
			magneticL = (Core_gpioInput >> 19) & 0x07;		
			magneticR = (Core_gpioInput >> 16) & 0x07;			
		#else
			magneticL = (Core_gpioInput >> 16) & 0x07;	
			magneticR = (Core_gpioInput >> 19) & 0x07;		
		#endif
	#endif		

		switch(magneticL) {
			case TRAY_DETECT_WELLPALTE_96:	sysConfig.trayLeft = WELLPALTE_96;	break;
			case TRAY_DETECT_VIALS_40:			sysConfig.trayLeft = TRAY_VIALS_40;	break;
			case TRAY_DETECT_VIALS_60: 			sysConfig.trayLeft = TRAY_VIALS_60;	break;
			case TRAY_DETECT_NONE:					sysConfig.trayLeft = TRAY_NONE;			break;
			default:												sysConfig.trayLeft = TRAY_NONE;			break;
		}

		switch(magneticR) {
			case TRAY_DETECT_WELLPALTE_96:	sysConfig.trayRight = WELLPALTE_96;	break;
			case TRAY_DETECT_VIALS_40:			sysConfig.trayRight = TRAY_VIALS_40;	break;
			case TRAY_DETECT_VIALS_60: 			sysConfig.trayRight = TRAY_VIALS_60;	break;
			case TRAY_DETECT_NONE:					sysConfig.trayRight = TRAY_NONE;			break;
			default:												sysConfig.trayRight = TRAY_NONE;			break;
		}

#endif

	applySystemConfig();
//	initCoordinate();  

}

void initRunSetup()
{
		runSetup.tempOnoff = OFF;
		runSetup.temp	=	TEMP_DEFAULT; // 20
		
		devState.oldTempOnoff = OFF;	// 		
		devState.oldTemp = (runSetup.temp + 1);
		devState.tempReadyUnder = runSetup.temp - TEMP_READY_RANGE;
		devState.tempReadyOver = runSetup.temp + TEMP_READY_RANGE;
		
runSetup.injectionStart	=	INJ_START_ANYTIME;

		runSetup.injectionMode = INJ_MODE_FULL_LOOP;
		runSetup.analysisTime	=	10.0;
//runSetup.flushVolume = NEEDLE_TUBING_VOL_DEFAULT;
runSetup.flushEnable = NO;
runSetup.flushVolume = FLUSH_VOLUME_DEFAULT;
		runSetup.injectionVolume = 10;
		runSetup.washBetween = WASH_BETWEEN_INJECTIONS;

//		runSetup.washTimes = WASH_TIMES_DEFAULT;
		switch(sysConfig.syringeVol) {
			case SYRINGE_VOL_250UL:
				runSetup.washVolume	=	250;		
				break;
			case SYRINGE_VOL_500UL:
				runSetup.washVolume	=	500;
				break;
			case SYRINGE_VOL_1000UL:
				runSetup.washVolume	=	500;
				break;
			case SYRINGE_VOL_2500UL:
				runSetup.washVolume	=	500;
				break;
			default:
				runSetup.washVolume	=	500;
				break;
		}

dp("syringeVol=%d washVolume = %d washVolumeMax = %d\n",sysConfig.syringeVol,runSetup.washVolume,devSet.washVolumeMax);

		runSetup.washNeedleBeforeInj = WASH_NEEDLE_BEFORE_INJ_default;
		runSetup.washInjportBeforeInj	=	WASH_INJPORT_BEFORE_INJ_default;
		runSetup.washNeedleAfterInj	=	WASH_NEEDLE_AFTER_INJ_default;
		runSetup.washInjportAfterInj = WASH_INJPORT_AFTER_INJ_default;

//	runSetup.syringeSpeed	=	6;
		runSetup.syringeUpSpdInj	=	SPEED_LEVEL_6;		//SYRINGE_SPEED_6;
		runSetup.syringeDnSpdInj	=	SPEED_LEVEL_6;	//SYRINGE_SPEED_6;
		runSetup.needleHeight	=	2;	// 2~6mm

//		runSetup.waitTime	=	0.5;
		runSetup.waitTime_AfterAspirate = 0.5;
		runSetup.waitTime_AfterDispense = 0.5;

//		runSetup.useMicroPump = YES;
		runSetup.skipMissingSample = YES;		// NO	<--	Error
		runSetup.airSegment	=	NO;
runSetup.airSegmentVol = AIR_SEGMENT_DEFAULT;//5uL
//		runSetup.headSpacePressure = YES;

		runSetup.microPumpTime = MICRO_PUMP_TIME; // sec
//		devSet.microPumpTimeCount = runSetup.microPumpTime * 10000;
		devSet.microPumpTimeCount = runSetup.microPumpTime * MICRO_PUMP_TIME_CONVERSION;
		
}

void initSequence()
{
	sequence.firstSamplePos.uPos.tray	=	TRAY_LEFT;
	sequence.firstSamplePos.uPos.etc = 0;
	sequence.firstSamplePos.uPos.x = 0;
	sequence.firstSamplePos.uPos.y = 0;

	sequence.lastSamplePos.uPos.tray = TRAY_LEFT;
	sequence.lastSamplePos.uPos.etc	=	0;
	sequence.lastSamplePos.uPos.x	=	0;
	sequence.lastSamplePos.uPos.y	=	0;

	sequence.injectionTimes	=	1;
//	sequence.waitTime	=	0;

// 분석해야할	바이알 위치	-	initSequence()
	devState.sample.pos	=	sequence.firstSamplePos.pos;
// 분석중인 샘플
	devState.curSample.pos = sequence.firstSamplePos.pos;
	devState.injectVial.pos = sequence.firstSamplePos.pos;
	devState.lastSample.pos	=	sequence.lastSamplePos.pos;
	
	devState.injectionTimes	=	sequence.injectionTimes;
//	devState.waitTime	=	sequence.waitTime;

}

void initAuxEvntFn()
{

}

void initAuxEvntValue()
{
	timeEvent.useAuxTimeEvent = OFF;
	timeEvent.initAux = OFF;
	timeEvent.endTime = 0.0;
	timeEvent.auxOutSigTime = AUX_OUT_SIGNAL_TIME_disable;

	for(int i = 0 ; i < AUX_EVENT_COUNT ; i++) {
		timeEvent.auxEvent[i].auxOnoff = OFF;
		timeEvent.auxEvent[i].fTime = 0.0;
	}





//=============================================================================
//=============================================================================



}

void initMixTable()
{
	for(int	i=0	;	i<MIX_TABLE_COUNT	;	i++) {
		mixSet.mixTable[i].action	=	MIX_ACTION_NONE;
		mixSet.mixTable[i].addFrom = MIX_FROM_SAMPLE;
		mixSet.mixTable[i].addTo = MIX_TO_DESTINATION;
		mixSet.mixTable[i].addVol	=	5;
		mixSet.mixTable[i].mixVial = MIX_TO_DESTINATION;
		mixSet.mixTable[i].mixTimes	=	0;
		mixSet.mixTable[i].mixVol	=	0;
		mixSet.mixTable[i].waitTime	=	0;
	}

/*
	mixSet.mixTable[0].action	=	MIX_ACTION_ADD;
	mixSet.mixTable[0].addFrom = MIX_FROM_SAMPLE;
	mixSet.mixTable[0].addTo = MIX_TO_DESTINATION;
	mixSet.mixTable[0].addVol	=	50;
	mixSet.mixTable[0].mixVial = MIX_TO_DESTINATION;			// ---
	mixSet.mixTable[0].mixTimes	=	0;	// ---
	mixSet.mixTable[0].mixVol	=	0;		// ---
	mixSet.mixTable[0].waitTime	=	0;	// ---

	mixSet.mixTable[1].action	=	MIX_ACTION_MIX;
	mixSet.mixTable[1].addFrom = MIX_FROM_SAMPLE;	// ---
	mixSet.mixTable[1].addTo = MIX_TO_DESTINATION;// ---
	mixSet.mixTable[1].addVol	=	25;	// ---
	mixSet.mixTable[1].mixVial = MIX_TO_DESTINATION;
	mixSet.mixTable[1].mixTimes	=	2;
	mixSet.mixTable[1].mixVol	=	30;
	mixSet.mixTable[1].waitTime	=	0;	// ---

	mixSet.mixTable[2].action	=	MIX_ACTION_WAIT;	// ---
	mixSet.mixTable[2].addFrom = MIX_FROM_SAMPLE;	// ---
	mixSet.mixTable[2].addTo = MIX_TO_DESTINATION;	// ---
	mixSet.mixTable[2].addVol	=	20;		// ---
	mixSet.mixTable[2].mixVial = MIX_TO_DESTINATION;	// ---
	mixSet.mixTable[2].mixTimes	=	1;	// ---
	mixSet.mixTable[2].mixVol	=	3;		// ---
	mixSet.mixTable[2].waitTime	=	5;
*/
}

void initMixSetup()
{
	mixSet.useMixPrgm	=	NO;//YES;

	mixSet.relativeDestination = NO;
/*	
	mixSet.firstDestPos.uPos.x = 4;
	mixSet.firstDestPos.uPos.y = 9;
	mixSet.firstDestPos.uPos.etc = 0;
	mixSet.firstDestPos.uPos.tray	=	TRAY_RIGHT;
*/
	mixSet.relativeReagentA	=	NO;
/*	mixSet.reagentPosA.uPos.x	=	3;
	mixSet.reagentPosA.uPos.y	=	7;
	mixSet.reagentPosA.uPos.etc	=	0;
	mixSet.reagentPosA.uPos.tray = TRAY_RIGHT;
*/
	mixSet.relativeReagentB	=	NO;
/*	mixSet.reagentPosB.uPos.x	=	2;
	mixSet.reagentPosB.uPos.y	=	4;
	mixSet.reagentPosB.uPos.etc	=	0;
	mixSet.reagentPosB.uPos.tray = TRAY_RIGHT;
*/
	mixSet.relativeReagentC	=	NO;
/*	mixSet.reagentPosC.uPos.x	=	1;
	mixSet.reagentPosC.uPos.y	=	4;
	mixSet.reagentPosC.uPos.etc	=	0;
	mixSet.reagentPosC.uPos.tray = TRAY_RIGHT;
*/
	mixSet.relativeReagentD	=	NO;
/*	mixSet.reagentPosD.uPos.x	=	3;
	mixSet.reagentPosD.uPos.y	=	3;
	mixSet.reagentPosD.uPos.etc	=	0;
	mixSet.reagentPosD.uPos.tray = TRAY_RIGHT;
*/
//MIX_TABLE_COUNT
//	mixSet.tableCount	=	3;
}

void initDiagData()
{
	diagData.N5Dcheck = DIAG_RESULT_NONE;			// Analog -5V
	diagData.V5Dcheck = DIAG_RESULT_NONE;			// Analog +5V
	diagData.V1_5Dcheck = DIAG_RESULT_NONE;		// FPGA Core +1.5V
	diagData.V12Dcheck = DIAG_RESULT_NONE;		// Heater Power +12V
	
	diagData.V2_5Dcheck = DIAG_RESULT_NONE;		// ADC Reference Voltage +2.5V
	diagData.V3_3Dcheck = DIAG_RESULT_NONE;		// CPU Power +3.3V

	diagData.N5Dvalue = -5.0;
	diagData.V5Dvalue = 5.0;
	diagData.V1_5Dvalue = 1.5;
	diagData.V12Dvalue = 12;
	diagData.V2_5Dvalue = 2.5;
	diagData.V3_3Dvalue = 3.3;
	
	diagData.Tempcheck = DIAG_RESULT_NONE;		// 온도	
	diagData.btHeaterTest = DIAG_RESULT_NONE;		// 히터(펠티에) 검사, 30도에서 40도까지 상승시간 검사
	diagData.btCoolerTest = DIAG_RESULT_NONE;		// 쿨러(페티에) 검사, 40도에서 20도까지 하강시간 검사
	
	diagData.btLeakTest1 = DIAG_RESULT_NONE;		// leak check
	diagData.btLeakTest2 = DIAG_RESULT_NONE;		// leak check

	diagData.btTempSensor = DIAG_RESULT_NONE;		// 온도 센서 - 전압(저항)값 검사 및 저항 변화율 검사,
									// 일정시간동안 히팅 후에 저항값 변화 검사

	devState.diagState = DIAGNOSTICS_NONE;								
	devState.diagMenu = 0;//DIAGNOSTICS_NONE;
	devState.diagTimeCount = 0;
	pAction_Fn[AS_ACTION_DIAGNOSTICS] = actionDiagnostics;

}

void initSpecial(unsigned char load)
{
	unsigned char readBuf[10];	
	unsigned char size;

	if(load == DATA_LOAD) {
		size = sizeof(special);

		eepromRead_Data(SPECIAL_DATA_SAVE,(size + 1),&readBuf);

		if(readBuf[0]	== DATA_SAVE_FLAG) {
			memcpy(&special, &readBuf[1], size);
			if(special.Buzzer_onoff > ON) special.Buzzer_onoff = OFF;
			if(special.Door_check > ON) special.Door_check = NO;
			if(special.Leak_check > ON) special.Leak_check = NO;
			if(special.LED_onoff > ON) special.LED_onoff = NO;		
		}	
		else {
			special.LED_onoff = ON;
			special.Buzzer_onoff = OFF;
			special.Door_check = NO;
			special.Leak_check = NO;

			special.Reserved[0] = NO;
			special.Reserved[1] = NO;
			special.Reserved[2] = NO;
			special.Reserved[3] = NO;	
		}
	}

	if(special.LED_onoff == ON) {	EXT_LED_CON_ON;	}
	else {												EXT_LED_CON_OFF;}

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

//	BUZZER_CON_OFF;

}

int usedTimeCheck(unsigned char act)
{
	unsigned char saveFlag;
	unsigned char data[50];
	int ret = 0;
	
	switch(act) {
		case DATA_LOAD:
			eepromRead_Data(USED_TIME_DATA_SAVE, 1 , &saveFlag);
			if(saveFlag == DATA_SAVE_FLAG) {
//				eepromRead_Data((USED_TIME_DATA_ADDR), sizeof(usedTime), &(usedTime));
				eepromRead_Data((USED_TIME_DATA_ADDR), sizeof(usedTime), (uint8_t *)( &(usedTime)));
				ret = OK;
dfp("\n");				
			}
			else {
				usedTime.time = 0;	// 기기 사용시간 (60min)
				usedTime.runTime = 0;	// 기기 사용시간 - run동작 (10min)
				usedTime.heatTime = 0;	// Tray Cooling & heating 시간 (10min)

				// 주입 및 Washing이 끝나면 	
//				usedTime.needle = 0;	// 이동거리 cm
				usedTime.x = 0;	// 이동거리 cm	--> meter로 변경
				usedTime.y = 0;	// 이동거리 cm	
				usedTime.z = 0;	// 이동거리 cm		
					
				usedTime.syringe = 0;	// 이동거리 cm
				usedTime.valve = 0;		// 이동 포트 수 
				usedTime.injector = 0;	// 인젝터 동작 수
				usedTime.injectionCnt = 0;	// 인젝션횟수 -				
				ret = ERROR;
			}
			break;
		case DATA_SAVE:
			if(LcasState.state == STATE_READY) {
				data[0] = DATA_SAVE_FLAG;
				memcpy(&data[1], &(usedTime) , sizeof(usedTime));
				eepromWrite_Data((USED_TIME_DATA_SAVE), sizeof(usedTime) + 1, &(data[0]));
 				devState.usedTimeSave	= YES;
 				ret = YES;
			}			
			else {
				devState.usedTimeSave	= NO;
				ret = NO;
			}
			break;
		case DATA_MODIFY:
			data[0] = DATA_SAVE_FLAG;
			memcpy(&data[1], &(usedTime) , sizeof(usedTime));
			eepromWrite_Data((USED_TIME_DATA_SAVE), sizeof(usedTime) + 1, &(data[0]));

			ret = OK;
 			break;
	}
	
//	return 0;
	return ret;
}


int usedTimeSaveFunc()
{
	static uint32_t usedTimeCnt = 0;	// 사용시간 1초
	static uint32_t usedRunTimeCnt = 0;	// 사용시간
	static uint32_t usedTimeCnt_Min = 0;	// 사용시간

	if(LcasState.state == STATE_RUN/*STATE_READY)*/ || LcasState.state == STATE_ANALYSIS) {
			if(++usedRunTimeCnt == 60) {	// 60초	, 1분
 			usedRunTimeCnt = 0;
			usedTime.runTime++;
			}
	}

	// 사용시간 저장
	if(++usedTimeCnt == 60) {	// 60초	, 1분
		usedTimeCnt = 0;
		usedTime.time++;
		if(++usedTimeCnt_Min == 60) {	// 1hour
			usedTimeCnt_Min = 0;
		}
		else if(usedTimeCnt_Min == 10) {	// 10분 단위 - 저장 (10,70,130,190분 ... )
			usedTimeCheck(DATA_SAVE);			// 사용시간 저장 - ready 상태 일때만 저장
		}
		else {
			if(devState.usedTimeSave == NO) {	// 저장안되었다면 저장
				usedTimeCheck(DATA_SAVE);
			}
		}
	}

}


int encoderCountCheck(int encoderCount, int readCount) 
{
	if(encoderCount > readCount) {
		return (encoderCount - readCount);
	}
	else 
		return (readCount - encoderCount);
}

void remoteControl(uint32_t signalIn)
{
//	uint32_t signalIn;		// // ready & Power Btn check
// R6_SIGNAL_IN  ---	[bit2: Power button] / [bit1: remote start] / [bit0:remote ready]

	// remote start out
	if(remoteStartOutSig ==	YES) {	// 시그널	출력해야 하다. - injection 후  on 되었으니 off
		if(++remoteStartOutCnt > remoteStartOutTime) {
			remoteStartOutCnt	=	0;
			remoteStartOutSig= NO;
			REMOTE_START_OUT_CONTACT_OPEN;
		}
	}

// remote	및 기타	입력
	if(gbFabricRemoteInt ==	YES) {	// 리모드단자 입력이 들어옴.
		// remote	start	in

		if(signalIn & 0x02 == 1) {	// start in
			if(++remoteStartInCnt	>	remoteStartInTime) {
				remoteStartInCnt = 0;
				gbFabricRemoteInt= NO;
				if(LcasState.state == STATE_READY) {
								startLoadingCommand();
				send_SelfMessage(SELF_START_LOADING	,SELF_START_LOADING_EXTIN);
				dp("startLoadingCommand\n\r");
				}
				else {
				// 무시
				//				send_SelfMessage(SELF_COMMAND_REJECT,devState.command);	// 실행할 수 없음.
				}
			}
		}
		else {
			remoteStartInCnt = 0;
			gbFabricRemoteInt	=	NO;
		}
	}

	if(signalIn & 0x01 == 1) {	// ready in
		remoteReadyIn = READY;
	}
	else {
		remoteReadyIn = NOT_READY;
	}
	
}

void CheckPowerBtn(uint32_t signalIn)
{

#if 0
	static uint32_t powSwDet = NO;
	static uint32_t powSwOff = NO;
//	static uint32_t powSwDetCnt = 10000;// 맨처음 전원 온 시 꺼지지 않도록...
	static uint32_t powSwDetCnt = 0;// 맨처음 전원 온 시 꺼지지 않도록...
	static uint32_t powBtnBuzzer = 0;	

//	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
// powerSW  - 		cpu_onoff_det -- GPIO_6

#if POWER_BTN_CONTROL_ADD
//		if((Core_gpioInput & (0x1 << CPU_ONOFF_DET_ID)) == 0) {	
		if(signalIn & 0x4 == 0) {
			if(powSwDet == NO) {
				BUZZER_CON_ON;
				powSwDet = YES;				
			}
		}

		if(powSwDet == YES) {
			if(powBtnBuzzer++ == BUZZER_OFF_SEQUENCE_TIME) {
				BUZZER_CON_OFF;
				POW_BTN_DET_CLR();
			}
			
			if(powSwDetCnt++ > POWER_OFF_SEQUENCE_TIME) {
				powSwOff = YES;
				POW_CON_5V_CMD(OFF);
			}
		}

		if(powSwOff == YES) {
			POW_CON_5V_CMD(OFF);
		}
#endif

//dfp("signalIn = 0x%08x\n",signalIn);
#else
		static uint32_t	cnt	=	0;

		// ============= Power Button detection
			if((signalIn & 0x04) ==	0x0) {
				if(++cnt ==	1) {
					BUZZER_CON_ON;
		dp("button signalIn = %x\n",signalIn);
		//PowerOff_LED();
		devState.powerOffLed = ON;
				}
				else if(cnt	== 2)	{	// 200msec
					BUZZER_CON_OFF;
				}
			}
#endif

}

#if 0
void powerOffFunction()
{
	pLed_Control_Fn = led_Control;
	ALL_LED_OFF;
	pLed_Control_Fn = led_Control_Blank;				
	CPU_PWR_CON_LOW;	// power Off
}
#endif


#if 0
#define DOOR_CHECK_TIME			3
#define LEAK_CHECK_TIME			5

void CheckDeviceState()
{
	static unsigned char Leak = 0;
	static unsigned char Door = 0;
	static unsigned char buzzer = 0;

	if(special.Door_check == ON /*&& LcasState.state == STATE_RUN*/) {	 // 2 - High : error : open

		if((Core_gpioInput & 0x00000004) ) {
//				if(LcasState.state == STATE_RUN) Door++;
			if(LcasState.state == STATE_RUN || LcasState.state == STATE_INITILIZE || LcasState.state == STATE_ANALYSIS) Door++;
		}
		else {
			Door = 0;
			devState.DoorErrorSend = YES;
		}
		if(Door > DOOR_CHECK_TIME) {
			Door = DOOR_CHECK_TIME;
			devState.DoorError = YES;
			if(devState.DoorErrorSend == YES) {
				devState.DoorErrorSend = NO;
//					sendMsg
//motorBreakAll();
//motorSleepAll();
//sleepModeError();
dfp("SELF_ERROR_DOOR_OPEN\n");

				errorFunction(SELF_ERROR_DOOR_OPEN);
			}
		}
		else {
			devState.DoorError = NO;
		}
	}
		
	if(special.Leak_check == ON) {	// 5 - low : error : leak
		if((Core_gpioInput & 0x00000020) ) {	// 정상
			Leak = 0;
			devState.LeakErrorSend = YES;
//LcasState.errorCode = SELF_NO_ERROR;				
		}
		else {	// leak...
			Leak++;
		}
		
		if(Leak > LEAK_CHECK_TIME) {
			Leak = LEAK_CHECK_TIME;
			devState.LeakError = YES;
			if(devState.LeakErrorSend == YES) {
				devState.LeakErrorSend = NO;

				errorFunction(SELF_ERROR_LEAK);  //					sendMsg
dfp("SELF_ERROR_LEAK\n");
			}
		}
		else {
			if(devState.LeakError == YES) {
				LcasState.errorCode = SELF_NO_ERROR;
			}
			devState.LeakError = NO;
		}
	}



	if(LcasState.state == STATE_FAULT) {
		if(special.Buzzer_onoff == ON ) {
			if(++buzzer	< 3) {	BUZZER_CON_ON;	}
			else {
				BUZZER_CON_OFF;
				if(buzzer > 10) buzzer = 0;
			}
		}
	}
	else {
//		BUZZER_CON_OFF; -- stateChange()에서
	}

}

#else

#define DOOR_CHECK_TIME			3
#define LEAK_CHECK_TIME			5

#define	VOLT_LEAK_ERROR_CHECK_MIN			1.5f			// 1.5Volt이상 일경우	에러
#define	VOLT_TERMAL1_ERROR_CHECK_MIN			3.0f	// 3.0V(11V)	2.5f			// 2.5Volt(9.09Volt)이하 일경우	에러
#define	VOLT_TERMAL2_ERROR_CHECK_MIN			3.0f	// 3.0V(11V)	2.5f			// 2.5Volt(9.09Volt)이하 일경우	에러
#define	VOLT_DOOR_ERROR_CHECK_MIN			1.5f			// 1.5Volt이상 일경우	에러

extern uint32_t	adc_Sensor[4];
extern float errVolt[4];

void CheckDeviceState()		// error Check
{

	static unsigned char Leak = 0;
	static unsigned char Door = 0;
	static unsigned char buzzer = 0;

	errVolt[0] = adc_Sensor[0] * DIGIT_TO_VOLT_FACTOR;	// D_err
	if(special.Door_check == YES) {
		if(errVolt[0] > VOLT_DOOR_ERROR_CHECK_MIN) {
			if(LcasState.state == STATE_RUN || LcasState.state == STATE_INITILIZE || LcasState.state == STATE_ANALYSIS) Door++;
		}
		else {
			Door = 0;
			devState.DoorErrorSend = YES;
		}
		
		if(Door > DOOR_CHECK_TIME) {
			Door = DOOR_CHECK_TIME;
			devState.DoorError = YES;
			if(devState.DoorErrorSend == YES) {
				devState.DoorErrorSend = NO;
dfp("SELF_ERROR_DOOR_OPEN\n");
				errorFunction(SELF_ERROR_DOOR_OPEN);
			}
		}
		else {
			devState.DoorError = NO;
		}
	}

	errVolt[1] = adc_Sensor[1] * DIGIT_TO_VOLT_FACTOR;	// L_err
	if(special.Leak_check == ON) {	// 5 - low : error : leak
		if(errVolt[1] < VOLT_LEAK_ERROR_CHECK_MIN) {
			Leak++;
		}
		else {
			Leak = 0;
			devState.LeakErrorSend = YES;
		}

		if(Leak > LEAK_CHECK_TIME) {
			Leak = LEAK_CHECK_TIME;
			devState.LeakError = YES;
			if(devState.LeakErrorSend == YES) {
				devState.LeakErrorSend = NO;
				errorFunction(SELF_ERROR_LEAK);  //					sendMsg
dfp("SELF_ERROR_LEAK\n");
			}
		}
		else {
			if(devState.LeakError == YES) {
				LcasState.errorCode = SELF_NO_ERROR;
			}
			devState.LeakError = NO;
		}
	}

#if 0
	if(LcasState.fTemp	>	OVEN_TEMPSENSOR_ERROR_MAX	|| LcasState.fTemp	<	OVEN_TEMPSENSOR_ERROR_MIN	)	{
		if(devState.sensorError !=	YES) {
			devState.sensorError	=	YES;
LedState_Fault();

			gOVEN_STATE.uErrorCode = OVENERR_TEMPSENSOR_ERROR;
			ovenStt.uErrorCode = gOVEN_STATE.uErrorCode;

			sendSelfMsgError(gOVEN_STATE.uErrorCode);
			sendSelfMsgState(OVEN_STATE_FAULT);
			gOVEN_STATE.btState	=	OVEN_STATE_FAULT;
	//			Error	처리	fault
			ErrorProcess(1);
			dp("Temp Error\n");
		}
	}
	else devState.sensorError = NO;
#endif
#if 0
	errVolt[2] = adc_Sensor[2] * DIGIT_TO_VOLT_FACTOR;	// T1_err
	if(errVolt[2]	<	VOLT_TERMAL1_ERROR_CHECK_MIN)	{	// 12V(3.3V) 가	정상 - 에러	발생하면 0V	근처
		if(ovenStt.thError1 !=	YES) {
			ovenStt.thError1 = YES;
//				LED_Control_Error(OVENERR_THERMAL1_ERROR);
LedState_Fault();

			gOVEN_STATE.uErrorCode = OVENERR_THERMAL1_ERROR;
			ovenStt.uErrorCode = gOVEN_STATE.uErrorCode;

			sendSelfMsgError(gOVEN_STATE.uErrorCode);
			sendSelfMsgState(OVEN_STATE_FAULT);
			gOVEN_STATE.btState	=	OVEN_STATE_FAULT;
	//			Error	처리	fault
			ErrorProcess(1);
			dp("thError1\n");
		}
	}
	else ovenStt.thError1 = NO;

	errVolt[3] = adc_Sensor[3] * DIGIT_TO_VOLT_FACTOR;	// T2_err
	if(errVolt[3]	<	VOLT_TERMAL2_ERROR_CHECK_MIN)	{	// 12V(3.3V) 가	정상 - 에러	발생하면 0V	근처
		if(ovenStt.thError2 !=	YES) {
			ovenStt.thError2 = YES;
//				LED_Control_Error(OVENERR_THERMAL2_ERROR);
LedState_Fault();

			gOVEN_STATE.uErrorCode = OVENERR_THERMAL2_ERROR;
			ovenStt.uErrorCode = gOVEN_STATE.uErrorCode;

			sendSelfMsgError(gOVEN_STATE.uErrorCode);
			sendSelfMsgState(OVEN_STATE_FAULT);
			gOVEN_STATE.btState	=	OVEN_STATE_FAULT;
	//			Error	처리	fault
			ErrorProcess(1);
			dp("thError2\n");
		}
	}
	else ovenStt.thError2 = NO;

//	if(ovenStt.injError[0])	{	}
//	if(ovenStt.injError[1])	{	}

	if(devState.LeakError !=	YES	&& devState.thError1	!= YES &&	devState.thError2 !=	YES	 &&	devState.DoorError != YES && ovenStt.sensorError	!= YES &&	ovenStt.bServiceErr	!= YES)	{
		gOVEN_STATE.uErrorCode = OVENERR_NONE;

	}
	else {
		heaterOnOff(OFF);
dp("===heaterOnOff(OFF)\n");
	}

	if(ovenStt.uErrorCode == OVENERR_NONE) {
		BUZZER_CON_OFF;
		buzzerTime = 0;
	}
	else {
		if(gOVEN_CONFIG.Buzzer_onoff == YES) {
			if(buzzerTime == 1) {
				buzzerTime = 0;
				if(ovenStt.uErrorCode == OVENERR_LEAK_ERROR) {
					BUZZER_CON_ON;
				}
				else {
					BUZZER_CON_OFF;
				}
			}
			else {
				buzzerTime = 1;
				BUZZER_CON_ON;
			}
		}
	}

	return 0;	
#endif

	if(LcasState.state == STATE_FAULT) {
		if(special.Buzzer_onoff == ON ) {
			if(++buzzer	< 3) {	BUZZER_CON_ON;	}
			else {
				BUZZER_CON_OFF;
				if(buzzer > 10) buzzer = 0;
			}
		}
	}
	else {
//		BUZZER_CON_OFF; -- stateChange()에서
	}

}

#endif

