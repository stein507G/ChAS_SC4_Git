
#ifndef __COM_TYPE_AS_H__
#define __COM_TYPE_AS_H__

//#pragma pack(push,1)

#include "Def_stepCon.h"

// =================================================================
// PC에서 저에용도로 사용한다.
// 기기에서는 
// =================================================================
typedef struct _MOTOR_CTRL_t
{
	int bCtrlEnable[STEP_MOTOR_COUNT];
	int bDir[STEP_MOTOR_COUNT];			// 0: CW		1:CCW
	int bInfinite[STEP_MOTOR_COUNT];		// 0:			1: infinite
	unsigned int nStep[STEP_MOTOR_COUNT];
	unsigned int nSpeedMin[STEP_MOTOR_COUNT];	// 속도Address
	unsigned int nSpeedMax[STEP_MOTOR_COUNT];	// 속도Address
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

	float fTemp;				// tray의 현재 온도 
	unsigned char tempReady;	// 온도 안정 여부 - 현재온도와 설정값과의 차가 0.1도 이내로 1분이상 유지 
	unsigned char errorCode;  // 현재 에러 정보 - SelfMsg참조 

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

	float fTempSet;				// tray의 현재 온도 
//	unsigned char tempReady;	// 온도 안정 여부 - 현재온도와 설정값과의 차가 0.1도 이내로 1분이상 유지 
//	unsigned char errorCode;  // 현재 에러 정보 - SelfMsg참조 
	
//	time
	DATE_t sysDate;
	TIME_t sysTime;
} LCAS_BOARD_CONTROL_t;

typedef struct _XY_COORD_t
{
	int etcStep_X[5];						// (0:injector/1:wash/2:waste/3:etc)의 좌표를 Step수(Pulse)로 표시한것 
	int etcStep_Y[5];						// 

	int vialStep_L_X[WELLPALTE_96_COUNT_X];						// 바이알 좌표를 Step수(Pulse)로 표시한것 
	int vialStep_R_X[TRAY60_COUNT_Y];						// 바이알 좌표를 Step수(Pulse)로 표시한것 	
	int vialStep_L_Y[WELLPALTE_96_COUNT_X];	
	int vialStep_R_Y[TRAY60_COUNT_Y];	

//	int step_Z[17];			/*	#define POS_Z_TOP						0		// 맨위 - home - (걸리는 부분이 없어 자유롭게 이동가능함.)
	int step_Z[POS_Z_COUNT_MAX];			/*	#define POS_Z_TOP						0		// 맨위 - home - (걸리는 부분이 없어 자유롭게 이동가능함.)
													#define POS_Z_NEEDLE_EXCHANGE	15		// 니들교체가능 위치	
													#define POS_Z_SLEEP						16		// Sleep mode로 들어가기전 위치*/

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
#define ACTION_MENU_END		1		// 현재 메뉴 완료
#define ACTION_MENU_ERR_VIAL	2		// vial 없음 - Sample없음 
#define ACTION_MENU_ERR_VIAL_END_MIX	3		// vial 없음 - Reagents나 Destination없음 더이상 분석진행 안됨.

#define ACTION_MENU_ERR_Z			4		//  XY이동할 수 없음 - Z축 낮음

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

// COMMAND_STOP  명령 처리 
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
//#define TEMP_CTRL_START		3			// 부팅후 맨처음 

//#if NEW_BOARD_ENCODER_2020
typedef struct _MOTOR_ACT_SET_t
{
	int32_t pos;
	uint32_t Tray;
	uint32_t option;
	uint32_t act;		// 재 동작횟수 
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
	
	unsigned char bSleepMode;		// 현재 슬립모드인다?
	unsigned char bCheckHome;		// 현재 슬립모드이후 센서원점을 찾았나?

#if FPGA_VERSION_TRIM	
	unsigned int btMotorState[STEP_MOTOR_COUNT];			// 모터상태
	MOTOR_ACT_SET_t mActSet[STEP_MOTOR_COUNT];			// 모터동작 설정저장 - (Encoder)실패시 사용
#else
	unsigned char btMotorState[STEP_MOTOR_COUNT];			// 모터상태
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
//	unsigned char curSvcWashCnt;			// 몇번 샐행했는가?

//	unsigned char curNdlWashBeforeCnt;			// 몇번 샐행했는가?
//	unsigned char curInjWashBeforeCnt;			// 몇번 샐행했는가?
//	unsigned char curNdlWashAfterCnt;			// 몇번 샐행했는가?
//	unsigned char curInjWashAfterCnt;			// 몇번 샐행했는가?

	unsigned int serviceWashCnt;			// 
	unsigned int curSvcWashCnt;			// 몇번 샐행했는가?

	unsigned int curNdlWashBeforeCnt;			// 몇번 샐행했는가?
	unsigned int curInjWashBeforeCnt;			// 몇번 샐행했는가?
	unsigned int curNdlWashAfterCnt;			// 몇번 샐행했는가?
	unsigned int curInjWashAfterCnt;			// 몇번 샐행했는가?

	float curStep[STEP_MOTOR_COUNT];	// 현재위치 (스텝 - pulse) 	
//int actStep[STEP_MOTOR_COUNT];
//unsigned short curPos[STEP_MOTOR_COUNT];
//unsigned short actPos[STEP_MOTOR_COUNT];
	unsigned char curTray;								// 0:left	1:Right 2:다른위치(injector/wash/waste) - Tray	
//unsigned short actTray;								// 0:left	1:Right 2:다른위치(injector/wash/waste) - Tray

	VIAL_POS_t sample;			// 분석해야할 sample (devState.actionMenu == AS_ACTION_RUN , 현재 실행중(devState.actionMenu == AS_ACTION_RUN) 
	VIAL_POS_t curSample;		// 분석중인 sample
	VIAL_POS_t lastSample;	// 분석해야할 마지막 vial

	VIAL_POS_t injectVial;	// 분석해야할 vial - Mix 동작 실행후 : 인젝터에 주입되어야 하는 바이알 

	unsigned char mixPrgmFinish; 	// mix 프로그램이 실행되었는가? - 한 시퀀스에 한번 실행한다.
	unsigned char mixTableNo;	// 현재 테이블 번호
//	unsigned char mixWait;	// 현재 wait 실행중인가? - 일정시간이상실행하면 sleep상태로 만들거나 sleep상태에서 깨어나는 동작을 추가하기 위함.


	VIAL_POS_t firstDestVial;
	
	VIAL_POS_t reagentVialA;
	VIAL_POS_t reagentVialB;
	VIAL_POS_t reagentVialC;
	VIAL_POS_t reagentVialD;
	
	unsigned char injectionTimes;		// 주입횟수(1-100)
	unsigned char curInjCnt;				// 현재 주입 실행 횟수 - 주입이 끝난 시점에서의 횟수 , 세척은 포함 안함.
//  float waitTime;				// 대기시간

//	unsigned int injectionVolumeMax;

	unsigned char seqAction;	// 시퀀스 동작 남아 있나? - 시퀀스 데이터 전송후 활성화 

  unsigned char isNextInjection;	// 다음 주입이 남았나? 현재샘플이 끝나고 다음 샘플이 남아있는 경우도 YES
  unsigned char doNextSample;	// 현재 샘플은 끝났다. 다음 샘플로 넘어가나?

  

  unsigned char vialExist;	// 현재 샘플 바이알이 있는가?

  unsigned char missVialResponse;	// MIssing Vial 처리에 관한 응답 - 제어기에서 self Message 전송(SELF_USER_RESPONSE)

//  unsigned char missVialSkip;	// MIssing Vial 처리 - 현재 샘플을 Skip해야함.

	unsigned char curDir[STEP_MOTOR_COUNT];				
//int actDir[STEP_MOTOR_COUNT];				// 이동해야 할 방향
	unsigned char bInfinite[STEP_MOTOR_COUNT];		// 0x0:	set step		0x40: infinite

//	int bMoveOk[STEP_MOTOR_COUNT];			// 동작이 완료되엇나?

	unsigned char homeFindEnd[STEP_MOTOR_COUNT];	// 홈찾기 완료 - initialize 시 에 사용 YES NO

// 제어기에서 전송된 command	// PACKCODE_LCAS_COMMAND	-- run / stop / start / pause /abort
// 다음 동작을 어떻게 할 것인지?
	unsigned char command;	
	unsigned char initCommand;	// 현재 초기화 명령을 받은 상태인가 
	
	unsigned char	setTempReset;

	float tempReadyUnder;	// tempReady 하한값
	float tempReadyOver;	// tempReady 상한값

	float oldTemp;
	unsigned char	oldTempOnoff;
	
	unsigned char curAuxEventNo;	// 현재 실행중인 
	unsigned char curAuxState;		// 현재 상태 0:OFF, 1:ON
	unsigned char curAuxOutIng;		// 현재출력진행중 : Aux 출력이 펄스출력일때 yes 면 반대 출력 해야함.	
	unsigned char auxOutSigTime;	// 시그널 출격시간 
	unsigned char auxOutSigTimeCnt; // 시그널 출격시간 체크	- 100ms

	unsigned int auxTime100msCnt[AUX_EVENT_COUNT];	// Aux 출력시간 - 100ms
	
	unsigned int analysisTime100msCnt;		// 분석시간 카운트 
	unsigned int analysisTime100ms;				// 분석시간 - 설정값 

	unsigned char sysinfoSave;						// system inform data 저장여부

//	LCAS_USED_TIME_t uTime;
	unsigned char usedTimeSave;					//  현재 ready상태가 아니라서 저장을 못했을 경우 = NO;

//	unsigned char vialPlateCheck;

	unsigned char diagState;			// Diag State
	unsigned char diagMenu;	// pDiag_Fn[]	

	unsigned int diagTimeCount;	// Heating 및 Cooling Time

	unsigned char LeakError;	// low = Error (leak)
	unsigned char LeakErrorSend;	// 에러를 다시 체크할 것인가?	
	unsigned char DoorError;	// high = Error (Not Detect) open
	unsigned char DoorErrorSend;	// 에러를 다시 체크할 것인가?

	// RYU_SF2
//	unsigned char thError1;
//	unsigned char thError2;
//	unsigned char sensorError;
	
	unsigned char adReadValid;	// ADC를 읽을 수 ?
	unsigned char powerCheck;		// 전원검사 완료 ?
	unsigned char powerState;		// 전원상태-에러가 있는가?

	unsigned char powerOffLed; 	// 

	unsigned char save_EEPROM_sysConfig; 	// 변경된 값을 eeprom에 저장해야한다.
	unsigned char save_EEPROM_special;
	
} LCAS_DEV_STATE_t;


// unsigned char adjustAct;			// 현재상태
#define ADJUST_ACT_NOT_READY		0		// Adjust Mode가 아니거나 준비가 안됨.
#define ADJUST_ACT_INIT					1		// 초기화(홈포지션 찾기) 진행중
#define ADJUST_ACT_READY				2		// 초기화(홈포지션 찾기)완료 
#define ADJUST_ACT_ADJUST				3		// 조정 진행중 - 저장 안된 상태 
#define ADJUST_ACT_EXCHANGE			4		// needle , syringe 교체위치로 이동중이거나 이동완료

typedef struct _LCAS_DEV_SET_t
{
	unsigned char adjustDate;		// 날짜
	unsigned char adjustState;	// 0: 실행한 적 없음 NO , 1: 완료 -  YES
//	unsigned char ing;		// YES NO
	unsigned char adjustOldMotor;
	unsigned char adjustCurMotor;	// 현재 실행중인 모터 , adjust동작 아닐 경우 -1
	unsigned char adjustAct;			// 현재상태
	
	ADJUST_DEV_DATA_t adjust;			// data	
	ADJUST_DEV_DATA_t adjustOld;	// data	
	
#if EEPROM_ADJUST_DATA_SAVE_FIX	
	ADJUST_DEV_DATA_t adjust_2nd;	// saveFlag는 checksum으로 이용	
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
	
//	unsigned char findHomeUnitStep[STEP_MOTOR_COUNT]; // 홈찾을 때 
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
//	int ndlHeight;		// Z축 Home에서 8800 : vial top (Cap include) - 1mm위 
	// Z축 vial top (Cap include) - 1mm위에서 - 15 x 1600 : Vial Bottom

	unsigned int syrExchangeHeight;
// vial Height 
// === 2ml
// Cap X : 32mm
// Depth : 31mm
// Cap include 33.3mm
// Depth(Cap include) : 대략 32mm
// === 4ml
// Cap X : 45mm
// Depth : 43.8mm 이상
// Cap include 47.mm 추측
// Depth(Cap include) : 대략 45mm

	float left_z_bottom;		// 내부
	float right_z_bottom;		// 외부

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

	uint16_t accumul_cnt;		// repeat 누적

} Step_Accel_t;

























// 아래는 사용하지 마시길
// 삭제하자 
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
