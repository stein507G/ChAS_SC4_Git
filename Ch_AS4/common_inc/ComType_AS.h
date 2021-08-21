
#ifndef __COM_TYPE_AS_H__
#define __COM_TYPE_AS_H__

//#pragma pack(push,1)

#include "Def_stepCon.h"

// =================================================================
// PC���� �����뵵�� ����Ѵ�.
// ��⿡���� 
// =================================================================
typedef struct _MOTOR_CTRL_t
{
	int bCtrlEnable[STEP_MOTOR_COUNT];
	int bDir[STEP_MOTOR_COUNT];			// 0: CW		1:CCW
	int bInfinite[STEP_MOTOR_COUNT];		// 0:			1: infinite
	unsigned int nStep[STEP_MOTOR_COUNT];
	unsigned int nSpeedMin[STEP_MOTOR_COUNT];	// �ӵ�Address
	unsigned int nSpeedMax[STEP_MOTOR_COUNT];	// �ӵ�Address
	int bUnit[STEP_MOTOR_COUNT];			// 0: pulse		1: rev
	int bSensorCheck[STEP_MOTOR_COUNT];	// 0:			1: check
	int nStopBreak[STEP_MOTOR_COUNT];		// 0: stop		1: break	2: sleep	3: reset

	int bReset;
	int bStop;
	int bBreak;
	int bRun;

} MOTOR_CTRL_t;

typedef struct _LCAS_BOARD_STATE_t
{
// << GPIO IN >> -- Core_gpioInput
//	gpio-in 0: rotory sw 0 (switch1)
//	gpio-in 1: rotory sw 1 (switch2)
//	gpio-in 2: door err
//	gpio-in 3: startIn(interrupt)
//	gpio-in 4: readyIn
//	gpio-in 5: leak Sensor
//	gpio-in 6: rotory sw 2 (Cpu Onoff det)
//	gpio-in 7 ~ 15 : gnd
//	gpio-in 16: Magnetic sen 0
//	gpio-in 17: Magnetic sen 1
//	gpio-in 18: Magnetic sen 2
//	gpio-in 19: Magnetic sen 3
//	gpio-in 20: Magnetic sen 4
//	gpio-in 21: Magnetic sen 5
	unsigned int gpioInput;
// << GPIO OUT >> -- Core_gpio_pattern
//	gpio-out 8: LED S1
//	gpio-out 9: LED S2
//	gpio-out 10: LED S3
//	gpio-out 11: Ready out
//	gpio-out 12: start out
//	gpio-out 13: cpu power control
//	gpio-out 14: buzzer control
//	gpio-out 15: gpio out15
	unsigned int ledState;
	unsigned int onOffState;	// pump , cooling , 

// x , y , z , syr ,val , inj	
	unsigned char btMotorState[6]; 	

	float fTemp;				// tray�� ���� �µ� 
	unsigned char tempReady;	// �µ� ���� ���� - ����µ��� ���������� ���� 0.1�� �̳��� 1���̻� ���� 
	unsigned char errorCode;  // ���� ���� ���� - SelfMsg���� 

//	time
	DATE_t sysDate;
	TIME_t sysTime;
} LCAS_BOARD_STATE_t;

typedef struct _LCAS_BOARD_CONTROL_t
{
// << GPIO IN >> -- Core_gpioInput
//	gpio-in 0: rotory sw 0 (switch1)
//	gpio-in 1: rotory sw 1 (switch2)
//	gpio-in 2: door err
//	gpio-in 3: startIn(interrupt)
//	gpio-in 4: readyIn
//	gpio-in 5: leak Sensor
//	gpio-in 6: rotory sw 2 (Cpu Onoff det)
//	gpio-in 7 ~ 15 : gnd
//	gpio-in 16: Magnetic sen 0
//	gpio-in 17: Magnetic sen 1
//	gpio-in 18: Magnetic sen 2
//	gpio-in 19: Magnetic sen 3
//	gpio-in 20: Magnetic sen 4
//	gpio-in 21: Magnetic sen 5
// << GPIO OUT >> -- Core_gpio_pattern
//	gpio-out 8: LED S1
//	gpio-out 9: LED S2
//	gpio-out 10: LED S3
//	gpio-out 11: Ready out
//	gpio-out 12: start out
//	gpio-out 13: cpu power control
//	gpio-out 14: buzzer control
//	gpio-out 15: gpio out15
//	unsigned char gpioInput;
	unsigned char ledOnoff;
	unsigned char onOffCmd;	// pump , cooling , fan 
//	unsigned char btMotorState[6]; 	

	float fTempSet;				// tray�� ���� �µ� 
//	unsigned char tempReady;	// �µ� ���� ���� - ����µ��� ���������� ���� 0.1�� �̳��� 1���̻� ���� 
//	unsigned char errorCode;  // ���� ���� ���� - SelfMsg���� 
	
//	time
	DATE_t sysDate;
	TIME_t sysTime;
} LCAS_BOARD_CONTROL_t;

typedef struct _XY_COORD_t
{
	int etcStep_X[5];						// (0:injector/1:wash/2:waste/3:etc)�� ��ǥ�� Step��(Pulse)�� ǥ���Ѱ� 
	int etcStep_Y[5];						// 

	int vialStep_L_X[WELLPALTE_96_COUNT_X];						// ���̾� ��ǥ�� Step��(Pulse)�� ǥ���Ѱ� 
	int vialStep_R_X[TRAY60_COUNT_Y];						// ���̾� ��ǥ�� Step��(Pulse)�� ǥ���Ѱ� 	
	int vialStep_L_Y[WELLPALTE_96_COUNT_X];	
	int vialStep_R_Y[TRAY60_COUNT_Y];	

//	int step_Z[17];			/*	#define POS_Z_TOP						0		// ���� - home - (�ɸ��� �κ��� ���� �����Ӱ� �̵�������.)
	int step_Z[POS_Z_COUNT_MAX];			/*	#define POS_Z_TOP						0		// ���� - home - (�ɸ��� �κ��� ���� �����Ӱ� �̵�������.)
													#define POS_Z_NEEDLE_EXCHANGE	15		// �ϵ鱳ü���� ��ġ	
													#define POS_Z_SLEEP						16		// Sleep mode�� ������ ��ġ*/

	float step_1ul_SYR;		// step / 1ul 

	int valStep[5];
	
	int injStep[5];
/*
#define	POS_VAL_WASHING			0
#define	POS_VAL_NEEDLE			1
#define UNIT_STEP_VAL		267

#define	POS_INJ_LOAD			0
#define	POS_INJ_INJECT		1
#define UNIT_STEP_INJ		267
*/
} XY_COORD_t;

// =================================================================================
// Auto Sampler ==> State
// =================================================================================
#define ACTION_MENU_ING		0
#define ACTION_MENU_END		1		// ���� �޴� �Ϸ�
#define ACTION_MENU_ERR_VIAL	2		// vial ���� - Sample���� 
#define ACTION_MENU_ERR_VIAL_END_MIX	3		// vial ���� - Reagents�� Destination���� ���̻� �м����� �ȵ�.

#define ACTION_MENU_ERR_Z			4		//  XY�̵��� �� ���� - Z�� ����

#define	AS_ACTION_NONE									(0)
#define	AS_ACTION_INITILIZE							(1)
	#define	AS_INIT_SUB_NONE								(0)
	#define AS_INIT_SUB_FIND_HOME_ALL					(1)

	#define AS_INIT_SUB_CHECK_VIAL_PLATE				(2)

/*
		#define	AS_INIT_SUB_FIND_HOME_VAL_INJ_Z	(1)	
		#define	AS_INIT_SUB_FIND_HOME_X				(2)
		#define	AS_INIT_SUB_FIND_HOME_Y					(3)	
		#define	AS_INIT_SUB_GO_WASTE_PORT				(4)		
		#define AS_INIT_SUB_FIND_HOME_SYR				(5)
		#define AS_INIT_SUB_GO_HOME					(6)
*/	
	#define	AS_INIT_SUB_WASH_SYRINGE				(7)
	#define	AS_INIT_SUB_WASH_NEEDLE				(8)
	#define AS_INIT_SUB_WASH_INJECTOR			(9)
	#define AS_INIT_SUB_WASH_END_GO_HOME	(10)	
	#define AS_INIT_SUB_SLEEP	(11)	
	
/*	
//	#define AS_INIT_SUB_WASH_INJECTOR			(8)
	
	#define AS_INIT_SUB_GO_Z				(4)	//	(9)
	#define AS_INIT_SUB_GO_XY_WASTE			(10)	
	#define AS_INIT_SUB_GO_Z_WASTE			(11)	
	#define AS_INIT_SUB_GO_Z_READY			(12)	

	#define AS_INIT_SUB_GO_Z_HOME			(13)	
	#define AS_INIT_SUB_GO_XY_HOME			(14)		
	#define AS_INIT_SUB_XY_READY			(15)
*/
#define AS_ACTION_READY								(2)
	#define	AS_READY_SUB_NONE							(0)
	#define	AS_READY_SUB_REMOTE_OUT				(1)

#define AS_ACTION_MIX									(3)
	#define AS_MIX_SUB_NONE									(0)
	#define AS_MIX_SUB_WAKE_UP							(1)
	#define AS_MIX_SUB_PRGM									(2)
	#define AS_MIX_SUB_SLEEP								(3)
	#define AS_MIX_SUB_SET_NEXT_VIAL				(4)	


	#define	AS_MIX_SUB_WASH_BEFORE_CHECK		(3)
	#define	AS_MIX_SUB_NEEDLE_WASH_BEFORE		(4)
	#define	AS_MIX_SUB_INJECTOR_WASH_BEFORE		(5)


	
#define AS_ACTION_RUN									(4)
	#define	AS_RUN_SUB_NONE									(0)
	#define AS_RUN_SUB_WAKE_UP							(1)
	
//#define AS_RUN_SUB_MIX_PRGM							(2)
	
	#define	AS_RUN_SUB_WASH_BETWEEN_CHECK_BEFORE		(2)		// 
	#define	AS_RUN_SUB_NEEDLE_WASH_BEFORE		(3)
	#define	AS_RUN_SUB_INJECTOR_WASH_BEFORE		(4)
//	#define	AS_RUN_SUB_GO_VIAL							(5)
	#define	AS_RUN_SUB_GO_VIAL_AND_INJECTION			(5)	

#define	AS_RUN_SUB_SET_NEXT_VIAL		(9)	
	#define	AS_RUN_SUB_WASH_BETWEEN_CHECK_AFTER		(6)
	#define	AS_RUN_SUB_NEEDLE_WASH_AFTER		(7)
	#define	AS_RUN_SUB_INJECTOR_WASH_AFTER		(8)

#define	AS_RUN_SUB_SET_NEXT_VIAL		(9)
	
	#define AS_RUN_SUB_GO_HOME_WAIT_RUN_TIME	(10)
	#define AS_RUN_SUB_SLEEP								(11)
	#define AS_RUN_SUB_REPEAT_INJECTION			(12)
	#define AS_RUN_SUB_END_SEQUENCE				(13)	
	
#define AS_RUN_SUB_WASH_END_SEQUENCE				(13)	


#define AS_ACTION_FAULT								(5)
#define AS_ACTION_STANDBY							(6)
//#define AS_ACTION_PAUSE								(5)
#define AS_ACTION_DIAGNOSTICS					(7)
	#define	AS_DIAG_SUB_NONE							(0)


//#define AS_ACTION_CALIBRATION					(7)
#define AS_ACTION_ADJUST								(8)
	#define AS_ADJUST_HOME_POS										(1)	// all motors go home position
	#define AS_ADJUST_MOTOR_INIT									(2) // select motor adjust point
	#define AS_ADJUST_MOTOR_SET										(3)
	#define AS_ADJUST_MODE_END										(5)
//	#define AS_ADJUST_NDL_SYR_EXCHANGE						(4)	//	
//	#define AS_ADJUST_SYRINGE_EXCHANGE						(6)
	#define AS_ADJUST_NEEDLE_REPLACE						(4)	//	
	#define AS_ADJUST_SYRINGE_REPLACE						(6)

	#define AS_ADJUST_MODE_SLEEP								(7)	


#define AS_ACTION_SERVICE_WASH								(9)
	#define AS_SERVICE_WASH_WAKEUP				(1)
	#define AS_SERVICE_WASH_NEEDLE_INT		(2)
//	#define AS_SERVICE_WASH_NEEDLE_EXT	(3)
	#define AS_SERVICE_WASH_NEEDLE_ALL		(4)
	#define AS_SERVICE_WASH_INJECTOR			(5)
//	#define AS_SERVICE_WASH_WASTE				(6)
	#define AS_SERVICE_WASH_END						(7)
	#define AS_SERVICE_WASH_SLEEP					(8)

// COMMAND_STOP  ��� ó�� 
#define AS_ACTION_STOP_FUNCTION								(10)

#if BOARD_TEST_PRGM
#define BOARD_TEST_FUNCTION								(11)
#define MACHINE_TEST_FUNCTION								(11)

#define AS_ACTION_FUNC_MAX			BOARD_TEST_FUNCTION
#define AS_STATE_CODE_MAX			BOARD_TEST_FUNCTION

#elif MACHINE_TEST_PRGM
#define BOARD_TEST_FUNCTION						(11)
#define MACHINE_TEST_FUNCTION					(11)
//========= machineTest_Mode_X_Y_Z()
	#define M_TEST_XYZ_NONE										(0)
	#define M_TEST_XYZ_X_LEFT_LONG						(1)
	#define M_TEST_XYZ_X_RIGHT_LONG						(2)	
	#define M_TEST_XYZ_X_LEFT_SHORT						(3)		
	#define M_TEST_XYZ_X_RIGHT_SHORT						(4)		

	#define M_TEST_XYZ_Y_FTONT_LONG						(5)	
	#define M_TEST_XYZ_Y_BACK_LONG						(6)		
	#define M_TEST_XYZ_Y_FTONT_SHORT						(7)	
	#define M_TEST_XYZ_Y_BACK_SHORT						(8)		

	#define M_TEST_XYZ_Z_UP_LONG						(9)		
	#define M_TEST_XYZ_Z_DOWN_LONG						(10)		
	#define M_TEST_XYZ_Z_UP_SHORT						(11)		
	#define M_TEST_XYZ_Z_DOWN_SHORT						(12)		

	#define M_TEST_XYZ_1CYCLE									(13)
//============================================================== machineTest_Mode_VAL_SYR()		
//========= machineTest_Mode_VAL_SYR()
	#define M_TEST_FN_NONE										(0)
	#define M_TEST_FN_VAL_CW_90									(1)
	#define M_TEST_FN_VAL_CW_STEP								(2)	
	#define M_TEST_FN_VAL_CCW_90										(3)
	#define M_TEST_FN_VAL_CCW_STEP									(4)	

	#define M_TEST_FN_SYR_UP_FULL									(5)
	#define M_TEST_FN_SYR_UP_STEP									(6)	
	#define M_TEST_FN_SYR_DOWN_FULL								(7)
	#define M_TEST_FN_SYR_DOWN_STEP								(8)	

	#define M_TEST_FN_VAL_SYR_1CYCLE							(9)
	
	#define M_TEST_FN_VAL_HOME										(10)		
	#define M_TEST_FN_SYR_HOME										(11)			
	
	#define M_TEST_FN_VAL_SYR_HOME								(12)		
	
	#define M_TEST_FN_VAL_SYR_AUTO								(13)			
//============================================================== machineTest_Mode_VAL_SYR()		
#define AS_ACTION_FUNC_MAX			MACHINE_TEST_FUNCTION
#define AS_STATE_CODE_MAX				MACHINE_TEST_FUNCTION

#else
#define AS_ACTION_FUNC_MAX			AS_ACTION_STOP_FUNCTION
#define AS_STATE_CODE_MAX				AS_ACTION_STOP_FUNCTION
#endif


#define TEMP_NOT_RESET		0
#define TEMP_NEW_VALUE		1
#define TEMP_ISO_MODE			1
#define TEMP_PROG_MODE		2
// 20191206
//#define TEMP_CTRL_START		3			// ������ ��ó�� 

//#if NEW_BOARD_ENCODER_2020
typedef struct _MOTOR_ACT_SET_t
{
	int32_t pos;
	uint32_t Tray;
	uint32_t option;
	uint32_t act;		// �� ����Ƚ�� 
} MOTOR_ACT_SET_t;
//#endif

typedef struct _LCAS_DEV_STATE_t
{
	unsigned char stateOld;

//	unsigned char btErrorCode;

	unsigned char connected;
	unsigned char disconnected;

	unsigned char actionMenu;
	unsigned char actionSubmenu;
	unsigned char actionMenuOld;
	unsigned char actionSubmenuOld;
	
	unsigned char bSleepMode;		// ���� ��������δ�?
	unsigned char bCheckHome;		// ���� ����������� ���������� ã�ҳ�?

#if FPGA_VERSION_TRIM	
	unsigned int btMotorState[STEP_MOTOR_COUNT];			// ���ͻ���
	MOTOR_ACT_SET_t mActSet[STEP_MOTOR_COUNT];			// ���͵��� �������� - (Encoder)���н� ���
#else
	unsigned char btMotorState[STEP_MOTOR_COUNT];			// ���ͻ���
#endif	
  /*parameter S0_READY = 4'h0;
  parameter S2_ACCEL_1 = 4'h1;
  parameter S2_ACCEL_2 = 4'h2;
  parameter S2_ACCEL_3 = 4'h3;
  parameter S2_ACCEL_4 = 4'h4;
  parameter S3_CONST = 4'h6;
  parameter S4_DEACC_1 = 4'h7;
  parameter S4_DEACC_2 = 4'h8;
  parameter S4_DEACC_3 = 4'h9;
  parameter S4_DEACC_4 = 4'ha;
  parameter S5_BREAK = 4'hc;
  parameter S6_SLEEP = 4'hd;
  parameter S7_RESET = 4'he;
  parameter S8_INFINITE = 4'hf;
	*/

//	unsigned char serviceWashCnt;			// 
//	unsigned char curSvcWashCnt;			// ��� �����ߴ°�?

//	unsigned char curNdlWashBeforeCnt;			// ��� �����ߴ°�?
//	unsigned char curInjWashBeforeCnt;			// ��� �����ߴ°�?
//	unsigned char curNdlWashAfterCnt;			// ��� �����ߴ°�?
//	unsigned char curInjWashAfterCnt;			// ��� �����ߴ°�?

	unsigned int serviceWashCnt;			// 
	unsigned int curSvcWashCnt;			// ��� �����ߴ°�?

	unsigned int curNdlWashBeforeCnt;			// ��� �����ߴ°�?
	unsigned int curInjWashBeforeCnt;			// ��� �����ߴ°�?
	unsigned int curNdlWashAfterCnt;			// ��� �����ߴ°�?
	unsigned int curInjWashAfterCnt;			// ��� �����ߴ°�?

	float curStep[STEP_MOTOR_COUNT];	// ������ġ (���� - pulse) 	
//int actStep[STEP_MOTOR_COUNT];
//unsigned short curPos[STEP_MOTOR_COUNT];
//unsigned short actPos[STEP_MOTOR_COUNT];
	unsigned char curTray;								// 0:left	1:Right 2:�ٸ���ġ(injector/wash/waste) - Tray	
//unsigned short actTray;								// 0:left	1:Right 2:�ٸ���ġ(injector/wash/waste) - Tray

	VIAL_POS_t sample;			// �м��ؾ��� sample (devState.actionMenu == AS_ACTION_RUN , ���� ������(devState.actionMenu == AS_ACTION_RUN) 
	VIAL_POS_t curSample;		// �м����� sample
	VIAL_POS_t lastSample;	// �м��ؾ��� ������ vial

	VIAL_POS_t injectVial;	// �м��ؾ��� vial - Mix ���� ������ : �����Ϳ� ���ԵǾ�� �ϴ� ���̾� 

	unsigned char mixPrgmFinish; 	// mix ���α׷��� ����Ǿ��°�? - �� �������� �ѹ� �����Ѵ�.
	unsigned char mixTableNo;	// ���� ���̺� ��ȣ
//	unsigned char mixWait;	// ���� wait �������ΰ�? - �����ð��̻�����ϸ� sleep���·� ����ų� sleep���¿��� ����� ������ �߰��ϱ� ����.


	VIAL_POS_t firstDestVial;
	
	VIAL_POS_t reagentVialA;
	VIAL_POS_t reagentVialB;
	VIAL_POS_t reagentVialC;
	VIAL_POS_t reagentVialD;
	
	unsigned char injectionTimes;		// ����Ƚ��(1-100)
	unsigned char curInjCnt;				// ���� ���� ���� Ƚ�� - ������ ���� ���������� Ƚ�� , ��ô�� ���� ����.
//  float waitTime;				// ���ð�

//	unsigned int injectionVolumeMax;

	unsigned char seqAction;	// ������ ���� ���� �ֳ�? - ������ ������ ������ Ȱ��ȭ 

  unsigned char isNextInjection;	// ���� ������ ���ҳ�? ��������� ������ ���� ������ �����ִ� ��쵵 YES
  unsigned char doNextSample;	// ���� ������ ������. ���� ���÷� �Ѿ��?

  

  unsigned char vialExist;	// ���� ���� ���̾��� �ִ°�?

  unsigned char missVialResponse;	// MIssing Vial ó���� ���� ���� - ����⿡�� self Message ����(SELF_USER_RESPONSE)

//  unsigned char missVialSkip;	// MIssing Vial ó�� - ���� ������ Skip�ؾ���.

	unsigned char curDir[STEP_MOTOR_COUNT];				
//int actDir[STEP_MOTOR_COUNT];				// �̵��ؾ� �� ����
	unsigned char bInfinite[STEP_MOTOR_COUNT];		// 0x0:	set step		0x40: infinite

//	int bMoveOk[STEP_MOTOR_COUNT];			// ������ �Ϸ�Ǿ���?

	unsigned char homeFindEnd[STEP_MOTOR_COUNT];	// Ȩã�� �Ϸ� - initialize �� �� ��� YES NO

// ����⿡�� ���۵� command	// PACKCODE_LCAS_COMMAND	-- run / stop / start / pause /abort
// ���� ������ ��� �� ������?
	unsigned char command;	
	unsigned char initCommand;	// ���� �ʱ�ȭ ����� ���� �����ΰ� 
	
	unsigned char	setTempReset;

	float tempReadyUnder;	// tempReady ���Ѱ�
	float tempReadyOver;	// tempReady ���Ѱ�

	float oldTemp;
	unsigned char	oldTempOnoff;
	
	unsigned char curAuxEventNo;	// ���� �������� 
	unsigned char curAuxState;		// ���� ���� 0:OFF, 1:ON
	unsigned char curAuxOutIng;		// ������������� : Aux ����� �޽�����϶� yes �� �ݴ� ��� �ؾ���.	
	unsigned char auxOutSigTime;	// �ñ׳� ��ݽð� 
	unsigned char auxOutSigTimeCnt; // �ñ׳� ��ݽð� üũ	- 100ms

	unsigned int auxTime100msCnt[AUX_EVENT_COUNT];	// Aux ��½ð� - 100ms
	
	unsigned int analysisTime100msCnt;		// �м��ð� ī��Ʈ 
	unsigned int analysisTime100ms;				// �м��ð� - ������ 

	unsigned char sysinfoSave;						// system inform data ���忩��

//	LCAS_USED_TIME_t uTime;
	unsigned char usedTimeSave;					//  ���� ready���°� �ƴ϶� ������ ������ ��� = NO;

//	unsigned char vialPlateCheck;

	unsigned char diagState;			// Diag State
	unsigned char diagMenu;	// pDiag_Fn[]	

	unsigned int diagTimeCount;	// Heating �� Cooling Time

	unsigned char LeakError;	// low = Error (leak)
	unsigned char LeakErrorSend;	// ������ �ٽ� üũ�� ���ΰ�?	
	unsigned char DoorError;	// high = Error (Not Detect) open
	unsigned char DoorErrorSend;	// ������ �ٽ� üũ�� ���ΰ�?

	// RYU_SF2
//	unsigned char thError1;
//	unsigned char thError2;
//	unsigned char sensorError;
	
	unsigned char adReadValid;	// ADC�� ���� �� ?
	unsigned char powerCheck;		// �����˻� �Ϸ� ?
	unsigned char powerState;		// ��������-������ �ִ°�?

	unsigned char powerOffLed; 	// 

	unsigned char save_EEPROM_sysConfig; 	// ����� ���� eeprom�� �����ؾ��Ѵ�.
	unsigned char save_EEPROM_special;
	
} LCAS_DEV_STATE_t;


// unsigned char adjustAct;			// �������
#define ADJUST_ACT_NOT_READY		0		// Adjust Mode�� �ƴϰų� �غ� �ȵ�.
#define ADJUST_ACT_INIT					1		// �ʱ�ȭ(Ȩ������ ã��) ������
#define ADJUST_ACT_READY				2		// �ʱ�ȭ(Ȩ������ ã��)�Ϸ� 
#define ADJUST_ACT_ADJUST				3		// ���� ������ - ���� �ȵ� ���� 
#define ADJUST_ACT_EXCHANGE			4		// needle , syringe ��ü��ġ�� �̵����̰ų� �̵��Ϸ�

typedef struct _LCAS_DEV_SET_t
{
	unsigned char adjustDate;		// ��¥
	unsigned char adjustState;	// 0: ������ �� ���� NO , 1: �Ϸ� -  YES
//	unsigned char ing;		// YES NO
	unsigned char adjustOldMotor;
	unsigned char adjustCurMotor;	// ���� �������� ���� , adjust���� �ƴ� ��� -1
	unsigned char adjustAct;			// �������
	
	ADJUST_DEV_DATA_t adjust;			// data	
	ADJUST_DEV_DATA_t adjustOld;	// data	
	
#if EEPROM_ADJUST_DATA_SAVE_FIX	
	ADJUST_DEV_DATA_t adjust_2nd;	// saveFlag�� checksum���� �̿�	
	ADJUST_DEV_DATA_t adjustTmp;	// data		
	int adjust_ver;	//	
//	char firmware_ver[10];	//
#endif
	unsigned char trayLeft_rowNo;
	unsigned char trayLeft_colNo;
	unsigned char trayRight_rowNo;
	unsigned char trayRight_colNo;

	unsigned char ucChkHome[STEP_MOTOR_COUNT];	// YES NO
	unsigned char ucChkHome_R[STEP_MOTOR_COUNT];	// YES NO 

	unsigned char homeDir[STEP_MOTOR_COUNT];
	
	unsigned char chkHomeSpeed[STEP_MOTOR_COUNT]; //
	unsigned char chkHomeSpeedMin[STEP_MOTOR_COUNT]; //
	
//	unsigned char findHomeUnitStep[STEP_MOTOR_COUNT]; // Ȩã�� �� 
	unsigned int chkHomeStepCnt[STEP_MOTOR_COUNT]; //

unsigned int minSpeed[STEP_MOTOR_COUNT]; //
unsigned int maxSpeed[STEP_MOTOR_COUNT]; //

	unsigned char ucChkHomeMethod[STEP_MOTOR_COUNT];	// CMD_STOP , CMD_BREAK

	unsigned int baseAddr[STEP_MOTOR_COUNT];
#if	FPGA_VERSION_TRIM
	unsigned int accelStepAddr[STEP_MOTOR_COUNT];
	unsigned int deaccStepAddr[STEP_MOTOR_COUNT];
	unsigned int totalStepAddr[STEP_MOTOR_COUNT];
	unsigned int startSpeedAddr[STEP_MOTOR_COUNT];
	
	unsigned int ctrlPortAddr[STEP_MOTOR_COUNT];

	unsigned int selMotorAddr[STEP_MOTOR_COUNT];
	unsigned int encoderCtrlAddr[STEP_MOTOR_COUNT];

	unsigned int selStateAddr[STEP_MOTOR_COUNT];
	
//	unsigned int selMotor[STEP_MOTOR_COUNT];
#endif

//	unsigned char microStep[STEP_MOTOR_COUNT];

	unsigned int injectionVolumeMax;

unsigned int washVolumeMax;
	
	unsigned char syrSpeedInj[10];
	unsigned char syrSpeedWash[10];
//	int ndlDepth;
//	int ndlHeight;		// Z�� Home���� 8800 : vial top (Cap include) - 1mm�� 
	// Z�� vial top (Cap include) - 1mm������ - 15 x 1600 : Vial Bottom

	unsigned int syrExchangeHeight;
// vial Height 
// === 2ml
// Cap X : 32mm
// Depth : 31mm
// Cap include 33.3mm
// Depth(Cap include) : �뷫 32mm
// === 4ml
// Cap X : 45mm
// Depth : 43.8mm �̻�
// Cap include 47.mm ����
// Depth(Cap include) : �뷫 45mm

	float left_z_bottom;		// ����
	float right_z_bottom;		// �ܺ�

	unsigned char posCheckEnable;
	unsigned char posCheckMotor;
	unsigned char posCheck1;
	unsigned char posCheck2;

	unsigned char IP_switch;
	unsigned char msen_switch;

	unsigned int microPumpTimeCount;
	unsigned int microPumpVolt;
	unsigned int microPumpPwmCount;

} LCAS_DEV_SET_t;







union utData {
	uint32_t	ulData;
	uint8_t		ucData[4];
	uint16_t	uiData[2];
};


typedef struct _Step_Accel_t
{
	float			ar1;	// 1,2,3, ... ,75,74, ... , 3,2,1,0

	uint16_t s_interval;	// *** step gap  ***

//	uint16_t s_pulse_repeat;		// *** repeat ***

	uint16_t accumul_cnt;		// repeat ����

} Step_Accel_t;

























// �Ʒ��� ������� ���ñ�
// �������� 
typedef struct _LCAS_CONTROL_t
{
	unsigned char btCommand;

	unsigned char btWashingCount;
	int bWashingStartStop;

	unsigned char btWashSyringeCount;
	unsigned char btWashExternalCount;
	unsigned char btInitWashingCount;

	unsigned int uiSampleLoopVol;
	unsigned int uiSyringeVol;

	unsigned char btInjectionSpeed;

	unsigned int uiInjectionVol;
	unsigned char btInjectionCount;
	float fAnalysisTime;

	unsigned char btVialNo;
	unsigned char btVialNoEnd;
	unsigned char btTrayNo;

	int bRunStop;
} LCAS_CONTROL_t;

//#pragma pack(pop)
#endif	//  __COM_TYPE_AS_H__
