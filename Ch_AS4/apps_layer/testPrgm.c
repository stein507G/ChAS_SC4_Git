
/**************************************************************************/
/* Standard	Includes */
/**************************************************************************/

#include "config.h"

/**************************************************************************/
/* Driver	Includes */
/**************************************************************************/

// ryu

extern char pPacketBuff[TCP_SEND_PACKET_BUFF_SIZE];
extern struct tcp_pcb *netAS_PCB;

#include "net_AS.h"
#include "LcasAction_task.h"
#include "MotorCtrl_task.h"

#include "EEPROM.h"
#include "led_task.h"

#include "testPrgm.h"

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
extern uint32_t	gbTempReadTick;
//extern uint32_t	gbPIDCtrlTick;

extern uint32_t	gbFabricGpioInt;
extern uint32_t	remoteStartInCnt;
extern uint32_t	remoteStartInTime;

extern uint32_t	remoteStartOutSig;// = NO;	// 리모트	단자로 출력해야	된다.
extern uint32_t	remoteStartOutCnt;// = 0;
extern uint32_t	remoteStartOutTime;//	=	5;

extern float tempConst_A;
extern float tempConst_B;
extern float tempConst_C;

#if RS422_ENABLE
extern RS422_ST rs422if;
//extern RS422_BUF tx;
#endif


extern LCAS_DEV_STATE_t devState;
LCAS_BOARD_STATE_t	LcasBoardState;
LCAS_BOARD_CONTROL_t LcasBoardCmd; 
MOTOR_CTRL_t MotorCtrlSet;//,tmpMotorCtrlSet;

uint32_t uiMachineTestMode;


uint32_t uiBuzzerCnt;

void initMotorCtrlSet()
{
	for(int i=0; i< STEP_MOTOR_COUNT ; i++) {
		MotorCtrlSet.bDir[i] = DIR_CW;
		MotorCtrlSet.bInfinite[i] = NO;
	}
}

int testMotor(int value)
{
}

int testSensor(int value)
{
}

#if BOARD_TEST_PRGM
void boardTest_Function_Set()
{
	pTest_Fn[0] = testMotor;
	pTest_Fn[1] = testSensor;

	devState.actionMenu	=	BOARD_TEST_FUNCTION;
	devState.actionSubmenu = AS_INIT_SUB_NONE;

}

#endif

#if MACHINE_TEST_PRGM
void machineTest_Function_Set()
{

//	pMachineTest_Fn = machineTest_Mode_Blank;
/*
	pMachineTest_Fn[0] = machineTest_Mode_X;
	pMachineTest_Fn[1] = machineTest_Mode_Y;
	pMachineTest_Fn[2] = machineTest_Mode_Z;
	pMachineTest_Fn[3] = machineTest_Mode_SYR;
	pMachineTest_Fn[4] = machineTest_Mode_VAL;
	pMachineTest_Fn[5] = machineTest_Mode_INJ;
	pMachineTest_Fn[6] = machineTest_Mode_VAL_SYR;
	pMachineTest_Fn[7] = machineTest_Mode_BOARD_ALL;
*/

// 전원을 켰을 때 초기값을 검사하여 테스트 모드를 정한다.
	uiMachineTestMode = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
//	uiMachineTestMode = (uiMachineTestMode >> 16) & 0x07;
	uiMachineTestMode = (uiMachineTestMode >> 16) & 0x3F;	
	dfp("uiMachineTestMode = 0x%08x\n",uiMachineTestMode);

/*
	switch(uiMachineTestMode) {
		case TEST_MODE_X:				pMachineTest_Fn = machineTest_Mode_X;			break;
		case TEST_MODE_Y:				pMachineTest_Fn = machineTest_Mode_Y;			break;
		case TEST_MODE_Z:				pMachineTest_Fn = machineTest_Mode_Z;			break;
		case TEST_MODE_SYR:			pMachineTest_Fn = machineTest_Mode_SYR;		break;
		case TEST_MODE_VAL:			pMachineTest_Fn = machineTest_Mode_VAL;		break;
		case TEST_MODE_INJ:			pMachineTest_Fn = machineTest_Mode_INJ;		break;
		case TEST_MODE_VAL_SYR:		pMachineTest_Fn = machineTest_Mode_VAL_SYR;		break;
		case TEST_MODE_BOARD_ALL:	pMachineTest_Fn = machineTest_Mode_BOARD_ALL;	break;
	}
*/

	switch(uiMachineTestMode) {
//		case TEST_MODE_X:					pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_X;			break;
//		case TEST_MODE_Y:					pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_Y;			break;
//		case TEST_MODE_Z:					pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_Z;			break;
//		case TEST_MODE_SYR:				pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_SYR;		break;
//		case TEST_MODE_VAL:				pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_VAL;		break;
//		case TEST_MODE_INJ:				pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_INJ;		break;

		// SW2만 누르면서 부팅
		case TEST_MODE_X_Y_Z:			pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_X_Y_Z;	break;
		// SW3만 누르면서 부팅
		case TEST_MODE_VAL_SYR:		pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_VAL_SYR;		break;
		// 아무것도 안누른 상태 
		case TEST_MODE_BOARD_ALL:	pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_BOARD_ALL;	break;
		
		default: pAction_Fn[MACHINE_TEST_FUNCTION] = machineTest_Mode_X_Y_Z;		break;
	}

	devState.actionMenu	=	MACHINE_TEST_FUNCTION;
	devState.actionSubmenu = AS_INIT_SUB_NONE;

	uiBuzzerCnt = 10;

}

#endif

//===============================================================
// devState.actionMenu
// devState.actionSubmenu
//===============================================================
























int machine_InitHome(int value)
{


}
#if 0
void CheckBoardState() 	// 보드테스트나 기구테스트시에 사용한다. 
{
	static uint32_t powSwDet = NO;
	static uint32_t powSwDetCnt = 10000;// 맨처음 전원 온 시 꺼지지 않도록...
	static unsigned char Leak = NO;
	static unsigned char Door = NO;
	
//		Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));

	LcasBoardState.gpioInput = Core_gpioInput;

	LcasBoardState.ledState = Core_gpio_pattern;
	LcasBoardState.onOffState = Core_gpio_pattern;

	// powerSW  - 		cpu_onoff_det -- GPIO_6
	LcasBoardState.btMotorState[MOTOR_X] = devState.btMotorState[MOTOR_X];
	LcasBoardState.btMotorState[MOTOR_Y] = devState.btMotorState[MOTOR_Y];
	LcasBoardState.btMotorState[MOTOR_Z] = devState.btMotorState[MOTOR_Z];
	LcasBoardState.btMotorState[MOTOR_SYR] = devState.btMotorState[MOTOR_SYR];
	LcasBoardState.btMotorState[MOTOR_VAL] = devState.btMotorState[MOTOR_VAL];
	LcasBoardState.btMotorState[MOTOR_INJ] = devState.btMotorState[MOTOR_INJ];		

#if 0
diprintf("Core_gpioInput = 0x%08x\n", Core_gpioInput);
diprintf("X = 0x%02x Y = 0x%02x Z = 0x%02x Syr = 0x%02x Val = 0x%02x Inj = 0x%02x\n", 
devState.btMotorState[MOTOR_X],
devState.btMotorState[MOTOR_Y],
devState.btMotorState[MOTOR_Z],
devState.btMotorState[MOTOR_SYR],
devState.btMotorState[MOTOR_VAL],
devState.btMotorState[MOTOR_INJ]	);	
#endif
	LcasBoardState.fTemp = LcasState.fTemp;
	LcasBoardState.tempReady = LcasState.fTemp;
	LcasBoardState.errorCode = LcasState.errorCode;

	LcasBoardState.sysDate.year = sysInform.cSysDate.year;
	LcasBoardState.sysDate.month = sysInform.cSysDate.month;
	LcasBoardState.sysDate.date = sysInform.cSysDate.date;
	LcasBoardState.sysTime.hour = sysInform.cSysTime.hour;
	LcasBoardState.sysTime.minute = sysInform.cSysTime.hour;
	LcasBoardState.sysTime.second = sysInform.cSysTime.hour;

	if(uiBuzzerCnt > 0) {	
		BUZZER_CON_ON;	
		--uiBuzzerCnt;		
	}
	else {
		BUZZER_CON_OFF;
	}
	
}
#endif

#if 0
void motorMoving()
{
	static uint32_t powSwDet = NO;
	static uint32_t powSwDetCnt = 10000;// 맨처음 전원 온 시 꺼지지 않도록...
	static unsigned char Leak = NO;
	static unsigned char Door = NO;

	uint32_t who = MOTOR_Z;

	static uint32_t Core_gpioInput_old;
	
	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		dfp("Core_gpioInputn = 0x%08x\n",Core_gpioInput);	
		Core_gpioInput_old = Core_gpioInput;
	}

// MSEN[5:6] == [21:16]

	switch(Core_gpioInput & 0x00300000) {
		case 0x00000000:	// 둘다

			break;
		case 0x00100000:
			who = MOTOR_X; // 아래 버튼 - X 축동작 버튼
			if(!(Core_gpioInput & 0x00010000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 40);		
			dfp("=======HOME_DIR_X = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00020000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 40);
			dfp("=========R_HOME_DIR_X = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00040000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 1600);		
			dfp("=======HOME_DIR_X = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00080000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 1600);		
			dfp("=========R_HOME_DIR_X = 0x%08x\n",Core_gpioInput);						
				}
			}
			else {	// 버튼에서 손떼면 스톱(감속)
				stepCtrlCmd(who , devState.curDir[who] | CMD_STOP);
			}

			break;

		case 0x00200000:	// 위버튼 누르면  - Y 축 동작 버튼
			who = MOTOR_Y;
			if(!(Core_gpioInput & 0x00010000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_Y | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 40);		
			dfp("=======HOME_DIR_Y = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00020000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_Y | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 40);
			dfp("=========R_HOME_DIR_Y = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00040000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_Y | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 1600);		
			dfp("=======HOME_DIR_Y = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00080000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_Y | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 1600);		
			dfp("=========R_HOME_DIR_Y = 0x%08x\n",Core_gpioInput);						
				}
			}
			else {	// 버튼에서 손떼면 스톱(감속)
				stepCtrlCmd(who , devState.curDir[who] | CMD_STOP);
			}
			
			break;
		case 0x00300000:	// 안눌림. -  Z 축 동작
			who = MOTOR_Z;
			if(!(Core_gpioInput & 0x00010000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_Z | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 50);		
			dfp("=======HOME_DIR_Z = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00020000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_Z | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 50);
			dfp("=========R_HOME_DIR_Z = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00040000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_Z | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 2400);		
			dfp("=======HOME_DIR_Z = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00080000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_Z | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 2400);		
			dfp("=========R_HOME_DIR_Z = 0x%08x\n",Core_gpioInput);						
				}
			}	
			else {	// 버튼에서 손떼면 스톱(감속)
				stepCtrlCmd(who , devState.curDir[who] | CMD_STOP);
			}
			
			break;
	}

}
#endif




#if MACHINE_TEST_PRGM

int machineTest_Mode_X(int value)
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin,keyin1;

	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		keyin1 = keyin >> 3;
		dfp("keyin = 0x%08x  0x%08x\n",keyin,keyin1);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// 
			break;		
		case TEST_MODE_SW1_SW2:		//  - sensing
			break;
		case TEST_MODE_SW1_SW3:		//  - step
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:		break;

		case TEST_MODE_SW3:		break;

		case TEST_MODE_SW2_SW3:		break;

		case TEST_MODE_SW4:		// 
			break;
		case TEST_MODE_SW4_SW2:		//  - sensing
			break;
		case TEST_MODE_SW4_SW3:		//  - step
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// 
			break;
		case TEST_MODE_SW5_SW2:		//  - sensing
			break;
		case TEST_MODE_SW5_SW3:		//  - step
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// 
			break;
		case TEST_MODE_SW6_SW2:		//  - sensing
			break;
		case TEST_MODE_SW6_SW3:		//  - step
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		

	}
		dfp("uiMachineTestMode = 0x%08x\n",uiMachineTestMode);
}

int machineTest_Mode_Y(int value)
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin,keyin1;

	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		keyin1 = keyin >> 3;
		dfp("keyin = 0x%08x  0x%08x\n",keyin,keyin1);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// 
			break;		
		case TEST_MODE_SW1_SW2:		//  - sensing
			break;
		case TEST_MODE_SW1_SW3:		//  - step
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:		break;

		case TEST_MODE_SW3:		break;

		case TEST_MODE_SW2_SW3:		break;

		case TEST_MODE_SW4:		// 
			break;
		case TEST_MODE_SW4_SW2:		//  - sensing
			break;
		case TEST_MODE_SW4_SW3:		//  - step
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// 
			break;
		case TEST_MODE_SW5_SW2:		//  - sensing
			break;
		case TEST_MODE_SW5_SW3:		//  - step
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// 
			break;
		case TEST_MODE_SW6_SW2:		//  - sensing
			break;
		case TEST_MODE_SW6_SW3:		//  - step
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		

	}
		dfp("uiMachineTestMode = 0x%08x\n",uiMachineTestMode);
}

int machineTest_Mode_Z(int value)
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin,keyin1;

	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		keyin1 = keyin >> 3;
		dfp("keyin = 0x%08x  0x%08x\n",keyin,keyin1);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// 
			break;		
		case TEST_MODE_SW1_SW2:		//  - sensing
			break;
		case TEST_MODE_SW1_SW3:		//  - step
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:		break;

		case TEST_MODE_SW3:		break;

		case TEST_MODE_SW2_SW3:		break;

		case TEST_MODE_SW4:		// 
			break;
		case TEST_MODE_SW4_SW2:		//  - sensing
			break;
		case TEST_MODE_SW4_SW3:		//  - step
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// 
			break;
		case TEST_MODE_SW5_SW2:		//  - sensing
			break;
		case TEST_MODE_SW5_SW3:		//  - step
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// 
			break;
		case TEST_MODE_SW6_SW2:		//  - sensing
			break;
		case TEST_MODE_SW6_SW3:		//  - step
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		

	}
		dfp("uiMachineTestMode = 0x%08x\n",uiMachineTestMode);
}

int machineTest_Mode_SYR(int value)
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin,keyin1;

	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		keyin1 = keyin >> 3;
		dfp("keyin = 0x%08x  0x%08x\n",keyin,keyin1);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// 
			break;		
		case TEST_MODE_SW1_SW2:		//  - sensing
			break;
		case TEST_MODE_SW1_SW3:		//  - step
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:		break;

		case TEST_MODE_SW3:		break;

		case TEST_MODE_SW2_SW3:		break;

		case TEST_MODE_SW4:		// 
			break;
		case TEST_MODE_SW4_SW2:		//  - sensing
			break;
		case TEST_MODE_SW4_SW3:		//  - step
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// 
			break;
		case TEST_MODE_SW5_SW2:		//  - sensing
			break;
		case TEST_MODE_SW5_SW3:		//  - step
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// 
			break;
		case TEST_MODE_SW6_SW2:		//  - sensing
			break;
		case TEST_MODE_SW6_SW3:		//  - step
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		

	}
		dfp("uiMachineTestMode = 0x%08x\n",uiMachineTestMode);
}

int machineTest_Mode_VAL(int value)
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin,keyin1;

	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		keyin1 = keyin >> 3;
		dfp("keyin = 0x%08x  0x%08x\n",keyin,keyin1);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// 
			break;		
		case TEST_MODE_SW1_SW2:		//  - sensing
			break;
		case TEST_MODE_SW1_SW3:		//  - step
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:		break;

		case TEST_MODE_SW3:		break;

		case TEST_MODE_SW2_SW3:		break;

		case TEST_MODE_SW4:		// 
			break;
		case TEST_MODE_SW4_SW2:		//  - sensing
			break;
		case TEST_MODE_SW4_SW3:		//  - step
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// 
			break;
		case TEST_MODE_SW5_SW2:		//  - sensing
			break;
		case TEST_MODE_SW5_SW3:		//  - step
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// 
			break;
		case TEST_MODE_SW6_SW2:		//  - sensing
			break;
		case TEST_MODE_SW6_SW3:		//  - step
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		

	}
		dfp("uiMachineTestMode = 0x%08x\n",uiMachineTestMode);
}

int machineTest_Mode_INJ(int value)
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin,keyin1;

	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		keyin1 = keyin >> 3;
		dfp("keyin = 0x%08x  0x%08x\n",keyin,keyin1);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// 
			break;		
		case TEST_MODE_SW1_SW2:		//  - sensing
			break;
		case TEST_MODE_SW1_SW3:		//  - step
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:		break;

		case TEST_MODE_SW3:		break;

		case TEST_MODE_SW2_SW3:		break;

		case TEST_MODE_SW4:		// 
			break;
		case TEST_MODE_SW4_SW2:		//  - sensing
			break;
		case TEST_MODE_SW4_SW3:		//  - step
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// 
			break;
		case TEST_MODE_SW5_SW2:		//  - sensing
			break;
		case TEST_MODE_SW5_SW3:		//  - step
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// 
			break;
		case TEST_MODE_SW6_SW2:		//  - sensing
			break;
		case TEST_MODE_SW6_SW3:		//  - step
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		

	}
		dfp("uiMachineTestMode = 0x%08x\n",uiMachineTestMode);
}


#if 0
void machineTest_KeyIn()
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin,keyin1;

	static uint32_t func = 0;
	static uint32_t func_first = 0;
	
	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		keyin1 = keyin >> 3;
		dfp("keyin = 0x%08x  func= %d\n",keyin,func);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// CCW
			func = 1;
			break;
		case TEST_MODE_SW1_SW2:		// CCW - sensing
			break;
		case TEST_MODE_SW1_SW3:		// CCW - step
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:
			func = 2;
			func_first = 1;
			break;

		case TEST_MODE_SW3:
			if(func == 2) {	// 이전 키가 SW2이면 
				if(func_first == 1) {
					subMenu_ihpv = 1;			
					func_first = 0;
					if(sleepChkVal(MOTOR_VAL)) { 
						motorReadyCmd(MOTOR_VAL);
					}
				}
 				if(initHomePos_Val() == ACTION_MENU_END) {func = 0; func_first = 0;}
			}
			else {
				func = 3;
			}
			break;

		case TEST_MODE_SW2_SW3:		break;

		case TEST_MODE_SW4:		// CW
			break;
		case TEST_MODE_SW4_SW2:		// CW - sensing
			break;
		case TEST_MODE_SW4_SW3:		// CW - step
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// up
			break;
		case TEST_MODE_SW5_SW2:		// up - sensing
			break;
		case TEST_MODE_SW5_SW3:		// up - step
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// down
			break;
		case TEST_MODE_SW6_SW2:		// down - sensing
			break;
		case TEST_MODE_SW6_SW3:		// down - step
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		

	}

}
#endif

// ============================================================================
// ############################################################################
// X Y Z 테스트 
// ============================================================================
//#define M_TEST_XYZ_SPEED_X							(149)
#define M_TEST_XYZ_SPEED_X							ACT_SPEED_MAX_X
//#define M_TEST_XYZ_LONG_STEP_X					(FIND_HOME_UNIT_STEP_X  * 1600)
#define M_TEST_XYZ_LONG_STEP_X					(FIND_HOME_UNIT_STEP_X  * 1280)
#define M_TEST_XYZ_SHORT_STEP_X					(FIND_HOME_UNIT_STEP_X  * 40)

//#define M_TEST_XYZ_SPEED_Y							(149)
#define M_TEST_XYZ_SPEED_Y							ACT_SPEED_MAX_Y
//#define M_TEST_XYZ_LONG_STEP_Y					(FIND_HOME_UNIT_STEP_Y  * 1600)
#define M_TEST_XYZ_LONG_STEP_Y					(FIND_HOME_UNIT_STEP_Y  * 1280)
#define M_TEST_XYZ_SHORT_STEP_Y					(FIND_HOME_UNIT_STEP_Y  * 40)

//#define M_TEST_XYZ_SPEED_Z							(149)
#define M_TEST_XYZ_SPEED_Z							ACT_SPEED_MAX_Z
//#define M_TEST_XYZ_LONG_STEP_Z					(FIND_HOME_UNIT_STEP_Z  * 2400)
#define M_TEST_XYZ_LONG_STEP_Z					(FIND_HOME_UNIT_STEP_Z  * 1920)
#define M_TEST_XYZ_SHORT_STEP_Z					(FIND_HOME_UNIT_STEP_Z  * 50)


uint8_t subMenu_mtxyz = 1;

int machineTest_Mode_X_Y_Z(int value)
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin;
	static uint32_t func = 0;
	static uint32_t acting = 0;

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif
	
	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		dfp("keyin = 0x%08x  func= %d\n",keyin,acting);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_X_LEFT_LONG;
				acting = 1;
			}
			break;		
		case TEST_MODE_SW1_SW2:		// 2번 버튼을 누른상태에서 1번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_Y_BACK_LONG;
				acting = 1;
			}
			break;
		case TEST_MODE_SW1_SW3:		// 3번 버튼을 누른상태에서 1번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_X_LEFT_SHORT;
				acting = 1;
			}
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:
			if(acting == 0) {
//				acting = 2;
			}
			break;

		case TEST_MODE_SW3:
			if(acting == 0) {
//				acting = 3;
			}
			break;
		case TEST_MODE_SW2_SW3: //  1cycle Test : 2번 버튼을 누른상태에서 3번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_1CYCLE;
				acting = 2;
			}
			break;
		case TEST_MODE_SW4:		//  
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_X_RIGHT_LONG;
				acting = 4;
			}
			break;
		case TEST_MODE_SW4_SW2:		// 2번 버튼을 누른상태에서 4번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_Y_FTONT_LONG;				
				acting = 4;
			}
			break;
		case TEST_MODE_SW4_SW3:		// 3번 버튼을 누른상태에서 4번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_X_RIGHT_SHORT;
				acting = 4;
			}
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// z-up
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_Z_UP_LONG;
				acting = 5;
			}
			break;
		case TEST_MODE_SW5_SW2:		// 2번 버튼을 누른상태에서 5번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_Y_BACK_SHORT;				
				acting = 5;
			}
			break;
		case TEST_MODE_SW5_SW3:		// 3번 버튼을 누른상태에서 5번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_Z_UP_SHORT;
				acting = 5;
			}
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// down
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_Z_DOWN_LONG;
				acting = 6;
			}
			break;
		case TEST_MODE_SW6_SW2:		// 2번 버튼을 누른상태에서 6번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_Y_FTONT_SHORT;
				acting = 6;
			}
			break;
		case TEST_MODE_SW6_SW3:		// 3번 버튼을 누른상태에서 6번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_XYZ_Z_DOWN_SHORT;
				acting = 6;
			}
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		
	}

	if(	mTest_X_Y_Z_Act(func) == 	ACTION_MENU_END) {
		acting = 0;
	}
	
}

int mTest_X_Y_Z_Act(int func)
{

	static uint32_t subMenu = 1;
	static uint32_t acting = 0;
	
// =============================================================
	switch(devState.actionSubmenu) {
		case M_TEST_XYZ_NONE:		break;
		
		case M_TEST_XYZ_X_LEFT_LONG:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_X)) { motorReadyCmd(MOTOR_X);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_X))	{
//						stepControl(MOTOR_X, R_HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_X * 1600	);
						stepControl(MOTOR_X, R_HOME_DIR_X | CMD_START, M_TEST_XYZ_SPEED_X, M_TEST_XYZ_LONG_STEP_X);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_X))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_XYZ_X_RIGHT_LONG:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_X)) { motorReadyCmd(MOTOR_X);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_X))	{
//						stepControl(MOTOR_X, HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_X * 1600);					
						stepControl(MOTOR_X, HOME_DIR_X | CMD_START, M_TEST_XYZ_SPEED_X, M_TEST_XYZ_LONG_STEP_X);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_X))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_XYZ_X_LEFT_SHORT:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_X)) { motorReadyCmd(MOTOR_X);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_X))	{
						stepControl(MOTOR_X, R_HOME_DIR_X | CMD_START, M_TEST_XYZ_SPEED_X, M_TEST_XYZ_SHORT_STEP_X);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_X))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_XYZ_X_RIGHT_SHORT:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_X)) { motorReadyCmd(MOTOR_X);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_X))	{
						stepControl(MOTOR_X, HOME_DIR_X | CMD_START, M_TEST_XYZ_SPEED_X, M_TEST_XYZ_SHORT_STEP_X);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_X))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;

		case M_TEST_XYZ_Y_FTONT_LONG:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_Y)) { motorReadyCmd(MOTOR_Y);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_Y))	{
						stepControl(MOTOR_Y, R_HOME_DIR_Y | CMD_START, M_TEST_XYZ_SPEED_Y, M_TEST_XYZ_LONG_STEP_Y);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_Y))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_XYZ_Y_BACK_LONG:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_Y)) { motorReadyCmd(MOTOR_Y);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_Y))	{
						stepControl(MOTOR_Y, HOME_DIR_Y | CMD_START, M_TEST_XYZ_SPEED_Y, M_TEST_XYZ_LONG_STEP_Y);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_Y))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_XYZ_Y_FTONT_SHORT:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_Y)) { motorReadyCmd(MOTOR_Y);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_Y))	{
						stepControl(MOTOR_Y, R_HOME_DIR_Y | CMD_START, M_TEST_XYZ_SPEED_Y, M_TEST_XYZ_SHORT_STEP_Y);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_Y))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_XYZ_Y_BACK_SHORT:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_Y)) { motorReadyCmd(MOTOR_Y);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_Y))	{
						stepControl(MOTOR_Y, HOME_DIR_Y | CMD_START, M_TEST_XYZ_SPEED_Y, M_TEST_XYZ_SHORT_STEP_Y);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_Y))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;

		case M_TEST_XYZ_Z_UP_LONG:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_Z)) { motorReadyCmd(MOTOR_Z);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_Z))	{
    dfp("M_TEST_XYZ_Z_UP_LONG=%d\n",M_TEST_XYZ_Z_UP_LONG);		      
						stepControl(MOTOR_Z, HOME_DIR_Z | CMD_START, M_TEST_XYZ_SPEED_Z, M_TEST_XYZ_LONG_STEP_Z);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_Z))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_XYZ_Z_DOWN_LONG:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_Z)) { motorReadyCmd(MOTOR_Z);}					
					subMenu++;
					break;
				case 2:
		      if(readyChk(MOTOR_Z))	{
						stepControl(MOTOR_Z, R_HOME_DIR_Z | CMD_START, M_TEST_XYZ_SPEED_Z, M_TEST_XYZ_LONG_STEP_Z);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_Z))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_XYZ_Z_UP_SHORT:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_Z)) { motorReadyCmd(MOTOR_Z);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_Z))	{
						stepControl(MOTOR_Z, HOME_DIR_Z | CMD_START, M_TEST_XYZ_SPEED_Z, M_TEST_XYZ_SHORT_STEP_Z);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_Z))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_XYZ_Z_DOWN_SHORT:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChk(MOTOR_Z)) { motorReadyCmd(MOTOR_Z);}					
					subMenu++;			
					break;
				case 2:
		      if(readyChk(MOTOR_Z))	{
						stepControl(MOTOR_Z, R_HOME_DIR_Z | CMD_START, M_TEST_XYZ_SPEED_Z, M_TEST_XYZ_SHORT_STEP_Z);
						subMenu++;			
		      }
					break;
				case 3:
					if(readyChk(MOTOR_Z))	{
						devState.actionSubmenu = M_TEST_XYZ_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;

		case M_TEST_XYZ_1CYCLE:
#if 0			
			if(mTest_Mode_XYZ_1Cycle(func) == ACTION_MENU_END) {
				devState.actionSubmenu = M_TEST_XYZ_NONE;
				return ACTION_MENU_END;
			}
#else
			devState.actionSubmenu = M_TEST_XYZ_NONE;
			subMenu = 1;
			return ACTION_MENU_END;
#endif						
			break;
	}

	return ACTION_MENU_ING;	
}

int mTest_Mode_XYZ_1Cycle(int func)
{

}

// ============================================================================
// ############################################################################
// 3way Valve 와 syringe 테스트 
// cw - injection port
// ccw - washing poirt
// ============================================================================
#define M_TEST_SYRINGE_INJ_VOLUME			200.0f // 100.0f
#define M_TEST_SYRINGE_STEP_VALUE			(ADJUST_STEP_UNIT_SYR * 4)
#define M_TEST_SYRINGE_UP_SPEED				WASH_SPEED_UP_SYR
#define M_TEST_SYRINGE_DOWN_SPEED			WASH_SPEED_DOWN_SYR

uint8_t subMenu_mtvsc = 1;
uint8_t subMenu_mtvh = 1;
uint8_t subMenu_mtsh = 1;

int machineTest_Mode_VAL_SYR(int value)
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin;
	static uint32_t func = 0;
	static uint32_t acting = 0;

#if	DEBUG_LCASACTION_SUBMENU_NO
static uint32_t	subMenu	=	0;

if(devState.actionSubmenu !=	subMenu)	{
		dfp("devState.actionSubmenu=%d\n",devState.actionSubmenu);
		subMenu = devState.actionSubmenu;
}
#endif
	
	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		dfp("keyin = 0x%08x  func= %d\n",keyin,acting);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// CW
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_FN_VAL_CW_90;
				acting = 1;
			}
			break;		
		case TEST_MODE_SW1_SW2:		// Val home : 2번 버튼을 누른상태에서 1번 키누름 
			if(acting == 0) {
#if 0		// 센서설치해야함.		
				devState.actionSubmenu = M_TEST_FN_VAL_HOME;
				acting = 1;
#endif				
			}
			break;
		case TEST_MODE_SW1_SW3:		// CW - step : 3번 버튼을 누른상태에서 1번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_FN_VAL_CW_STEP;
				acting = 1;
			}
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:
			if(acting == 0) {
//				acting = 2;
			}
			break;

		case TEST_MODE_SW3:
			if(acting == 0) {
//				acting = 3;
			}
			break;
		case TEST_MODE_SW2_SW3: //  1cycle Test : 2번 버튼을 누른상태에서 3번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_FN_VAL_SYR_1CYCLE;
				acting = 2;
			}
			break;
		case TEST_MODE_SW4:		// CCW 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_FN_VAL_CCW_90;
				acting = 4;
			}
			break;
		case TEST_MODE_SW4_SW2:		// Syr home : 2번 버튼을 누른상태에서 4번 키누름 
			if(acting == 0) {
//				devState.actionSubmenu = M_TEST_FN_SYR_HOME;				
//				acting = 4;
			}
			break;
		case TEST_MODE_SW4_SW3:		// CCW - step : 3번 버튼을 누른상태에서 4번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_FN_VAL_CCW_STEP;
				acting = 4;
			}
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// up
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_FN_SYR_UP_FULL;
				acting = 5;
			}
			break;
		case TEST_MODE_SW5_SW2:		// Syr home : 2번 버튼을 누른상태에서 5번 키누름 
			if(acting == 0) {
#if 0				// 센서설치해야함.			
				devState.actionSubmenu = M_TEST_FN_SYR_HOME;				
				acting = 5;
#endif				
			}
			break;
		case TEST_MODE_SW5_SW3:		// up - step : 3번 버튼을 누른상태에서 5번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_FN_SYR_UP_STEP;
				acting = 5;
			}
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// down
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_FN_SYR_DOWN_FULL;
				acting = 6;
			}
			break;
		case TEST_MODE_SW6_SW2:		// down - sensing : 2번 버튼을 누른상태에서 6번 키누름 
			if(acting == 0) {
//				devState.actionSubmenu = M_TEST_FN_SYR_UP_STEP;
//				acting = 6;
			}
			break;
		case TEST_MODE_SW6_SW3:		// down - step : 3번 버튼을 누른상태에서 6번 키누름 
			if(acting == 0) {
				devState.actionSubmenu = M_TEST_FN_SYR_DOWN_STEP;
				acting = 6;
			}
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		
	}

//	mTest_VAL_SYR_Act(func);
	if(	mTest_VAL_SYR_Act(func) == 	ACTION_MENU_END) {
		acting = 0;
	}

}

int mTest_VAL_SYR_Act(int func)
{

	static uint32_t subMenu = 1;
	static uint32_t acting = 0;
	
//mTest_VAL_SYR_Act_1Cycle
//
// =============================================================
// Washing : CCW
// 90도 : UNIT_STEP_VAL * (int)(MICRO_STEP_VAL) = 50 x 8 = 400;
	switch(devState.actionSubmenu) {
		case M_TEST_FN_NONE:		break;
		
		case M_TEST_FN_VAL_CW_90:		
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChkVal(MOTOR_VAL)) {
						motorReadyCmd(MOTOR_VAL);		
					}
					subMenu++;			
					break;
				case 2:
					if(readyChkVal(MOTOR_VAL))	{
						moveStepSpeed(MOTOR_VAL, 0, ACT_SPEED_MAX_VAL, -(UNIT_STEP_VAL * (int)(MICRO_STEP_VAL)));
						subMenu++;
					}
					break;
				case 3:
					if(readyChkVal(MOTOR_VAL))	{
						devState.actionSubmenu = M_TEST_FN_NONE;
						dfp("goVal - POS_VAL_NEEDLE %d \n",func);										
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_FN_VAL_CW_STEP:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChkVal(MOTOR_VAL)) {
						motorReadyCmd(MOTOR_VAL);		
					}
					subMenu++;			
					break;
				case 2:
					if(readyChkVal(MOTOR_VAL))	{
						moveStepSpeed(MOTOR_VAL, 0, ADJUST_SPEED_VAL, -ADJUST_STEP_UNIT_VAL);			
						subMenu++;			
					}
					break;
				case 3:
					if(readyChkVal(MOTOR_VAL))	{					
						devState.actionSubmenu = M_TEST_FN_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}
			break;
		case M_TEST_FN_VAL_CCW_90:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChkVal(MOTOR_VAL)) {
						motorReadyCmd(MOTOR_VAL);		
					}
					subMenu++;			
					break;
				case 2:
					if(readyChkVal(MOTOR_VAL))	{
						moveStepSpeed(MOTOR_VAL, 0, ACT_SPEED_MAX_VAL, (UNIT_STEP_VAL * (int)(MICRO_STEP_VAL)));				
						subMenu++;
					}
					break;
				case 3:
					if(readyChkVal(MOTOR_VAL))	{					
		dfp("goVal - POS_VAL_WASHING %d \n",func);						
						devState.actionSubmenu = M_TEST_FN_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}		
			break;		
		case M_TEST_FN_VAL_CCW_STEP:
			switch(subMenu) {
				case 1:	// 모터 초기화 RESET
					if(sleepChkVal(MOTOR_VAL)) {
						motorReadyCmd(MOTOR_VAL);		
					}
					subMenu++;			
					break;
				case 2:
					if(readyChkVal(MOTOR_VAL))	{			
						moveStepSpeed(MOTOR_VAL, 0, ADJUST_SPEED_VAL, ADJUST_STEP_UNIT_VAL);						
						subMenu++;
					}
					break;
				case 3:
					if(readyChkVal(MOTOR_VAL))	{					
						devState.actionSubmenu = M_TEST_FN_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}		
			break;
			
		case M_TEST_FN_SYR_UP_FULL:
			switch(subMenu) {
				case 1:
					if(sleepChk(MOTOR_SYR)) { motorReadyCmd(MOTOR_SYR);}
					subMenu++;			
					break;
				case 2:
					if(readyChk(MOTOR_SYR))	{
						moveStepSpeed(MOTOR_SYR, 0, M_TEST_SYRINGE_UP_SPEED, -(SYR_PULSE_PER_VOL_500_FLOAT * M_TEST_SYRINGE_INJ_VOLUME) );						
						subMenu++;						
					}
					break;
				case 3:
					if(readyChk(MOTOR_SYR))	{					
						devState.actionSubmenu = M_TEST_FN_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}		
			break;		
		case M_TEST_FN_SYR_UP_STEP:
			switch(subMenu) {
				case 1:
					if(sleepChk(MOTOR_SYR)) { motorReadyCmd(MOTOR_SYR);}
					subMenu++;			
					break;
				case 2:
					if(readyChk(MOTOR_SYR))	{
						moveStepSpeed(MOTOR_SYR, 0, ADJUST_SPEED_SYL, -M_TEST_SYRINGE_STEP_VALUE);						
						subMenu++;						
					}
					break;
				case 3:
					if(readyChk(MOTOR_SYR))	{					
						devState.actionSubmenu = M_TEST_FN_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}		
			break;		
		case M_TEST_FN_SYR_DOWN_FULL:		
			switch(subMenu) {
				case 1:
					if(sleepChk(MOTOR_SYR)) { motorReadyCmd(MOTOR_SYR);}
					subMenu++;			
					break;
				case 2:
					if(readyChk(MOTOR_SYR))	{
						moveStepSpeed(MOTOR_SYR, 0, M_TEST_SYRINGE_DOWN_SPEED, (SYR_PULSE_PER_VOL_500_FLOAT * M_TEST_SYRINGE_INJ_VOLUME) );						
						subMenu++;						
					}
					break;
				case 3:
					if(readyChk(MOTOR_SYR))	{					
						devState.actionSubmenu = M_TEST_FN_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}		
			break;		
		case M_TEST_FN_SYR_DOWN_STEP:
			switch(subMenu) {
				case 1:
					if(sleepChk(MOTOR_SYR)) { motorReadyCmd(MOTOR_SYR);}
					subMenu++;			
					break;
				case 2:
					if(readyChk(MOTOR_SYR))	{
						moveStepSpeed(MOTOR_SYR, 0, ADJUST_SPEED_SYL, M_TEST_SYRINGE_STEP_VALUE);						
						subMenu++;						
					}
					break;
				case 3:
					if(readyChk(MOTOR_SYR))	{					
						devState.actionSubmenu = M_TEST_FN_NONE;
						subMenu = 1;
						return ACTION_MENU_END;
					}
					break;
			}		
			break;		
		case M_TEST_FN_VAL_SYR_1CYCLE:
//			washport --> down 100ul --> injection port --> up 100ul
			if(machineTest_Mode_VAL_SYR_1Cycle(func) == ACTION_MENU_END) {
				devState.actionSubmenu = M_TEST_FN_NONE;
				return ACTION_MENU_END;
			}
			break;

		case M_TEST_FN_VAL_HOME:	// valve home find
			if(machineTest_Mode_VAL_Home(func) == ACTION_MENU_END) {
				devState.actionSubmenu = M_TEST_FN_NONE;
				return ACTION_MENU_END;
			}
			break;

		case M_TEST_FN_SYR_HOME:	// syringe home check
			if(machineTest_Mode_SYR_Home(func) == ACTION_MENU_END) {
				devState.actionSubmenu = M_TEST_FN_NONE;
				return ACTION_MENU_END;
			}
			break;

		case M_TEST_FN_VAL_SYR_AUTO:	// valve syringe home check
			break;
 
	}

	return ACTION_MENU_ING;	
}

int machineTest_Mode_VAL_Home(int func)
{
	static int error = NO;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_mtvh != Menu) {
    dfp("subMenu_mtvh=%d\n",subMenu_mtvh);
    Menu = subMenu_mtvh;
}
#endif

	switch(subMenu_mtvh) {
		case 1:
			devState.homeFindEnd[MOTOR_VAL] = NO;

			subMenu_mtvh++;
			break;
		case 2:
			if(devState.homeFindEnd[MOTOR_VAL] == NO) {
				initHomePos_Val();
			}
			else {
				subMenu_mtvh++;
			}
			break;
		case 3:
			subMenu_mtvh = 1;
			return ACTION_MENU_END;			
			break;
	}

  return ACTION_MENU_ING;			
}

int machineTest_Mode_SYR_Home(int func)
{
	static int error = NO;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_mtsh != Menu) {
    dfp("subMenu_mtsh=%d\n",subMenu_mtsh);
    Menu = subMenu_mtsh;
}
#endif

	switch(subMenu_mtsh) {
		case 1:
			devState.homeFindEnd[MOTOR_SYR] = NO;
			subMenu_mtsh++;
			break;
		case 2:
			if(devState.homeFindEnd[MOTOR_SYR] == NO) {
				initHomePos_Syr();
			}
			else {
				subMenu_mtsh++;
			}
			break;
		case 3:
			subMenu_mtsh = 1;
			return ACTION_MENU_END;			
			break;
	}

  return ACTION_MENU_ING;		
}	

//			washport --> down 250ul --> injection port --> up 250ul
int machineTest_Mode_VAL_SYR_1Cycle(int func)
{
	static int error = NO;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_mtvsc != Menu) {
    dfp("subMenu_mtvsc=%d\n",subMenu_mtvsc);
    Menu = subMenu_mtvsc;
}
#endif

	switch(subMenu_mtvsc) {

		case 1:	// 모터 초기화 RESET
			if(sleepChkVal(MOTOR_VAL)) {
				motorReadyCmd(MOTOR_VAL);		
			}
			subMenu_mtvsc++;			
			break;
		case 2:
//dfp("<<<aspirateSample --- goPosVial>>>!! == devState.btMotorState[MOTOR_Z]=%x\n",devState.btMotorState[MOTOR_Z]);	
			if(readyChkVal(MOTOR_VAL)) {
				moveStepSpeed(MOTOR_VAL, 0, ACT_SPEED_MAX_VAL, (UNIT_STEP_VAL * (int)(MICRO_STEP_VAL)));		//	wash port ccw
				subMenu_mtvsc++;
			}
			break;
		case 3:
			if(readyChk(MOTOR_VAL))	{			
//				WAIT_TIME_VALUE_CASE_STATE(subMenu_mtvsc,1000);
				WAIT_TIME_VALUE_CASE_STATE(subMenu_mtvsc,DELAY_TIME_TICK_COUNT_0_1_SEC);
      }
      break;
		case 4:
				moveStepSpeed(MOTOR_SYR, 0, M_TEST_SYRINGE_DOWN_SPEED, (SYR_PULSE_PER_VOL_500_FLOAT * M_TEST_SYRINGE_INJ_VOLUME) );						
				subMenu_mtvsc++;
			break;

		case 5:
			if(sleepChk(MOTOR_SYR)) { motorReadyCmd(MOTOR_SYR);}
			subMenu_mtvsc++;			
			break;

		case 6:
			if(readyChk(MOTOR_SYR))	{			
//				WAIT_TIME_VALUE_CASE_STATE(subMenu_mtvsc,NEXT_CASE_DELAY_RUN);
//				WAIT_TIME_VALUE_CASE_STATE(subMenu_mtvsc,20000);
				WAIT_TIME_VALUE_CASE_STATE(subMenu_mtvsc,DELAY_TIME_TICK_COUNT_2_SEC);
      }
      break;
		case 7:
				moveStepSpeed(MOTOR_VAL, 0, ACT_SPEED_MAX_VAL, -(UNIT_STEP_VAL * (int)(MICRO_STEP_VAL)));		//	inj port	cw		
				subMenu_mtvsc++;				
			break;
		case 8:
			if(readyChk(MOTOR_VAL))	{			
//				WAIT_TIME_VALUE_CASE_STATE(subMenu_mtvsc,1000);
				WAIT_TIME_VALUE_CASE_STATE(subMenu_mtvsc,DELAY_TIME_TICK_COUNT_0_1_SEC);
      }
      break;
		case 9:
				moveStepSpeed(MOTOR_SYR, 0, M_TEST_SYRINGE_UP_SPEED, -(SYR_PULSE_PER_VOL_500_FLOAT * M_TEST_SYRINGE_INJ_VOLUME) );						
				subMenu_mtvsc++;
			break;
		case 10:
			if(readyChk(MOTOR_SYR))	{			
				subMenu_mtvsc = 1;				
				return ACTION_MENU_END;
			}
			break;


	}

  return ACTION_MENU_ING;	
			
}


int machineTest_Mode_BOARD_ALL(int value)
{
	static uint32_t Core_gpioInput_old;
	uint32_t keyin,keyin1;

	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		Core_gpioInput_old = Core_gpioInput;
		
		keyin = (Core_gpioInput >> 16) & 0x003f;
		keyin1 = keyin >> 3;
		dfp("keyin = 0x%08x  0x%08x\n",keyin,keyin1);
	}

	switch(keyin) {
		case TEST_MODE_NONE:		break;
		
		case TEST_MODE_SW1:		// 
			break;		
		case TEST_MODE_SW1_SW2:		//  - sensing
			break;
		case TEST_MODE_SW1_SW3:		//  - step
			break;
		case TEST_MODE_SW1_SW2_SW3:			break;

		case TEST_MODE_SW2:		break;

		case TEST_MODE_SW3:		break;

		case TEST_MODE_SW2_SW3:		break;

		case TEST_MODE_SW4:		// 
			break;
		case TEST_MODE_SW4_SW2:		//  - sensing
			break;
		case TEST_MODE_SW4_SW3:		//  - step
			break;
		case TEST_MODE_SW4_SW2_SW3:		break;
		
		case TEST_MODE_SW5:		// 
			break;
		case TEST_MODE_SW5_SW2:		//  - sensing
			break;
		case TEST_MODE_SW5_SW3:		//  - step
			break;
		case TEST_MODE_SW5_SW2_SW3:		break;
		
		case TEST_MODE_SW6:		// 
			break;
		case TEST_MODE_SW6_SW2:		//  - sensing
			break;
		case TEST_MODE_SW6_SW3:		//  - step
			break;
		case TEST_MODE_SW6_SW2_SW3:		break;		

	}
		dfp("uiMachineTestMode = 0x%08x\n",uiMachineTestMode);
		
}
#endif

#if 0
void machineTest_Mode()
{
	static uint32_t powSwDet = NO;
	static uint32_t powSwDetCnt = 10000;// 맨처음 전원 온 시 꺼지지 않도록...
	static unsigned char Leak = NO;
	static unsigned char Door = NO;

	uint32_t who = MOTOR_Z;

	static uint32_t Core_gpioInput_old;

	uint32_t Mode;
	
	Core_gpioInput = (*((uint32_t volatile *)(COREGPIO_APBBIF_0_IN_REG)));
	
	if(Core_gpioInput != Core_gpioInput_old) {
		dfp("Core_gpioInputn = 0x%08x\n",Core_gpioInput);	
		Core_gpioInput_old = Core_gpioInput;
	}

// MSEN[5:0] == [21:16]
	Mode = Core_gpioInput & 0x00380000; // [21:19] 의 슬라이드 스위치값에 따라 모드 결정됨 
dfp("machineTest_Mode = 0x%08\n",Mode);	

	switch(Mode) {
		case TEST_MODE_X:
			machineTest_Mode_X(Mode);
			break;
		case TEST_MODE_Y:
			machineTest_Mode_Y(Mode);			
			break;
		case TEST_MODE_Z:
			machineTest_Mode_Z(Mode);			
			break;
		case TEST_MODE_SYR:
			machineTest_Mode_SYR(Mode);			
			break;
		case TEST_MODE_VAL:
			machineTest_Mode_VAL(Mode);			
			break;
		case TEST_MODE_INJ:
			machineTest_Mode_INJ(Mode);			
			break;
		case TEST_MODE_VAL_SYR:
			machineTest_Mode_VAL_SYR(Mode);			
			break;
		case TEST_MODE_BOARD_ALL:
			machineTest_Mode_BOARD_ALL(Mode);			
			break;
	}

		
	switch(Core_gpioInput & 0x00380000) {		// [21:19] 의 슬라이드 스위치 검사 
		case 0x00000000:	// 둘다

			break;
		case 0x00100000:
			who = MOTOR_X; // 아래 버튼 - X 축동작 버튼
			if(!(Core_gpioInput & 0x00010000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 40);		
			dfp("=======HOME_DIR_X = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00020000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 40);
			dfp("=========R_HOME_DIR_X = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00040000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 1600);		
			dfp("=======HOME_DIR_X = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00080000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_X | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 1600);		
			dfp("=========R_HOME_DIR_X = 0x%08x\n",Core_gpioInput);						
				}
			}
			else {	// 버튼에서 손떼면 스톱(감속)
				stepCtrlCmd(who , devState.curDir[who] | CMD_STOP);
			}

			break;

		case 0x00200000:	// 위버튼 누르면  - Y 축 동작 버튼
			who = MOTOR_Y;
			if(!(Core_gpioInput & 0x00010000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_Y | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 40);		
			dfp("=======HOME_DIR_Y = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00020000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_Y | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 40);
			dfp("=========R_HOME_DIR_Y = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00040000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_Y | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 1600);		
			dfp("=======HOME_DIR_Y = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00080000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_Y | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 1600);		
			dfp("=========R_HOME_DIR_Y = 0x%08x\n",Core_gpioInput);						
				}
			}
			else {	// 버튼에서 손떼면 스톱(감속)
				stepCtrlCmd(who , devState.curDir[who] | CMD_STOP);
			}
			
			break;
		case 0x00300000:	// 안눌림. -  Z 축 동작
			who = MOTOR_Z;
			if(!(Core_gpioInput & 0x00010000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_Z | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 50);		
			dfp("=======HOME_DIR_Z = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00020000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_Z | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 50);
			dfp("=========R_HOME_DIR_Z = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00040000)) {
				if(readyChk(who)) {
					stepControl(who , HOME_DIR_Z | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 2400);		
			dfp("=======HOME_DIR_Z = 0x%08x\n",Core_gpioInput);						
				}
			}
			else if(!(Core_gpioInput & 0x00080000) ) {
				if(readyChk(who)) {		
					stepControl(who, R_HOME_DIR_Z | CMD_START, 149, FIND_HOME_UNIT_STEP_Z * 2400);		
			dfp("=========R_HOME_DIR_Z = 0x%08x\n",Core_gpioInput);						
				}
			}	
			else {	// 버튼에서 손떼면 스톱(감속)
				stepCtrlCmd(who , devState.curDir[who] | CMD_STOP);
			}
			
			break;
	}

}
#endif

#if BOARD_TEST_PRGM || MACHINE_TEST_PRGM
int send_BoardState()
{
	SEND_LCAS_PACKET(PACKCODE_LCAS_BOARD_STATE, LcasBoardState, LcasBoardState);
	return 0;
}
#endif

#if BOARD_TEST_PRGM
void motorControlWho(uint32_t who)
{
dp("motorControlWho nStopBreak[%d] = [%d]\n",who,MotorCtrlSet.nStopBreak[who]);
	switch(MotorCtrlSet.nStopBreak[who]) {
		case PC_CMD_STOP:		// sensor check stop
			if(MotorCtrlSet.bSensorCheck[who] == YES) {
				devSet.ucChkHomeMethod[who] = CMD_STOP;
				goSensorHome(who, devSet.chkHomeSpeed[who], devSet.chkHomeStepCnt[who]);
			}
			else {	// stop
				stepCtrlCmd(who , 	MotorCtrlSet.bDir[who] | CMD_STOP);
			}
			break;
		case PC_CMD_BREAK:		// sensor check break
			if(MotorCtrlSet.bSensorCheck[who] == YES) {
				devSet.ucChkHomeMethod[who] = CMD_BREAK;
				goSensorHome(who, devSet.chkHomeSpeed[who], devSet.chkHomeStepCnt[who]);
			}
			else {	// break
				stepCtrlCmd(who , 	MotorCtrlSet.bDir[who] | CMD_BREAK);
			}
			break;
		case PC_CMD_SLEEP:		// sleep
				motorSleepCmd(who , MotorCtrlSet.bDir[who]);
			break;
		case PC_CMD_RESET:		// reset
				stepCtrlCmd(who , 	MotorCtrlSet.bDir[who] | CMD_RESET);		
			break;
		case PC_CMD_RUN:		// run - 여기까지 안온다.
			break;
	}
}
#endif

#define PC_CMD_STOP			0
#define PC_CMD_BREAK		1
#define PC_CMD_SLEEP		2
#define PC_CMD_RESET		3
#define PC_CMD_RUN			4

#if 0
void motorControlFn()
{
	uint32_t who;

	for(int i=0 ;i < STEP_MOTOR_COUNT; i++) {
		if(MotorCtrlSet.bCtrlEnable[i] == YES) {
			if(MotorCtrlSet.nStopBreak[i] == PC_CMD_RUN) {
				stepControl(i, MotorCtrlSet.bDir[i] | CMD_START | MotorCtrlSet.bInfinite[i], MotorCtrlSet.nSpeedMax[i] | MotorCtrlSet.nSpeedMin[i] << 8, MotorCtrlSet.nStep[i]);
//				stepControl(i, MotorCtrlSet.bDir[i] | CMD_START | MotorCtrlSet.bInfinite[i], MotorCtrlSet.nSpeedMax[i] | MotorCtrlSet.nSpeedMin[i], MotorCtrlSet.nStep[i]);
			}
			else {
				motorControlWho(i);
			}
		}
	}
}
#else
void motorControlFn()
{
	uint32_t who;
	int ready = YES;

	for(int i=0 ;i < STEP_MOTOR_COUNT; i++) {
		if(sleepChk(i)) {
			motorReadyCmd(i);
			ready = NO;
		}
	}

	if(ready == YES) {
		for(int i=0 ;i < STEP_MOTOR_COUNT; i++) {		
			if(MotorCtrlSet.bCtrlEnable[i] == YES) {			
				stepControl(i, MotorCtrlSet.bDir[i] | CMD_START | MotorCtrlSet.bInfinite[i], MotorCtrlSet.nSpeedMax[i] | MotorCtrlSet.nSpeedMin[i] << 8, MotorCtrlSet.nStep[i]);
			}
		}
	}
}
#endif

int	netBoardCmd(char *pData)
{
	LCAS_BOARD_STATE_t tmp;

	USE_LCAS_PACKET(pData,tmp);
	if(1) {

	}
	else {
		errorFunction(SELF_ERROR_COMMAND_UNKNOWN);
		SEND_LCAS_PACKET(PACKCODE_LCAS_BOARD_CMD, LcasBoardCmd, LcasBoardCmd);
	}
}


#if 0
#define NVM_PAGE_SIZE   128
int nvm_main(void)
{
  const uint8_t pattern_a[] = { 0x41u, 0xABu, 0xCDu, 0xEFu, 0xAAu, 0xBBu };
  const uint8_t pattern_b[] = { 0x42u, 0x12u, 0x34u, 0x56u, 0x78u, 0x99u };

  const uint32_t nvm_addr = 0x60031000u;
  uint8_t * p_nvm = (uint8_t *)nvm_addr;

  /* Initialize the driver with the required SmartFusion device type */
  NVM_init(NVM_A2F200_DEVICE );

	if ( 'B' == *p_nvm )
  {
      NVM_write( nvm_addr, pattern_a, sizeof(pattern_a) );
dp("pattern_a\n");      
/*      
      NVM_write( nvm_addr + NVM_PAGE_SIZE, pattern_c, sizeof(pattern_c) );
      NVM_write( nvm_addr + (2 * NVM_PAGE_SIZE) - (NVM_PAGE_SIZE / 2), pattern_e, sizeof(pattern_e) );
      NVM_write( nvm_addr + (2 * NVM_PAGE_SIZE) + 2, pattern_a, sizeof(pattern_a) );
*/      
  }
  else
  {
      NVM_write( nvm_addr, pattern_b, sizeof(pattern_b) );
dp("pattern_b\n");            
/*      
      NVM_write( nvm_addr + NVM_PAGE_SIZE, pattern_d, sizeof(pattern_d) );
      NVM_write( nvm_addr + (2 * NVM_PAGE_SIZE) - (NVM_PAGE_SIZE / 4), pattern_e, sizeof(pattern_e) );
      NVM_write( nvm_addr + (2 * NVM_PAGE_SIZE) + 4, pattern_a, sizeof(pattern_a) );
*/
  }
}
#endif

#if 0
static uint8_t g_master_tx_buf[32]= "12345678901234567890";
static uint8_t g_master_rx_buf[32];
static uint8_t g_tx_length=15;

/*------------------------------------------------------------------------------
 * Perform read transaction with parameters gathered from the command line
 * interface. This function is called as a result of the user's input in the
 * command line interface.
 */
mss_i2c_status_t do_read_transaction
(
    uint8_t serial_addr,
    uint8_t * rx_buffer,
    uint8_t read_length
)
{
    mss_i2c_status_t status;

    MSS_I2C_read(I2C_INSTANCE, serial_addr, rx_buffer, read_length, MSS_I2C_RELEASE_BUS);

    status = MSS_I2C_wait_complete(I2C_INSTANCE, DEMO_I2C_TIMEOUT);

    return status;
}

/*------------------------------------------------------------------------------
 * Perform write transaction with parameters gathered from the command line
 * interface. This function is called as a result of the user's input in the
 * command line interface.
 */
mss_i2c_status_t do_write_transaction
(
    uint8_t serial_addr,
    uint8_t * tx_buffer,
    uint8_t write_length
)
{
    mss_i2c_status_t status;

//while(1) 
{
    MSS_I2C_write(I2C_INSTANCE, serial_addr, tx_buffer, write_length, MSS_I2C_RELEASE_BUS);

//    MSS_I2C_write(I2C_INSTANCE_0, serial_addr, tx_buffer, write_length, MSS_I2C_RELEASE_BUS);

//   delay_btw_transfers (10000000);
}
    status = MSS_I2C_wait_complete(I2C_INSTANCE, DEMO_I2C_TIMEOUT);
dp("status = %d \n",status);
    return status;
}

mss_i2c_status_t do_write_transaction0
(
    uint8_t serial_addr,
    uint8_t * tx_buffer,
    uint8_t write_length
)
{
    mss_i2c_status_t status;

    MSS_I2C_write(I2C_INSTANCE_0, serial_addr, tx_buffer, write_length, MSS_I2C_RELEASE_BUS);

    status = MSS_I2C_wait_complete(I2C_INSTANCE_0, DEMO_I2C_TIMEOUT);
    
    return status;
}

void test_eeprom()
{
	uint16_t size = 10;
  uint16_t start_address=0;
  uint8_t write_data[10] = {10,9,8,7,6,5,4,3,2,1};
  uint8_t read_data[10];

   mss_i2c_status_t instance;

//  EEPROM_read(start_address,size,read_data);
  for(int i= 0; i<10;i++) {
  	dp("%d\n",read_data[i]);
  }
 	dp("read end \n");
   /* Wrting data to EEPROM */
//  EEPROM_write(start_address,size,write_data);
instance = do_write_transaction(EEPROM_SLAVE_ADDRESS, g_master_tx_buf, g_tx_length);
//do_write_transaction0(EEPROM_SLAVE_ADDRESS, g_master_tx_buf, g_tx_length);

//        delay_btw_transfers (10000000);
  
  //MSS_UART_polled_tx( &g_mss_uart0,write_success_text , sizeof(write_success_text));
  /* reading data from EEPROM */
//  EEPROM_read(start_address,size,read_data);

  for(int i= 0; i<10;i++) {
  	dp("%d\n",read_data[i]);
  }

instance = do_read_transaction(EEPROM_SLAVE_ADDRESS, g_master_rx_buf, sizeof(g_master_rx_buf));  
  for(int i= 0; i<32;i++) {
  	dp("%d\n",g_master_rx_buf[i]);
  }
  
//  MSS_UART_polled_tx( &g_mss_uart0,read_text , sizeof(read_text));
//  MSS_UART_polled_tx( &g_mss_uart0,read_data , size);
//  *rx_buff = NULL;
//  size = 0;


}


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
#endif

