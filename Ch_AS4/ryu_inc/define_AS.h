#ifndef DEFINE_AS_H_
#define DEFINE_AS_H_

#if QT_PROG_ADJUST
#else
#include "Chrozen_AS_hw_platform.h"		//#include "LCAS_SOC_2016_hw_platform.h"
#endif

#include "option_AS.h"

#if 1 // 20210429
// 수정하지 말것 - 네트웍 연결안됨.
#define SYSTEM_MODEL				"YI9650"
#define CONNECT_CHECK_MODEL				"YL9152 LC AutoSampler"
#else
#define SYSTEM_MODEL				"YL9152 LC AutoSampler"
#endif

// 20181012 - Adjust data 백업을 위해서 새롭게
#define HOME_ADJUST_VERSION_CHECK_DATA	0x7234AA55
#define	TEST_EEPROM											0
#define	TEST_EEPROM_VERIFY							0

#define MECH_VER				7 // 8 //	6 // 5  // 3 - 높은 것  // 2 - 내자리
#define BOARD_REV				3 // 2018
//#define BOARD_REV				4 // 20200422

#if MECH_VER == 6	// Board  Rev 3 // 기구물 변경 6 - 맨처음 출시된 제품
//	#define NDEBUG_DP
// 20191228
	#define FIRMWARE_VERSION		" 1.1.3"	// 맨앞은 space
	#define MACHINE_VER		" 1.3.6"				// 3: Board   6: Machine
#elif MECH_VER == 7// Board  Rev 3 // 기구물 변경 7
//	#define NDEBUG_DP
// 기구물 변경 7 - 20191223
// 20191228
//	#define FIRMWARE_VERSION		" 2.3.7"	// 맨앞은 space
// 20200109 - Board  Rev 3 / 기구물 변경 8 - Cooling / Y / ... / Vial Height

	#if NEW_BOARD_ENCODER_2020
// Eencoder 추가  // 20200814
//		#define FIRMWARE_VERSION		" 3.4.9"	// 맨앞은 space
// 20201204				// z축 속도 변경 
//		#define FIRMWARE_VERSION		" 3.4.12"	// 맨앞은 space
// 20210125				// ChroZen AS : RGB LED 적용
//		#define FIRMWARE_VERSION		" 4.0.1"	// 맨앞은 space
// 20210429				// ChroZen AS : RGB LED 적용 - 출고용
//		#define FIRMWARE_VERSION		" 5.04"	// 맨앞은 space
//		#define MACHINE_VER		" 5.04"
// 20210601
// init command delay 줄임 : 딜레이가 길어서 패킷처리를 못하는 형태인 듯 
//		#define FIRMWARE_VERSION		" 5.05"	// 맨앞은 space
//		#define MACHINE_VER		" 5.05"
// 20210609
//#if POZ_UPPER_DETECT_ERROR_DELAY 
//		#define FIRMWARE_VERSION		" 5.06"	// 맨앞은 space
//		#define MACHINE_VER		" 5.06"
// 20210709
// KCJ -TCPIP
//		#define FIRMWARE_VERSION		" 5.10"	// 맨앞은 space
//		#define MACHINE_VER		" 5.10"
// 20210723
// KCJ - TCPIP 
//		#define FIRMWARE_VERSION		" 5.11"	// 맨앞은 space
//		#define MACHINE_VER		" 5.11"
// 20210730
// goPosETC_Z_check() 수정 - error_16
		#define FIRMWARE_VERSION		" 5.12"	// 맨앞은 space
		#define MACHINE_VER		" 5.12"

	#else
	// 20200513 - Board  Rev 3 & 4
		#define FIRMWARE_VERSION		" 3.3.8"	// 맨앞은 space
		#define MACHINE_VER		" 3.3.8"				// 2: firmware 3: Board   8: Machine
	#endif	
#endif

// 이동 
#define SYSTEM_DISP_MODEL		"YL9150 Plus LC AutoSampler"
#define SYSTEM_VERSION			MACHINE_VER








//########################################################################################################
#define FPGA_BASE_ADDR   		(0x50000000U)		// #define COREMEMCTRL_0     0x50000000U
//########################################################################################################
#define COMMON_CONTROL						0x0000U			// sel[14:12] = "000"
#define LED_SWITCH								0x1000U			// sel[14:12] = "001"
#define TRXM_CONTROL							0x2000U			// sel[14:12] = "010"
#define ADC_IF_MUX								0x3000U			// sel[14:12] = "011"
//#define STEPCONTROL18_0						0x4000U			// sel[14:12] = "100"
//#define STEPCONTROL18_1						0x5000U			// sel[14:12] = "101"
//#define STEPCONTROL13_0						0x6000U			// sel[14:12] = "110"
#define STEPCONTROL18_0						(FPGA_BASE_ADDR + (0x4000U << 2))			// sel[14:12] = "100"
#define STEPCONTROL18_1						(FPGA_BASE_ADDR + (0x5000U	<< 2))		// sel[14:12] = "101"
#define STEPCONTROL13_0						(FPGA_BASE_ADDR + (0x6000U	<< 2))		// sel[14:12] = "110"
#define PWM_CON8_0								0x7000U			// sel[14:12] = "111"
//########################################################################################################


//================================================================================
#define COMMON_CTRL_BASE_ADDR 				COMMON_CONTROL
// write
	#define  W1_START_CONFIG 				(COMMON_CONTROL + 0x00000001)	//					(COMMON_CTRL_BASE_ADDR + (0x00000001 << 2))
	//	0 : Rising Edge Detection
	//	1 : Falling Edge Detection

	#define  W2_LED_EXT_CON 				(COMMON_CONTROL + 0x00000002)	//					(COMMON_CTRL_BASE_ADDR + (0x00000002 << 2))
	#define  W3_BUZZER_CON	 				(COMMON_CONTROL + 0x00000003)	//					(COMMON_CTRL_BASE_ADDR + (0x00000003 << 2))
	#define  W4_MOTOR_POW_CON	 			(COMMON_CONTROL + 0x00000004)	//					(COMMON_CTRL_BASE_ADDR + (0x00000004 << 2))
	#define  W5_REMOTE_OUT	 				(COMMON_CONTROL + 0x00000005)	//					(COMMON_CTRL_BASE_ADDR + (0x00000005 << 2))
	#define  W6_START_OUT	 					(COMMON_CONTROL + 0x00000006)	//					(COMMON_CTRL_BASE_ADDR + (0x00000006 << 2))

//	#define  W10_ENABLE_STARTOUT			0x00000010	//		 		(COMMON_CTRL_BASE_ADDR + (0x00000010 << 2))
//	#define  W11_STARTOUT_PULSETIME_DATA	0x00000011	// 		(COMMON_CTRL_BASE_ADDR + (0x00000011 << 2))
// read
	#define  R0_FPGA_VER 					(COMMON_CONTROL + 0x00000000)	
	#define  R6_SIGNAL_IN 				(COMMON_CONTROL + 0x00000006)	//	[bit2: Power button] / [bit1: remote start] / [bit0:remote ready]
		

//================================================================================
#define LED_SWITCH_CTRL_BASE_ADDR 				LED_SWITCH
// write
	#define  W0_POWER_BTN 					(LED_SWITCH + 0x00000000)	//				(LED_SWITCH_CTRL_BASE_ADDR + (0x00000000 << 2))
		#define 	POW_BTN_EN					(0)			// bit
		#define		POW_CON_5V_IN 			(3)			// bit
	#define  T1_POW_BTN_DET_CLR 		(LED_SWITCH + 0x00000001)	// Touch
	#define  W2_POWER_BTN_TIME 			(LED_SWITCH + 0x00000002)
		#define		ST_ON_BTN_DET_TIME_DEFAULT			0x012C						// 300 x 10ms
//		#define 	ST_OFF_SEQUENCE_TIME_DEFAULT		0x02BC						// 500 x 10ms
		#define 	ST_OFF_SEQUENCE_TIME_DEFAULT		0x012C						// 300 x 10ms
		#define 	POWER_OFF_SEQUENCE_TIME					(ST_OFF_SEQUENCE_TIME_DEFAULT /	20)					// 500msec 단위
		#define 	BUZZER_OFF_SEQUENCE_TIME				1					// 500msec				// CheckDeviceState()
		
	#define  T5_FLED_CONFIG_LOAD 		(LED_SWITCH + 0x00000005)	// Touch
	#define  W6_FLED_COLOR_TIME1		(LED_SWITCH + 0x00000006)
		#define 	FLED_COLOR1					0xFF0000			// 0xff0000						// Color 설정 ( [23:16]Green / [15:8] Red / [7:0] Blue
//		#define 	FLED_COLOR1					0x800040									// Color 설정 ( [23:16]Green / [15:8] Red / [7:0] Blue
		#define 	FLED_COLOR_TIME1				4										// 유지 시간 설정 : (100mSec 단위) : 100msec x 10 = 1sec
	#define  W7_FLED_COLOR_TIME2		(LED_SWITCH + 0x00000007)
		#define 	FLED_COLOR2					0xFF0000				//0x00ff00
		#define 	FLED_COLOR_TIME2				5										// 100msec x 10 = 1sec
	#define  W8_FLED_COLOR_TIME3		(LED_SWITCH + 0x00000008)
		#define 	FLED_COLOR3					0xFF0000			//	0x0000ff
		#define 	FLED_COLOR_TIME3				1										// 100msec x 10 = 1sec


//================================================================================
#define PWM_CON_BASE_ADDR 				PWM_CON8_0
/*
	#define  S1_PWM1_OFFSET 		(PWM_CON8_0 + 0x00000000)		// PADDR[4:2]		parameter S1_PWM1 = 3'b000; --> b0 0000
	#define  S2_PWM2_OFFSET 		(PWM_CON8_0 + 0x00000004)		// PADDR[4:2]		parameter S2_PWM2 = 3'b001; --> b0 0100
	#define  S3_PWM3_OFFSET 		(PWM_CON8_0 + 0x00000008)		// PADDR[4:2]		parameter S3_PWM3 = 3'b010; --> b0 1000
	#define  S4_PWM4_OFFSET 		(PWM_CON8_0 + 0x0000000C)		// PADDR[4:2]		parameter S4_PWM4 = 3'b011; --> b0 1100
	#define  S5_CTRL_OFFSET 		(PWM_CON8_0 + 0x00000010)		// PADDR[4:2]		parameter S4_PWM4 = 3'b011; --> b1 0000

	#define S5_FAN_ON_OFF
	#define S6_HEAT_COOL
	#define S7_ON_OFF
	#define S8_PUMP_ON_OFF
*/

	#define  W0_PWM1_OFFSET 		(PWM_CON8_0 + 0x00000000)
	#define  W1_PWM2_OFFSET 		(PWM_CON8_0 + 0x00000001)
	#define  W2_PWM3_OFFSET 		(PWM_CON8_0 + 0x00000002)
	#define  W3_PWM4_OFFSET 		(PWM_CON8_0 + 0x00000003)
//	#define  W4_CTRL_OFFSET 		0x00000004
	#define  W4_FAN_ON_OFF 			(PWM_CON8_0 + 0x00000004)
	#define  W5_HEAT_COOL				(PWM_CON8_0 + 0x00000005)
	#define  W6_ON_OFF					(PWM_CON8_0 + 0x00000006)
	#define  W7_PUMP_ON_OFF			(PWM_CON8_0 + 0x00000007)





#if FPGA_VERSION_TRIM
// 모터제어포트 순서를 바꾼다.
//#define  TWOPORTRAM_BASEADDR_MOTOR_X			STEPCONTROL18_0
//#define  TWOPORTRAM_BASEADDR_MOTOR_Y			STEPCONTROL18_1
//#define  TWOPORTRAM_BASEADDR_MOTOR_Z			STEPCONTROL18_2
//#define  TWOPORTRAM_BASEADDR_MOTOR_SYR		STEPCONTROL18_3
//#define  TWOPORTRAM_BASEADDR_MOTOR_VAL		STEPCONTROL13_0
//#define  TWOPORTRAM_BASEADDR_MOTOR_INJ		STEPCONTROL13_1

#define  TWOPORTRAM_BASEADDR_MOTOR_Z			STEPCONTROL18_0
#define  TWOPORTRAM_BASEADDR_MOTOR_Y			STEPCONTROL18_1
#define  TWOPORTRAM_BASEADDR_MOTOR_X			STEPCONTROL18_0
#define  TWOPORTRAM_BASEADDR_MOTOR_SYR		STEPCONTROL18_1
#define  TWOPORTRAM_BASEADDR_MOTOR_VAL		STEPCONTROL13_0
#define  TWOPORTRAM_BASEADDR_MOTOR_INJ		STEPCONTROL13_0

	#define  BASEADDR_MOTOR_X_Z					STEPCONTROL18_0
	#define  BASEADDR_MOTOR_Y_SYR				STEPCONTROL18_1
	#define  BASEADDR_MOTOR_VAL_INJ			STEPCONTROL13_0


	#define  BASEADDR_MOTOR_X					STEPCONTROL18_0
	#define  BASEADDR_MOTOR_Y					STEPCONTROL18_1
	#define  BASEADDR_MOTOR_Z					STEPCONTROL18_0
	#define  BASEADDR_MOTOR_SYR				STEPCONTROL18_1
	#define  BASEADDR_MOTOR_VAL				STEPCONTROL13_0
	#define  BASEADDR_MOTOR_INJ				STEPCONTROL13_0


//=============================================================================================
// BASEADDR_MOTOR
#if 1
// 0 ~ 149 : Ram Data - Step data
#if 1   // SF2
	#define  ACCEL_OFFSET 		(0x000003C0	<< 2)	// PADDR[9:3]  7'b1111000;  => b11 1100 0000 => 3C0
	#define  DEACC_OFFSET 		(0x000003C8	<< 2)		// PADDR[9:3]  7'b1111001;  => b11 1100 1000 => 3C8
	#define  TOTAL_OFFSET 		(0x000003D0	<< 2)		// PADDR[9:3]  7'b1111010;  => b11 1101 0000 => 3D0
	#define  START_SPEED  		(0x000003D8	<< 2)	// PADDR[9:3]  7'b1111011;  => b11 1101 1000 => 3D8
	#define  CTRL_PORT 		  	(0x000003E0	<< 2)		// PADDR[9:3]  7'b1111100;  => b11 1110 0000 => 3E0

	#define  S5_SEL_MOTOR  		(0x000003E8	<< 2)		// PADDR[9:3]  7'b1111101;  => b11 1110 1000 => 3E8
	#define  S6_COUNT_CTRL 		(0x000003F0	<< 2)		// PADDR[9:3]  7'b1111110;  => b11 1111 0000 => 3F0
	#define  S7_SEL_STATE			(0x000003F8	<< 2)		// 						 7'b1111111;	=> b11 1111 1000 => 3F8
#else
	#define  ACCEL_OFFSET 		0x000003C0		// PADDR[9:3]  7'b1111000;  => b11 1100 0000 => 3C0
	#define  DEACC_OFFSET 		0x000003C8		// PADDR[9:3]  7'b1111001;  => b11 1100 1000 => 3C8
	#define  TOTAL_OFFSET 		0x000003D0		// PADDR[9:3]  7'b1111010;  => b11 1101 0000 => 3D0
	#define  START_SPEED  		0x000003D8		// PADDR[9:3]  7'b1111011;  => b11 1101 1000 => 3D8
	#define  CTRL_PORT 		  	0x000003E0		// PADDR[9:3]  7'b1111100;  => b11 1110 0000 => 3E0

	#define  S5_SEL_MOTOR  		0x000003E8		// PADDR[9:3]  7'b1111101;  => b11 1110 1000 => 3E8
	#define  S6_COUNT_CTRL 		0x000003F0		// PADDR[9:3]  7'b1111110;  => b11 1111 0000 => 3F0
	#define  S7_SEL_STATE			0x000003F8		// 						 7'b1111111;	=> b11 1111 1000 => 3F8
#endif
#else
	#define  ACCEL_OFFSET 		0x00000078		// PADDR[9:3]  7'b1111000;  => b11 1100 0000 => 3C0
	#define  DEACC_OFFSET 		0x00000079		// PADDR[9:3]  7'b1111001;  => b11 1100 1000 => 3C8
	#define  TOTAL_OFFSET 		0x0000007a		// PADDR[9:3]  7'b1111010;  => b11 1101 0000 => 3D0
	#define  START_SPEED  		0x0000007b		// PADDR[9:3]  7'b1111011;  => b11 1101 1000 => 3D8
	#define  CTRL_PORT 		  	0x0000007c		// PADDR[9:3]  7'b1111100;  => b11 1110 0000 => 3E0

	#define  S5_SEL_MOTOR  		0x0000007d		// PADDR[9:3]  7'b1111101;  => b11 1110 1000 => 3E8
	#define  S6_COUNT_CTRL 		0x0000007e		// PADDR[9:3]  7'b1111110;  => b11 1111 0000 => 3F0
	#define  S7_SEL_STATE			0x0000007f		// 						 7'b1111111;	=> b11 1111 1000 => 3F8
#endif
//=============================================================================================

//	#define STEPCONTROL18_0						0x4000U			// sel[14:12] = "100"
//	#define  BASEADDR_MOTOR_X_Z					STEPCONTROL18_0

//	#define  CTRL_PORT_ADDR_MOTOR_X_Z				( FPGA_BASE_ADDR + ((BASEADDR_MOTOR_X_Z + CTRL_PORT) << 2) )

	
//=============================================================================================
	#define  CTRL_PORT_ADDR_MOTOR_X_Z				(BASEADDR_MOTOR_X_Z + CTRL_PORT)
	#define  CTRL_PORT_ADDR_MOTOR_Y_SYR			(BASEADDR_MOTOR_Y_SYR + CTRL_PORT)
//	#define  CTRL_PORT_ADDR_MOTOR_Z				(BASEADDR_MOTOR_Z + CTRL_PORT)
//	#define  CTRL_PORT_ADDR_MOTOR_SYR			(BASEADDR_MOTOR_SYR + CTRL_PORT)
	#define  CTRL_PORT_ADDR_MOTOR_VAL_INJ			(BASEADDR_MOTOR_VAL_INJ + CTRL_PORT)
//	#define  CTRL_PORT_ADDR_MOTOR_INJ			(BASEADDR_MOTOR_INJ + CTRL_PORT)

//=============================================================================================
	#define  SEL_MOTOR_ADDR_MOTOR_X_Z				(BASEADDR_MOTOR_X_Z + S5_SEL_MOTOR)
	#define  SEL_MOTOR_ADDR_MOTOR_Y_SYR			(BASEADDR_MOTOR_Y_SYR + S5_SEL_MOTOR)
	#define  SEL_MOTOR_ADDR_MOTOR_VAL_INJ		(BASEADDR_MOTOR_VAL_INJ + S5_SEL_MOTOR)

	#define  ENCODER_CTRL_ADDR_MOTOR_X_Z				(BASEADDR_MOTOR_X_Z + S6_COUNT_CTRL)
	#define  ENCODER_CTRL_ADDR_MOTOR_Y_SYR			(BASEADDR_MOTOR_Y_SYR + S6_COUNT_CTRL)
	#define  ENCODER_CTRL_ADDR_MOTOR_VAL_INJ		(BASEADDR_MOTOR_VAL_INJ + S6_COUNT_CTRL)

	#define  SEL_STATE_ADDR_MOTOR_X_Z				(BASEADDR_MOTOR_X_Z + S7_SEL_STATE)
	#define  SEL_STATE_ADDR_MOTOR_Y_SYR			(BASEADDR_MOTOR_Y_SYR + S7_SEL_STATE)
	#define  SEL_STATE_ADDR_MOTOR_VAL_INJ		(BASEADDR_MOTOR_VAL_INJ + S7_SEL_STATE)

//=============================================================================================
//bit3 ~ bit1 -> 0 ~ 5 : x,y,z,syr,val,inj
//bit4 -> 1 : hold0
//bit5 -> 1 : hold1

	#define CMD_SEL_MOTOR(WHO)	*((uint32_t volatile *)(devSet.selMotorAddr[WHO])) = WHO;
/*
	#define CMD_SEL_MOTOR_X			*((uint32_t volatile *)(SEL_MOTOR_ADDR_MOTOR_X_Z)) = 0x0
	#define CMD_SEL_MOTOR_Z			*((uint32_t volatile *)(SEL_MOTOR_ADDR_MOTOR_X_Z)) = 0x1
	#define CMD_SEL_MOTOR_Y			*((uint32_t volatile *)(SEL_MOTOR_ADDR_MOTOR_Y_SYR)) = 0x0
	#define CMD_SEL_MOTOR_SYR			*((uint32_t volatile *)(SEL_MOTOR_ADDR_MOTOR_Y_SYR)) = 0x1
	#define CMD_SEL_MOTOR_VAL			*((uint32_t volatile *)(SEL_MOTOR_ADDR_MOTOR_VAL_INJ)) = 0x0
	#define CMD_SEL_MOTOR_INJ			*((uint32_t volatile *)(SEL_MOTOR_ADDR_MOTOR_VAL_INJ)) = 0x1
*/

//	#define CMD_CTRL_PORT(WHO, CMD)	*((uint32_t volatile *)(devSet.ctrlPortAddr[WHO])) = (CMD);






//=============================================================================================
//bit0 -> 1 : Clear (1->0)
//bit1 -> 1 : hold
//bit2 -> 1 : enable
//삭제 bit3 -> 0 : up / 1 : Down
	#define CMD_ENCODER_CLR_CNT(WHO)		{	*((uint32_t volatile *)(devSet.encoderCtrlAddr[WHO])) = 0x01; *((uint32_t volatile *)(devSet.encoderCtrlAddr[WHO])) = 0x0;}
/*
	#define CMD_ENCODER_CLR_CNT_X_Z				*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_X_Z)) = 0x1; *((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_X_Z)) = 0x0
	#define CMD_ENCODER_CLR_CNT_Y_SYR			*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_Y_SYR)) = 0x1; *((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_Y_SYR)) = 0x0
	#define CMD_ENCODER_CLR_CNT_VAL_INJ		*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_VAL_INJ)) = 0x1; *((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_VAL_INJ)) = 0x0
*/

// clear 후 enable
	#define CMD_ENCODER_CLR_CNT_ENABLE(WHO)		{	*((uint32_t volatile *)(devSet.encoderCtrlAddr[WHO])) = 0x01; *((uint32_t volatile *)(devSet.encoderCtrlAddr[WHO])) = 0x4;}


	#define CMD_ENCODER_DISABLE(WHO)		{	*((uint32_t volatile *)(devSet.encoderCtrlAddr[WHO])) = 0x00;}
	#define CMD_ENCODER_ENABLE(WHO)		{	*((uint32_t volatile *)(devSet.encoderCtrlAddr[WHO])) = 0x04;}
	#define CMD_ENCODER_HOLD_CNT(WHO)		{	*((uint32_t volatile *)(devSet.encoderCtrlAddr[WHO])) = 0x06;}
	#define CMD_ENCODER_ACT_CNT(WHO)		{	*((uint32_t volatile *)(devSet.encoderCtrlAddr[WHO])) = 0x04;}

// 	110b (enable / hold / clear)
	#define CMD_ENCODER_HOLD_CNT_X_Z			*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_X_Z)) = 0x6
	#define CMD_ENCODER_ACT_CNT_X_Z				*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_X_Z)) = 0x4
	#define CMD_ENCODER_HOLD_CNT_Y_SYR		*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_Y_SYR)) = 0x6
	#define CMD_ENCODER_ACT_CNT_Y_SYR			*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_Y_SYR)) = 0x4
	#define CMD_ENCODER_HOLD_CNT_VAL_INJ	*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_VAL_INJ)) = 0x6
	#define CMD_ENCODER_ACT_CNT_VAL_INJ		*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_VAL_INJ)) = 0x4

	#define CMD_ENCODER_ENABLE_X_Z				*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_X_Z)) = 0x4
	#define CMD_ENCODER_DISABLE_X_Z				*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_X_Z)) = 0x0
	#define CMD_ENCODER_ENABLE_Y					*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_Y_SYR)) = 0x4
	#define CMD_ENCODER_DISABLE_SYR				*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_Y_SYR)) = 0x0
	#define CMD_ENCODER_ENABLE_VAL_INJ		*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_VAL_INJ)) = 0x4
	#define CMD_ENCODER_DISABLE_VAL_INJ		*((uint32_t volatile *)(ENCODER_CTRL_ADDR_MOTOR_VAL_INJ)) = 0x0

	#define CMD_SEL_STATE(WHO)	*((uint32_t volatile *)(devSet.selStateAddr[WHO])) = WHO;

#else
#endif


#if 0
	#define  READ_RAM_ADDR  	0x000003E8		// PADDR[9:3]  7'b1111101;  => b11 1110 1000 => 3E8
	#define  TEST_RAM_CON   	0x000003F0		// PADDR[9:3]  7'b1111110;  => b11 1110 1000 => 3F0

	#define	ISO_TOTAL_OFFSET	0x000003D0		// PADDR[9:3]  7'b1111010;  => b11 1101 0000 => 3D0
	#define	ISO_SET_SPEED			0x000003D8		// PADDR[9:3]  7'b1111011;  => b11 1101 1000 => 3D8
	#define	ISO_CTRL_PORT 		0x000003E0		// PADDR[9:3]  7'b1111100;  => b11 1110 0000 => 3E0
#endif






// ex)
	//(*((uint32_t volatile *)(FABRIC_STEPSET_APB20_BASE_ADDR + CTRL_PORT)) = ctrl_port);

//core_gpio.h -- #include "cpu_types.h" 에서 자꾸에러가 난다.
// 헤더 파일 적용 순서를 바꾼다. - property의 설정에서 헤더 파일순서
// ${workspace_loc:/Webserver_MSS_CM3_0_hw_platform/hal}
// ${workspace_loc:/Webserver_MSS_CM3_0_hw_platform/hal/CortexM3}
// ${workspace_loc:/Webserver_MSS_CM3_0_hw_platform/hal/CortexM3/GNU} 뒤로 이동

// 중복선언
// addr_t







//    static
extern unsigned char bInitTicks;

// Simple periodic polling function
#define PERIODIC_FUNCTION(var,time,function,err) \
		get_System_ticks(); \
    if((sysTicks - var) > time) \
    {                               \
        var += time;                \
        err = function;             \
    }

#if SOC_TYPE_SF2
#define NEXT_CASE_DELAY_INTERVAL		300
#define NEXT_CASE_DELAY_RUN					500
#define STEP_DELAY_INTERVAL		10			// 10ms
#define WAIT_AFTER_INJECTION				5000			// 5초

#define WAIT_HOME_FIND_INTERVAL_100MS		100
#define WAIT_HOME_FIND_INTERVAL_200MS		200	//2000
#define WAIT_HOME_FIND_INTERVAL		WAIT_HOME_FIND_INTERVAL_200MS

#define WAIT_MIX_PROGRAM				500			// 0.5초
#define WAIT_MICRO_PUMP_START				500			// 0.5초

#define MICRO_PUMP_TIME_CONVERSION	1000		// 초단위로변환 

#define WAKEUP_DELAY_TIME			500		// 0.5초

#define CONVERT_WAIT_TIME_TO_COUNT			1000		//  time(sec) to time tick

#define MOVE_XY_DELAY_TIME		200		// 0.2초

#define DELAY_TIME_TICK_COUNT_0_1_SEC		100		// 0.1초
#define DELAY_TIME_TICK_COUNT_2_SEC		2000		// 2초

//#define DELAY_Z_ERROR_RECHECK_TIME		200		// 200ms
#define DELAY_Z_ERROR_RECHECK_TIME		500		// 500ms  - 20210730

#define HOME_FINE_CHECK_MAX_TIME_X		7000	// 7초 홈 찾기 
#define HOME_FINE_CHECK_MAX_TIME_Y		7000	// 7초 홈 찾기 
#define HOME_FINE_CHECK_MAX_TIME_Z		7000	// 7초 홈 찾기 


#else
#define NEXT_CASE_DELAY_INTERVAL		3000
#define NEXT_CASE_DELAY_RUN					5000
#define STEP_DELAY_INTERVAL		100			// 10ms
#define WAIT_AFTER_INJECTION				50000			// 5초

#define WAIT_HOME_FIND_INTERVAL_100MS		1000
#define WAIT_HOME_FIND_INTERVAL_200MS		2000	//2000
#define WAIT_HOME_FIND_INTERVAL		WAIT_HOME_FIND_INTERVAL_200MS

#define WAIT_MIX_PROGRAM				5000			// 0.5초
#define WAIT_MICRO_PUMP_START				5000			// 0.5초

#define MICRO_PUMP_TIME_CONVERSION	10000

#define WAKEUP_DELAY_TIME			5000		// 0.5초

#define CONVERT_WAIT_TIME_TO_COUNT			10000		//  time(sec) to time tick

#define MOVE_XY_DELAY_TIME		2000		// 0.2초

#define DELAY_TIME_TICK_COUNT_0_1_SEC		1000		// 0.1초
#define DELAY_TIME_TICK_COUNT_2_SEC		20000		// 2초

#define DELAY_Z_ERROR_RECHECK_TIME		2000		// 200ms

#define HOME_FINE_CHECK_MAX_TIME_X		70000	// 7초 홈 찾기 
#define HOME_FINE_CHECK_MAX_TIME_Y		70000	// 7초 홈 찾기 
#define HOME_FINE_CHECK_MAX_TIME_Z		70000	// 7초 홈 찾기 
#endif


#define GO_NEXT_CASE_TIME(var,time,value) \
		get_System_ticks(); \
    if((sysTicks - (var)) > (time)) \
    { \
        var += time; \
        ++value; \
    }

#define  WAIT_TIME_VALUE_CASE_STATE(VALUE,TIME)  \
	if(bInitTicks == NO) {	\
			init_ticks(&nextCaseDelay);	\
			bInitTicks = YES; \
	}	\
	else {	\
			get_System_ticks();	\
	    if((sysTicks - nextCaseDelay) > (TIME))	{ \
	        ++VALUE; \
	        bInitTicks = NO; \
	    } \
	}

#define	RYU_Delay(delay) \
	{ \
	  uint32_t tickstart = 0;	\
	  tickstart = get_Systicks(); \
	  while((get_Systicks() - tickstart) < (delay))	 {} \
	}
// ex)
	//(*((uint32_t volatile *)(COREGPIO_BASE_ADDR + GPIO_OUT_REG_OFFSET)) = (gpio_pattern & 0xfffffffe));



// CoreGpio Reading
#define POWER_SW_OFF_TIME			50		// 5sec

#define SWITCH_1_ID						0
#define SWITCH_2_ID						1
#define DOOR_ERROR						2
#define START_IN_ID						3
#define READY_IN_ID						4
#define LEAK_SEN_ID						5
#define CPU_ONOFF_DET_ID			6
#define POWER_BUTTON_ID				CPU_ONOFF_DET_ID


#define FIRST_YEAR			2001u
#define LAST_YEAR			2099u
#define SECOND_YEAR			86400u


#define TEMP_SUM_TIME		16  // 8

#if TEMP_SUM_TIME == 16
  #define TEMP_ADC_SUM		16000
  #define TEMP_AVE_DIV		4

	#define SUM_DIGIT_TO_EXT_TEMP_FACTOR		1.86264514923095703125e-6		 // 500(10mV) / 2^24 / 16 = 500 / 2^28
#elif TEMP_SUM_TIME == 8
  #define TEMP_ADC_SUM		8000
  #define TEMP_AVE_DIV		3

	#define SUM_DIGIT_TO_EXT_TEMP_FACTOR		3.7252902984619140625e-6		 // 500(10mV) / 2^24 / 8 = 500 / 2^27
#else
  #define TEMP_ADC_SUM		4000
  #define TEMP_AVE_DIV		2

	#define SUM_DIGIT_TO_EXT_TEMP_FACTOR		7.450580596923828125e-6  		 // 500(10mV) / 2^24 / 4 = 500 / 2^26	 수정 할 것.
#endif

#if 0
//======================================================
// X40
/*	
  #define MAIN_TEMP_CONST_A		6.1357931E-14
  #define MAIN_TEMP_CONST_B		1.0832671e5 
  #define MAIN_TEMP_CONST_C		-11.981412
*/
// 전압 
//y = 6.9082889E-01x2 + 3.6348411E+01x - 1.1981412E+01
  #define MAIN_TEMP_CONST_A		0.6993407 
  #define MAIN_TEMP_CONST_B		36.3269082
  #define MAIN_TEMP_CONST_C		-11.9732419
/*	
// moving 8
//y = 1.0794201E-02x2 + 4.5435513E+00x - 1.1981412E+01		
  #define MAIN_TEMP_CONST_A		1.0794201E-02 
  #define MAIN_TEMP_CONST_B		4.5435513
  #define MAIN_TEMP_CONST_C		-11.9732419
*/
FD//======================================================
	#else
//======================================================
// X60
/*	
	#define MAIN_TEMP_CONST_A		2.7270192E-14
  #define MAIN_TEMP_CONST_B		7.2217804E-06
  #define MAIN_TEMP_CONST_C		-1.1981412E+01
*/
// 전압 
//y = 3.0703506E-01x2 + 2.4232274E+01x - 1.1981412E+01
	#define MAIN_TEMP_CONST_A		0.30703506
  #define MAIN_TEMP_CONST_B		24.232274
  #define MAIN_TEMP_CONST_C		-11.981412
/*  
// moving 8
//y = 4.7974228E-03x2 + 3.0290342E+00x - 1.1981412E+01R² = 9.9999981E-01
	#define MAIN_TEMP_CONST_A		4.7974228E-03
  #define MAIN_TEMP_CONST_B		3.0290342
  #define MAIN_TEMP_CONST_C		-11.981412
*/
//======================================================
#endif

	#define DIGIT_TO_VOLT_FACTOR				2.98023223876953125e-7	// 5volt / 2^24
	#define DIGIT_TO_EXT_TEMP_FACTOR		2.98023223876953125e-5	// 500(10mV) / 2^24

	#define POW_V33_DIGIT_TO_VOLT_FACTOR		(DIGIT_TO_VOLT_FACTOR * 2.0f)
	#define POW_V5_DIGIT_TO_VOLT_FACTOR		(DIGIT_TO_VOLT_FACTOR * 3.0f)
	#define POW_V5N_DIGIT_TO_VOLT_FACTOR		(DIGIT_TO_VOLT_FACTOR * 3.0f)			// 사용한함.
	#define POW_V12_DIGIT_TO_VOLT_FACTOR		(DIGIT_TO_VOLT_FACTOR * 11.0f)





#endif	// #ifndef DEFINE_AS_H_

