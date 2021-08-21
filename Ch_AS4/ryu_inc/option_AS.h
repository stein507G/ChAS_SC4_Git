#ifndef OPTION_AS_H_
#define OPTION_AS_H_


#define BOARD_TEST_PRGM				0			// �����׽�Ʈ�� �������� - �ڵ� �ݺ� �
#define MACHINE_TEST_PRGM			0			// �ⱸ���� �׽�Ʈ - Key B/d �Է��̿�

// debug_inform.h
#define NDEBUG_DP

// 20210503
#define SOC_TYPE_SF2					1
	// SYS_TICKS_SF2

#define NET_CONNECT_TEST			0

// 20210603 - RTC Write 
#define RTC_WRITE_SF2			1



//===========================================================================================
// 20200514 Encoder ���� - new Board�� ������.
//===========================================================================================
#define NEW_BOARD_ENCODER_2020			1
#if NEW_BOARD_ENCODER_2020
	#define FPGA_VERSION_TRIM			1
		#define FPGA_VERSION_TRIM_BREAK			1				// motorResetCmd   or   motorReadyCmd
	// 20200515
		#define FPGA_VERSION_TRIM_ADJUST_SLEEP		1			// AS_ADJUST_MODE_SLEEP

		#define GO_POS_ETC_USE					1

		#define BUZZER_MODIFY						1					// ��������� �ݴ� 
//===========================================================================================
#else
//===========================================================================================
// 20200507 TRIM - old Board�� �����. - old board����� ��� �Ʒ� ���� ���� 
//===========================================================================================
	#define FPGA_VERSION_TRIM			1
		#define FPGA_VERSION_TRIM_BREAK			1				// motorResetCmd   or   motorReadyCmd
	// 20200515
		#define FPGA_VERSION_TRIM_ADJUST_SLEEP		1			// AS_ADJUST_MODE_SLEEP
#endif

#define FPGA_VERSION_TRIM_PUMP_TIME			1

//==================================================================================
// readyChkEncoder_Syr()
// #if SYRINGE_ENCODER_REMOVE_WASHING
// 20200526
// Washing ����
// SYRINGE_ENCODER_REMOVE_END
// endsequence ���� - ���� ã�� ���� 
// ������ �������� ���� - ���� ã�� ���� 
// 
//==================================================================================
#define SYRINGE_ENCODER_REMOVE_WASHING			1
#define SYRINGE_ENCODER_REMOVE_END					1
#define SYRINGE_ENCODER_REMOVE_INIT					1
//===========================================================================================

// 20200527 ���� ���̾� ���� ��ô ����. ���� 
#define WASH_BETWEEN_VIALS_BEFORE_FIX				1


// 20200529
// Diagnosis���� ��� �߰� 
// Photo interrupter �׽�Ʈ �뵵�� ����Ѵ�.
//#define PHOTO_SENSOR_TEST						1			


// 20200609
// ���̾� Ʈ���� �¿� 
#define VIAL_TRAY_CONFIG_FIX				1


// #define STEPCONTROL_1_0                 0x40057000U
// CoreAPB3_0�� 32bit�� �����Ͽ���. offset�� 4byte������
	#define  FABRIC_STEPSET_APB20_BASE_ADDR		STEPCON_0//STEP_SET16_0

/*
#define COREGPIO_0                      0x40050000U
#define STEPCONTROL18_0                 0x40051000U
#define STEPCONTROL18_1                 0x40052000U
#define STEPCONTROL18_2                 0x40053000U
#define STEPCONTROL18_3                 0x40054000U
#define STEPCONTROL13_0                 0x40055000U
#define STEPCONTROL13_1                 0x40056000U
#define PWM_CON8_0                      0x40057000U
*/

//#define  STEPSET_BASEADDR_MOTOR_X					STEPCONTROL18_0//STEPASCON18_0

// 20200813
// - whahing port ���� ���� 5mm �� ���� ������ , Vial height ���� ���ϸ� 5mm���� 
#define  COORD_Z_WASH_MODIFY				1	

// injection height �� ���� Vial bottom �� ����Ǵ� ���� ����
// Wash, Waste
#define  VIAL_BOTTOM_CAL_FIX				1

// Injection ����(Inj port z)  adjust �ʱⰪ ���� (3mm)
#define  INJECTION_DEPTH_MODIFY			1

// vial heigth adjust �ʱⰪ ����
#define  VIAL_TOP_MODIFY				1


// 20210127
// RGB LED Control
//#define  FLED_CONTROL_ADD				1
// POWER BUTTON Control
#define  POWER_BTN_CONTROL_ADD				1


















		// Autotunning ����
#define AUTO_TUNNING_CONTROL		0

#define HEATING_SYSTEM					0
#define COOLING_SYSTEM					1
//#define AUTO_TUNNING_TARGET			HEATING_SYSTEM
#define AUTO_TUNNING_TARGET			COOLING_SYSTEM

#if	AUTO_TUNNING_TARGET == HEATING_SYSTEM
#define AUTO_TUNNING_SET_TEMP	40.0		// Heating
#else
//#define AUTO_TUNNING_SET_TEMP	15.0		// Cooling
#define AUTO_TUNNING_SET_TEMP	6.0		// Cooling - 20191128
#endif


#define MOTOR_POWER_DOWN_MODE			1						// 20171213

#define MICRO_PUMP_VOLTAGE_ADJUSTMENT			1			// 20171216

#define VIAL_HEIGHT_DATUM			1			// vial ���� ����
#define VIAL_POS_MODIFY				1			// Ʈ���� �¿�Ʋ��������

/*
#define EEPROM_SAVE_MODIFY		1			// eeprom������ �ѹ��� ���� �ֵ��� - ���� ����
#define HOME_CHECK_MODIFY_1ST		1		// Ȩüũ�� ���ݴ� Ȩ�������� �̵�
#define HOME_CHECK_MODIFY_2ND		2		// Ȩüũ�� �ٷ� ���� �� reset -> ready
#define HOME_CHECK_MODIFY		HOME_CHECK_MODIFY_1ST	//HOME_CHECK_MODIFY_2ND
*/
//#define MODIFY_HOME_CHECK		1
//#define MODIFY_HOME_Z_CHECK	1

// home check and go home
// ���Գ����� Ȩ���� �̵��� ���� üũ
#define MODIFY_GO_HOME_AND_CHECK	0

// �����ǵ�����  sleep ����
#define INJECTION_END_MOTOR_SLEEP		1
// ������ ������ sleep ����
#define READY_MODE_MOTOR_SLEEP		1
// run �� ��� ���¿��� Sleep
//#define RUN_WAIT_MOTOR_SLEEP		1

// Power Down mode�� ���۽�Ų��.

// ���� �߻��� COMMAND_INIT ����� �޾��� ��� subMenu�ʱ�ȭ
#define INIT_SUBMENU	         1

// Step ���� �÷�������Ʈ ��� ���� (goSyr())
//#define FLOAT_CAL_ERROR_FIX			1			// �������� 20180920
//#define FLOAT_INPUT_GOSYR_FIX			1		// �������� 20180920

// 20170927 �����Ǻ����Է� �Ҽ��� 1�ڸ�
//#define FLOAT_INPUT_INJECTION_VOLUME			1

// 20170927 ȥ�����α׷������Է� �Ҽ��� 1�ڸ�
#define FLOAT_INPUT_MIX_VOLUME			1

#define VIAL_PLATE_CHECK				0	// vialPlateCheck

// �������� 20180920
//#define POZ_UPPER_DETECT_MODIFY		1  // z�� �̵��� upper check �߰� -upper���� üũ �ȵǰ� detect��ġ���� üũ �Ǿ�� ��.

// z��Ʒ��� �̵���(Wash , Injector)  �̵��� ������ �߸� ���ؼ��ؼ� ���� �߻�
// - ������ ��Ȯġ ������ �� ���� ���� üũ�� �̺�å
//#define POZ_UPPER_DETECT_ERROR_FIX		1  // 20180920
#define POZ_UPPER_DETECT_ERROR_DELAY		1  // 20210604

#define POZ_UPPER_DETECT_ERROR_HOME_DELAY		1  // 20210614	// init_home

#define HOME_CHECK_RETRY_COUNT		3  // 20210614	// init_home ���� �߻��� ��õ� 



//#define MOVE_XY_DELAY_MODIFY	1		// �������� 20180920
//#define WAKEUP_DELAY_MODIFY		1		// �������� 20180920

// ���������� �ʰ� �ѹ��� �о ó���ϵ��� �Ѵ�.
//#define EEPROM_READING_SPEED_UP		1		// �������� 20191223

// 20170927 microPickInjection���� ���
#define TRANSPORT_LIQUID_USE			1

//#define VALVE_HOME_CHECK_MODIFY		1
//#define INJECTOR_HOME_CHECK_MODIFY		1

// 20171213
// Fault ���¿����� COMMAND_INIT����
#define RECEIVE_COMMAND_INIT_FAULT		0
// ��� ���¿��� COMMAND_STOP ����
#define RECEIVE_COMMAND_STOP_ANY		1

// 20200602
// initialize �߿��� ��� ����.
#define RECEIVE_COMMAND_INIT_ANY		1

// 20181010
// �ʱ⿡ ���嵥���͸� �Ҿ� ������츦 ����Ͽ� ����
// ������� ���
// �����Ϳ� ���� CheckSum
#define EEPROM_ADJUST_DATA_SAVE_FIX			1

// 20191205 - �������̵��� - �ⱸ���� Ʋ���� ������ϴ� ������
#define MODIFY_MAX_SPEED_Z			1

// 20201130	-
// ���� �������� �Ŀ��� ���̰� z���� ����(�ϰ�)��Ű�� ������ �߻��Ѵ�.
// ������ �� ���� - �׶��� ���� ����
#define Z_MOTOR_NOISE_FIX				1
// 20191223 - �ƾ�巹�� ����������� - �ø��� �Է½� �����ǵ���
#define MAKE_MAC_ADDR_SERIAL		1
	#define MAKE_MAC			0xAA
	#define LOAD_MAC			0x00
	#define APPLY_MAC			0x55

#define LOAD_IPADDRESS    0xAA
#define DEFAULT_IPADDRESS    0x55

#define MODIFY_MAX_SPEED_X		1
// 20191223 -- modify speed Y axis - �ⱸ�� ���� �ҷ� - �����Ϸ� ������
#define MODIFY_MAX_SPEED_Y		1

//#define MODIFY_DEFAULT_SYRINGE_DOWN_SPEED			1		// SPEED_LEVEL_3

#define MODIFY_ADJUST_PRGM_TEST_FN	1

// 20200109 -- tcp_close ������ ���ϵ���
#define FIX_TCP_CLOSE_EXCUTE_1			1
#define FIX_TCP_CLOSE_EXCUTE_2			1			// �������� �����ؾ� �ȴ�. - ������ ���ϴ�?

// 20200217 -- Driver IC ���� ����
// Z���� Sleep mode���� Power down mode�� ������ġ �����Ѵ�.
#define POWER_DOWN_SYR_MODIFY		1	// Power Down - Syringe
#define POWER_DOWN_Z_MODIFY		1		// Power Down - Z�� ����

// 20200514
#define SYRINGE_HOME_FIND_FIX		1 // Ȩã�� FIX










// 20210611
#define HOME_FIND_TIME_CHECK_X		1 // Ȩã�� - �����ð����� �� ã���� ���� 
#define HOME_FIND_TIME_CHECK_Y		1 // Ȩã�� - �����ð����� �� ã���� ���� 
#define HOME_FIND_TIME_CHECK_Z		1 // Ȩã�� - �����ð����� �� ã���� ���� 

// 20210615
#define Y_ENCODER_CHECK_ERR_MODIFY			1

// 20210618
#define CHECK_POSITION_MODIFY			1		// �����߻��� checkPosition() �����
#if CHECK_POSITION_MODIFY
	#define Z_POS_CHECK_MASK				0x20
	#define Z_POS_CHECK1						0x30
	#define Z_POS_CHECK2						0x20
#else
	#define Z_POS_CHECK_MASK				0x30
	#define Z_POS_CHECK1						0x30
	#define Z_POS_CHECK2						0x20
#endif



//########################################################################################
// TCP_IP
//########################################################################################
#define		LWIP_VERSION_2				0

// 20210617
#define TCP_SERVER_RECV_MODIFY			1
// 20210618
#define TCP_SERVER_CONNECTION_ONLY_ONE			0


// ��â�� ���� TCP���� 
// lwipopts.h
#define KCJ_TCP_LWIPOPTS_H						1	
// lwIP Send/Recv �� ť ���� ������ Ethernet Interrupt ����ȭ ó��.
#define KCJ_TCP_INTR			0



#define KCJ_TCP_CONNECTION_CLOSE				1


#define KCJ_TCP_SERVER		1					// 20210722


//#define KCJ_USED_RECV_QUEUE							1


#define USED_RECV_QUEUE		1	//(+)210715 CJKIM, Received data process changing

#define SERVER_CLOSE_RUN_STOP_CMD			0


//########################################################################################
// RS422
// 20210727
//########################################################################################
#define RS422_ENABLE			1

#define RS422_MY_ID					RS422_ID_AS

#define RS422_MASTER			0
#define RS422_SLAVE				1

#define RS422_DISABLE_INTR			1			// erhernet ����Ǹ� RS422���ͷ�Ʈ ��Ȱ��
#define RS422_SIZE_4BYTE_ALIGN 			1		// size -- 4 byte align


// ����(ť)�� 4BYte ������ ����
#define RS422_BUFFER_4BYTE		0
#define RS422_USE_MEMCPY 			0




#endif	// #ifndef OPTION_AS_H_
