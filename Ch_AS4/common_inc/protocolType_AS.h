
#ifndef __PROTOCOL_TYPE_AS_H__
#define __PROTOCOL_TYPE_AS_H__

//#pragma pack(push,1)
#if QT_PROG_ADJUST
#define FLOAT_INPUT_INJECTION_VOLUME			1
// 20170927 혼합프로그램볼륨입력 소수점 1자리
#define FLOAT_INPUT_MIX_VOLUME			1
#endif

#include "Def_stepCon.h"
//#########################################################################################
//===============================================================================
// 통신프로토콜 
// 1. run중에는 설정값을 수정할 수 없음. - selfMessage 전송 (SELF_COMMAND_REJECT) 
// 2. 잘못된 명령 - selfMessage(SELF_ERROR, SELF_ERROR_COMMAND_UNKNOWN) 전송후 현재기기의 설정상태(패킷)를 전송함.
// 3. 데이터를 요청하는 패킷을 수신하면 응답(RESPONSE_LCAS_TABLE_PACKET)을 전송하고,
//   요청패킷이 아닌 시스템설정 명령을 수신 받으면 신호(ACK_LCAS_TABLE_PACKET)을 전송한다.
//   인지할 수 없는 패킷이나 잘못된 패킷의 경우는 selfMessage 전송 [SELF_ERROR]-(SELF_ERROR_COMMAND_UNKNOWN)
//===============================================================================
//#########################################################################################
typedef struct _TIME_t
{
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
} TIME_t;

typedef struct _DATE_t
{
	unsigned char year;
	unsigned char month;
	unsigned char date;
} DATE_t;

//=========================================================================================
//#define PACKCODE_LCAS_SYSTEM_INFORM			(PACKET_DEVICE_ID_YLAS + 0x400)
//-----------------------------------------------------------------------------------------
// 응답 
// 설정명령수신 : ACK_LCAS_TABLE_PACKET
// 설정값요청 : RESPONSE_LCAS_TABLE_PACKET
// 설정값은 디바이스(AutoSampler)에 저장됨 
//-----------------------------------------------------------------------------------------
typedef struct _SYSTEM_INFORM_t
{
	char cModel[32];			// 기기 정보 ex) "YL9152 LC AutoSampler"
	char cVersion[9];			// 버전		 ex) "1.0.0"

	char cSerialNo[17];		// 시리얼번호 - Serial No ex) AS 2345 161031 7654 --> AS 2345 161031 1DE6 
												// 1DE6 -> 9999-2345 = 7654 (Hex 1DE6)

#if 0
	char cInstallDate[11];	// 인스톨날짜	 ex) "20170306"
													// cInstallDate[3] : year(0 ~ 99)
													// cInstallDate[2] : month
													// cInstallDate[1] : date

	char cSysDate[11];		// ex) "20170306"
												// cSysDate[3] : year(0 ~ 99)
												// cSysDate[2] : month
												// cSysDate[1] : date

	char cSysTime[9];			// ex) "190630"
												// cSysTime[3] : time(0 ~ 23)
												// cSysTime[2] : minute
												// cSysTime[1] : sec
#else
	DATE_t cInstallDate;	// 인스톨날짜	 
													// yaer : year(0 ~ 99)
													// month : month
													// date : date

	DATE_t cSysDate;		// 현재날짜 
												// year : year(0 ~ 99)
												// month : month
												// date : date

	TIME_t cSysTime;			// 현재시각
#endif
	char cIPAddress[16];	// ex) "10.10.10.52"
	char cPortNo[5];			// ex) "4242"
} SYSTEM_INFORM_t;
//=========================================================================================

//=========================================================================================
#define TRAY_COOLING_NOT_INSTALL		0
#define TRAY_COOLING_INSTALL				1

#define SAMPLE_LOOP_VOLUME_MAX	10000		

#define SYRINGE_VOL_250UL				0
#define SYRINGE_VOL_500UL				1
#define SYRINGE_VOL_1000UL			2
#define SYRINGE_VOL_2500UL			3

#define NEEDLE_TUBING_VOL_DEFAULT		400
#define NEEDLE_TUBING_VOL_MAX		1000

#define TRAY_VIALS_40					0
#define TRAY_VIALS_60					1
#define WELLPALTE_96					2
#define TRAY_NONE					3

#define TRAY_DETECT_VIALS_40		5	//101b
#define TRAY_DETECT_VIALS_60		6	//110b
#define TRAY_DETECT_WELLPALTE_96	3	// 011b
#define TRAY_DETECT_NONE					7 // 111b

//#define TRAY_VIALS_40					0
#define TRAY40_COUNT_X_MAX		TRAY40_COUNT_X	//	4		// count    4 x 10
#define TRAY40_COUNT_Y_MAX		TRAY40_COUNT_Y	//	10		// count	

//#define TRAY_VIALS_60					1
#define TRAY60_COUNT_X_MAX		TRAY60_COUNT_X	//		5		// count		5 x 12
#define TRAY60_COUNT_Y_MAX		TRAY60_COUNT_Y  //		12	// count

//#define WELLPALTE_96					2
#define WELLPALTE_96_COUNT_X_MAX		WELLPALTE_96_COUNT_X	//	12	
#define WELLPALTE_96_COUNT_Y_MAX		WELLPALTE_96_COUNT_Y	//	8		

#define PROCESS_TYPE_ROW			0
#define PROCESS_TYPE_COLUMN		1

#define USE_PREP_MODE_NO		0
#define USE_PREP_MODE_YES		1

#define INJECTOR_MARKER_TIME_NONE				0		// 출력안함.
#define INJECTOR_MARKER_TIME_100ms			1
#define INJECTOR_MARKER_TIME_200ms			2
#define INJECTOR_MARKER_TIME_500ms			3
#define INJECTOR_MARKER_TIME_1sec				4
#define INJECTOR_MARKER_TIME_2sec				5

#define INJECTOR_MARKER_TIME_default		INJECTOR_MARKER_TIME_100ms

#define START_IN_SIGNAL_TIME_disable		0		// 입력받지 않음
#define START_IN_SIGNAL_TIME_100ms			1
#define START_IN_SIGNAL_TIME_200ms			2
#define START_IN_SIGNAL_TIME_500ms			3
#define START_IN_SIGNAL_TIME_1sec				4
#define START_IN_SIGNAL_TIME_2sec				5

#define START_IN_SIGNAL_TIME_default		START_IN_SIGNAL_TIME_disable

//-----------------------------------------------------------------------------------------
// #define PACKCODE_LCAS_SYSTEM_CONFIG			(PACKET_DEVICE_ID_YLAS + 0x410)
//-----------------------------------------------------------------------------------------
// 응답 
// 설정명령수신 : ACK_LCAS_TABLE_PACKET
// 설정값요청 : RESPONSE_LCAS_TABLE_PACKET
// 설정값은 디바이스(AutoSampler)에 저장됨 
//-----------------------------------------------------------------------------------------
typedef struct _SYSTEM_CONFIG_t
{
	unsigned int trayCooling;			// ( 0:Not install , 1:Install)
	
	unsigned int sampleLoopVol;  	// 0000 - 10000 uL	, During Prep Mode the loop volume is fixed at 10000uL
	unsigned int syringeVol;			// 0 : 250uL  1: 500uL(default)  2:1000uL 3: 2500uL in Prep Mode
	unsigned int needleTubeVol;		// needle Tubing 용량(ul) : 250uL(default) 	// 000 - 1000 uL

	unsigned int trayLeft;		// left(back ) tray
	unsigned int trayRight;		// right(front) tray
										// 0 = 40 VIALS
										// 1 = 60 VIALS
										// 2 = 96 VIALS

	unsigned int processType;	// 주입진행순서
										// 1 = ROW
										// 2 = COLUMN

	unsigned int syringeUpSpdWash;	// 수정 default:(10)  default:(6)  (1~10) 1:LOW			10:HIGH
	unsigned int syringeDnSpdWash;	// 수정 default:(3)   default:(6)  (1~10) 1:LOW			10:HIGH

	unsigned int injectMarkerTime;	// 스타트 시그널 출력 시간 (ms) ,	100ms ~ 2000ms (100ms default)
	unsigned int startInSigTime;		// 스타트시그널 인식 최소시간 (default : 입력받지 않음)

	unsigned char useMicroPump;		// 외부세정용 마이크로 펌프 사용여부 - // RunSetup에서 이동	
} SYSTEM_CONFIG_t;

//=========================================================================================

//=========================================================================================
#define TEMP_DEFAULT					20
#define TEMP_MAX							65	// 60
#define TEMP_MIN							-4	// 4

#define TEMP_READY_RANGE			0.1f

#define INJ_START_ANYTIME				0	// 스타트 신호를 받으면 즉시 시작
#define INJ_START_REMOTE_READY	1	// remote ready
#define INJ_START_TEMP_READY		2 // remote & temp ready
#define INJ_START_ALL_READY			3

//#define INJ_MODE_NONE					0
#define INJ_MODE_FULL_LOOP		0
#define INJ_MODE_PARTIAL_LOOP	1
#define INJ_MODE_MICOR_PICK		2

#define FLUSH_VOLUME_MAX		50
#define FLUSH_VOLUME_DEFAULT		10

#define WASH_BETWEEN_NONE					0			// 세척안함.
#define WASH_BETWEEN_INJECTIONS		1
#define WASH_BETWEEN_VIALS				2
#define WASH_BETWEEN_ANALYSYS_END	3			// seqence 끝나면 한다.

#define WASH_TIMES_MAX						10
#define WASH_NEEDLE_BEFORE_INJ_default		0
#define WASH_INJPORT_BEFORE_INJ_default		0
#define WASH_NEEDLE_AFTER_INJ_default			1
#define WASH_INJPORT_AFTER_INJ_default		1


#define SYRINGE_SPEED_MIN					1
#define SYRINGE_SPEED_MAX					10

#define NEEDLE_HEIGHT_MIN					2.0
#define NEEDLE_HEIGHT_MAX					6.0

#define WAIT_TIME_MAX							30	// sec

#define AIR_SEGMENT_DEFAULT				5			// 5L
#define AIR_SEGMENT_VOL_MIN				0			
#define AIR_SEGMENT_VOL_MAX				20			

#define AIR_SEGMENT_VOL_END				1			

#define MICRO_PUMP_TIME						3 //sec  -  runSetup.microPumpTime
#define MICRO_PUMP_VOLT_DEFAULT				5 //  - adjust.microPumpVolt

#define MICRO_PUMP_UNIT_PWM						5 // adjust 1 

//#define MICRO_PUMP_PWM_MIN						100 // 5V 정도 
#define MICRO_PUMP_PWM_MIN						0 // 5V 정도 

//#define MICRO_PUMP_POWER_CENTER				120  // adjust 1 
#define MICRO_PUMP_POWER_START			210		// 처음 기동을 위한 시작 전압 

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


//-----------------------------------------------------------------------------------------
// #define PACKCODE_LCAS_RUN_SETUP			(PACKET_DEVICE_ID_YLAS + 0x420)
//-----------------------------------------------------------------------------------------
// 응답 
// 설정명령수신 : ACK_LCAS_TABLE_PACKET
// 설정값요청 : RESPONSE_LCAS_TABLE_PACKET
// 설정값은 디바이스(AutoSampler)에 저장됨 
//-----------------------------------------------------------------------------------------
typedef struct _LCAS_RUN_SETUP_t
{
	unsigned char tempOnoff;			// SYSTEM_CONFIG_t의 trayCooling 설정에 따라 사용유무 적용
	int temp;						// Tray 설정온도 (4도 ~ 60도)
	unsigned char injectionStart;		// 사용안함.
		// 0 - anytime : 스타트 명령(신호)를 받으면 즉시 시작 
		// 1 - remote ready
		// 2 - temp ready : trayCooling 사용하고 temp on시 적용 온도가 안정되면 시작
			//	: 온도 off시에는 즉시 시작
		// 3 - all ready : remote & temp ready
	
	unsigned char injectionMode;	
											// 0 = Full loop injection mode					: 100uL
											// 1 = Partial loop fill injection mode	: 0-50uL	// Sample loop의 1/2까지 가능 
											// 2 = μL pick up injection mode				: 0-27uL
#if TRANSPORT_LIQUID_USE											
											// μL pick up injection mode 에서는 각각의 Tray의 맨 마지막 Vial이 Transport Liquid Vial로 자동지정됨.
#endif											

	float analysisTime;		// minute

	unsigned char flushEnable;	// 

	float flushVolume;		// FLUSH_VOLUME_DEFAULT
												
	float injectionVolume; 	// 실린지용량의 1/5 보다 작게  0~500
																	// 실린지 설정에 따라 최대값(실린지용량의 1/5)이 달라진다.
																	// sampleLoopVol 의 3x(100ul이하) or 2x(100~500ul) or 1.5x(500ul초과)

	unsigned char washBetween;	// 세척간격 
												// 0 = NONE			//
												// 1 = BETWEEN INJECTIONS
												// 2 = BETWEEN VIALS
												// 3 = ANALYSIS END
	unsigned int washVolume;			// 세척량 - 최대 : 시린지볼륨

// 세척횟수(0~9) - Mix 와 sample injection에서 동일하게 적용(동작)
	unsigned char washNeedleBeforeInj;		// default 0 - needle 및 needle tubing내부
	unsigned char washNeedleAfterInj;			// default 1
	unsigned char washInjportBeforeInj;		// default 0 - 인젝터 세척 횟수
	unsigned char washInjportAfterInj;		// default 1

	unsigned char syringeUpSpdInj;	// default:(6)  (1~10) 1:LOW			10:HIGH
	unsigned char syringeDnSpdInj;	// default:(6)  (1~10) 1:LOW			10:HIGH
	float needleHeight;	// 2~6mm(0.5mm단위) - 바이알(외부)의 밑부분에서 부터
	
	float waitTime_AfterAspirate;			// 샘플흡입후 대기시간 - 점도 있는 시료를 위한 (0~30sec)
	float waitTime_AfterDispense;			// 샘플배출후 대기시간 - 점도 있는 시료를 위한 (0~30sec)
		
//	unsigned char useMicroPump;			// 외부세정용 마이크로 펌프 사용여부

	unsigned char skipMissingSample;	// 바이알이 없으면 selfMessage 전송
				// 					0 = NO		// error - send_SelfMessage(SELF_ERROR, SELF_ERROR_MISSING_VIAL_WAIT);
				// default 	1 = Skip missing sample position - send_SelfMessage(SELF_ERROR, SELF_ERROR_MISSING_VIAL_SKIP);
													
	unsigned char airSegment;	// default (0) : 0 = Not use
														// 1 = Use air segment

	float airSegmentVol;		//default(5uL) 	(1~20)

#if FPGA_VERSION_TRIM_PUMP_TIME
	unsigned char microPumpTime;	// sec
	unsigned char reserved1;
	unsigned char reserved2;
	unsigned char reserved3;
#else
	int reserved;			// 
#endif

// ======================================================
} LCAS_RUN_SETUP_t;

//#########################################################################################
#define AUX_CONTACT_OPEN		0					// Contact open
#define AUX_CONTACT_CLOSE		1					// Contact Close
#define AUX_TOGGLE			2
#define AUX_PULSE				3				// 초기값에 따라 동작이 달라짐 초기값이 OFF이면 

#define AUX_EVENT_TIME_MAX	10000
//#########################################################################################
typedef struct _AUX_EVENT
{
	float fTime;			// 실행시간 [min]	- (0.0~10000) ,  테이블의 끝에는 fTime값을 음수로 설정
	unsigned char auxOnoff;		 	// 0:OFF, 1:ON, // 2:TOGGLE
} AUX_EVENT;

#define AUX_EVENT_COUNT		20

#define AUX_OUT_SIGNAL_TIME_disable		0
#define AUX_OUT_SIGNAL_TIME_100ms			1
#define AUX_OUT_SIGNAL_TIME_200ms			2
#define AUX_OUT_SIGNAL_TIME_500ms			3
#define AUX_OUT_SIGNAL_TIME_1sec			4
#define AUX_OUT_SIGNAL_TIME_2sec			5

#define AUX_OU_SIGNAL_TIME_default		AUX_OUT_SIGNAL_TIME_disable

//===============================================================================
// #define PACKCODE_LCAS_TIME_EVENT			(PACKET_DEVICE_ID_YLAS + 0x430)
//===============================================================================
typedef struct _LCAS_TIME_EVENT_t
{
	unsigned char useAuxTimeEvent;	//0:사용하지 않음, 1:사용
	
	unsigned char initAux;			 // 0:OFF, 1:ON,

	AUX_EVENT auxEvent[AUX_EVENT_COUNT];

	float endTime;	// 타임이벤트 종료시간 [min] - 마지막 상태유지

	unsigned char auxOutSigTime;	// pulse 출력시간 : 100ms ~ 2000ms (disable default)
	
} LCAS_TIME_EVENT_t;
//#########################################################################################


//#########################################################################################
#define TRAY_LEFT								0
#define TRAY_RIGHT							1
#define TRAY_ETC								2		// 0:inject		, 1: wash   2: waste

#define TRAY_FRONT							TRAY_LEFT
#define TRAY_BACK								TRAY_RIGHT

typedef struct _U_VIAL_POS_t
{
	unsigned char tray;			// 0: Left, 1: Right 2: 기타위치
	unsigned char etc;			// tray가 기타위치(2)일 경우 참조
													/*	#define POS_XY_INJECTOR			0		
															#define POS_XY_WASTE				1		
															#define POS_XY_WASH					2		
															#define POS_XY_EXCHANGE			3		
															#define POS_XY_HOME					4		*/
	unsigned char x;				// 0~5 - row(가로 행) (A~F)  			SYSTEM_CONFIG_t의 trayLeft설정(Tray의 종류)에 따라 최대가 달라짐
	unsigned char y;				// 0~11 - column(세로 열) (1~12) 	SYSTEM_CONFIG_t의 trayLeft설정(Tray의 종류)에 따라 최대가 달라짐
} U_VIAL_POS_t;

typedef union _VIAL_POS_t
{
	U_VIAL_POS_t uPos;
	int pos;
} VIAL_POS_t;

#define MIX_TABLE_COUNT		20		// alias - 15step

//  unsigned char action
#define MIX_ACTION_NONE			0
#define MIX_ACTION_ADD			1
#define MIX_ACTION_MIX			2
#define MIX_ACTION_WAIT			3
// unsigned char addFrom
#define MIX_FROM_SAMPLE				0
#define MIX_FROM_REAGENT_A		1
#define MIX_FROM_REAGENT_B		2
#define MIX_FROM_REAGENT_C		3
#define MIX_FROM_REAGENT_D		4
#define MIX_FROM_WASH					5
// unsigned char addTo
#define MIX_TO_SAMPLE					0
#define MIX_TO_DESTINATION		1

#define MIX_TIMES_MAX					9

#define MIX_WAIT_TIME_MAX			6000

typedef struct _LCAS_MIX_TABLE_t
{
	unsigned char action;		// 테이블의 끝에는 action값을 MIX_ACTION_NONE으로 설정
	unsigned char addFrom;
	unsigned char addTo;
#if FLOAT_INPUT_MIX_VOLUME	
	float addVol;		// uL (0~Syringe Vol)
#else	
	unsigned short addVol;		// uL (0~Syringe Vol)
#endif
	unsigned char mixVial;	// 0: Sample    1: Destination --- 무조건 Destination
	unsigned char mixTimes;	// 1~9
#if FLOAT_INPUT_MIX_VOLUME	
	float mixVol;		// uL (0~Syringe Vol) 
#else
	unsigned short mixVol;		// uL (0~Syringe Vol)
#endif
	unsigned short waitTime;	// sec (0 ~ MIX_WAIT_TIME_MAX)
} LCAS_MIX_TABLE_t;

//===============================================================================
// #define PACKCODE_LCAS_MIX_SETUP			(PACKET_DEVICE_ID_YLAS + 0x440)
//===============================================================================
// 한 Sample 당 1번 실행한다.
// Mix 프로그램 실행후 주입되는 시료는 
// LCAS_MIX_TABLE_t 에서 최종적으로 addTo에 설정된 바이알의 시료가 주입된다.
typedef struct _LCAS_MIX_t
{
	unsigned char useMixPrgm;		// 0 = not Use Mix Prgm		// 1 = Use Mix Prgm

	unsigned char relativeDestination;	// 0 - , 1 - relative destination	

	unsigned char relativeReagentA;	
	unsigned char relativeReagentB;	
	unsigned char relativeReagentC;	
	unsigned char relativeReagentD;	

	VIAL_POS_t firstDestPos;
	
	VIAL_POS_t reagentPosA;
	VIAL_POS_t reagentPosB;
	VIAL_POS_t reagentPosC;
	VIAL_POS_t reagentPosD;

	LCAS_MIX_TABLE_t mixTable[MIX_TABLE_COUNT];
} LCAS_MIX_t;

//===============================================================================
// #define PACKCODE_LCAS_SEQUENCE			(PACKET_DEVICE_ID_YLAS + 0x460)
//===============================================================================
#define INJECTION_TIMES_MAX				100
// 전송받으면 
// ready option check
// 주입후 start signal 
// ??? runSetup의 analysisTime 시간 + 대기시간 후에 다음 시료(반복)주입
typedef struct _LCAS_SEQUENCE_t
{
	VIAL_POS_t firstSamplePos;		// 시작 바이알 위치
	VIAL_POS_t lastSamplePos;			// 끝 바이알 위치
	unsigned char injectionTimes;	// 바이알 당 주입횟수(1~INJECTION_TIMES_MAX)
//	unsigned int injectionTimes;	// 바이알 당 주입횟수(1~INJECTION_TIMES_MAX)
//float intervalTime;							// 대기시간 - 다음 주입 시작전 --필요없다???-- runSetup의 analysisTime에서 통합 관리
} LCAS_SEQUENCE_t;

//===============================================================================
// #define PACKCODE_LCAS_COMMAND			(PACKET_DEVICE_ID_YLAS + 0x470)
//===============================================================================
// 응답(ACK) - 	정상적인 명령 SEND_ACK_LCAS_PACKET(pData) 응답하거나, erorr (self message) 전송
#define COMMAND_NONE				0
#define COMMAND_LOADING_INJECTION		1		// Sample loading작업 시작 & Injection
							//  Start	 : STATE_READY 상태에서만 실행가능 - 불가능 상태일 때는 send_SelfMessage(SELF_COMMAND_REJECT,0);
#define COMMAND_END					2		// 현재실행중인것(현재 바이알)은 완료하고 실행중인 sequence를 끝낸다. - PACKCODE_LCAS_RUN_SETUP 전송후 다시 시작가능
#define COMMAND_PAUSE				3		// 잠시중단 - STATE_RUN or STATE_ANALYSIS 일때 
#define COMMAND_RESUME			4		// pause이후 다시 시작


#if 0
#define COMMAND_STOP				5		// 즉시 중단 - COMMAND_INIT 명령(reset)으로 초기화 - 기기 껐다가 다시켜야 함.
#else
// 2017.04.18
// 수정할 것
#define COMMAND_ABORT				5		// 즉시 중단 - COMMAND_INIT 명령(reset)으로 초기화 - 기기 껐다가 다시켜야 함.
#define COMMAND_STOP				9		// 즉시 중단 - 남은 시퀀스는 삭제한다.
#endif

#define COMMAND_INIT				6		// stop, Abort 이후 :  초기화 작업 - 전원을 켰을 때 동작과 같음. (reset)
#define COMMAND_LOADING			7		// Sample loading - Loading만 단독으로 실행
#define COMMAND_INJECTION		8		// Sample Injection - 이전에 샘플이 loading되어 있어야 함.


typedef struct _LCAS_COMMAND_t
{
	unsigned char btCommand;
	unsigned char btSubCommand1;
	unsigned char btSubCommand2;
} LCAS_COMMAND_t;

//===============================================================================

#define	STATE_INITILIZE				1			// 전원이 ON되고 초기화 작업진행중
#define	STATE_READY						2			
#define	STATE_RUN 						3			// Sample loading작업중, mixing동작도 포함.
#define	STATE_ANALYSIS				4			// 인젝션 이후 스타트(mark out)이후 분석시간 동안 
#define	STATE_DEVICE_ADJUST		5			// 기기 영점조정
#define	STATE_SERVICE_WASH		6			// 사용자가 임의로 Washing 명령을 줄때 - 사용자가 세척 명령을 보냄.
#define	STATE_DIAGNOSTICS			7			// 1초마다 [PACKCODE_LCAS_DIAG_DATA]전송
#define	STATE_TEMP_CALIBRATION		8
#define	STATE_PAUSE						9			// STATE_PAUSE
#define	STATE_FAULT						10		// stop상태

//#define	STATE_MIXING					9
//#define	STATE_SLEEP						12

//===============================================================================
// 제어기에 전송용 상태값
// 전송시 대입해서 전송 - 1초에 한번 갱신 된다.
// 1초에 1번 전송
//===============================================================================
// #define PACKCODE_LCAS_STATE			(PACKET_DEVICE_ID_YLAS + 0x480)
//===============================================================================
typedef struct _LCAS_STATE_t
{
	unsigned char state;
// 현재 실행대기중 이거나 실행중인 샘플 바이알 
// state가 run이거나 Sequence wait
	VIAL_POS_t sample; 
	VIAL_POS_t injectVial;	// 인젝션할 바이알 - mix Program에서 사용
	
	// 현재 주입 실행 횟수 - 주입이 끝난 시점에서의 횟수(인젝션후 세척동작은 포함 안함).
	// 바이알에서 로딩후 세척까지 
	int curInjCnt;				
	float fTemp;				// tray의 현재 온도 
	unsigned char tempReady;	// 온도 안정 여부 - 현재온도와 설정값과의 차가 0.1도 이내로 1분이상 유지 
	unsigned char errorCode;  // 현재 에러 정보 - SelfMsg참조 
// unsigned char bMixing;	// 현재mixing 중인가
// unsigned char mixTableNo;	// 현재 실행중인 mixing table
} LCAS_STATE_t;


//===============================================================================
// #define PACKCODE_LCAS_ADJUST_DATA			(PACKET_DEVICE_ID_YLAS + 0x500)
//===============================================================================
// 인젝터를 기준으로 정한다.
// 홈방향으로 이동하면(+) / 홈과 반대로 이동(-)
#define ADJUST_INJ_PORT_VALUE_X			0
#define ADJUST_INJ_PORT_VALUE_Y			1
#define ADJUST_INJ_PORT_VALUE_Z			2
#define ADJUST_SYRINGE_Z						3
#define ADJUST_VALVE_X							4
#define ADJUST_INJECTOR_X						5

#define DATA_SAVE_FLAG						0xA5
// 홈방향으로 이동하면(+) / 홈과 반대로 이동(-)
typedef struct _ADJUST_DEV_DATA_t
{
	unsigned char saveFlag[6];	// x,y,z,syr,val,inj
	signed char	value[6]; // 홈방향으로 이동하면(+) / 홈과 반대로 이동(-)
	
	unsigned char saveVL[3];		// 바이알 좌표 조정값 저장여부 0:40vial ,1:60vial ,2:96vial - wellplate
	signed char leftVial_x[3];
	signed char leftVial_y[3];
	
	unsigned char saveVR[3];		// 바이알 좌표 조정값 저장여부 0:40vial ,1:60vial ,2:96vial - wellplate	
	signed char rightVial_x[3];
	signed char rightVial_y[3];

	unsigned char saveWash;		// wash port , waste port 좌표 조정값 저장여부
	signed char wash_x;
	signed char wash_y;

	unsigned char saveWaste;		// wash port , waste port 좌표 조정값 저장여부	
	signed char waste_x;
	signed char waste_y;

//#if VIAL_HEIGHT_DATUM	// Board  Rev 3 // 기구물 변경 6
	unsigned char saveVialHeight;		// 
	signed char vialHeight;	

	unsigned char saveVialHeight_xy;
	signed char vialHeight_x;		
	signed char vialHeight_y;			
//#endif

#if MICRO_PUMP_VOLTAGE_ADJUSTMENT
	unsigned char saveMicroPump;
	unsigned char microPumpVolt;
#endif	

#if	EEPROM_ADJUST_DATA_SAVE_FIX
//	uint32_t dummy[4];
#endif
} ADJUST_DEV_DATA_t;

// unsigned char command;
#define SERVICE_NONE			0
#define SERVICE_ADJUST		1
	// [SERVICE_ADJUST]의 응답은 
	// 정상적인 경우는 [PACKCODE_LCAS_ADJUST_DATA]로 응답하고
	// 비정상적인 명령의 경우는 [PACKCODE_LCAS_SLFEMSG]를 전송한다.
//unsigned char subCommand1;	// adjust :who(0~12)
	#define ADJUST_NEEDLE_XY			0				// 움직일 수 있는 방향 : x,y
	#define ADJUST_NEEDLE_HEIGHT	1				// 조정가능한 방향 : x,y,z
	#define ADJUST_SYRINGE				2				// : z
	#define ADJUST_VALVE_POS			3				// : x(ccw,cw)
	#define ADJUST_INJECTOR_POS				4				// : x(ccw,cw)
	#define ADJUST_WASHING_PORT					5				// : x,y
	#define ADJUST_WASTE_PORT						6				// : x,y  -- Washing Port와 동일 
	#define ADJUST_VIAL_LEFT						7				// : x,y
	#define ADJUST_VIAL_RIGHT						8				// : x,y
//#if VIAL_HEIGHT_DATUM	// Board  Rev 3 // 기구물 변경 6
	#define ADJUST_VIAL_HEIGHT					9				// : z -- vial 높이 기준 
	#define ADJUST_VIAL_HEIGHT_XY					10				// : xy -- vial 
	
	#define ADJUST_MICRO_PUMP_VOLTAGE			11		// micro pump voltage
//#endif

// unsigned char subCommand2;	// adjust :direction(1~12) & save (13~ )
	// ADJUST_DIRECTION
	#define ADJUST_NONE				0
	#define ADJUST_LEFT				1
	#define ADJUST_LEFT_X2		2
	#define ADJUST_RIGHT			3
	#define ADJUST_RIGHT_X2		4
	#define ADJUST_UP					5
	#define ADJUST_UP_X2			6
	#define ADJUST_DOWN				7
	#define ADJUST_DOWN_X2		8
	#define ADJUST_BACK				9
	#define ADJUST_BACK_X2		10
	#define ADJUST_FRONT			11
	#define ADJUST_FRONT_X2		12

	#define ADJUST_INIT_POS		13	// adjust 할 수 있도록 준비 - 조정위치로 이동
	#define ADJUST_INIT_POS_END		14	// [ADJUST_INIT_POS]명령을 받고 초기화가 완료되면 PC에 전송한다.
	#define ADJUST_SAVE				15		// 조정된 값을 저장
	#define ADJUST_INIT_VALUE		16		// 조정값 초기화
	#define ADJUST_QUIT					17		// Adjustment를 완료한다.

//STATE_READY상태이거나 STATE_DIAGNOSTICS상태에서 실행가능
//STATE_DIAGNOSTICS상태일때는 1초마다 [PACKCODE_LCAS_DIAG_DATA]패킷을 전송한다.
#define SERVICE_DIAGNOSTICS				2
	#define DIAGNOSTICS_NONE	(0)
	#define DIAGNOSTICS_STOP	(1)		//	현재 Diagnostics 동작 정지
	#define DIAGNOSTICS_QUIT	(2)		//	STATE_DIAGNOSTICS상태를 빠져나간다.(STATE_READY상태로 전환)
	#define DIAG_POWER_CHECK	(3)		//	전원검사 및 전압값 데이터 전송
	#define DIAG_HEATER_CHECK	(4)		//	적정파워 인가하고 일정시간이후 측정된 온도 검사, 히팅속도 검사
	#define DIAG_COOLER_CHECK	(5)		//	적정파워 인가하고 일정시간이후 측정된 온도 검사, 쿨링속도 검사
	#define DIAG_SENSOR_CHECK	(6)		//	적정파워 인가하고 일정시간이후 측정된 온도 검사
	#define DIAG_LEAK_CHECK		(7)		//  Leak
	#define DIAG_PHOTO_CHECK	(8)		//  Photointerrupt Sensor

// [STATE_TEMP_CALIBRATION] 상태로 된다.	
#define SERVICE_TEMP_CALIBRATION			3
// TEMP_CAL_START - 온도를 35도로 설정
// TEMP_CAL_TEMP_MEA - tempReady(1초마다 전송되는 상태값)가 (1)이되면 제어기에서 Measure값 입력받아 전송
// AutoSampler는 TEMP_CAL_TEMP_MEA을 받으면 온도를 10도로 설정 
// TEMP_CAL_TEMP_MEA - tempReady가 (1)이되면 제어기에서 Measure값 입력받아 전송
// AutoSampler는 save,reset,quit명령을 실행 

//  unsigned char subCommand1 : 
	#define TEMP_CAL_NONE						0
	#define TEMP_CAL_LOAD						0
	#define TEMP_CAL_START					1	
	#define TEMP_CAL_TEMP_MEA1			2		// 측정값 ex)35.5  / 10.2
	#define TEMP_CAL_TEMP_MEA2			3		// 측정값 ex)35.5  / 10.2
	#define TEMP_CAL_SAVE						4		// 저장
	#define TEMP_CAL_RESET					5		// Factor값을 1로 
	#define TEMP_CAL_QUIT						6		// STATE_READY 상태로
	// subCommand2  - 설정온도 , 측정온도

	#define TEMP_CAL_READY1					7		// 35도 Ready
	#define TEMP_CAL_READY2					8 	// 10도 Ready 


	#define TEMP_CAL_SET_1ST					35
	#define TEMP_CAL_SET_2ND					10

	#define TEMP_CAL_ERROR_MIN				-10
	#define TEMP_CAL_ERROR_MAX				10

/*
#define SERVICE_SYRINGE_EXCHANGE				7	
#define SERVICE_NDL_SYR_EXCHANGE				4			
	#define NDL_SYR_EXCHANGE_NONE			0		
	#define NDL_SYR_EXCHANGE_START		1		// 니들 시린지를 교체할 수 있도록 교체 가능위치로 이동 	
	#define NDL_SYR_EXCHANGE_QUIT			2		// 홈체크후 홈위치로 이동 		
*/
#define SERVICE_SYRINGE_REPLACE				7	
#define SERVICE_NEEDLE_REPLACE				4			
	#define NDL_SYR_REPLACE_NONE			0		
	#define NDL_SYR_REPLACE_START		1		// 니들 시린지를 교체할 수 있도록 교체 가능위치로 이동 	
	#define NDL_SYR_REPLACE_QUIT			2		// 홈체크후 홈위치로 이동 		


#define SERVICE_WASH_WASTE				5				
	#define WASH_WASTE_NEEDLE_INT			0
	#define WASH_WASTE_NEEDLE_ALL			1	
	#define WASH_WASTE_INJECTOR				2		
	// subCommand2  - 동작횟수		(1~10)



#define SERVICE_USED_TIME					6				
	#define USED_TIME_NONE				0
	#define USED_TIME_REQUEST			1
	#define USED_TIME_CLEAR				2	
	#define USED_TIME_SET					3		

#define SERVICE_MAC_INIT				8
#define SERVICE_IP_INIT					9

#define SERVICE_MIRO_PUMP_ON_OFF			10	
	#define MIRO_PUMP_CMD_ON			1			
	#define MIRO_PUMP_CMD_OFF			2			
//===============================================================================
// #define PACKCODE_LCAS_SERVICE			(PACKET_DEVICE_ID_YLAS + 0x510)
// 응답(ACK)
//===============================================================================
// ADJUST Action
// 1. Adjust Mode로 진입전 기기(오토셈플러)에 ADJUST_DEV_DATA_t 설정값을 요청한다.
// 2. 교정하고자 하는 부분의 초기화 신호를 보낸다.
// 	command : SERVICE_ADJUST
// 	subCommand1 : ADJUST_NEEDLE_XY
//  subCommand2 : ADJUST_INIT_POS
		// ---> 교정 초기화 완료되면 기기에서 완료신호를 보낸다.
		// 	command : SERVICE_ADJUST
		// 	subCommand1 : ADJUST_NEEDLE_XY
		//  subCommand2 : ADJUST_INIT_POS_END
// 3. 기구물 조정 명령을 보낸다.
// 	command : SERVICE_ADJUST
// 	subCommand1 : ADJUST_NEEDLE_XY
//  subCommand2 : ADJUST_LEFT
// 4. 조정이 완료되면 저장(취소/...) 신호를 보낸다.
// 	command : SERVICE_ADJUST
// 	subCommand1 : ADJUST_NEEDLE_XY
//  subCommand2 : ADJUST_SAVE / 

typedef struct _LCAS_SERVICE_t
{
  unsigned char command;
  unsigned char subCommand1;
  unsigned char subCommand2;

	float value;	// 추가 [20170405]
} LCAS_SERVICE_t;

// unsigned char btMessage;
#define SELF_STATE				1	// 사용안함.
#define SELF_ERROR				2
	// 전송받은 명령이 알수 없는 명령이거나 설정값이 범위를 벗어난 경우-SelfMsg이후 현재 AutoSampler에 설정된 값을 전송한다
	#define SELF_NO_ERROR			0
	#define SELF_ERROR_COMMAND_UNKNOWN			1
	// adjustment 실행한적이 없음. - 전원을 켰을 때 초기화하기 전에 체크하여 전송한다.
	// adjustment를 꼭 한번은 실행해야 한다.
	#define SELF_ERROR_NO_ADJUSTMENT_DATA		2	
	// missing vial - 동작중 바이알을 감지하지 못하면 메시지를 전송한다.
	#define SELF_ERROR_MISSING_VIAL_SKIP		3		// skipMissingSample 설정값이 (1) 일 때 발생
	#define SELF_ERROR_MISSING_VIAL_WAIT		4		// skipMissingSample 설정값이 (0) 일 때 발생 - SELF_USER_RESPONSE (제어기의 사용자 입력을 기다린다.)
	#define SELF_ERROR_OPERATION						5 	// 기구물 에러
	#define SELF_ERROR_DOOR_OPEN	(6) // 도어열림
	#define SELF_ERROR_LEAK			(7) // Leak
	#define SELF_ERROR_TEMPERATURE	(8) // 온도 이상

	#define SELF_ERROR_MOTOR_X_AXIS					9 	
	#define SELF_ERROR_MOTOR_Y_AXIS					10 	
	#define SELF_ERROR_MOTOR_Z_AXIS					11 	
	#define SELF_ERROR_MOTOR_SYRINGE				12 	
	#define SELF_ERROR_MOTOR_VALVE					13 	
	#define SELF_ERROR_MOTOR_INJECTOR				14 	
	#define SELF_ERROR_POSITION_XY					15
	#define SELF_ERROR_POSITION_Z						16


	#define	SELF_ERROR_HOME_CHK_X					20
	#define	SELF_ERROR_HOME_CHK_Y					21
	#define	SELF_ERROR_HOME_CHK_Z					22
	#define	SELF_ERROR_HOME_CHK_SYR					23
	#define	SELF_ERROR_HOME_CHK_VAL					24
	#define	SELF_ERROR_HOME_CHK_INJ					25

	#define	SELF_ERROR_ENCODER_COUNT					30	
	#define	SELF_ERROR_ENCODER_COUNT_X					30
	#define	SELF_ERROR_ENCODER_COUNT_Y					31
	#define	SELF_ERROR_ENCODER_COUNT_Z					32
	#define	SELF_ERROR_ENCODER_COUNT_SYR					33
	#define	SELF_ERROR_ENCODER_COUNT_VAL					34
	#define	SELF_ERROR_ENCODER_COUNT_INJ					35


	#define MOVE_ERR_BOTH__VIAL_AND_Z_AXIS			70
	#define MOVE_ERR__VIAL_CHECK						71
	#define MOVE_ERR__Z_AXIS_CHECK					72
	
	#define SELF_ERROR_CODE_MAX		(255)

	
#define SELF_START_LOADING			3	// loading작업 시작 // 0: PC명령  1: Extin(외부입력) 2: 반복분석자동 시작 
	#define SELF_START_LOADING_PCIN			0
	#define SELF_START_LOADING_EXTIN		1
	#define SELF_START_LOADING_AUTO			2

#define SELF_START_INJECTION		4	//injection 
#define SELF_EXTOUT					5 // 외부단자 출력
#define SELF_STOP					6	// [COMMAND_STOP] 명령이 들어왔을 때 Stop후 전송

#define SELF_COMMAND_REJECT		7		// 전송 명령 거부 , 전송된 명령을 실행할 수 없는 상태일 떄 전송됨.
	#define ADJUST_VALUE_ERROR_0		1
	#define ADJUST_VALUE_ERROR_1		2

#define SELF_END_SEQUENCE			8		// 시퀀스 동작 수행 완료

#define SELF_USER_RESPONSE		9	// 제어기에서 응답해야함(제어기에서 AutoSampler로 전송). - Missing Vial 사용자 선택에 대한 응답
	#define MISSING_VIAL_NO_RESPONSE	0		// 
	#define MISSING_VIAL_SKIP				1		// Vial Skip
	#define MISSING_VIAL_RETRY			2		// Vial 체크 및 흡입과정 다시 시도 
	#define MISSING_VIAL_ABORT			3		// STOP	- Sequence 중단 , 이후 ready상태로
	#define MISSING_VIAL_STOP				MISSING_VIAL_ABORT
	
#define SELF_UPDATE				10	// unsigned char btNewValue;--> 0: 장비업데이트필요 1:SW업데이트필요 2:성공 3:실패
#define SELF_ABORT				11	// [COMMAND_ABORT] 명령이 들어왔을 때 즉시스톱[Abort,Stop]후 전송

//===============================================================================
// #define PACKCODE_LCAS_SLFEMSG			(PACKET_DEVICE_ID_YLAS + 0x490)
//===============================================================================
// PC에서 START(STOP)명령 받으면 응답(ACK)만한다. Selfmsg안보냄
// 리모트 스타트의 경우는  Selfmsg전송
typedef struct _LCAS_SELFMSG_t
{
// 상태가 변할 때 전송된다.
	unsigned char ucMessage;
	unsigned char ucNewValue;
} LCAS_SELFMSG_t;


//===============================================================================
// #define PACKCODE_LCAS_DIAG_DATA						(PACKET_DEVICE_ID_YLAS + 0x520)
//===============================================================================
// 전압체크 - 전원전압 체크후 전송
//[STATE_DIAGNOSTICS]상태일때는 1초마다 [PACKCODE_LCAS_DIAG_DATA]패킷을 전송한다.
#define DIAG_RESULT_NONE		0
#define DIAG_RESULT_PASS		1
#define DIAG_RESULT_FAIL		2
//#define DIAG_RESULT_PROCESSING	3
//#define DIAG_RESULT_REJECT	4

// 진단 요청에 대하여 진단이 완료되면 결과 값을 전송 - 끝나면 한번만 전송한다.
typedef struct _LCAS_DIAG_DATA {
	unsigned char Current;	// 현재 동작중인 검사 
	
	unsigned char N5Dcheck;			// Analog -5V		[0]
	unsigned char V5Dcheck;			// Analog +5V		[1]
	unsigned char V1_5Dcheck;		// FPGA Core +1.5V		[5]
	unsigned char V12Dcheck;		// Heater Power +12V	[6]
	
	unsigned char V2_5Dcheck;		// ADC Reference Voltage +2.5V		[11]
	unsigned char V3_3Dcheck;		// CPU Power +3.3V								[10]

	float N5Dvalue;
	float V5Dvalue;
	float V1_5Dvalue;
	float V12Dvalue;
	float V2_5Dvalue;
	float V3_3Dvalue;
	
	unsigned char Tempcheck;		// 온도	
	unsigned char btHeaterTest;		// 히터(펠티에) 검사, 30도에서 40도까지 상승시간 검사
	unsigned char btCoolerTest;		// 쿨러(페티에) 검사, 40도에서 20도까지 하강시간 검사
	
	unsigned char btLeakTest1;		// leak check
	unsigned char btLeakTest2;		// leak check

	unsigned char btTempSensor;		// 온도 센서 - 전압(저항)값 검사 및 저항 변화율 검사,
									// 일정시간동안 히팅 후에 저항값 변화 검사
} LCAS_DIAG_DATA_t;

//===============================================================================
//#define PACKCODE_LCAS_TEMPCAL_DATA				(PACKET_DEVICE_ID_YLAS + 0x530)
//===============================================================================
typedef struct _LCAS_TEMP_CAL {
	float setTemp1;
	float setTemp2;
	
	float meaTemp1;
	float meaTemp2;

	float constA;		// 0
	float constB;		// x의 계수
	float constC;		// 절편(상수)
	
//	LcasState.fTemp = tempConst_B * uAdcSum + tempConst_C;
//	LcasState.fTemp = (tempConst_B + tempCalData.constB) * uAdcSum + (tempConst_C + tempCalData.constC);
	
} LCAS_TEMP_CAL_t;

//===============================================================================
// #define PACKCODE_LCAS_SPECIAL 						(PACKET_DEVICE_ID_YLAS + (0x620)	// - 추가(20170914)
//===============================================================================
typedef struct _LCAS_SPECIAL {
	unsigned char LED_onoff;
	unsigned char Door_check;
	unsigned char Leak_check;
	unsigned char Buzzer_onoff;

	unsigned char Reserved[4];
} LCAS_SPECIAL_t;


//===============================================================================
// #define PACKCODE_LCAS_USED_TIME						(PACKET_DEVICE_ID_YLAS + 0x540)
//===============================================================================
#define USED_TIME_NONE			0
#define USED_TIME_DEVICE		1
#define USED_TIME_RUNTIME		2
#define USED_TIME_HEATTIME	3
//#define USED_TIME_NEEDLE		4
#define USED_TIME_X					5
#define USED_TIME_Y					6
#define USED_TIME_Z					7
#define USED_TIME_SYR				8
#define USED_TIME_VAL				9
#define USED_TIME_INJ				10
#define USED_TIME_INJPORT		11
typedef struct _LCAS_USED_TIME {
	unsigned int time;	// 기기 사용시간 (1min)
	unsigned int runTime;	// 기기 사용시간 - run동작 (1min)
	unsigned int heatTime;	// Tray Cooling & heating 시간 (1min)

// 주입 및 Washing이 끝나면 	
//	unsigned int needle;	// 이동거리 mm
	unsigned int x;	// 이동거리 mm	--> meter로 변경
	unsigned int y;	// 이동거리 mm	
	unsigned int z;	// 이동거리 mm		
	
	unsigned int syringe;	// 이동거리 mm
	unsigned int valve;		// 이동 포트 수 
	unsigned int injector;	// 인젝터 동작 수
	unsigned int injectionCnt;	// 인젝션횟수 - injection port 사용횟수
} LCAS_USED_TIME_t;

typedef struct _LCAS_USER_DEFINE_CONTROL {

} LCAS_USER_DEFINE_CONTROL_t;



typedef struct _ETHERNET_CONFIG_t
{
	char cModifyChar[9];			// ex) "IPADDR"
	
	char cFirmwareVersion[9];			// 버전		 ex) "1.1.3"
	char cMachineVersion[9];			// 버전		 ex) "1.3.8"

	char cIPAddress[16];	// ex) "10.10.10.52"
	char cGateway[16];	// ex) "10.10.10.1"
	char cNetmask[16];			// ex) "255.255.255.0"
	char cPortNo[5];			// ex) "4242"

	char cMacAddress[18];	// ex) "00:11:22:33:44:55"	

	char cdummy1[18];	// ex) 
	char cdummy2[18];	// ex) 
	char cdummy3[18];	// ex) 
} ETHERNET_CONFIG_t;



//#pragma pack(pop)
#endif	//  __PROTOCOL_TYPE_AS_H__

