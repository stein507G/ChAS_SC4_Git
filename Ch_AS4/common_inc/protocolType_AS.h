
#ifndef __PROTOCOL_TYPE_AS_H__
#define __PROTOCOL_TYPE_AS_H__

//#pragma pack(push,1)
#if QT_PROG_ADJUST
#define FLOAT_INPUT_INJECTION_VOLUME			1
// 20170927 ȥ�����α׷������Է� �Ҽ��� 1�ڸ�
#define FLOAT_INPUT_MIX_VOLUME			1
#endif

#include "Def_stepCon.h"
//#########################################################################################
//===============================================================================
// ����������� 
// 1. run�߿��� �������� ������ �� ����. - selfMessage ���� (SELF_COMMAND_REJECT) 
// 2. �߸��� ��� - selfMessage(SELF_ERROR, SELF_ERROR_COMMAND_UNKNOWN) ������ �������� ��������(��Ŷ)�� ������.
// 3. �����͸� ��û�ϴ� ��Ŷ�� �����ϸ� ����(RESPONSE_LCAS_TABLE_PACKET)�� �����ϰ�,
//   ��û��Ŷ�� �ƴ� �ý��ۼ��� ����� ���� ������ ��ȣ(ACK_LCAS_TABLE_PACKET)�� �����Ѵ�.
//   ������ �� ���� ��Ŷ�̳� �߸��� ��Ŷ�� ���� selfMessage ���� [SELF_ERROR]-(SELF_ERROR_COMMAND_UNKNOWN)
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
// ���� 
// ������ɼ��� : ACK_LCAS_TABLE_PACKET
// ��������û : RESPONSE_LCAS_TABLE_PACKET
// �������� ����̽�(AutoSampler)�� ����� 
//-----------------------------------------------------------------------------------------
typedef struct _SYSTEM_INFORM_t
{
	char cModel[32];			// ��� ���� ex) "YL9152 LC AutoSampler"
	char cVersion[9];			// ����		 ex) "1.0.0"

	char cSerialNo[17];		// �ø����ȣ - Serial No ex) AS 2345 161031 7654 --> AS 2345 161031 1DE6 
												// 1DE6 -> 9999-2345 = 7654 (Hex 1DE6)

#if 0
	char cInstallDate[11];	// �ν��糯¥	 ex) "20170306"
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
	DATE_t cInstallDate;	// �ν��糯¥	 
													// yaer : year(0 ~ 99)
													// month : month
													// date : date

	DATE_t cSysDate;		// ���糯¥ 
												// year : year(0 ~ 99)
												// month : month
												// date : date

	TIME_t cSysTime;			// ����ð�
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

#define INJECTOR_MARKER_TIME_NONE				0		// ��¾���.
#define INJECTOR_MARKER_TIME_100ms			1
#define INJECTOR_MARKER_TIME_200ms			2
#define INJECTOR_MARKER_TIME_500ms			3
#define INJECTOR_MARKER_TIME_1sec				4
#define INJECTOR_MARKER_TIME_2sec				5

#define INJECTOR_MARKER_TIME_default		INJECTOR_MARKER_TIME_100ms

#define START_IN_SIGNAL_TIME_disable		0		// �Է¹��� ����
#define START_IN_SIGNAL_TIME_100ms			1
#define START_IN_SIGNAL_TIME_200ms			2
#define START_IN_SIGNAL_TIME_500ms			3
#define START_IN_SIGNAL_TIME_1sec				4
#define START_IN_SIGNAL_TIME_2sec				5

#define START_IN_SIGNAL_TIME_default		START_IN_SIGNAL_TIME_disable

//-----------------------------------------------------------------------------------------
// #define PACKCODE_LCAS_SYSTEM_CONFIG			(PACKET_DEVICE_ID_YLAS + 0x410)
//-----------------------------------------------------------------------------------------
// ���� 
// ������ɼ��� : ACK_LCAS_TABLE_PACKET
// ��������û : RESPONSE_LCAS_TABLE_PACKET
// �������� ����̽�(AutoSampler)�� ����� 
//-----------------------------------------------------------------------------------------
typedef struct _SYSTEM_CONFIG_t
{
	unsigned int trayCooling;			// ( 0:Not install , 1:Install)
	
	unsigned int sampleLoopVol;  	// 0000 - 10000 uL	, During Prep Mode the loop volume is fixed at 10000uL
	unsigned int syringeVol;			// 0 : 250uL  1: 500uL(default)  2:1000uL 3: 2500uL in Prep Mode
	unsigned int needleTubeVol;		// needle Tubing �뷮(ul) : 250uL(default) 	// 000 - 1000 uL

	unsigned int trayLeft;		// left(back ) tray
	unsigned int trayRight;		// right(front) tray
										// 0 = 40 VIALS
										// 1 = 60 VIALS
										// 2 = 96 VIALS

	unsigned int processType;	// �����������
										// 1 = ROW
										// 2 = COLUMN

	unsigned int syringeUpSpdWash;	// ���� default:(10)  default:(6)  (1~10) 1:LOW			10:HIGH
	unsigned int syringeDnSpdWash;	// ���� default:(3)   default:(6)  (1~10) 1:LOW			10:HIGH

	unsigned int injectMarkerTime;	// ��ŸƮ �ñ׳� ��� �ð� (ms) ,	100ms ~ 2000ms (100ms default)
	unsigned int startInSigTime;		// ��ŸƮ�ñ׳� �ν� �ּҽð� (default : �Է¹��� ����)

	unsigned char useMicroPump;		// �ܺμ����� ����ũ�� ���� ��뿩�� - // RunSetup���� �̵�	
} SYSTEM_CONFIG_t;

//=========================================================================================

//=========================================================================================
#define TEMP_DEFAULT					20
#define TEMP_MAX							65	// 60
#define TEMP_MIN							-4	// 4

#define TEMP_READY_RANGE			0.1f

#define INJ_START_ANYTIME				0	// ��ŸƮ ��ȣ�� ������ ��� ����
#define INJ_START_REMOTE_READY	1	// remote ready
#define INJ_START_TEMP_READY		2 // remote & temp ready
#define INJ_START_ALL_READY			3

//#define INJ_MODE_NONE					0
#define INJ_MODE_FULL_LOOP		0
#define INJ_MODE_PARTIAL_LOOP	1
#define INJ_MODE_MICOR_PICK		2

#define FLUSH_VOLUME_MAX		50
#define FLUSH_VOLUME_DEFAULT		10

#define WASH_BETWEEN_NONE					0			// ��ô����.
#define WASH_BETWEEN_INJECTIONS		1
#define WASH_BETWEEN_VIALS				2
#define WASH_BETWEEN_ANALYSYS_END	3			// seqence ������ �Ѵ�.

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

//#define MICRO_PUMP_PWM_MIN						100 // 5V ���� 
#define MICRO_PUMP_PWM_MIN						0 // 5V ���� 

//#define MICRO_PUMP_POWER_CENTER				120  // adjust 1 
#define MICRO_PUMP_POWER_START			210		// ó�� �⵿�� ���� ���� ���� 

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
// ���� 
// ������ɼ��� : ACK_LCAS_TABLE_PACKET
// ��������û : RESPONSE_LCAS_TABLE_PACKET
// �������� ����̽�(AutoSampler)�� ����� 
//-----------------------------------------------------------------------------------------
typedef struct _LCAS_RUN_SETUP_t
{
	unsigned char tempOnoff;			// SYSTEM_CONFIG_t�� trayCooling ������ ���� ������� ����
	int temp;						// Tray �����µ� (4�� ~ 60��)
	unsigned char injectionStart;		// ������.
		// 0 - anytime : ��ŸƮ ���(��ȣ)�� ������ ��� ���� 
		// 1 - remote ready
		// 2 - temp ready : trayCooling ����ϰ� temp on�� ���� �µ��� �����Ǹ� ����
			//	: �µ� off�ÿ��� ��� ����
		// 3 - all ready : remote & temp ready
	
	unsigned char injectionMode;	
											// 0 = Full loop injection mode					: 100uL
											// 1 = Partial loop fill injection mode	: 0-50uL	// Sample loop�� 1/2���� ���� 
											// 2 = ��L pick up injection mode				: 0-27uL
#if TRANSPORT_LIQUID_USE											
											// ��L pick up injection mode ������ ������ Tray�� �� ������ Vial�� Transport Liquid Vial�� �ڵ�������.
#endif											

	float analysisTime;		// minute

	unsigned char flushEnable;	// 

	float flushVolume;		// FLUSH_VOLUME_DEFAULT
												
	float injectionVolume; 	// �Ǹ����뷮�� 1/5 ���� �۰�  0~500
																	// �Ǹ��� ������ ���� �ִ밪(�Ǹ����뷮�� 1/5)�� �޶�����.
																	// sampleLoopVol �� 3x(100ul����) or 2x(100~500ul) or 1.5x(500ul�ʰ�)

	unsigned char washBetween;	// ��ô���� 
												// 0 = NONE			//
												// 1 = BETWEEN INJECTIONS
												// 2 = BETWEEN VIALS
												// 3 = ANALYSIS END
	unsigned int washVolume;			// ��ô�� - �ִ� : �ø�������

// ��ôȽ��(0~9) - Mix �� sample injection���� �����ϰ� ����(����)
	unsigned char washNeedleBeforeInj;		// default 0 - needle �� needle tubing����
	unsigned char washNeedleAfterInj;			// default 1
	unsigned char washInjportBeforeInj;		// default 0 - ������ ��ô Ƚ��
	unsigned char washInjportAfterInj;		// default 1

	unsigned char syringeUpSpdInj;	// default:(6)  (1~10) 1:LOW			10:HIGH
	unsigned char syringeDnSpdInj;	// default:(6)  (1~10) 1:LOW			10:HIGH
	float needleHeight;	// 2~6mm(0.5mm����) - ���̾�(�ܺ�)�� �غκп��� ����
	
	float waitTime_AfterAspirate;			// ���������� ���ð� - ���� �ִ� �÷Ḧ ���� (0~30sec)
	float waitTime_AfterDispense;			// ���ù����� ���ð� - ���� �ִ� �÷Ḧ ���� (0~30sec)
		
//	unsigned char useMicroPump;			// �ܺμ����� ����ũ�� ���� ��뿩��

	unsigned char skipMissingSample;	// ���̾��� ������ selfMessage ����
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
#define AUX_PULSE				3				// �ʱⰪ�� ���� ������ �޶��� �ʱⰪ�� OFF�̸� 

#define AUX_EVENT_TIME_MAX	10000
//#########################################################################################
typedef struct _AUX_EVENT
{
	float fTime;			// ����ð� [min]	- (0.0~10000) ,  ���̺��� ������ fTime���� ������ ����
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
	unsigned char useAuxTimeEvent;	//0:������� ����, 1:���
	
	unsigned char initAux;			 // 0:OFF, 1:ON,

	AUX_EVENT auxEvent[AUX_EVENT_COUNT];

	float endTime;	// Ÿ���̺�Ʈ ����ð� [min] - ������ ��������

	unsigned char auxOutSigTime;	// pulse ��½ð� : 100ms ~ 2000ms (disable default)
	
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
	unsigned char tray;			// 0: Left, 1: Right 2: ��Ÿ��ġ
	unsigned char etc;			// tray�� ��Ÿ��ġ(2)�� ��� ����
													/*	#define POS_XY_INJECTOR			0		
															#define POS_XY_WASTE				1		
															#define POS_XY_WASH					2		
															#define POS_XY_EXCHANGE			3		
															#define POS_XY_HOME					4		*/
	unsigned char x;				// 0~5 - row(���� ��) (A~F)  			SYSTEM_CONFIG_t�� trayLeft����(Tray�� ����)�� ���� �ִ밡 �޶���
	unsigned char y;				// 0~11 - column(���� ��) (1~12) 	SYSTEM_CONFIG_t�� trayLeft����(Tray�� ����)�� ���� �ִ밡 �޶���
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
	unsigned char action;		// ���̺��� ������ action���� MIX_ACTION_NONE���� ����
	unsigned char addFrom;
	unsigned char addTo;
#if FLOAT_INPUT_MIX_VOLUME	
	float addVol;		// uL (0~Syringe Vol)
#else	
	unsigned short addVol;		// uL (0~Syringe Vol)
#endif
	unsigned char mixVial;	// 0: Sample    1: Destination --- ������ Destination
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
// �� Sample �� 1�� �����Ѵ�.
// Mix ���α׷� ������ ���ԵǴ� �÷�� 
// LCAS_MIX_TABLE_t ���� ���������� addTo�� ������ ���̾��� �÷ᰡ ���Եȴ�.
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
// ���۹����� 
// ready option check
// ������ start signal 
// ??? runSetup�� analysisTime �ð� + ���ð� �Ŀ� ���� �÷�(�ݺ�)����
typedef struct _LCAS_SEQUENCE_t
{
	VIAL_POS_t firstSamplePos;		// ���� ���̾� ��ġ
	VIAL_POS_t lastSamplePos;			// �� ���̾� ��ġ
	unsigned char injectionTimes;	// ���̾� �� ����Ƚ��(1~INJECTION_TIMES_MAX)
//	unsigned int injectionTimes;	// ���̾� �� ����Ƚ��(1~INJECTION_TIMES_MAX)
//float intervalTime;							// ���ð� - ���� ���� ������ --�ʿ����???-- runSetup�� analysisTime���� ���� ����
} LCAS_SEQUENCE_t;

//===============================================================================
// #define PACKCODE_LCAS_COMMAND			(PACKET_DEVICE_ID_YLAS + 0x470)
//===============================================================================
// ����(ACK) - 	�������� ��� SEND_ACK_LCAS_PACKET(pData) �����ϰų�, erorr (self message) ����
#define COMMAND_NONE				0
#define COMMAND_LOADING_INJECTION		1		// Sample loading�۾� ���� & Injection
							//  Start	 : STATE_READY ���¿����� ���డ�� - �Ұ��� ������ ���� send_SelfMessage(SELF_COMMAND_REJECT,0);
#define COMMAND_END					2		// ����������ΰ�(���� ���̾�)�� �Ϸ��ϰ� �������� sequence�� ������. - PACKCODE_LCAS_RUN_SETUP ������ �ٽ� ���۰���
#define COMMAND_PAUSE				3		// ����ߴ� - STATE_RUN or STATE_ANALYSIS �϶� 
#define COMMAND_RESUME			4		// pause���� �ٽ� ����


#if 0
#define COMMAND_STOP				5		// ��� �ߴ� - COMMAND_INIT ���(reset)���� �ʱ�ȭ - ��� ���ٰ� �ٽ��Ѿ� ��.
#else
// 2017.04.18
// ������ ��
#define COMMAND_ABORT				5		// ��� �ߴ� - COMMAND_INIT ���(reset)���� �ʱ�ȭ - ��� ���ٰ� �ٽ��Ѿ� ��.
#define COMMAND_STOP				9		// ��� �ߴ� - ���� �������� �����Ѵ�.
#endif

#define COMMAND_INIT				6		// stop, Abort ���� :  �ʱ�ȭ �۾� - ������ ���� �� ���۰� ����. (reset)
#define COMMAND_LOADING			7		// Sample loading - Loading�� �ܵ����� ����
#define COMMAND_INJECTION		8		// Sample Injection - ������ ������ loading�Ǿ� �־�� ��.


typedef struct _LCAS_COMMAND_t
{
	unsigned char btCommand;
	unsigned char btSubCommand1;
	unsigned char btSubCommand2;
} LCAS_COMMAND_t;

//===============================================================================

#define	STATE_INITILIZE				1			// ������ ON�ǰ� �ʱ�ȭ �۾�������
#define	STATE_READY						2			
#define	STATE_RUN 						3			// Sample loading�۾���, mixing���۵� ����.
#define	STATE_ANALYSIS				4			// ������ ���� ��ŸƮ(mark out)���� �м��ð� ���� 
#define	STATE_DEVICE_ADJUST		5			// ��� ��������
#define	STATE_SERVICE_WASH		6			// ����ڰ� ���Ƿ� Washing ����� �ٶ� - ����ڰ� ��ô ����� ����.
#define	STATE_DIAGNOSTICS			7			// 1�ʸ��� [PACKCODE_LCAS_DIAG_DATA]����
#define	STATE_TEMP_CALIBRATION		8
#define	STATE_PAUSE						9			// STATE_PAUSE
#define	STATE_FAULT						10		// stop����

//#define	STATE_MIXING					9
//#define	STATE_SLEEP						12

//===============================================================================
// ����⿡ ���ۿ� ���°�
// ���۽� �����ؼ� ���� - 1�ʿ� �ѹ� ���� �ȴ�.
// 1�ʿ� 1�� ����
//===============================================================================
// #define PACKCODE_LCAS_STATE			(PACKET_DEVICE_ID_YLAS + 0x480)
//===============================================================================
typedef struct _LCAS_STATE_t
{
	unsigned char state;
// ���� �������� �̰ų� �������� ���� ���̾� 
// state�� run�̰ų� Sequence wait
	VIAL_POS_t sample; 
	VIAL_POS_t injectVial;	// �������� ���̾� - mix Program���� ���
	
	// ���� ���� ���� Ƚ�� - ������ ���� ���������� Ƚ��(�������� ��ô������ ���� ����).
	// ���̾˿��� �ε��� ��ô���� 
	int curInjCnt;				
	float fTemp;				// tray�� ���� �µ� 
	unsigned char tempReady;	// �µ� ���� ���� - ����µ��� ���������� ���� 0.1�� �̳��� 1���̻� ���� 
	unsigned char errorCode;  // ���� ���� ���� - SelfMsg���� 
// unsigned char bMixing;	// ����mixing ���ΰ�
// unsigned char mixTableNo;	// ���� �������� mixing table
} LCAS_STATE_t;


//===============================================================================
// #define PACKCODE_LCAS_ADJUST_DATA			(PACKET_DEVICE_ID_YLAS + 0x500)
//===============================================================================
// �����͸� �������� ���Ѵ�.
// Ȩ�������� �̵��ϸ�(+) / Ȩ�� �ݴ�� �̵�(-)
#define ADJUST_INJ_PORT_VALUE_X			0
#define ADJUST_INJ_PORT_VALUE_Y			1
#define ADJUST_INJ_PORT_VALUE_Z			2
#define ADJUST_SYRINGE_Z						3
#define ADJUST_VALVE_X							4
#define ADJUST_INJECTOR_X						5

#define DATA_SAVE_FLAG						0xA5
// Ȩ�������� �̵��ϸ�(+) / Ȩ�� �ݴ�� �̵�(-)
typedef struct _ADJUST_DEV_DATA_t
{
	unsigned char saveFlag[6];	// x,y,z,syr,val,inj
	signed char	value[6]; // Ȩ�������� �̵��ϸ�(+) / Ȩ�� �ݴ�� �̵�(-)
	
	unsigned char saveVL[3];		// ���̾� ��ǥ ������ ���忩�� 0:40vial ,1:60vial ,2:96vial - wellplate
	signed char leftVial_x[3];
	signed char leftVial_y[3];
	
	unsigned char saveVR[3];		// ���̾� ��ǥ ������ ���忩�� 0:40vial ,1:60vial ,2:96vial - wellplate	
	signed char rightVial_x[3];
	signed char rightVial_y[3];

	unsigned char saveWash;		// wash port , waste port ��ǥ ������ ���忩��
	signed char wash_x;
	signed char wash_y;

	unsigned char saveWaste;		// wash port , waste port ��ǥ ������ ���忩��	
	signed char waste_x;
	signed char waste_y;

//#if VIAL_HEIGHT_DATUM	// Board  Rev 3 // �ⱸ�� ���� 6
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
	// [SERVICE_ADJUST]�� ������ 
	// �������� ���� [PACKCODE_LCAS_ADJUST_DATA]�� �����ϰ�
	// ���������� ����� ���� [PACKCODE_LCAS_SLFEMSG]�� �����Ѵ�.
//unsigned char subCommand1;	// adjust :who(0~12)
	#define ADJUST_NEEDLE_XY			0				// ������ �� �ִ� ���� : x,y
	#define ADJUST_NEEDLE_HEIGHT	1				// ���������� ���� : x,y,z
	#define ADJUST_SYRINGE				2				// : z
	#define ADJUST_VALVE_POS			3				// : x(ccw,cw)
	#define ADJUST_INJECTOR_POS				4				// : x(ccw,cw)
	#define ADJUST_WASHING_PORT					5				// : x,y
	#define ADJUST_WASTE_PORT						6				// : x,y  -- Washing Port�� ���� 
	#define ADJUST_VIAL_LEFT						7				// : x,y
	#define ADJUST_VIAL_RIGHT						8				// : x,y
//#if VIAL_HEIGHT_DATUM	// Board  Rev 3 // �ⱸ�� ���� 6
	#define ADJUST_VIAL_HEIGHT					9				// : z -- vial ���� ���� 
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

	#define ADJUST_INIT_POS		13	// adjust �� �� �ֵ��� �غ� - ������ġ�� �̵�
	#define ADJUST_INIT_POS_END		14	// [ADJUST_INIT_POS]����� �ް� �ʱ�ȭ�� �Ϸ�Ǹ� PC�� �����Ѵ�.
	#define ADJUST_SAVE				15		// ������ ���� ����
	#define ADJUST_INIT_VALUE		16		// ������ �ʱ�ȭ
	#define ADJUST_QUIT					17		// Adjustment�� �Ϸ��Ѵ�.

//STATE_READY�����̰ų� STATE_DIAGNOSTICS���¿��� ���డ��
//STATE_DIAGNOSTICS�����϶��� 1�ʸ��� [PACKCODE_LCAS_DIAG_DATA]��Ŷ�� �����Ѵ�.
#define SERVICE_DIAGNOSTICS				2
	#define DIAGNOSTICS_NONE	(0)
	#define DIAGNOSTICS_STOP	(1)		//	���� Diagnostics ���� ����
	#define DIAGNOSTICS_QUIT	(2)		//	STATE_DIAGNOSTICS���¸� ����������.(STATE_READY���·� ��ȯ)
	#define DIAG_POWER_CHECK	(3)		//	�����˻� �� ���а� ������ ����
	#define DIAG_HEATER_CHECK	(4)		//	�����Ŀ� �ΰ��ϰ� �����ð����� ������ �µ� �˻�, ���üӵ� �˻�
	#define DIAG_COOLER_CHECK	(5)		//	�����Ŀ� �ΰ��ϰ� �����ð����� ������ �µ� �˻�, �𸵼ӵ� �˻�
	#define DIAG_SENSOR_CHECK	(6)		//	�����Ŀ� �ΰ��ϰ� �����ð����� ������ �µ� �˻�
	#define DIAG_LEAK_CHECK		(7)		//  Leak
	#define DIAG_PHOTO_CHECK	(8)		//  Photointerrupt Sensor

// [STATE_TEMP_CALIBRATION] ���·� �ȴ�.	
#define SERVICE_TEMP_CALIBRATION			3
// TEMP_CAL_START - �µ��� 35���� ����
// TEMP_CAL_TEMP_MEA - tempReady(1�ʸ��� ���۵Ǵ� ���°�)�� (1)�̵Ǹ� ����⿡�� Measure�� �Է¹޾� ����
// AutoSampler�� TEMP_CAL_TEMP_MEA�� ������ �µ��� 10���� ���� 
// TEMP_CAL_TEMP_MEA - tempReady�� (1)�̵Ǹ� ����⿡�� Measure�� �Է¹޾� ����
// AutoSampler�� save,reset,quit����� ���� 

//  unsigned char subCommand1 : 
	#define TEMP_CAL_NONE						0
	#define TEMP_CAL_LOAD						0
	#define TEMP_CAL_START					1	
	#define TEMP_CAL_TEMP_MEA1			2		// ������ ex)35.5  / 10.2
	#define TEMP_CAL_TEMP_MEA2			3		// ������ ex)35.5  / 10.2
	#define TEMP_CAL_SAVE						4		// ����
	#define TEMP_CAL_RESET					5		// Factor���� 1�� 
	#define TEMP_CAL_QUIT						6		// STATE_READY ���·�
	// subCommand2  - �����µ� , �����µ�

	#define TEMP_CAL_READY1					7		// 35�� Ready
	#define TEMP_CAL_READY2					8 	// 10�� Ready 


	#define TEMP_CAL_SET_1ST					35
	#define TEMP_CAL_SET_2ND					10

	#define TEMP_CAL_ERROR_MIN				-10
	#define TEMP_CAL_ERROR_MAX				10

/*
#define SERVICE_SYRINGE_EXCHANGE				7	
#define SERVICE_NDL_SYR_EXCHANGE				4			
	#define NDL_SYR_EXCHANGE_NONE			0		
	#define NDL_SYR_EXCHANGE_START		1		// �ϵ� �ø����� ��ü�� �� �ֵ��� ��ü ������ġ�� �̵� 	
	#define NDL_SYR_EXCHANGE_QUIT			2		// Ȩüũ�� Ȩ��ġ�� �̵� 		
*/
#define SERVICE_SYRINGE_REPLACE				7	
#define SERVICE_NEEDLE_REPLACE				4			
	#define NDL_SYR_REPLACE_NONE			0		
	#define NDL_SYR_REPLACE_START		1		// �ϵ� �ø����� ��ü�� �� �ֵ��� ��ü ������ġ�� �̵� 	
	#define NDL_SYR_REPLACE_QUIT			2		// Ȩüũ�� Ȩ��ġ�� �̵� 		


#define SERVICE_WASH_WASTE				5				
	#define WASH_WASTE_NEEDLE_INT			0
	#define WASH_WASTE_NEEDLE_ALL			1	
	#define WASH_WASTE_INJECTOR				2		
	// subCommand2  - ����Ƚ��		(1~10)



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
// ����(ACK)
//===============================================================================
// ADJUST Action
// 1. Adjust Mode�� ������ ���(������÷�)�� ADJUST_DEV_DATA_t �������� ��û�Ѵ�.
// 2. �����ϰ��� �ϴ� �κ��� �ʱ�ȭ ��ȣ�� ������.
// 	command : SERVICE_ADJUST
// 	subCommand1 : ADJUST_NEEDLE_XY
//  subCommand2 : ADJUST_INIT_POS
		// ---> ���� �ʱ�ȭ �Ϸ�Ǹ� ��⿡�� �Ϸ��ȣ�� ������.
		// 	command : SERVICE_ADJUST
		// 	subCommand1 : ADJUST_NEEDLE_XY
		//  subCommand2 : ADJUST_INIT_POS_END
// 3. �ⱸ�� ���� ����� ������.
// 	command : SERVICE_ADJUST
// 	subCommand1 : ADJUST_NEEDLE_XY
//  subCommand2 : ADJUST_LEFT
// 4. ������ �Ϸ�Ǹ� ����(���/...) ��ȣ�� ������.
// 	command : SERVICE_ADJUST
// 	subCommand1 : ADJUST_NEEDLE_XY
//  subCommand2 : ADJUST_SAVE / 

typedef struct _LCAS_SERVICE_t
{
  unsigned char command;
  unsigned char subCommand1;
  unsigned char subCommand2;

	float value;	// �߰� [20170405]
} LCAS_SERVICE_t;

// unsigned char btMessage;
#define SELF_STATE				1	// ������.
#define SELF_ERROR				2
	// ���۹��� ����� �˼� ���� ����̰ų� �������� ������ ��� ���-SelfMsg���� ���� AutoSampler�� ������ ���� �����Ѵ�
	#define SELF_NO_ERROR			0
	#define SELF_ERROR_COMMAND_UNKNOWN			1
	// adjustment ���������� ����. - ������ ���� �� �ʱ�ȭ�ϱ� ���� üũ�Ͽ� �����Ѵ�.
	// adjustment�� �� �ѹ��� �����ؾ� �Ѵ�.
	#define SELF_ERROR_NO_ADJUSTMENT_DATA		2	
	// missing vial - ������ ���̾��� �������� ���ϸ� �޽����� �����Ѵ�.
	#define SELF_ERROR_MISSING_VIAL_SKIP		3		// skipMissingSample �������� (1) �� �� �߻�
	#define SELF_ERROR_MISSING_VIAL_WAIT		4		// skipMissingSample �������� (0) �� �� �߻� - SELF_USER_RESPONSE (������� ����� �Է��� ��ٸ���.)
	#define SELF_ERROR_OPERATION						5 	// �ⱸ�� ����
	#define SELF_ERROR_DOOR_OPEN	(6) // �����
	#define SELF_ERROR_LEAK			(7) // Leak
	#define SELF_ERROR_TEMPERATURE	(8) // �µ� �̻�

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

	
#define SELF_START_LOADING			3	// loading�۾� ���� // 0: PC���  1: Extin(�ܺ��Է�) 2: �ݺ��м��ڵ� ���� 
	#define SELF_START_LOADING_PCIN			0
	#define SELF_START_LOADING_EXTIN		1
	#define SELF_START_LOADING_AUTO			2

#define SELF_START_INJECTION		4	//injection 
#define SELF_EXTOUT					5 // �ܺδ��� ���
#define SELF_STOP					6	// [COMMAND_STOP] ����� ������ �� Stop�� ����

#define SELF_COMMAND_REJECT		7		// ���� ��� �ź� , ���۵� ����� ������ �� ���� ������ �� ���۵�.
	#define ADJUST_VALUE_ERROR_0		1
	#define ADJUST_VALUE_ERROR_1		2

#define SELF_END_SEQUENCE			8		// ������ ���� ���� �Ϸ�

#define SELF_USER_RESPONSE		9	// ����⿡�� �����ؾ���(����⿡�� AutoSampler�� ����). - Missing Vial ����� ���ÿ� ���� ����
	#define MISSING_VIAL_NO_RESPONSE	0		// 
	#define MISSING_VIAL_SKIP				1		// Vial Skip
	#define MISSING_VIAL_RETRY			2		// Vial üũ �� ���԰��� �ٽ� �õ� 
	#define MISSING_VIAL_ABORT			3		// STOP	- Sequence �ߴ� , ���� ready���·�
	#define MISSING_VIAL_STOP				MISSING_VIAL_ABORT
	
#define SELF_UPDATE				10	// unsigned char btNewValue;--> 0: ��������Ʈ�ʿ� 1:SW������Ʈ�ʿ� 2:���� 3:����
#define SELF_ABORT				11	// [COMMAND_ABORT] ����� ������ �� ��ý���[Abort,Stop]�� ����

//===============================================================================
// #define PACKCODE_LCAS_SLFEMSG			(PACKET_DEVICE_ID_YLAS + 0x490)
//===============================================================================
// PC���� START(STOP)��� ������ ����(ACK)���Ѵ�. Selfmsg�Ⱥ���
// ����Ʈ ��ŸƮ�� ����  Selfmsg����
typedef struct _LCAS_SELFMSG_t
{
// ���°� ���� �� ���۵ȴ�.
	unsigned char ucMessage;
	unsigned char ucNewValue;
} LCAS_SELFMSG_t;


//===============================================================================
// #define PACKCODE_LCAS_DIAG_DATA						(PACKET_DEVICE_ID_YLAS + 0x520)
//===============================================================================
// ����üũ - �������� üũ�� ����
//[STATE_DIAGNOSTICS]�����϶��� 1�ʸ��� [PACKCODE_LCAS_DIAG_DATA]��Ŷ�� �����Ѵ�.
#define DIAG_RESULT_NONE		0
#define DIAG_RESULT_PASS		1
#define DIAG_RESULT_FAIL		2
//#define DIAG_RESULT_PROCESSING	3
//#define DIAG_RESULT_REJECT	4

// ���� ��û�� ���Ͽ� ������ �Ϸ�Ǹ� ��� ���� ���� - ������ �ѹ��� �����Ѵ�.
typedef struct _LCAS_DIAG_DATA {
	unsigned char Current;	// ���� �������� �˻� 
	
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
	
	unsigned char Tempcheck;		// �µ�	
	unsigned char btHeaterTest;		// ����(��Ƽ��) �˻�, 30������ 40������ ��½ð� �˻�
	unsigned char btCoolerTest;		// ��(��Ƽ��) �˻�, 40������ 20������ �ϰ��ð� �˻�
	
	unsigned char btLeakTest1;		// leak check
	unsigned char btLeakTest2;		// leak check

	unsigned char btTempSensor;		// �µ� ���� - ����(����)�� �˻� �� ���� ��ȭ�� �˻�,
									// �����ð����� ���� �Ŀ� ���װ� ��ȭ �˻�
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
	float constB;		// x�� ���
	float constC;		// ����(���)
	
//	LcasState.fTemp = tempConst_B * uAdcSum + tempConst_C;
//	LcasState.fTemp = (tempConst_B + tempCalData.constB) * uAdcSum + (tempConst_C + tempCalData.constC);
	
} LCAS_TEMP_CAL_t;

//===============================================================================
// #define PACKCODE_LCAS_SPECIAL 						(PACKET_DEVICE_ID_YLAS + (0x620)	// - �߰�(20170914)
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
	unsigned int time;	// ��� ���ð� (1min)
	unsigned int runTime;	// ��� ���ð� - run���� (1min)
	unsigned int heatTime;	// Tray Cooling & heating �ð� (1min)

// ���� �� Washing�� ������ 	
//	unsigned int needle;	// �̵��Ÿ� mm
	unsigned int x;	// �̵��Ÿ� mm	--> meter�� ����
	unsigned int y;	// �̵��Ÿ� mm	
	unsigned int z;	// �̵��Ÿ� mm		
	
	unsigned int syringe;	// �̵��Ÿ� mm
	unsigned int valve;		// �̵� ��Ʈ �� 
	unsigned int injector;	// ������ ���� ��
	unsigned int injectionCnt;	// ������Ƚ�� - injection port ���Ƚ��
} LCAS_USED_TIME_t;

typedef struct _LCAS_USER_DEFINE_CONTROL {

} LCAS_USER_DEFINE_CONTROL_t;



typedef struct _ETHERNET_CONFIG_t
{
	char cModifyChar[9];			// ex) "IPADDR"
	
	char cFirmwareVersion[9];			// ����		 ex) "1.1.3"
	char cMachineVersion[9];			// ����		 ex) "1.3.8"

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

