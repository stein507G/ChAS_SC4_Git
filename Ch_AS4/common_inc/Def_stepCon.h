#ifndef __DEF_STEPCON_H__
#define __DEF_STEPCON_H__


/////////////////////////////////////////////////////////////////////////////
//=============================================================================
// Step Motor Control�� �������õ� ���׵鸸 ����� ��
// 2016.02
//=============================================================================
#define STEP_MOTOR_COUNT		6

#define STEP_FREQUENCY						4666667. 	// 4687500. //2343750. // 4687500. //9375000.	// 2343750.
#define STEP_DATA_ARRAY_COUNT			150 // 156 //192 //240	//150
#define STEP_ANGLE								1.8f

#define STEP_GAP_CONST						1400000		//1406250										// Freq / 360 * STEP_ANGLE * 60

//#define TARGET_RPM								600 	// 300
//#define ACC_INTERVAL							27			// 11.4

#define STEP_FREQUENCY_X						STEP_FREQUENCY // 2343750. // 4687500. //9375000.	// 2343750.
#define STEP_FREQUENCY_Y						STEP_FREQUENCY // 4687500. //9375000.	// 2343750.
#define STEP_FREQUENCY_Z						STEP_FREQUENCY // 4687500. //9375000.	// 2343750.
#define STEP_FREQUENCY_SYR					STEP_FREQUENCY // 4687500. //9375000.	// 2343750.

#define STEP_FREQUENCY_VAL					STEP_FREQUENCY // 4687500. //9375000.	// 2343750.
#define STEP_FREQUENCY_INJ					STEP_FREQUENCY // 4687500. //9375000.	// 2343750.

// 2343750Hz , 600rpm , acc 11

#define TARGET_RPM_X								600 //450	//500		//600
#define TARGET_RPM_Y								600			// 600
#define TARGET_RPM_Z								600			// 600
#define TARGET_RPM_SYR							600			// 600

#define TARGET_RPM_VAL							600			// 600
#define TARGET_RPM_INJ							600			// 600

#define LAST_RPM_X								600 //300	//450	//500		//600
#define LAST_RPM_Y								600 //300	//450	//500		//600
#define LAST_RPM_Z								600 //300	//450	//500		//600
#define LAST_RPM_SYR							600 //	300	//450	//500		//600
#define LAST_RPM_VAL								300	//450	//500		//600
#define LAST_RPM_INJ								300	//450	//500		//600

// 21.5f�� �ּҰ��̴�. - ���Ϸ��� FPGA�� ���� �����ؾ� ��.
#define START_RPM_X									22
#define START_RPM_Y									22 //21.5f
#define START_RPM_Z									22
#define START_RPM_SYR									22
#define START_RPM_VAL									22
#define START_RPM_INJ									22

//#define ACC_INTERVAL_X							12	//11.4  		// 27
#define ACC_INTERVAL_X							12	//16 //14	//	11.2 // 11.7(4)	// 12.8(2) //13	//11	//12	//11.4  		// 27
#define ACC_INTERVAL_Y							12	//11.4  		// 27
#define ACC_INTERVAL_Z							12	//11.4  		// 27
#define ACC_INTERVAL_SYR						12	//11.4  		// 27

#define ACC_INTERVAL_VAL						12	//11.4  		// 27
#define ACC_INTERVAL_INJ						12	//11.4  		// 27

#define MICRO_STEP_FULL_STEP			1		// 16 // 4 // 2 // 1
#define MICRO_STEP_HALF_STEP			2		// 16 // 4 // 2 // 1
#define MICRO_STEP_QUARTER_STEP			4		// 16 // 4 // 2 // 1	
#define MICRO_STEP_EIGHT_STEP			8		// 16 // 4 // 2 // 1
#define MICRO_STEP_SIXTEEN_STEP		16	// 16 // 4 // 2 // 1

#define MICRO_STEP_X							MICRO_STEP_EIGHT_STEP
#define MICRO_STEP_Y							MICRO_STEP_EIGHT_STEP
#define MICRO_STEP_Z							MICRO_STEP_EIGHT_STEP
#define MICRO_STEP_SYR						MICRO_STEP_EIGHT_STEP
#define MICRO_STEP_VAL						MICRO_STEP_EIGHT_STEP
#define MICRO_STEP_INJ						MICRO_STEP_EIGHT_STEP

#define MOTOR_X			0
#define MOTOR_Y			1
#define MOTOR_Z			2
#define MOTOR_SYR		3
#define MOTOR_VAL		4
#define MOTOR_INJ		5

#define CTRL_PORT_DIR			0
#define CTRL_PORT_START		1
#define CTRL_PORT_RESET		2
#define CTRL_PORT_SLEEP		3
#define CTRL_PORT_STOP 		4
#define CTRL_PORT_BREAK		5
#define CTRL_PORT_INFINITE		6
#define CTRL_PORT_CALIBRATE		7

#define CMD_DIR_CCW				0x01 // 1
#define CMD_START					0x02 // 2
#define CMD_START_LOW			0x00 // 2
#define CMD_RESET					0x04 // 4
#define CMD_SLEEP					0x08 // 8
#define CMD_STOP 					0x10 // 16
#define CMD_BREAK					0x20 // 32
#define CMD_INFINITE			0x40 // 64
#define CMD_CALIBRATE			0x80 // 128
// 20200504
//#define CMD_CLR_CNT			0x80 // 128
//#define CMD_WAKEUP			0x40 // 64
#define CMD_WAKEUP			0x80

#define DIR_CW					0x00000000
#define DIR_CCW					0x00000001

#define HOME_DIR_X				DIR_CCW
#define HOME_DIR_Y				DIR_CCW
#define HOME_DIR_Z				DIR_CW
#define HOME_DIR_SYR			DIR_CCW
#define HOME_DIR_VAL			DIR_CW
#define HOME_DIR_INJ			DIR_CW

#define R_HOME_DIR_X				DIR_CW
#define R_HOME_DIR_Y				DIR_CW
#define R_HOME_DIR_Z				DIR_CCW
#define R_HOME_DIR_SYR			DIR_CW
#define R_HOME_DIR_VAL			DIR_CCW
#define R_HOME_DIR_INJ			DIR_CCW

#define HOMECHK_CNT_MOTOR_X				31200 // 40000			// 190mm / 9.7536 * 1600 = 31168 , 
#define HOMECHK_CNT_MOTOR_Y				28800 // 40000			// 175mm / 9.7536 * 1600 = 28708, 
#define HOMECHK_CNT_MOTOR_Z				40000	// 60.96mm
#define HOMECHK_CNT_MOTOR_SYR			49600 // 64000	// 75mm	/ 2.4384f * 1600 = 49213
#define HOMECHK_CNT_MOTOR_VAL			2000	// 1.25rev
#define HOMECHK_CNT_MOTOR_INJ			2000	// 1.25rev

#define HOMECHK_SPEEDMIN_MOTOR_X	0
#define HOMECHK_SPEEDMIN_MOTOR_Y	0
#define HOMECHK_SPEEDMIN_MOTOR_Z	0
#define HOMECHK_SPEEDMIN_MOTOR_SYR	0
#define HOMECHK_SPEEDMIN_MOTOR_VAL	0
#define HOMECHK_SPEEDMIN_MOTOR_INJ	0

// rpm�� ��� ��ȣ
#if 0 
#define HOMECHK_SPEEDMAX_MOTOR_X	25	//34 // 31	// Array
#define HOMECHK_SPEEDMAX_MOTOR_Y	25	//31	//31
#define HOMECHK_SPEEDMAX_MOTOR_Z	25	//31	//31
#define HOMECHK_SPEEDMAX_MOTOR_SYR	25	//31	//31
#define HOMECHK_SPEEDMAX_MOTOR_VAL	22	// (22~255)���� 60	// 100	// rpm
#define HOMECHK_SPEEDMAX_MOTOR_INJ	22	// (22~255)���� 60	// 100	// rpm
#else
#if NEW_BOARD_ENCODER_2020
#define HOMECHK_SPEEDMAX_MOTOR_X	50 // 40 // 30 // 60	//75	//95	// 65	//34 // 31	// Array
#define HOMECHK_SPEEDMAX_MOTOR_Y	50 // 40 // 30 // 60	//75	//95	// 65	//31	//31
#define HOMECHK_SPEEDMAX_MOTOR_Z	60 // 50 // 40 // 75	//95	// 65	//31	//31
#define HOMECHK_SPEEDMAX_MOTOR_SYR	60 // 50 // 40 // 75	//95	// 65	//31	//31
#else
#define HOMECHK_SPEEDMAX_MOTOR_X	60	//75	//95	// 65	//34 // 31	// Array
#define HOMECHK_SPEEDMAX_MOTOR_Y	60	//75	//95	// 65	//31	//31
#define HOMECHK_SPEEDMAX_MOTOR_Z	75	//95	// 65	//31	//31
#define HOMECHK_SPEEDMAX_MOTOR_SYR	75	//95	// 65	//31	//31
#endif
#if 1
#define HOMECHK_SPEEDMAX_MOTOR_VAL	10	// (22~255)���� 60	// 100	// rpm
#define HOMECHK_SPEEDMAX_MOTOR_INJ	10	// (22~255)���� 60	// 100	// rpm
#else
#define HOMECHK_SPEEDMAX_MOTOR_VAL	25	// (22~255)���� 60	// 100	// rpm
#define HOMECHK_SPEEDMAX_MOTOR_INJ	25	// (22~255)���� 60	// 100	// rpm
#endif
#endif


#define	ACT_SPEED_MIN_X						0
#define	ACT_SPEED_MIN_Y						0

#define	ACT_SPEED_MIN_Z						0
//#define	ACT_SPEED_MIN_Z						10  
#define	ACT_SPEED_MIN_SYR					0

#define	ACT_SPEED_MIN_VAL						0
#define	ACT_SPEED_MIN_INJ						0

//=========================================================================================
// ���������̺� ���� 
// #define TARGET_RPM_X								600 //450	//500		//600
// #define START_RPM_X									22
// 149 -->  600rpm
// 139 -->  561.2rpm
// 130 -->  526.3rpm
// 125 -->  506.9rpm
// 120 --> 487.5rpm
// 115 -->  468.1rpm
/*
	  rpm	  ����ð�
0	  22	0
1	  25.87	0.001704533
2	  29.75	0.003409067
3	  33.63	0.0051136
4	  37.51	0.006818133
5	  41.39	0.008522667
10	60.79	0.017045333
20	99.58	0.034090667
30	138.3	0.051136
40	177.1	0.068181333
50	215.9	0.085226667
59	250.8	0.100567467
60	254.7	0.102272
70	293.5	0.119317333
72	301.3	0.1227264
80	332.3	0.136362667
90	371.1	0.153408
98	402.1	0.167044267
99	406.0	0.1687488
100	409.9	0.170453333
110	448.7	0.187498667
115	468.1	0.196021333
120	487.5	0.204544
125	506.8	0.213066667
130	526.2 0.221589333
135	545.6	0.230112
140	565.0	0.238634667
145	584.4	0.247157333
146	588.3	0.248861867
147	592.2	0.2505664
148	596.1	0.252270933
149	600.0 0.253975467

*/
//=========================================================================================
#if MODIFY_MAX_SPEED_X
#define	ACT_SPEED_MAX_X						139 // (561.2rpm) // 130 // 125
#else
#define	ACT_SPEED_MAX_X						149	// (600rpm)
#endif

#if MODIFY_MAX_SPEED_Y
#define	ACT_SPEED_MAX_Y						125 // (506.8rpm) // 130
#else
#define	ACT_SPEED_MAX_Y						149
#endif

// 20201204
#if MODIFY_MAX_SPEED_Z				//ver 3.4.12
#define	ACT_SPEED_MAX_Z						144 // 145 // 139 
#else
#define	ACT_SPEED_MAX_Z						149
#endif

#define	ACT_SPEED_MAX_SYR					149			// 600rpm

//#define	ACT_SPEED_VAL						40	//	60			// rpm
//#define	ACT_SPEED_INJ						40	//	60			// rpm
#define	ACT_SPEED_MAX_VAL						40	//	60			// rpm
#define	ACT_SPEED_MAX_INJ						40	//	60			// rpm

#define SPEED_LEVEL_1				  1
#define SPEED_LEVEL_2					2 
#define SPEED_LEVEL_3					3  
#define SPEED_LEVEL_4					4 
#define SPEED_LEVEL_5					5 
#define SPEED_LEVEL_6					6 
#define SPEED_LEVEL_7					7 
#define SPEED_LEVEL_8					8 
#define SPEED_LEVEL_9					9 
#define SPEED_LEVEL_10				10

#define SYRINGE_SPEED_1				20
#define SYRINGE_SPEED_2				35
#define SYRINGE_SPEED_3				55
#define SYRINGE_SPEED_4				70
#define SYRINGE_SPEED_5				85
#define SYRINGE_SPEED_6				100
#define SYRINGE_SPEED_7				114
#define SYRINGE_SPEED_8				127
#define SYRINGE_SPEED_9				139
#define SYRINGE_SPEED_10			149

#define SYRINGE_SPEED_INJ_1				0			// 20
#define SYRINGE_SPEED_INJ_2				5			// 35
#define SYRINGE_SPEED_INJ_3				10		// 55
#define SYRINGE_SPEED_INJ_4				15		// 70
#define SYRINGE_SPEED_INJ_5				20		// 85
#define SYRINGE_SPEED_INJ_6				25		// 100
#define SYRINGE_SPEED_INJ_7				35		// 114
#define SYRINGE_SPEED_INJ_8				55		// 127
#define SYRINGE_SPEED_INJ_9				70		// 139
#define SYRINGE_SPEED_INJ_10			85		// 149


// washing �� �� ���
#define	WASH_SPEED_UP_SYR				(devSet.syrSpeedWash[sysConfig.syringeUpSpdWash-1])
#define	WASH_SPEED_DOWN_SYR			(devSet.syrSpeedWash[sysConfig.syringeDnSpdWash-1])

#define STEP_STATECHK_FLAG			0x0000000f
#define	STEP_STATE_READY				0
#define STEP_STATE_ACCEL_1 			1
#define STEP_STATE_ACCEL_2 			2
#define STEP_STATE_ACCEL_3			3
#define STEP_STATE_ACCEL_4			4
#define STEP_STATE_CONST				6
#define STEP_STATE_DEACC_1			7
#define STEP_STATE4_DEACC_2			8
#define STEP_STATE_DEACC_3			9
#define STEP_STATE_DEACC_4			10
#define STEP_STATE5_BREAK				12
#define STEP_STATE_SLEEP				13
#define STEP_STATE_RESET				14
#define STEP_STATE_INFINITE			15


// Step_AS_fsm_iso12.v
#define STEP_ISO_STATECHK_FLAG			0x0000000f
#define STEP_ISO_READY			0
#define STEP_ISO_RUN				1
#define STEP_ISO_BREAK			2
#define STEP_ISO_SLEEP			3
#define STEP_ISO_RESET			4
#define STEP_ISO_INFINITE		5

// ������°� BREAK�����̸� RESET��ȣ�� READY���·� �ȴ�. --> RESET�� ��ġ�� �ʰ� �ٷ� READY ���·� 
// ������°� SLEEP�����̸� 1/8step�� ��� 0x1250/4.6875MHz ����(1msec) reset�� Ready���·� �ȴ�.  -
// ������°� SLEEP�����̸� WAKE_UP��ȣ�� 1/8step�� ��� 0x1250/4.6875MHz ����(1msec) Sleep Off�� Ready���·� �ȴ�.  -
#if FPGA_VERSION_TRIM
	#if 1
		#define motorResetCmd(WHO)	 				{CMD_SEL_MOTOR(WHO);	*((uint32_t volatile *)(devSet.ctrlPortAddr[WHO])) = (devState.curDir[WHO] | CMD_RESET); }
		#define motorReadyCmd(WHO)	 				{CMD_SEL_MOTOR(WHO);	*((uint32_t volatile *)(devSet.ctrlPortAddr[WHO])) = (devState.curDir[WHO] | CMD_WAKEUP); }
		#define motorSleepCmd(WHO , DIR) 		{CMD_SEL_MOTOR(WHO);	*((uint32_t volatile *)(devSet.ctrlPortAddr[WHO])) = (DIR | CMD_SLEEP); }
		#define motorWakeupCmd(WHO)	 				motorReadyCmd(WHO)
	#endif

	#define readyChk(A) 		((devState.btMotorState[(A)] & STEP_STATECHK_FLAG) == STEP_STATE_READY)
	#define breakChk(A) 		((devState.btMotorState[(A)] & STEP_STATECHK_FLAG) == STEP_STATE5_BREAK)
	#define homeChkEnd(A)		((devState.btMotorState[(A)] & STEP_STATECHK_FLAG) == STEP_STATE5_BREAK)
	#define sleepChk(A)			((devState.btMotorState[(A)] & STEP_STATECHK_FLAG) == STEP_STATE_SLEEP)

	#define readyChkVal(A) 			readyChk(A)
	#define breakChkVal(A) 			breakChk(A)
	#define homeChkEndVal(A)		homeChkEnd(A)
	#define sleepChkVal(A)			sleepChk(A)

	#define readyChkInj(A) 			readyChk(A)	
#else
	#define motorResetCmd(WHO)	 				stepCtrlCmd(WHO, devState.curDir[WHO] | CMD_RESET)
	#define motorReadyCmd(WHO)	 				stepCtrlCmd(WHO, devState.curDir[WHO] | CMD_RESET)
	#define motorSleepCmd(WHO , DIR) 		stepCtrlCmd(WHO, DIR | CMD_SLEEP)
	#define motorWakeupCmd(WHO)	 				stepCtrlCmd(WHO, devState.curDir[WHO] | CMD_WAKEUP)

// readyChk() / breakChk() / homeChkEnd() / sleepChk() ��   
// readMotorState()���� �Լ� �о �����ϰ� �̤��� ��.
	#define readyChk(A) 		((devState.btMotorState[(A)] & STEP_STATECHK_FLAG) == STEP_STATE_READY)
	#define breakChk(A) 		((devState.btMotorState[(A)] & STEP_STATECHK_FLAG) == STEP_STATE5_BREAK)
	#define homeChkEnd(A)		((devState.btMotorState[(A)] & STEP_STATECHK_FLAG) == STEP_STATE5_BREAK)
	#define sleepChk(A)			((devState.btMotorState[(A)] & STEP_STATECHK_FLAG) == STEP_STATE_SLEEP)

	#define readyChkVal(A) 			readyChk(A)
	#define breakChkVal(A) 			breakChk(A)
	#define homeChkEndVal(A)		homeChkEnd(A)
	#define sleepChkVal(A)			sleepChk(A)
#endif


// 20200516
// MM_PER_REV_SYR => 2.4384mm ,, 2.4384mm/12 = 0.2032mm - �� ī��Ʈ�� �̵��Ÿ� 
#if FPGA_VERSION_TRIM
//	#define ERR_ENCODER_COUNT				1 // 2
	#define ERR_ENCODER_COUNT_X			2 // 1 // 2
	#define ERR_ENCODER_COUNT_Y			2 // 1 // 2 // 3
	#define ERR_ENCODER_COUNT_Z			2
	#define ERR_ENCODER_COUNT_SYR		2 // 	1 // 2
	#define ERR_ENCODER_COUNT_VAL			300
	#define ERR_ENCODER_COUNT_INJ			170		

//	#define ENCODER_CHECK					(encoderCountCheck(encoderCount,readCount) > ERR_ENCODER_COUNT)

	#define ENCODER_CHECK_Z				(encoderCountCheck(encoderCount,readCount) > ERR_ENCODER_COUNT_Z)
	#define ENCODER_CHECK_X				(encoderCountCheck(encoderCount_X,readCount) > ERR_ENCODER_COUNT_X)
	#define ENCODER_CHECK_Y				(encoderCountCheck(encoderCount_Y,readCount) > ERR_ENCODER_COUNT_Y)		
	
	#define ENCODER_CHECK_SYR			(encoderCountCheck(encoderCount,readCount) > ERR_ENCODER_COUNT_SYR)
	
//		#define ENCODER_CHECK_VAL			(readCount < ERR_ENCODER_COUNT_VAL)	
//		#define ENCODER_CHECK_INJ			(readCount < ERR_ENCODER_COUNT_INJ)

	#define errorEncoder(WHO)			errorFunction(SELF_ERROR_ENCODER_COUNT + WHO)
	
	#define errorEncoder_X()			errorFunction(SELF_ERROR_ENCODER_COUNT_X)
	#define errorEncoder_Y()			errorFunction(SELF_ERROR_ENCODER_COUNT_Y)
	#define errorEncoder_Z()			errorFunction(SELF_ERROR_ENCODER_COUNT_Z)
	#define errorEncoder_SYR()			errorFunction(SELF_ERROR_ENCODER_COUNT_SYR)
	#define errorEncoder_VAL()			errorFunction(SELF_ERROR_ENCODER_COUNT_VAL)
	#define errorEncoder_INJ()			errorFunction(SELF_ERROR_ENCODER_COUNT_INJ)	
#endif	

#if NEW_BOARD_ENCODER_2020

	#define readyChkEncoder_X()	{ \
			readCount = *((uint32_t volatile *)(BASEADDR_MOTOR_X_Z)); \
			readCount = (readCount >> 8) & 0x3ff; \
dfp("======  X  readCount=[%d] [%d]\n", readCount , encoderCount_X); \
CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_X); \
			if(ENCODER_CHECK_X) {	errorEncoder(MOTOR_X); } \
	}

	#define readyChkEncoder_Y()	{ \
			readCount = *((uint32_t volatile *)(BASEADDR_MOTOR_Y_SYR)); \
			readCount = (readCount >> 8) & 0x3ff; \
dfp("======  Y  readCount=[%d] [%d]\n", readCount , encoderCount_Y); \
	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_Y); \
			if(ENCODER_CHECK_Y) {	errorEncoder(MOTOR_Y); } \
	}


	#define readyChkEncoder_Z()	{ \
			readCount = *((uint32_t volatile *)(BASEADDR_MOTOR_X_Z)); \
			readCount = (readCount >> 8) & 0x3ff; \
dfp("======  Z  readCount=[%d] [%d]\n", readCount , encoderCount); \
	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_Z); \
			if(ENCODER_CHECK_Z) {	errorEncoder(MOTOR_Z); } \
	}

#else
	#define readyChkEncoder_X()
	#define readyChkEncoder_Y()
	#define readyChkEncoder_Z() 
#endif

#if FPGA_VERSION_TRIM	
//==================================================================================
// readyChkEncoder_Syr()
// #if SYRINGE_ENCODER_REMOVE_WASHING
// Washing ����
// #if SYRINGE_ENCODER_REMOVE_END
// endsequence ���� - ���� ã�� ���� 
// ������ �������� ���� - ���� ã�� ���� 
// 
//==================================================================================
	#define readyChkEncoder_Syr()	{ \
			readCount = *((uint32_t volatile *)(BASEADDR_MOTOR_Y_SYR)); \
			readCount = (readCount >> 8) & 0x3ff; \
dfp("xxxxxxxxxxxxx======  Syringe  readCount=[%d] [%d]\n", readCount , encoderCount); \
				CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_SYR); \
			if(ENCODER_CHECK_SYR) {	errorEncoder(MOTOR_SYR); } \
	}
	
	#define readyChkEncoder_Val()	{ \
		if(retVal) { \
			readCount	= *((uint32_t volatile *)(BASEADDR_MOTOR_VAL_INJ)); \
			readCount = (readCount >> 8) & 0x3ff; \
dfp("======  Valve  readCount=[%d]\n", readCount ); \
				CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_VAL); \
			if(readCount < ERR_ENCODER_COUNT_VAL)	{	errorEncoder(MOTOR_VAL);} \
		} \
	}

	#define readyChkEncoder_Inj()	{ \
		if(retVal) { \
			readCount	= *((uint32_t volatile *)(BASEADDR_MOTOR_VAL_INJ)); \
			readCount = (readCount >> 8) & 0x3ff; \
dfp("======  Injector  readCount=[%d] \n", readCount ); \
				CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_INJ); \
			if(readCount < ERR_ENCODER_COUNT_INJ) {	errorEncoder(MOTOR_INJ);} \
		} \
	}

#else
		#define readyChkEncoder_Syr()
		#define readyChkEncoder_Val()
		#define readyChkEncoder_Inj()
#endif

//#define goRetryPos(A,X)			goRetryPos_##A(X)
#define goRetryPos(X,A1,A2,A3)			goRetryPos_##X(A1, A2, A3)


//#define isStartPoint(A) 		(devState.curStep[(A)] == 0)

//========================================================================================================
// Photointerrupters slit size ==> 0.5mmx2.1mm
// 1.8�� = 0.012192mm --> 1/8 microstep --> 8 pulse		1mm / 0.012192mm = 82.021 
// 1rev = 2.4384mm
// 82.021 * 8 =  656.168 pulse
#define MM_PER_STEP_Z				0.012192f			// 1.8��(8Pulse) �� �̵��Ÿ� mm
#define MM_PER_STEP_SYR			0.012192f			// 1.8��(8Pulse) �� �̵��Ÿ� mm
#define MM_PER_REV_Z					2.4384f				// 1ȸ�� �� �̵��Ÿ� mm
#define MM_PER_REV_SYR				2.4384f				// 1ȸ�� �� �̵��Ÿ� mm
#if 0
#define LEN_1MM_Z						656.168f
#define LEN_1MM_SYR					656.168f
#define STEP_1MM_Z						656
#define STEP_1MM_SYR					656
#endif
//========================================================================================================
// 1.8�� = 0.048768mm --> 1/8 microstep --> 8 pulse   1mm / 0.048768mm = 20.505
// 20.505 * 8 = 164.042
#define MM_PER_STEP_X				0.048768f			// 1.8��(8Pulse) �� �̵��Ÿ� mm
#define MM_PER_STEP_Y				0.048768f			// 1.8��(8Pulse) �� �̵��Ÿ� mm
#define MM_PER_REV_X				9.7536f				// 1ȸ�� �� �̵��Ÿ� mm
#define MM_PER_REV_Y				9.7536f				// 1ȸ�� �� �̵��Ÿ� mm

#if 0
#define LEN_1MM_X						164.042f
#define LEN_1MM_Y						164.042f
#define STEP_1MM_X					164			// 164 �뷫 160������ 
#define STEP_1MM_Y					164			// 164 �뷫 160������ 
#endif
//==============================================================================
// Ȩ��ġ ã�� �� 1.8�� ������ �����̱� ���� �� 
// devSet.findHomeUnitStep[MOTOR_X] = MICRO_STEP_X * 2;
// ����
/*
#define STEP_FIND_HOME_X				16
#define STEP_FIND_HOME_Y				16
#define STEP_FIND_HOME_Z				16
#define STEP_FIND_HOME_SYR				16
#define STEP_FIND_HOME_VAL				8
#define STEP_FIND_HOME_INJ				8 //12	//16	//19			// 266.6667 - 208 = 38 --> 38 / 2 = 19
*/
#define FIND_HOME_UNIT_STEP_X			(MICRO_STEP_X * 2)
#define FIND_HOME_UNIT_STEP_Y			(MICRO_STEP_Y * 2)
#define FIND_HOME_UNIT_STEP_Z			(MICRO_STEP_Z * 2)
#define FIND_HOME_UNIT_STEP_SYR			(MICRO_STEP_SYR * 2)
#define FIND_HOME_UNIT_STEP_VAL			(MICRO_STEP_VAL)
#define FIND_HOME_UNIT_STEP_INJ			(MICRO_STEP_INJ)

//==============================================================================
// Ȩ��ġ ã���� ���ݴ� �̵� (�����Ҿ����ؼ�)
#if 0
#define STEP_MORE_HOME_X				160			// 0.97536mm
#define STEP_MORE_HOME_Y				160			// 0.97536mm
#define STEP_MORE_HOME_Z				160			// 0.24384mm  = (160/8) * 0.012192 = 20 * 0.012192

#define STEP_MORE_ADJUST_VALUE_Z				82			// 82 * 0.12192 =  1mm

#define STEP_MORE_HOME_SYR				16
#define STEP_MORE_HOME_VAL				8
#define STEP_MORE_HOME_INJ				8 //12	//16	//19			// 266.6667 - 208 = 38 --> 38 / 2 = 19

#define STEP_INJ_DEGREE_45	200			//25*8
#else
#define STEP_MORE_HOME_X				(MICRO_STEP_X * 20) //160			// 0.97536mm
#define STEP_MORE_HOME_Y				(MICRO_STEP_Y * 20) //160			// 0.97536mm
#define STEP_MORE_HOME_Z				(MICRO_STEP_Z * 20) // 160			// 0.24384mm  = (160/8) * 0.012192 = 20 * 0.012192

#define STEP_MORE_HOME_SYR			(MICRO_STEP_SYR * 2) // 	16
#define STEP_MORE_HOME_VAL			(MICRO_STEP_VAL) //	8
#define STEP_MORE_HOME_INJ			(MICRO_STEP_INJ) // 	8 //12	//16	//19			// 266.6667 - 208 = 38 --> 38 / 2 = 19

#define STEP_INJ_DEGREE_45			(MICRO_STEP_INJ * 25) 	//  200			//25*8
#define STEP_VAL_DEGREE_45 			STEP_INJ_DEGREE_45

#define STEP_INJ_DEGREE_18			(MICRO_STEP_INJ * 10) 	//  200			//10*20
#define STEP_VAL_DEGREE_18 			STEP_INJ_DEGREE_18
#endif
// 0(home) 
// + 43(RF1) + 13.5(RE1) + 13.5(RD1) + 13.5(RC1) + 13.5(RB1) + 13.5(RA1) 
// + 19(LF1) + 13.5(LE1) + 13.5(LD1) + 13.5(LC1) + 13.5(LB1) + 13.5(LA1)
// RF1(7054 : 7053.8057742782152230971128608924)
// 43 / MM_PER_STEP_X

// ===============================================================================
// X,Y �̵���� - �������� 
// Washing port�� �������� ��.
// x�� �������� �̵��� + 
// Y�� �������� �̵��� +
// Z�� �Ʒ��� �̵��� +
// ===============================================================================
#define ACT_XY_POS									0 // ��ǥ�Է� , mm / uL ... (tray xy ��ǥ�Է�)
#define ACT_XY_POS_ADD							3 // ������ġ���� �߰� ��ǥ�Է� , mm / uL
#define ACT_MOVE_STEP_ABSOLUTE			1	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
#define ACT_MOVE_STEP_RELATE				2 // ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)

#define ACT_Z_POS										0 // ��ǥ�Է� , mm / uL ... (z ��ǥ�Է�)

//========================================================================
#define POS_XY_INJECTOR			0		
#define POS_XY_WASTE				1		
#define POS_XY_WASH					2		
#define POS_XY_EXCHANGE			3		
#define POS_XY_HOME					4		

//========================================================================
#define COORD_X_INJECTOR			0
#define COORD_X_WASTE					0
#define COORD_X_WASH					0

#if MECH_VER == 6
	#define COORD_X_EXCHANGE			27 // -- ��ó�� ��õ� ��ǰ
//	#define COORD_X_EXCHANGE			19.5
#elif MECH_VER == 7	
	#define COORD_X_EXCHANGE			19.5
#endif
#define COORD_X_HOME					0		// mm

//========================================================================
#define COORD_Y_INJECTOR			148.5				// washing port�� ����

#define COORD_Y_WASTE					7
#define COORD_Y_WASH					0	// mm
#if MECH_VER == 6
#define COORD_Y_EXCHANGE			COORD_Y_INJECTOR		// �� ó�� ��ǰ
//#define COORD_Y_EXCHANGE			(COORD_Y_INJECTOR	- 7.5)
#elif MECH_VER == 7	
#define COORD_Y_EXCHANGE			(COORD_Y_INJECTOR	- 7.5)
#else
#define COORD_Y_EXCHANGE			COORD_Y_INJECTOR		
#endif
#define COORD_Y_HOME					0		// mm

//=================================================================================================================
//=================================================================================================================
#define POS_Z_TOP						0		// ���� - home - (�ɸ��� �κ��� ���� �����Ӱ� �̵�������.)
#define POS_Z_BOTTOM				1		// �ǾƷ� - ���̾�(�ܺ�) 
#define POS_Z_INJECTION			2		// Injector ���� ������ġ
#define POS_Z_WASTE					3		// Waste Port ���� 
#define POS_Z_WASH					4		// WASH Port ��ô��ġ(����) 
#define POS_Z_WASH_ADJUST				5		// Adjust Mode ���� �����ϱ����� Z�� ��ġ

#define POS_Z_VIAL_LEFT_TOP				6		// 2ml Vial Cap ����
#define POS_Z_VIAL_LEFT_ADJUST		7		// Adjust
//#define POS_Z_VIAL_LEFT_BOTTOM		7		// Vial Bottom 2mm��--> (COORD_Z_BOTTOM - runSetup.needleHeight)
#define POS_Z_VIAL_LEFT_DETECT		8		

#define POS_Z_VIAL_RIGHT_TOP			9		// 2ml Vial Cap ����
#define POS_Z_VIAL_RIGHT_ADJUST		10		// Adjust
//#define POS_Z_VIAL_RIGHT_BOTTOM		10		// Vial Bottom 2mm��--> (COORD_Z_BOTTOM - runSetup.needleHeight)
#define POS_Z_VIAL_RIGHT_DETECT		11		

#define POS_Z_INJECTOR_TOP	12					// port �ܺ� �ֻ� 
#define POS_Z_INJECTOR_TOP_ADJUST	13		// POS_Z_INJECTOR_TOP 1mm��
//#define POS_Z_WASTE_TOP			14	// POS_Z_WASH_TOP�� ���� ��ġ 
#define POS_Z_WASH_TOP			14		

#define POS_Z_NEEDLE_EXCHANGE	15		// �ϵ鱳ü���� ��ġ

#define POS_Z_SLEEP						16		// Sleep mode�� ������ ��ġ

#define POS_Z_INJECTION_PORT_DETECT		17		
#define POS_Z_WASTE_DETECT		18		// 19
#define POS_Z_WASH_DETECT			18		

#define POS_Z_VIAL_LEFT_UPPER_DETECT		19		
#define POS_Z_VIAL_RIGHT_UPPER_DETECT		20		
#define POS_Z_INJECTION_PORT_UPPER_DETECT		21		
#define POS_Z_WASTE_UPPER_DETECT		22 // 23		
#define POS_Z_WASH_UPPER_DETECT			22		

#define POS_Z_VIAL_HEIGHT_ADJUST	23		// ���̾� üũ - injector top
#define POS_Z_NEEDLE_DEPTH_ADJUST	24		// Needle ����üũ -

#define POS_Z_COUNT_MAX				(POS_Z_NEEDLE_DEPTH_ADJUST + 1)
//=================================================================================================================
//=================================================================================================================

// 2ml vial
#define VIAL_60_HEIGHT				32
#define VIAL_60_DEPTH					31
#if	MECH_VER == 6
#define CAP_60_HEIGHT					2.0
#elif	MECH_VER == 7
#define CAP_60_HEIGHT					2.5
#else
#define CAP_60_HEIGHT					2.5
#endif
#define VIAL_CAP_60_HEIGHT		(VIAL_60_HEIGHT +	CAP_60_HEIGHT) //34	// Cap ���� �ܺλ�����
#define VIAL_CAP_60_DEPTH			(VIAL_60_DEPTH + CAP_60_HEIGHT) //33	// Cap ���� Cap ������ ���� �ǾƷ� ����(����)

#if	MECH_VER == 6
#define VIAL_40_HEIGHT					44.8
#elif	MECH_VER == 7
#define VIAL_40_HEIGHT					44.8
#else
#define VIAL_40_HEIGHT					45
#endif
#define VIAL_40_DEPTH					43.5
#define CAP_40_HEIGHT					3
#define VIAL_CAP_40_HEIGHT		(VIAL_40_HEIGHT +	CAP_40_HEIGHT) //48	// Cap ���� �ܺλ�����
#define VIAL_CAP_40_DEPTH			(VIAL_40_DEPTH + CAP_40_HEIGHT) //46.5	// Cap ���� Cap ������ ���� �ǾƷ� ����(����)

//#define VIAL_96_HEIGHT				16.9	// 14.6
//#define VIAL_96_DEPTH					10.5	// 10.9

#define WELLPALTE_96_HEIGHT				16.9 //���������ġ  16.9(�ٸ����̾�(40,60)�ܺιٴں��� �����ʱ���)  	// 14.6(�ܺ�) 
#define WELLPLATE_96_DEPTH				10.9 //	10.5	// ���������ġ 10.9 : ���� ���� 

//###########################################################################################
//===========================================================================================
// �ⱸ�� ������ �Ʒ� ��ũ�θ� �����ϸ� �ȴ�.
// COORD_Z_WASH_TOP
// COORD_Z_BOTTOM
// COORD_Z_INJECTOR_TOP
//===========================================================================================
//###########################################################################################

// needle(�ٴó�)�� �������� �����Ѵ�.
//===========================================================================================
//#define POS_Z_TOP						0		// ���� - home - (�ɸ��� �κ��� ���� �����Ӱ� �̵�������.)
#define COORD_Z_TOP						0		
	#define COORD_Z_NEEDLE_GUIDE_GAP		0			// �ϵ�� �ϵ鰡�̵� �Ÿ� // needle Guide�� needle �� ���� ���̷� (0.5���� ������)

#if MECH_VER == 6 || MECH_VER == 7	
	#define COORD_Z_WASH_TOP			(3.5 + COORD_Z_NEEDLE_GUIDE_GAP)		// ����������
	#define COORD_Z_WASH_TOP_TO_Z_BOTTOM			51.5										// ���̾˿ܺ��� �غκ� 
#endif	
//===========================================================================================	
//#define POS_Z_BOTTOM				1		// �ǾƷ� - ���̾�(�ܺ�) 	// ���̾� �ٴ�(�ܺ�) <--> �ٴ� ��	

#if MECH_VER == 6 || MECH_VER == 7	
#define COORD_Z_BOTTOM				(COORD_Z_WASH_TOP + COORD_Z_WASH_TOP_TO_Z_BOTTOM)	// ���̾� �ٴ�(�ܺ�) <--> �ٴ� ��, Needle Guide�ʹ� 0.0mm���� 
#endif
	#define COORD_Z_VIAL_60_TOP			(COORD_Z_BOTTOM - VIAL_CAP_60_HEIGHT) //(16 + COORD_Z_WASH_TOP)	// 	// 2ml vial ���� 
	#define COORD_Z_VIAL_40_TOP			(COORD_Z_BOTTOM - VIAL_CAP_40_HEIGHT) // (2 + COORD_Z_WASH_TOP) // (1.5 + COORD_Z_WASH_TOP)// 4ml vial ���� 
	#define COORD_Z_WELLPALTE_96_TOP			(COORD_Z_BOTTOM - WELLPALTE_96_HEIGHT) 
//#define COORD_Z_VIAL_TOP			COORD_Z_VIAL_60_TOP	//15.5 	// 2ml vial ���� 

// ���̾� ���� �ٴ�
	#define COORD_Z_VIAL_60_BOTTOM			(COORD_Z_VIAL_60_TOP + VIAL_CAP_60_DEPTH)
	#define COORD_Z_VIAL_40_BOTTOM			(COORD_Z_VIAL_40_TOP + VIAL_CAP_40_DEPTH)	// (2 + COORD_Z_WASH_TOP) // (1.5 + COORD_Z_WASH_TOP)// 4ml vial ���� 
	#define COORD_Z_WELLPALTE_96_BOTTOM			(COORD_Z_WELLPALTE_96_TOP + WELLPLATE_96_DEPTH)
//#define COORD_Z_VIAL_BOTTOM		48		//needleDepth = COORD_Z_VIAL_BOTTOM - runSetup.needleHeight;
//#define COORD_Z_VIAL_BOTTOM		(COORD_Z_VIAL_TOP + VIAL_CAP_DEPTH) //needleDepth = COORD_Z_VIAL_BOTTOM - runSetup.needleHeight;

#if MECH_VER == 6 || MECH_VER == 7	
	#define COORD_Z_VIAL_60_ADJUST			(COORD_Z_BOTTOM - VIAL_CAP_60_HEIGHT - 1.5) //(16 + COORD_Z_WASH_TOP)	// 	// 2ml vial ���� 
	#define COORD_Z_VIAL_40_ADJUST			(COORD_Z_BOTTOM - VIAL_CAP_40_HEIGHT - 1.5) // (2 + COORD_Z_WASH_TOP) // (1.5 + COORD_Z_WASH_TOP)// 4ml vial ���� 
	#define COORD_Z_WELLPALTE_96_ADJUST			(COORD_Z_BOTTOM - WELLPALTE_96_HEIGHT - 1) 
#else
	#define COORD_Z_VIAL_60_ADJUST			(COORD_Z_BOTTOM - VIAL_CAP_60_HEIGHT - 2) //(16 + COORD_Z_WASH_TOP)	// 	// 2ml vial ���� 
	#define COORD_Z_VIAL_40_ADJUST			(COORD_Z_BOTTOM - VIAL_CAP_40_HEIGHT - 2) // (2 + COORD_Z_WASH_TOP) // (1.5 + COORD_Z_WASH_TOP)// 4ml vial ���� 
	#define COORD_Z_WELLPALTE_96_ADJUST			(COORD_Z_BOTTOM - WELLPALTE_96_HEIGHT - 2) 
#endif

//===========================================================================================	
//#define POS_Z_INJECTION			2		// Injector ���� ������ġ
//#define POS_Z_INJECTOR_TOP	12					// port �ܺ� �ֻ� 
#if MECH_VER == 6 || MECH_VER == 7
#define COORD_Z_INJECTOR_TOP	(8.5 + COORD_Z_WASH_TOP)			// port �ܺ� �ֻ� - // ����������
//#define COORD_Z_INJECTOR_PORT_DEPTH		19.5 // 18.5	//19	// injector ��Ʈ ���� 
#define COORD_Z_INJECTOR_PORT_DEPTH		18.5 // 18.5	//19	// injector ��Ʈ ���� - needle ���������� injector ��Ʈ ���� 
#endif

#if MECH_VER == 6
#define COORD_Z_INJECTION			(COORD_Z_INJECTOR_TOP + COORD_Z_INJECTOR_PORT_DEPTH) // 31.1
#elif MECH_VER == 7
	#if INJECTION_DEPTH_MODIFY
		#define PORT_DEPTH_ADD			3
		#define COORD_Z_INJECTION			(COORD_Z_INJECTOR_TOP + COORD_Z_INJECTOR_PORT_DEPTH + PORT_DEPTH_ADD) 
	#else
		#define COORD_Z_INJECTION			(COORD_Z_INJECTOR_TOP + COORD_Z_INJECTOR_PORT_DEPTH) 	
	#endif
#define COORD_Z_NEEDLE_DEPTH_ADJUST			(COORD_Z_INJECTION - 9.5)	
#endif
//===========================================================================================	
//#define POS_Z_WASTE					3		// 
#define COORD_Z_WASTE					40

//===========================================================================================	
//#define POS_Z_WASH					4		// WASH Port ��ô��ġ 
#if COORD_Z_WASH_MODIFY							// 20200813
#define COORD_Z_WASH					52
#else
#define COORD_Z_WASH					COORD_Z_BOTTOM
#endif
//===========================================================================================	
//#define POS_Z_WASH_ADJUST				5		// wash Adjust Mode ���� �����ϱ����� Z�� ��ġ
#define COORD_Z_WASH_ADJUST				(COORD_Z_WASH_TOP - 1) //19	//23
	#define VIAL_DETECT_DEPTH			4
	#define COORD_Z_VIAL_60_DETECT			(COORD_Z_VIAL_60_TOP + VIAL_DETECT_DEPTH)
	#define COORD_Z_VIAL_40_DETECT			(COORD_Z_VIAL_40_TOP + VIAL_DETECT_DEPTH)	// (2 + COORD_Z_WASH_TOP) // (1.5 + COORD_Z_WASH_TOP)// 4ml vial ���� 
	#define COORD_Z_WELLPALTE_96_DETECT			(COORD_Z_WELLPALTE_96_TOP + VIAL_DETECT_DEPTH)

//===========================================================================================	
//#define POS_Z_INJECTOR_TOP_ADJUST	13		// POS_Z_INJECTOR_TOP 1mm��
#define COORD_Z_INJECTOR_TOP_ADJUST	(COORD_Z_INJECTOR_TOP - 1)

//===========================================================================================	
//#define POS_Z_NEEDLE_EXCHANGE	15		// �ϵ鱳ü���� ��ġ
#if MECH_VER == 6
#define COORD_Z_NEEDLE_EXCHANGE			(COORD_Z_INJECTION)  //-- ��ó�� ��õ� ��ǰ
#elif MECH_VER == 7
#define COORD_Z_NEEDLE_EXCHANGE			(COORD_Z_INJECTION - 3)
#else
#define COORD_Z_NEEDLE_EXCHANGE			(COORD_Z_INJECTION)
#endif

//#define POS_Z_SLEEP			16
#define COORD_Z_SLEEP_STEP			(40)		// 40 * 16  -- 40 * 16 step

//#define POS_Z_INJECTION_PORT_DETECT		17
//#define POS_Z_WASH_DETECT			18		
//#define POS_Z_WASTE_DETECT		19		
#define INJ_PORT_DETECT_DEPTH			4
#define WASH_PORT_DETECT_DEPTH			7
#define WASTE_PORT_DETECT_DEPTH			7

#define	POS_VAL_WASHING			0
#define	POS_VAL_NEEDLE			1
#define UNIT_STEP_VAL				50 	// 90�� 267

#define	POS_INJ_LOAD			0
#define	POS_INJ_INJECT			1
//#define UNIT_STEP_INJ			33.3	// 267
#define UNIT_STEP_INJ			33	// 33.3�� �ٻ�ġ�� ���� 264 , 267

#define POS_SYR_ZERO			0.0f

//===============================================================
// z , syr ==> MM_PER_STEP_Z(0.012192mm) * 4 = 0.048768mm  
// x , y ==> MM_PER_STEP_X(0.048768mm) * 2 = 0.097536mm  
// val , inj ==> 1.8��ȸ�� 
#define ADJUST_STEP_UNIT_X			(MICRO_STEP_X * 2)
#define ADJUST_STEP_UNIT_Y			(MICRO_STEP_Y * 2)
#define ADJUST_STEP_UNIT_Z			(MICRO_STEP_Z * 4)
#define ADJUST_STEP_UNIT_SYR		(MICRO_STEP_SYR * 4)
#define ADJUST_STEP_UNIT_VAL		(MICRO_STEP_VAL)
#define ADJUST_STEP_UNIT_INJ		(MICRO_STEP_INJ)


#define ADJUST_STEP_UNIT_SCALE_X		2
#define ADJUST_STEP_UNIT_SCALE_Y		2
#define ADJUST_STEP_UNIT_SCALE_Z		2
#define ADJUST_STEP_UNIT_SCALE_SYR		2
#define ADJUST_STEP_UNIT_SCALE_VAL		2
#define ADJUST_STEP_UNIT_SCALE_INJ		2

#define ADJUST_SPEED_SYL				SYRINGE_SPEED_4
#define ADJUST_SPEED_VAL				HOMECHK_SPEEDMAX_MOTOR_VAL
#define ADJUST_SPEED_INJ				HOMECHK_SPEEDMAX_MOTOR_INJ


#if FPGA_VERSION_TRIM
/*
	#define HOME1_CHECK						0x00000010
	#define HOME2_CHECK						0x00000020
	#define HOME_ALL_CHECK				0x00000030

	#define HOME1_SENSING						0x00000010
	#define HOME1_NOT_SENSING				0x00000000

	#define HOME2_SENSING						0x00000020
	#define HOME2_NOT_SENSING				0x00000000

	#define HOME_ALL_SENSING				0x00000030
	#define HOME_ALL_NOT_SENSING		0x00000000
*/

	#define X_HOME1_CHECK						0x10000000
	#define X_HOME1_SENSING					0x10000000
	#define X_HOME1_NOT_SENSING			0x00000000
	
	#define Y_HOME1_CHECK						0x00000010
	#define Y_HOME1_SENSING					0x00000010
	#define Y_HOME1_NOT_SENSING			0x00000000
	
	#define Z_HOME1_CHECK						0x00000010
	#define Z_HOME1_SENSING					0x00000010
	#define Z_HOME1_NOT_SENSING			0x00000000

	#define Z_HOME2_CHECK						0x00000020
	#define Z_HOME2_SENSING					0x00000020
	#define Z_HOME2_NOT_SENSING			0x00000000

	#define Z_HOME_ALL_CHECK				0x00000030
	#define Z_HOME_ALL_SENSING			0x00000030
	#define Z_HOME_ALL_NOT_SENSING	0x00000000

	#define SYR_HOME1_CHECK					0x10000000
	#define SYR_HOME1_SENSING				0x10000000
	#define SYR_HOME1_NOT_SENSING		0x00000000

// �޽��� ���ڴ� ī��Ʈ ȯ�� ���� 
	#define X_ENCODER_COUNTER_FACTOR	0.0075f
	#define Y_ENCODER_COUNTER_FACTOR	0.0075f
	#define Z_ENCODER_COUNTER_FACTOR	0.0075f
	#define SYR_ENCODER_COUNTER_FACTOR	0.0075f

	
// 90�� ������ ������ Ȩ ����(82��)
	#define VAL_HOME1_CHECK					0x00000010
	#define VAL_HOME1_SENSING				0x00000010		// ���� �κ�
	#define VAL_HOME1_NOT_SENSING		0x00000000

// 60�� ������ ������ Ȩ ����(52��)
	#define INJ_HOME1_CHECK					0x10000000
	#define INJ_HOME1_SENSING				0x10000000		// ���� �κ�
	#define INJ_HOME1_NOT_SENSING		0x00000000
#else
/*
	#define HOME1_CHECK						0x00000010
	#define HOME2_CHECK						0x00000020
	#define HOME_ALL_CHECK				0x00000030

	#define HOME1_SENSING						0x00000010
	#define HOME1_NOT_SENSING				0x00000000

	#define HOME2_SENSING						0x00000020
	#define HOME2_NOT_SENSING				0x00000000

	#define HOME_ALL_SENSING				0x00000030
	#define HOME_ALL_NOT_SENSING		0x00000000
*/
	#define X_HOME1_CHECK						0x00000010
	#define X_HOME1_SENSING					0x00000010
	#define X_HOME1_NOT_SENSING			0x00000000
	
	#define Y_HOME1_CHECK						0x00000010
	#define Y_HOME1_SENSING					0x00000010
	#define Y_HOME1_NOT_SENSING			0x00000000
	
	#define Z_HOME1_CHECK						0x00000010
	#define Z_HOME1_SENSING					0x00000010
	#define Z_HOME1_NOT_SENSING			0x00000000

	#define Z_HOME2_CHECK						0x00000020
	#define Z_HOME2_SENSING					0x00000020
	#define Z_HOME2_NOT_SENSING			0x00000000

	#define Z_HOME_ALL_CHECK				0x00000030
	#define Z_HOME_ALL_SENSING			0x00000030
	#define Z_HOME_ALL_NOT_SENSING	0x00000000

	#define SYR_HOME1_CHECK					0x00000010
	#define SYR_HOME1_SENSING				0x00000010
	#define SYR_HOME1_NOT_SENSING		0x00000000
// 90�� ������ ������ Ȩ ����(82��)
	#define VAL_HOME1_CHECK					0x00000010
	#define VAL_HOME1_SENSING				0x00000010		// ���� �κ�
	#define VAL_HOME1_NOT_SENSING		0x00000000
// 60�� ������ ������ Ȩ ����(52��)
	#define INJ_HOME1_CHECK					0x00000010
	#define INJ_HOME1_SENSING				0x00000010		// ���� �κ�
	#define INJ_HOME1_NOT_SENSING		0x00000000
#endif	

#define TRAY60_COUNT_X				5		// count
#define TRAY60_COUNT_Y				12	// count

#if	MECH_VER == 6 || MECH_VER == 7
#define TRAY60_RIGHT_START_X		46.5		// mm
#define TRAY60_LEFT_START_X			121.5 	// mm
#define TRAY60_START_Y			0					// mm
#endif

#define TRAY60_INTERVAL_X		13.5			// mm
#define TRAY60_INTERVAL_Y		13.5			// mm�� 

#define TRAY40_COUNT_X				4		// count
#define TRAY40_COUNT_Y				10	// count

#define TRAY40_RIGHT_START_X		(TRAY60_RIGHT_START_X + 2.25)	// 44.25		// mm
#define TRAY40_LEFT_START_X			(TRAY60_LEFT_START_X + 2.25) 	//119.25 	// mm
#define TRAY40_START_Y			0					// mm

#define TRAY40_INTERVAL_X		16.5			// mm
#define TRAY40_INTERVAL_Y		16.5			// mm

#if 0
#define TRAY96_COUNT_X		12
#define TRAY96_COUNT_Y		8
#else
#define WELLPALTE_96_COUNT_X		12
#define WELLPALTE_96_COUNT_Y		8
#endif

#define TRAY_COUNT_MAX_X			WELLPALTE_96_COUNT_X
#define TRAY_COUNT_MAX_Y			TRAY60_COUNT_Y

// �ⱸ�� �ϼ��� �ٽ� ����
// Left - Back
#define TRAY96_FRONT_START_X		58.5		// mm
#define TRAY96_FRONT_START_Y		86.5		 // mm  �ⱸ ���� 
//Right - Front
#define TRAY96_BACK_START_X			58.5 	// mm
#define TRAY96_BACK_START_Y			0.0 	// mm

#define TRAY96_RIGHT_START_X		TRAY96_BACK_START_X		// mm
#define TRAY96_LEFT_START_X			TRAY96_FRONT_START_X 	// mm
#define TRAY96_START_Y					TRAY96_BACK_START_Y		// mm

#define TRAY96_INTERVAL_X		9.0			// mm
#define TRAY96_INTERVAL_Y		9.0			// mm

// 500ul : 60mm
// 1rev  --> 1.8�� = 0.012192mm , 360�� = 2.4384mm
// 1ul --> 0.12mm --> 9.842519685039370078740157480315 * 8pulse = 78.74015748031496062992125984252
// 78.74�� �ݿø��Ͽ����

#define SYR_PULSE_PER_VOL_250_FLOAT 157.480314
#define SYR_PULSE_PER_VOL_250 			157

#define SYR_PULSE_PER_VOL_500_FLOAT	78.740157				// 1/8 step  
#define SYR_PULSE_PER_VOL_500				79 // 78.74015748031496062992125984252				// 1/8 step  

#define SYR_PULSE_PER_VOL_1000_FLOAT 39.3700785
#define SYR_PULSE_PER_VOL_1000			39

#define SYR_PULSE_PER_VOL_2500_FLOAT 15.7480314
#define SYR_PULSE_PER_VOL_2500 		 16

#define MIN_SPEED						0
#define MAX_SPEED						149
#define MIN_SPEED_DEFAULT		MIN_SPEED
#define MAX_SPEED_DEFAULT		MAX_SPEED

// =================================================================================
// Auto Sampler ==> Action
// =================================================================================
#define WASH_TIMES_DEFAULT_INIT		1		// 3
#define WASH_TIMES_DEFAULT				1		// 3

#endif	// #define __DEF_STEPCON_H__
