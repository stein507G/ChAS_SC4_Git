/*************************************************************************
 * (c) Copyright 2009 Actel Corporation.  All rights reserved.
 *
 *  Application demo for Smartfusion
 *
 *
 * Author : Actel Application Team
 * Rev     : 1.0.0.4
 *
 **************************************************************************/

/**************************************************************************/
/* Standard Includes */
/**************************************************************************/

#include "config.h"
//#include <stdio.h>
//#include <stdlib.h>

/**************************************************************************/
/* Driver Includes */
/**************************************************************************/

//#include "../drivers/mss_uart/mss_uart.h"

// ryu
//#include "def_com.h"
//#include "define_AS.h"
//#include "debug_inform.h"

//#include "../common_inc/Def_stepCon.h"
//#include "../common_inc/protocolType_AS.h"
//#include "../common_inc/comType_AS.h"

//#include "../common_inc/protocol_AS.h"
extern char pPacketBuff[TCP_SEND_PACKET_BUFF_SIZE];
extern struct tcp_pcb *netAS_PCB;


#include "net_AS.h"
#include "LcasAction_task.h"
#include "MotorCtrl_task.h"

#include "EEPROM.h"

// test
#include "Led_task.h"


//#include "main.h"

#if BOARD_TEST_PRGM || MACHINE_TEST_PRGM
#include "testPrgm.h"
#endif

uint32_t nextCaseDelay = 0;
uint32_t stepDelay = 0;
uint32_t check_time_cnt = 0;
extern uint32_t sysTicks;
extern uint32_t System_ticks;

//===============================================================
// subMenu
extern uint8_t subMenu_w;
extern uint8_t subMenu_wch;
extern uint8_t subMenu_es;
extern uint8_t subMenu_sma;
extern uint8_t subMenu_wxch;
extern uint8_t subMenu_ghx;
extern uint8_t subMenu_ghcx;
extern uint8_t subMenu_as;
extern uint8_t subMenu_ds;
extern uint8_t subMenu_fli;
extern uint8_t subMenu_pli;
extern uint8_t subMenu_mpi;
extern uint8_t subMenu_map;
extern uint8_t subMenu_mmp;
extern uint8_t subMenu_mwp;

extern uint8_t subMenu_gpvz;
extern uint8_t subMenu_gpez;
extern uint8_t subMenu_gpezc;
extern uint8_t actRetry_gpezc;

extern uint8_t subMenu_gpezci;
extern uint8_t subMenu_gpv;

extern uint8_t subMenu_ihpv;
extern uint8_t subMenu_ihpi;
extern uint8_t subMenu_ihps;
extern uint8_t subMenu_ihpz;
extern uint8_t subMenu_ihpx;
extern uint8_t subMenu_ihpy;
extern uint8_t subMenu_ihpa;

extern uint8_t subMenu_ami;
extern uint8_t subMenu_ane;
extern uint8_t subMenu_ase;

extern uint8_t subMenu_grpx;
extern uint8_t subMenu_grpy;
extern uint8_t subMenu_grpz;

extern uint8_t subMenu_rcrx;
extern uint8_t subMenu_rcry;
extern uint8_t subMenu_rcrz;

// mix
extern uint8_t subMenu_mp;

//===============================================================

#if 0
Step_Accel_t sAccel[6][STEP_DATA_ARRAY_COUNT];
#else
Step_Accel_t sAccel[STEP_DATA_ARRAY_COUNT];
uint16_t s_pulse_repeat[STEP_MOTOR_COUNT][STEP_DATA_ARRAY_COUNT];
#endif
//MOTOR_STATE_t mState;
LCAS_DEV_SET_t devSet;

extern SYSTEM_CONFIG_t sysConfig;
extern LCAS_DEV_STATE_t devState;

extern LCAS_RUN_SETUP_t runSetup;
extern LCAS_STATE_t LcasState;

extern LCAS_SEQUENCE_t sequence;
extern LCAS_SERVICE_t service;

extern LCAS_USED_TIME_t usedTime;

XY_COORD_t xyzCoord;



extern uint32_t gAction;

#if RS422_ENABLE
extern RS422_ST rs422if;
//extern RS422_BUF tx;
#endif


//void load_StepDataAddr(uint8_t who, uint32_t addr, uint32_t data)
void load_StepDataAddr(uint32_t who, uint32_t addr, uint32_t data)
{
	uint32_t baseAddr = TWOPORTRAM_BASEADDR_MOTOR_X; // ????
	
	switch(who) {
		case MOTOR_X: 				baseAddr = TWOPORTRAM_BASEADDR_MOTOR_X; break;
		case MOTOR_Y:					baseAddr = TWOPORTRAM_BASEADDR_MOTOR_Y; break;
		case MOTOR_Z:					baseAddr = TWOPORTRAM_BASEADDR_MOTOR_Z; break;
		case MOTOR_SYR: 			baseAddr = TWOPORTRAM_BASEADDR_MOTOR_SYR; break;
		case MOTOR_VAL:				baseAddr = TWOPORTRAM_BASEADDR_MOTOR_VAL; break;
		case MOTOR_INJ:				baseAddr = TWOPORTRAM_BASEADDR_MOTOR_INJ; break;
	}

//dp( "[%d] [%d] baseAddr = %x data = %d ) \n\r", who, addr, baseAddr,data);	
//	*((uint32_t volatile *)(TWO_PORT_RAM_BASE_ADDR + addr*4)) = data;
//	*((uint32_t volatile *)(baseAddr + addr*4)) = data;
	*((uint32_t volatile *) (baseAddr + (addr << 2))) = data;
}


//=============================================================================================
//�߿� -- ���� ����Ŭ���� 4.6875Mhz�� �ι�� �������̴�.
//     -- �������� �ӵ��� 600rpm // 1200rpm�� �����ϴ�(���� 60rpm)
//     -- ���ۼӵ� 15rpm - 300rpm�Է°� ��¼ӵ� 27�� ����� ���
//     -- ������ 
//=============================================================================================
void make_StepData(uint32_t who, uint32_t Freq, uint32_t rpm, float interval, uint32_t microStep)
{
	int n = STEP_DATA_ARRAY_COUNT/2; 
	int i,i_1;
	int k;
	uint16_t u16IntervalMax;		// ���� �ӵ� �޽� �� 
	uint16_t u16MaxSpeedCnt;			// ��� ������ �� ���ܼ�(�ִ�ӵ��� �Ǵ� ����)

	uint16_t s_pulse_repeat_1;		// repeat - 1
	uint16_t s_interval_1;					// interval - 1 ; //step gap - 1

	uint32_t s_memory;					// Two Port RAM �޸𸮿� ����� ������

	uint16_t rpm_1;//,rpm;
	float pps, r_pulse;

	uint16_t s_pulse_mod,s_pulse_mod_1;
//int16_t accumul_cnt;		// repeat ����
	uint16_t ar2;

dp( "[%d] FREQ = %dHz intervalConst=%d.%d microStep=1/%d\n\r maxRPM=%5d ", who, Freq, (int)interval, (int)( interval*10 - (int)interval*10 ), microStep,rpm);

	for(i=1; i <=	n ; i++) {
		k = i-1;
		sAccel[k].ar1 = i;
		sAccel[STEP_DATA_ARRAY_COUNT - i].ar1 = k;
	}

	r_pulse = (360. / STEP_ANGLE) * microStep;

//	rpm = TARGET_RPM;
//	pps = 360. / STEP_ANGLE * MICRO_STEP_COUNT * (rpm / 60.);
//	pps = 360. / STEP_ANGLE * microStep * (rpm / 60.);	// rpm= �����ӵ� , �����ӵ� PPS
		pps = r_pulse * (rpm / 60.);	// rpm= �����ӵ� , �����ӵ� PPS
		
//	sAccel[who][STEP_DATA_ARRAY_COUNT-1].s_interval = STEP_FREQUENCY / pps + 0.5;	// �ݿø�.
	sAccel[STEP_DATA_ARRAY_COUNT-1].s_interval = Freq / pps + 0.5;	// �ݿø�.
	
	for(i=(STEP_DATA_ARRAY_COUNT-2); i >=	0 ; i--) {	// rpm 
//		ar2 = sAccel[who][i].ar1 / ACC_INTERVAL + 0.99;	// �ڸ��ø� 		
		ar2 = sAccel[i].ar1 / interval + 0.99;	// �ڸ��ø� 		
		rpm_1 = rpm - ar2;

//		pps = (360. / STEP_ANGLE) * microStep * (rpm_1 / 60.);
		pps = r_pulse * (rpm_1 / 60.);
//		sAccel[who][i].s_interval = STEP_FREQUENCY / pps + 0.5;	// �ݿø�.
		sAccel[i].s_interval = Freq / pps + 0.5;	// �ݿø�.
		
		rpm = rpm_1;

//    dp( "[%3d] ar1=%3d interval=%5d pps=%4d rpm=%4d\n\r", i, (int)sAccel[i].ar1, sAccel[i].s_interval,(int)pps , rpm);
	}

dp( "minRPM=%4d interval(start=%5d end=%4d) \n\r", rpm, sAccel[0].s_interval, sAccel[149].s_interval);

	u16IntervalMax = sAccel[0].s_interval;
	s_pulse_mod_1 = 0;	
//	sAccel[0].s_pulse_repeat	= 1;
	s_pulse_repeat[who][0] = 1;

//	sAccel[0].accumul_cnt = sAccel[0].s_pulse_repeat; // =1;	
	sAccel[0].accumul_cnt = s_pulse_repeat[who][0]; // =1;		

	
//	s_pulse_repeat_1 = sAccel[0].s_pulse_repeat - 1;
	s_pulse_repeat_1 = s_pulse_repeat[who][0] - 1;
	s_interval_1 = sAccel[0].s_interval - 1;
	s_memory = (s_pulse_repeat_1 << 13) | s_interval_1;
	
	load_StepDataAddr(who,0,s_memory);
	
//dp( "[%3d] ar1=%3d interval=%5d repeat=%4d accumul=%4d\n\r", 0, (int)sAccel[who][0].ar1, sAccel[who][0].s_interval,
//	sAccel[who][0].s_pulse_repeat , sAccel[who][0].accumul_cnt);
uint32_t data;
uint32_t baseAddr;

	switch(who) {
		case MOTOR_X: 				baseAddr = TWOPORTRAM_BASEADDR_MOTOR_X; break;
		case MOTOR_Y:					baseAddr = TWOPORTRAM_BASEADDR_MOTOR_Y; break;
		case MOTOR_Z:					baseAddr = TWOPORTRAM_BASEADDR_MOTOR_Z; break;
		case MOTOR_SYR: 			baseAddr = TWOPORTRAM_BASEADDR_MOTOR_SYR; break;
		case MOTOR_VAL:				baseAddr = TWOPORTRAM_BASEADDR_MOTOR_VAL; break;
		case MOTOR_INJ:				baseAddr = TWOPORTRAM_BASEADDR_MOTOR_INJ; break;
	}

	for(i=1; i < STEP_DATA_ARRAY_COUNT ; i++) {
		i_1 = i - 1;
		s_pulse_mod = s_pulse_mod_1 + u16IntervalMax - ((s_pulse_mod_1 + u16IntervalMax) / sAccel[i].s_interval) * sAccel[i].s_interval;
//		sAccel[i].s_pulse_repeat = (s_pulse_mod_1 + u16IntervalMax) / sAccel[i].s_interval;
		s_pulse_repeat[who][i] = (s_pulse_mod_1 + u16IntervalMax) / sAccel[i].s_interval;

		
//		accumul_cnt = accumul_cnt + sAccel[i].s_pulse_repeat;
//		sAccel[i].accumul_cnt = sAccel[i_1].accumul_cnt + sAccel[i].s_pulse_repeat;
		sAccel[i].accumul_cnt = sAccel[i_1].accumul_cnt + s_pulse_repeat[who][i];

		s_pulse_mod_1 = s_pulse_mod;	

//		s_pulse_repeat_1 = sAccel[i].s_pulse_repeat - 1;
		s_pulse_repeat_1 = s_pulse_repeat[who][i] - 1;
		s_interval_1 = sAccel[i].s_interval - 1;
	
		s_memory = (s_pulse_repeat_1 << 13) | s_interval_1;		// ����Ǵ°� 17~13,12~0bit
		
		load_StepDataAddr(who,i,s_memory);

//    dp( "[%3d] ar1=%3d interval=%5d repeat=%4d accumul=%4d\n\r", i, (int)sAccel[who][i].ar1, sAccel[who][i].s_interval,
//			sAccel[who][i].s_pulse_repeat , sAccel[who][i].accumul_cnt);
	}

// �ִ��������� �ݺ��� 1���� �ϴ� ���� ���� �� - �ƴ϶� ���� ��� �� ����.

//	u16MaxSpeedCnt = accumul_cnt; //
	u16MaxSpeedCnt = sAccel[i].accumul_cnt;

#if 0
// ����Ǵ� ���� 1���� ������ ���� �Ѵ�. -- FPGA���� ����ϱ� ������ 
	for(i=0; i < STEP_DATA_ARRAY_COUNT ; i++) {
		s_pulse_repeat_1 = sAccel[who][i].s_pulse_repeat - 1;
		s_interval_1 = sAccel[who][i].s_interval - 1;
	
		s_memory = (s_pulse_repeat_1 << 13) | s_interval_1;
		
		load_StepDataAddr(i,s_memory);					
	}
#endif
	
	dp( " end : Step Data load!! \n\r" );

}

//#define STEP_DATA_ARRAY_COUNT		150
#define STEP_DATA_ARRAY_COUNT_ISO		150 // 156 // 192 //240
void make_StepData_ISO(uint32_t who, uint32_t Freq, uint32_t rpm, float s_rpm, uint32_t microStep)
{
	int n = STEP_DATA_ARRAY_COUNT_ISO/2; 
	int i,i_1;
	int k;
	uint16_t u16IntervalMax;		// ���� �ӵ� �޽� �� 
	uint16_t u16MaxSpeedCnt;			// ��� ������ �� ���ܼ�(�ִ�ӵ��� �Ǵ� ����)

	uint16_t s_pulse_repeat_1;		// repeat - 1
	uint16_t s_interval_1;					// interval - 1 ; //step gap - 1

	uint32_t s_memory;					// Two Port RAM �޸𸮿� ����� ������

	float rpm_1,i_rpm;
	float pps, r_pulse;

	uint16_t s_pulse_mod,s_pulse_mod_1;
//int16_t accumul_cnt;		// repeat ����
	uint16_t ar2;

//	uint16_t s_rpm;	// ���� RPM

//	s_rpm = interval;
	i_rpm = (rpm - s_rpm) / (STEP_DATA_ARRAY_COUNT_ISO - 1);
	r_pulse = (360. / STEP_ANGLE) * microStep;
dp( "[%d] FREQ = %dHz StartRPM=%d.%d microStep=1/%d\n\r maxRPM=%5d ", who, Freq, (int)s_rpm, (int)( s_rpm*10 - (int)s_rpm*10 ), microStep,rpm);
/*
	for(i=1; i <=	n ; i++) {
		k = i-1;
		sAccel[who][k].ar1 = i;
		sAccel[who][STEP_DATA_ARRAY_COUNT - i].ar1 = k;
	}
*/
	pps = r_pulse * (rpm / 60.);	// rpm= �����ӵ� , �����ӵ� PPS
	sAccel[STEP_DATA_ARRAY_COUNT_ISO-1].s_interval = Freq / pps + 0.5;	// �ݿø�.

	for(i=0 ; i < STEP_DATA_ARRAY_COUNT_ISO ; i++) {
		rpm_1 = i_rpm * i + s_rpm;
		pps =  r_pulse * (rpm_1 / 60.);
		sAccel[i].s_interval = Freq / pps + 0.5;	// �ݿø�.
//dp( "[%d] interval = %d pps = %d ) \n\r", i, sAccel[i].s_interval, pps);
	}

dp( "minRPM=%4d Pulse(start=%5d end=%4d) \n\r", (int)s_rpm, sAccel[0].s_interval, sAccel[149].s_interval);

	u16IntervalMax = sAccel[0].s_interval;
	s_pulse_mod_1 = 0;	
//	sAccel[0].s_pulse_repeat	= 1;
	s_pulse_repeat[who][0]	= 1;

//	sAccel[0].accumul_cnt = sAccel[0].s_pulse_repeat; // =1;	
	sAccel[0].accumul_cnt = s_pulse_repeat[who][0]; // =1;	
	
//	s_pulse_repeat_1 = sAccel[0].s_pulse_repeat - 1;
	s_pulse_repeat_1 = s_pulse_repeat[who][0] - 1;
	s_interval_1 = sAccel[0].s_interval - 1;
	s_memory = (s_pulse_repeat_1 << 13) | s_interval_1;
	
	load_StepDataAddr(who,0,s_memory);
	
	for(i=1; i < STEP_DATA_ARRAY_COUNT_ISO ; i++) {
		i_1 = i - 1;
		s_pulse_mod = s_pulse_mod_1 + u16IntervalMax - ((s_pulse_mod_1 + u16IntervalMax) / sAccel[i].s_interval) * sAccel[i].s_interval;
//		sAccel[i].s_pulse_repeat = (s_pulse_mod_1 + u16IntervalMax) / sAccel[i].s_interval;
		s_pulse_repeat[who][i] = (s_pulse_mod_1 + u16IntervalMax) / sAccel[i].s_interval;
		
//		sAccel[i].accumul_cnt = sAccel[i_1].accumul_cnt + sAccel[i].s_pulse_repeat;
		sAccel[i].accumul_cnt = sAccel[i_1].accumul_cnt + s_pulse_repeat[who][i];
		
		s_pulse_mod_1 = s_pulse_mod;	

//		s_pulse_repeat_1 = sAccel[i].s_pulse_repeat - 1;
		s_pulse_repeat_1 = s_pulse_repeat[who][i] - 1;
		s_interval_1 = sAccel[i].s_interval - 1;
	
		s_memory = (s_pulse_repeat_1 << 13) | s_interval_1;		// ����Ǵ°� 17~13,12~0bit
		
		load_StepDataAddr(who,i,s_memory);

	}

// �ִ��������� �ݺ��� 1���� �ϴ� ���� ���� �� - �ƴ϶� ���� ��� �� ����.

//	u16MaxSpeedCnt = accumul_cnt; //
	u16MaxSpeedCnt = sAccel[i].accumul_cnt;

	dp( " end : Step Data load!! \n\r" );

}

//==============================================================
// totalStep > 0 : (+)Ȩ�� �ݴ�� �̵�
// totalStep < 0 : (-)Ȩ�������� �̵�
// maxSpeed <= ACT_SPEED_MAX_X = 149
// maxSpeed <= ACT_SPEED_MAX_VAL(ACT_SPEED_MAX_INJ) = 255
// minSpedd = 0
//==============================================================
#define MOVE_STEP_SPEED_MAX			ACT_SPEED_MAX_X
#define MOVE_STEP_SPEED_INJ			ACT_SPEED_MAX_INJ
#define MOVE_STEP_SPEED_MIN			0
int moveStep(uint32_t who,int32_t tStep)
{
	int i;
	int totalHaif,totalStep;
	int stateZ;
	uint32_t AccelStep = 0;	// �ӵ��� ���̱� ���ؼ� ���������� ���������� ��ġ�� ����Ѵ�. ����� �ٽ� ���󺹱�
	uint32_t DeaccStep = 0;

	// ctrlCmd = Dir
	uint32_t baseAddr;// = BASEADDR_MOTOR_X;
	uint32_t rpm,step_gap_cnt;
	uint32_t dir;
	uint32_t ctrlCmd;

	if(tStep > 0) {	// �ݴ�Ȩ
		totalStep = tStep;
		if(devSet.homeDir[who]== DIR_CCW) {	// MOTOR_X , MOTOR_Y , MOTOR_SYR
			dir = DIR_CW;
			ctrlCmd = DIR_CW | CMD_START;
		}
		else {	// MOTOR_Z , MOTOR_VAL , MOTOR_INJ
			dir = DIR_CCW;
			ctrlCmd = DIR_CCW | CMD_START;
		}
	}
	else if(tStep < 0) {
		totalStep = -tStep;
		if(devSet.homeDir[who]== DIR_CCW) {// MOTOR_X , MOTOR_Y , MOTOR_SYR
			dir = DIR_CCW;
			ctrlCmd = DIR_CCW | CMD_START;
		}	
		else {	// MOTOR_Z , MOTOR_VAL , MOTOR_INJ
			dir = DIR_CW;
			ctrlCmd = DIR_CW | CMD_START;			
		}
	}
	else {
		return MOVE_CMD_OK;
	}

#if 0
	totalHaif = totalStep / 2;
#else
 	totalHaif = totalStep >> 1;
#endif

	baseAddr = devSet.baseAddr[who];

	switch(who) {
		case MOTOR_X:
		case MOTOR_Y:
//			stateZ = devState.btMotorState[MOTOR_Z] & 0x00000030;
			stateZ = devState.btMotorState[MOTOR_Z] & 0x30;
	// Calibration(Adjust)�����߿���
	// Z����ġ�� �˻����� �ʰ� �����δ�.
			if(devState.actionMenu != AS_ACTION_ADJUST) {
				switch(stateZ) {
					case 0x00:
dfp("Error...... MOVE_ERR_BOTH\n");
						return MOVE_ERR_BOTH;
					case 0x10:
dfp("Error...... MOVE_ERR_VIALCHK\n");
						return MOVE_ERR_VIALCHK;		
					case 0x20:
dfp("Error...... MOVE_ERR_NDLCHK\n");							
						return MOVE_ERR_NDLCHK;
					case 0x30:	// ok 	//			dp("OK...... MOVE_CMD_OK\n");								
dfp("OK...... MOVE_CMD_OK\n");												
						break;
				}
			}
		case MOTOR_Z:
		case MOTOR_SYR:
		case MOTOR_VAL:	//	baseAddr = BASEADDR_MOTOR_VAL; break;
		case MOTOR_INJ:	//	baseAddr = BASEADDR_MOTOR_INJ; 
			for(i=MOVE_STEP_SPEED_MIN; i <= MOVE_STEP_SPEED_MAX ; i++) {
/*				
					if( (totalHaif - AccelStep) > sAccel[who][i].s_pulse_repeat) {
						AccelStep += sAccel[who][i].s_pulse_repeat;
		//dp("[%3d] s_pulse_repeat=%3d  Acc=%d deacc=%d\n",i, sAccel[who][i].s_pulse_repeat, AccelStep, DeaccStep);
					}
					else 	break;
*/
					if( (totalHaif - AccelStep) > s_pulse_repeat[who][i]) {
						AccelStep += s_pulse_repeat[who][i];
		//dp("[%3d] s_pulse_repeat=%3d  Acc=%d deacc=%d\n",i, sAccel[who][i].s_pulse_repeat, AccelStep, DeaccStep);
					}
					else 	break;
					
			}

//====================================================================================
// AccelStep �� DeaccStep�� 0�̸� �ȵȴ�.(1���� ������ �ȵȴ�.)	
// TotalStep�� 2���ϸ� ������
// TotalStep = 1 : ���۾���.
// TotalStep = 2 : ��ӵ�.
//====================================================================================
			if(AccelStep < 1) {
				dp("AccelStep = %d\n",AccelStep);
				AccelStep = 1;
			}
			
			DeaccStep = totalStep - AccelStep;
			if(DeaccStep < 1) {
				dp("DeaccStep = %d\n",DeaccStep);				
				DeaccStep = 1;	
			}

			*((uint32_t volatile *)(baseAddr + TOTAL_OFFSET)) = totalStep - 1;		// �ϳ� ���� ������ �����Ѵ�(����). FPGA���� ���� ����ϵ���
			*((uint32_t volatile *)(baseAddr + START_SPEED)) = MOVE_STEP_SPEED_MIN;		// start_speed : �̰��� �迭�� ��ġ�̹Ƿ� �״�� ����
			*((uint32_t volatile *)(baseAddr + ACCEL_OFFSET)) = AccelStep - 1;		// �ϳ� ���� ������ �����Ѵ�(����). FPGA���� ���� ����ϵ���
			*((uint32_t volatile *)(baseAddr + DEACC_OFFSET)) = DeaccStep - 1;		// �ϳ� ���� ������ �����Ѵ�(����). FPGA���� ���� ����ϵ���
			*((uint32_t volatile *)(baseAddr + CTRL_PORT)) = ctrlCmd;
			break;		
	}

	devState.curDir[who] = ctrlCmd & 0x00000001;
	devState.bInfinite[who] = ctrlCmd & 0x00000040;	

	stepCtrlCmd(who, (ctrlCmd & ~CMD_START));

	return MOVE_CMD_OK;
}

//====================================================================
// setStep�� �ݴ�� �ӵ� �Է�
// minSpeed : Speed[15:8]
// maxSpeed : Speed[7:0]
// speed : 0 ~ 149
//====================================================================
int stepControl(uint32_t who, uint32_t ctrlCmd, uint32_t Speed, uint32_t totalStep)
{
	int i;
	int totalHaif = totalStep / 2;
	int stateZ;
	uint32_t AccelStep = 0;	// �ӵ��� ���̱� ���ؼ� ���������� ���������� ��ġ�� ����Ѵ�. ����� �ٽ� ���󺹱�
	uint32_t DeaccStep = 0;

	// ctrlCmd = Dir
	uint32_t maxSpeed = Speed & 0x000000ff;
	uint32_t minSpeed = (Speed >> 8) & 0x000000ff;

	uint32_t baseAddr;	// = BASEADDR_MOTOR_X; //
	uint32_t rpm,step_gap_cnt;

	baseAddr = devSet.baseAddr[who];

	switch(who) {
		case MOTOR_X:
		case MOTOR_Y:

//			stateZ = devState.btMotorState[MOTOR_Z] & 0x00000030;
			stateZ = devState.btMotorState[MOTOR_Z] & 0x30;
// �׽�Ʈ 			
		if(devState.actionMenu != AS_ACTION_ADJUST) {
			switch(stateZ) {
				case 0x00:
dfp("Error...... MOVE_ERR_BOTH %x\n",stateZ);
					return MOVE_ERR_BOTH;
				case 0x10:
dfp("Error...... MOVE_ERR_VIALCHK %x\n",stateZ);				
					return MOVE_ERR_VIALCHK;		
				case 0x20:
dfp("Error...... MOVE_ERR_NDLCHK %x\n",stateZ);					
					return MOVE_ERR_NDLCHK;
				case 0x30:	// ok 
dp("OK...... MOVE_CMD_OK %x \n",stateZ);
					break;
			}
		}

		case MOTOR_Z:		//	baseAddr = BASEADDR_MOTOR_Z; break;
		case MOTOR_SYR: //	baseAddr = BASEADDR_MOTOR_SYR; break;
		case MOTOR_VAL:	//	baseAddr = BASEADDR_MOTOR_VAL; break;
		case MOTOR_INJ:	//	baseAddr = BASEADDR_MOTOR_INJ; 
			for(i=minSpeed; i <= maxSpeed ; i++) {
#if 0	
					if((totalHaif - sAccel[who][i].accumul_cnt) < sAccel[who][i].s_pulse_repeat) {
		//dp("[%3d] s_pulse_repeat=%3d  Acc=%d deacc=%d\n",i, sAccel[who][i].s_pulse_repeat, AccelStep, DeaccStep);
						break;
					}
#else
/*
					if( (totalHaif - AccelStep) > sAccel[who][i].s_pulse_repeat) {
						AccelStep += sAccel[who][i].s_pulse_repeat;
		//dp("[%3d] s_pulse_repeat=%3d  Acc=%d deacc=%d\n",i, sAccel[who][i].s_pulse_repeat, AccelStep, DeaccStep);
					}
					else 	break;
*/					
					if( (totalHaif - AccelStep) > s_pulse_repeat[who][i]) {
						AccelStep += s_pulse_repeat[who][i];
//dp("[%3d] s_pulse_repeat=%3d  Acc=%d deacc=%d\n",i, s_pulse_repeat[who][i], AccelStep, DeaccStep);
					}
					else 	break;  

#endif					
			}

//====================================================================================
// AccelStep �� DeaccStep�� 0�̸� �ȵȴ�.(1���� ������ �ȵȴ�.)	
// TotalStep�� 2���ϸ� ������
// TotalStep = 1 : ���۾���.
// TotalStep = 2 : ��ӵ�.
//====================================================================================
			if(AccelStep < 1) {
				dfp("AccelStep = %d\n",AccelStep);
				AccelStep = 1;
			}
			
			DeaccStep = totalStep - AccelStep;
			if(DeaccStep < 1) {
				dfp("DeaccStep = %d\n",DeaccStep);				
				DeaccStep = 1;	
			}
//dfp("[%d] tot=%d Acc=%d deacc=%d min=%d max=%d\n",who,totalStep,AccelStep,DeaccStep, minSpeed, maxSpeed);		

			*((uint32_t volatile *)(baseAddr + TOTAL_OFFSET)) = totalStep - 1;		// �ϳ� ���� ������ �����Ѵ�(����). FPGA���� ���� ����ϵ���
			*((uint32_t volatile *)(baseAddr + START_SPEED)) = minSpeed;		// start_speed : �̰��� �迭�� ��ġ�̹Ƿ� �״�� ����
			*((uint32_t volatile *)(baseAddr + ACCEL_OFFSET)) = AccelStep - 1;		// �ϳ� ���� ������ �����Ѵ�(����). FPGA���� ���� ����ϵ���
			*((uint32_t volatile *)(baseAddr + DEACC_OFFSET)) = DeaccStep - 1;		// �ϳ� ���� ������ �����Ѵ�(����). FPGA���� ���� ����ϵ���

#if FPGA_VERSION_TRIM
//	*((uint32_t volatile *)(devSet.selMotorAddr[who]))	= who;
			CMD_SEL_MOTOR(who);
#endif			
			*((uint32_t volatile *)(baseAddr + CTRL_PORT)) = ctrlCmd;
			break;		
	}

	devState.curDir[who] = ctrlCmd & 0x00000001;
	devState.bInfinite[who] = ctrlCmd & 0x00000040;	
//dfp("who[%d] ctrlCmd[0x%x]\n",who,ctrlCmd);				
	stepCtrlCmd(who, (ctrlCmd & ~CMD_START));

	return MOVE_CMD_OK;
}

// Ȩ��ġ ã�� 
// injector , Valve�� ���� 
int moveStepSpeed(uint32_t who, uint32_t minSpeed, uint32_t maxSpeed, int32_t tStep)
{
	int i;
	int totalHaif,totalStep;
	int stateZ;
	uint32_t AccelStep = 0;	// �ӵ��� ���̱� ���ؼ� ���������� ���������� ��ġ�� ����Ѵ�. ����� �ٽ� ���󺹱�
	uint32_t DeaccStep = 0;

	// ctrlCmd = Dir
	// minSpeed : Speed[15:8]
	// maxSpeed : Speed[7:0]
//	uint32_t minSpeed = Speed & 0x000000ff;
//	uint32_t maxSpeed = (Speed >> 8) & 0x000000ff;
#if 0
	uint32_t maxSpeed = Speed & 0x000000ff;
	uint32_t minSpeed = (Speed >> 8) & 0x000000ff;
#endif

	uint32_t baseAddr;	// = BASEADDR_MOTOR_X; // ????
	uint32_t rpm,step_gap_cnt;
	uint32_t dir;
	uint32_t ctrlCmd;

	if(tStep > 0) {	// �ݴ�Ȩ
		totalStep = tStep;
		if(devSet.homeDir[who] == DIR_CCW) {	// MOTOR_X , MOTOR_Y , MOTOR_SYR
			dir = DIR_CW;
			ctrlCmd = DIR_CW | CMD_START;
		}
		else {	// MOTOR_Z , MOTOR_VAL , MOTOR_INJ
			dir = DIR_CCW;
			ctrlCmd = DIR_CCW | CMD_START;
		}
	}
	else if(tStep < 0) {
		totalStep = -tStep;
		if(devSet.homeDir[who]== DIR_CCW) {// MOTOR_X , MOTOR_Y , MOTOR_SYR
			dir = DIR_CCW;
			ctrlCmd = DIR_CCW | CMD_START;
		}	
		else {	// MOTOR_Z , MOTOR_VAL , MOTOR_INJ
			dir = DIR_CW;
			ctrlCmd = DIR_CW | CMD_START;			
		}
	}
	else {
// ���� ��ġ �������� �ʴ´�.	
		return MOVE_CMD_OK;
	}
	
#if 0
	totalHaif = totalStep / 2;
#else
 	totalHaif = totalStep >> 1;
#endif
	
	baseAddr = devSet.baseAddr[who];

	switch(who) {
		case MOTOR_X:
		case MOTOR_Y:
//			stateZ = devState.btMotorState[MOTOR_Z] & 0x00000030;
			stateZ = devState.btMotorState[MOTOR_Z] & 0x30;
	// Calibration(Adjust)�����߿���
	// Z����ġ�� �˻����� �ʰ� �����δ�.
			if(devState.actionMenu != AS_ACTION_ADJUST) {
				switch(stateZ) {
					case 0x00:
	dfp("Error...... MOVE_ERR_BOTH\n");
			send_SelfMessage(SELF_ERROR, MOVE_ERR_BOTH__VIAL_AND_Z_AXIS);	
						return MOVE_ERR_BOTH;
					case 0x10:
	dfp("Error...... MOVE_ERR_BOTH__VIAL_CHECK\n");								
			send_SelfMessage(SELF_ERROR, MOVE_ERR__VIAL_CHECK);	
						return MOVE_ERR_VIALCHK;		
					case 0x20:
	dfp("Error...... MOVE_ERR_BOTH__Z_AXIS_CHECK\n");								
			send_SelfMessage(SELF_ERROR, MOVE_ERR__Z_AXIS_CHECK);	
						return MOVE_ERR_NDLCHK;
					case 0x30:	// ok 
	//			dp("OK...... MOVE_CMD_OK\n");								
						break;
				}
			}
		case MOTOR_Z:
		case MOTOR_SYR:
		case MOTOR_VAL:	//	baseAddr = BASEADDR_MOTOR_VAL; break;
		case MOTOR_INJ:	//	baseAddr = BASEADDR_MOTOR_INJ; 
			for(i=minSpeed; i <= maxSpeed ; i++) {
/*				
					if( (totalHaif - AccelStep) > sAccel[who][i].s_pulse_repeat) {
						AccelStep += sAccel[who][i].s_pulse_repeat;
//dp("[%3d] s_pulse_repeat=%3d  Acc=%d deacc=%d\n",i, sAccel[who][i].s_pulse_repeat, AccelStep, DeaccStep);
					}
					else 	break;
*/
					if( (totalHaif - AccelStep) > s_pulse_repeat[who][i]) {
						AccelStep += s_pulse_repeat[who][i];
		//dp("[%3d] s_pulse_repeat=%3d  Acc=%d deacc=%d\n",i, sAccel[who][i].s_pulse_repeat, AccelStep, DeaccStep);
					}
					else 	break;
			}

//====================================================================================
// AccelStep �� DeaccStep�� 0�̸� �ȵȴ�.(1���� ������ �ȵȴ�.)	
// TotalStep�� 2���ϸ� ������
// TotalStep = 1 : ���۾���.
// TotalStep = 2 : ��ӵ�.
//====================================================================================
			if(AccelStep < 1) {
				dfp("AccelStep = %d\n",AccelStep);
				AccelStep = 1;
			}
			
			DeaccStep = totalStep - AccelStep;
			if(DeaccStep < 1) {
				dfp("DeaccStep = %d\n",DeaccStep);				
				DeaccStep = 1;	
			}
// 20200504			
//ctrlCmd = ctrlCmd | CMD_CLR_CNT;			
			
dfp("[%d] totalStep=%d  minSpeed=%d AccelStep=%d DeaccStep=%d ctrlCmd=0x%x\n",who,totalStep, minSpeed, AccelStep, DeaccStep, ctrlCmd);
			*((uint32_t volatile *)(baseAddr + TOTAL_OFFSET)) = totalStep - 1;		// �ϳ� ���� ������ �����Ѵ�(����). FPGA���� ���� ����ϵ���
			*((uint32_t volatile *)(baseAddr + START_SPEED)) = minSpeed;		// start_speed : �̰��� �迭�� ��ġ�̹Ƿ� �״�� ����
			*((uint32_t volatile *)(baseAddr + ACCEL_OFFSET)) = AccelStep - 1;		// �ϳ� ���� ������ �����Ѵ�(����). FPGA���� ���� ����ϵ���
			*((uint32_t volatile *)(baseAddr + DEACC_OFFSET)) = DeaccStep - 1;		// �ϳ� ���� ������ �����Ѵ�(����). FPGA���� ���� ����ϵ���

#if FPGA_VERSION_TRIM
//	*((uint32_t volatile *)(devSet.selMotorAddr[who]))	= who;
	CMD_SEL_MOTOR(who);
#endif			
			
			*((uint32_t volatile *)(baseAddr + CTRL_PORT)) = ctrlCmd;
			break;		
	}

		devState.curDir[who] = ctrlCmd & 0x00000001;
		devState.bInfinite[who] = ctrlCmd & 0x00000040;	
//dfp("who[%d] ctrlCmd[0x%x]\n",who,ctrlCmd);				
	stepCtrlCmd(who, (ctrlCmd & ~CMD_START));

	return MOVE_CMD_OK;
}

int goSensorHome(uint32_t who, uint32_t speed, uint32_t totalStep)
{
	uint32_t rpm = speed;
	uint32_t moveErr;

	switch(who) {
		case MOTOR_X: 				
			moveErr = stepControl(who, HOME_DIR_X | CMD_START, speed, totalStep);
			if(moveErr) return moveErr;
			else {
				devSet.ucChkHome[MOTOR_X] = YES;
			}
			break;
		case MOTOR_Y:
			moveErr = stepControl(who, HOME_DIR_Y | CMD_START, speed, totalStep);
			if(moveErr) return moveErr;
			else {
				devSet.ucChkHome[MOTOR_Y] = YES;
			}
			break;
		case MOTOR_Z:					
			stepControl(who, HOME_DIR_Z | CMD_START, speed, totalStep);
			devSet.ucChkHome[MOTOR_Z] = YES;
			break;
		case MOTOR_SYR: 	
			stepControl(who, HOME_DIR_SYR | CMD_START, speed, totalStep);
			devSet.ucChkHome[MOTOR_SYR] = YES;
			break;

		case MOTOR_VAL:			
			stepControl(who, HOME_DIR_VAL | CMD_START, rpm, totalStep);
			devSet.ucChkHome[MOTOR_VAL] = YES;
			break;
// ==============================================================================
// injector : ���ڴ��� Ȩ�� �д�. (�ո� �κ��� �д�.) - 60���� �ȵǰ� �շ�����.
//    �շ��ִ� �κ��� �� ��Ʈ���̿� �ִ� ��ġ�̰� , ���� �κп� ��Ʈ���� �ִ�.
// ���� �ٸ��� ã�´�. ���� �κ��� ã�´�. 
// Ȩ��ġ�� �ƴϸ� �ð� �������� Ȩ��ġ�� ã�� 
// �ٽ� �ð� �������� Ȩ�ƴ� �κ��� ã�´�. - �󸶰� �ð�������� ȸ�� -- ��
// ==============================================================================
		case MOTOR_INJ:	
			stepControl(who, HOME_DIR_INJ | CMD_START, rpm, totalStep);
			devSet.ucChkHome[MOTOR_INJ] = YES;
			break;
	}
	return MOVE_CMD_OK;
}

int goPosETC(unsigned char pos)
{
// neelde�� ���̾� �ɸ��� ����϶�.	
	goX(pos, TRAY_ETC, ACT_XY_POS);
	goY(pos, TRAY_ETC, ACT_XY_POS);
}

//#define MOVE_XY_DELAY_TIME		2000		// 0.5��
#define GO_POS_RETRY_COUNT		3

int goRetryPos_X(int32_t pos, uint32_t Tray, uint32_t option)
{
	int32_t	readCount=0;
	static int encoderCount = 0;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_grpx != Menu) {
    dfp("subMenu_grpx=%d\n",subMenu_grpx);
    Menu = subMenu_grpx;
}
#endif

	switch(subMenu_grpx) {
		case 1:
			if( devState.homeFindEnd[MOTOR_X] == NO) {
				initHomePos_X();
			}
			else {
				subMenu_grpx++;
			}
			break;
		case 2:
			devState.mActSet[MOTOR_X].act++ ;
			encoderCount = goX_Encoder(pos, Tray, option);
			subMenu_grpx++;			
			break;
		case 3:
			if(readyChk(MOTOR_X))	{
				if(ChkEncoder_X(encoderCount)) { // ok
					subMenu_grpx++;
				}
				else { // error
					if(devState.mActSet[MOTOR_X].act > GO_POS_RETRY_COUNT) { 
						errorEncoder(MOTOR_X);
					}
					else {
						devState.homeFindEnd[MOTOR_X] = NO;
						subMenu_grpx = 1;
					}
dfpErr("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxERROR subMenu_grpx =%d\n",subMenu_grpx);								
				}
			}
			break;
		case 4:
			subMenu_grpx = 1;
			return ACTION_MENU_END;
	}

  return ACTION_MENU_ING;	
}

int goRetryPos_Y(int32_t pos, uint32_t Tray, uint32_t option)
{
	int32_t	readCount=0;
	static int encoderCount = 0;
//	static int error = NO;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_grpy != Menu) {
    dfp("subMenu_grpy=%d\n",subMenu_grpy);
    Menu = subMenu_grpy;
}
#endif

	switch(subMenu_grpy) {
		case 1:
			if( devState.homeFindEnd[MOTOR_Y] == NO) {
				initHomePos_Y();
			}
			else {
				subMenu_grpy++;
			}
			break;
		case 2:
			devState.mActSet[MOTOR_Y].act++ ;
			encoderCount = goY_Encoder(pos, Tray, option);
			subMenu_grpy++;			
			break;
		case 3:
			if(readyChk(MOTOR_Y))	{
				if(ChkEncoder_Y(encoderCount)) { // ok
					subMenu_grpy++;
				}
				else { // error
					if(devState.mActSet[MOTOR_Y].act > GO_POS_RETRY_COUNT) { 
						errorEncoder(MOTOR_Y);
					}
					else {
						devState.homeFindEnd[MOTOR_Y] = NO;
						subMenu_grpy = 1;
					}
dfpErr("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxERROR subMenu_grpy =%d\n",subMenu_grpy);								
				}
			}
			break;
		case 4:
			subMenu_grpy = 1;
			return ACTION_MENU_END;

	}

  return ACTION_MENU_ING;	
			
}

int goRetryPos_Z(int32_t pos, uint32_t Tray, uint32_t option)
{
	int32_t	readCount=0;
	static int encoderCount = 0;
//	static int error = NO;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_grpz != Menu) {
    dfp("subMenu_grpz=%d\n",subMenu_grpz);
    Menu = subMenu_grpz;
}
#endif

	switch(subMenu_grpz) {
		case 1:
			if( devState.homeFindEnd[MOTOR_Z] == NO) {
				initHomePos_Z();
			}
			else {
				subMenu_grpz++;
			}
			break;
		case 2:
			devState.mActSet[MOTOR_Z].act++ ;
			encoderCount = goZ_Encoder(pos, option);
			subMenu_grpz++;			
			break;
		case 3:
			if(readyChk(MOTOR_Z))	{
				if(ChkEncoder_Z(encoderCount)) { // ok
					subMenu_grpz++;
				}
				else { // error
					if(devState.mActSet[MOTOR_Z].act > GO_POS_RETRY_COUNT) { 
						errorEncoder(MOTOR_Z);
					}
					else {
						devState.homeFindEnd[MOTOR_Z] = NO;
						subMenu_grpz = 1;
					}
dfpErr("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxERROR subMenu_grpz =%d\n",subMenu_grpz);								
				}
			}
			break;
		case 4:
			subMenu_grpz = 1;
			return ACTION_MENU_END;

	}

  return ACTION_MENU_ING;	
			
}

#if NEW_BOARD_ENCODER_2020
int ChkEncoder_X(int encoderCount)
{
	int count;
	int32_t	readCount=0;
	
	readCount = *((uint32_t volatile *)(BASEADDR_MOTOR_X_Z));
	readCount = (readCount >> 8) & 0x3ff;
	
dfp("======  X  read=[%d] Target=[%d]\n", readCount , encoderCount);

	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_X);

	count = encoderCount - readCount;

	if(count > ERR_ENCODER_COUNT_X || count < -ERR_ENCODER_COUNT_X) {
		return RE_ERR;
	}
	else {
		return RE_OK;
	}
}

int ChkEncoder_Y(int encoderCount)
{
	int count;
	int32_t	readCount=0;
	
	readCount = *((uint32_t volatile *)(BASEADDR_MOTOR_Y_SYR));
	readCount = (readCount >> 8) & 0x3ff;
	
dfp("======  Y  read=[%d] Target=[%d]\n", readCount , encoderCount);

	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_Y);

	count = encoderCount - readCount;

	if(count > ERR_ENCODER_COUNT_Y || count < -ERR_ENCODER_COUNT_Y) {
		return RE_ERR;
	}
	else {
		return RE_OK;
	}
}

int ChkEncoder_Z(int encoderCount)
{
	int count;
	int32_t	readCount=0;
	
	readCount = *((uint32_t volatile *)(BASEADDR_MOTOR_X_Z));
	readCount = (readCount >> 8) & 0x3ff;
	
dfp("======  Z  read=[%d] Target=[%d]\n", readCount , encoderCount);

	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_Z);

	count = encoderCount - readCount;

	if(count > ERR_ENCODER_COUNT_Z || count < -ERR_ENCODER_COUNT_Z) {
		return RE_ERR;
	}
	else {
		return RE_OK;
	}
}
#else
int ChkEncoder_X(int encoderCount)
{
		return RE_OK;
}

int ChkEncoder_Y(int encoderCount)
{
		return RE_OK;
}

int ChkEncoder_Z(int encoderCount)
{
		return RE_OK;
}
#endif

#if NEW_BOARD_ENCODER_2020
int readyChkRetry_X(int encoderCount)
{

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_rcrx != Menu) {
    dfp("subMenu_rcrx=%d\n",subMenu_rcrx);
    Menu = subMenu_rcrx;
}
#endif
	switch(subMenu_rcrx) {
		case 1:
			if(readyChk(MOTOR_X))	{
				devState.mActSet[MOTOR_X].act = 0;
				if(ChkEncoder_X(encoderCount)) {
					subMenu_rcrx=3;
				}
				else {
					devState.homeFindEnd[MOTOR_X] = NO;
					subMenu_rcrx++;
				}				
			}
			break;
		case 2:
dfp("=============================================Count X=%d\n",encoderCount);
			if(goRetryPos_X(devState.mActSet[MOTOR_X].pos, devState.mActSet[MOTOR_X].Tray, devState.mActSet[MOTOR_X].option)) {
				++subMenu_rcrx;
			}
			break;
		case 3:			
			subMenu_rcrx = 1;
			return ACTION_MENU_END;
	}

	return ACTION_MENU_ING;
}

int readyChkRetry_Y(int encoderCount)
{

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_rcry != Menu) {
    dfp("subMenu_rcry=%d\n",subMenu_rcry);
    Menu = subMenu_rcry;
}
#endif
	switch(subMenu_rcry) {
		case 1:
			if(readyChk(MOTOR_Y))	{
				devState.mActSet[MOTOR_Y].act = 0;
				if(ChkEncoder_Y(encoderCount)) {
					subMenu_rcry=3;
				}
				else {
					devState.homeFindEnd[MOTOR_Y] = NO;
					subMenu_rcry++;
				}				
			}
			break;
		case 2:
dfp("=============================================Count Y=%d\n",encoderCount);
			if(goRetryPos_Y(devState.mActSet[MOTOR_Y].pos, devState.mActSet[MOTOR_Y].Tray, devState.mActSet[MOTOR_Y].option)) {
				++subMenu_rcry;
			}
			break;
		case 3:			
			subMenu_rcry = 1;
			return ACTION_MENU_END;
	}

	return ACTION_MENU_ING;
}

int readyChkRetry_Z(int encoderCount)
{

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_rcrz != Menu) {
    dfp("subMenu_rcrz=%d\n",subMenu_rcrz);
    Menu = subMenu_rcrz;
}
#endif
	switch(subMenu_rcrz) {
		case 1:
			if(readyChk(MOTOR_Z))	{
				devState.mActSet[MOTOR_Z].act = 0;
				if(ChkEncoder_Z(encoderCount)) {
					// ���� ��ġ�� POS_Z_TOP �� ��� ���� üũ
					if(devState.mActSet[MOTOR_Z].pos == POS_Z_TOP && devState.mActSet[MOTOR_Z].option == ACT_Z_POS) {
		 				if((devState.btMotorState[MOTOR_Z] & Z_HOME1_CHECK) == Z_HOME1_SENSING) {
		 				}
		 				else {
//							errorEncoder(MOTOR_Z);
							errorFunction(SELF_ERROR_HOME_CHK_Z);
		 				}
					}
					subMenu_rcrz=3;
				}
				else {
					devState.homeFindEnd[MOTOR_Z] = NO;
					subMenu_rcrz++;
				}				
			}
			break;
		case 2:
dfp("=============================================Count Z=%d\n",encoderCount);
			if(goRetryPos_Z(devState.mActSet[MOTOR_Z].pos, devState.mActSet[MOTOR_Z].Tray, devState.mActSet[MOTOR_Z].option)) {
				++subMenu_rcrz;
			}
			break;
		case 3:
			subMenu_rcrz = 1;
			return ACTION_MENU_END;
	}

	return ACTION_MENU_ING;
}
#else
int readyChkRetry_X(int encoderCount)
{
	return readyChk(MOTOR_X);
}

int readyChkRetry_Y(int encoderCount)
{
	return readyChk(MOTOR_Y);
}

int readyChkRetry_Z(int encoderCount)
{
	return readyChk(MOTOR_Z);
}
#endif

// neelde�� ���̾� �ɸ��� ����϶�.	
int goPosVial(const VIAL_POS_t *pVial)
{
	int32_t	readCount=0;
	static int encoderCount_X = 0;
	static int encoderCount_Y = 0;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_gpv != Menu) {
    dfp("subMenu_gpv=%d\n",subMenu_gpv);
    Menu = subMenu_gpv;
}
#endif

	switch(subMenu_gpv) {
		case 1:
			encoderCount_Y = goY_Encoder(pVial->uPos.y, pVial->uPos.tray, ACT_XY_POS);
			subMenu_gpv++;
			break;
		case 2:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_gpv, MOVE_XY_DELAY_TIME); // runSetup.waitTime_AfterAspirate * CONVERT_WAIT_TIME_TO_COUNT);			
			break;
		case 3:
			encoderCount_X = goX_Encoder(pVial->uPos.x, pVial->uPos.tray, ACT_XY_POS);
			subMenu_gpv++;
			break;
		case 4:
			if(readyChkRetry_Y(encoderCount_Y)) {
				subMenu_gpv++;			
			}
			break;
		case 5:
			if(readyChkRetry_X(encoderCount_X)) {
				subMenu_gpv++;			
			}
			break;
		case 6:
			subMenu_gpv = 1;
			return ACTION_MENU_END;					
			break;		
	}
	return ACTION_MENU_ING;
}

//================================================================
// devState.curStep[MOTOR_X]
// devState.curTray;								// 0:left	1:Right 2:�ٸ���ġ(injector/wash/waste) - Tray	
//================================================================
int goX(int32_t pos, uint32_t Tray, uint32_t option)
{
	int32_t step;
	int32_t cur,act;
	int ret = 0;
	uint32_t encoderCount;
	
#if NEW_BOARD_ENCODER_2020
//	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_X);
#endif

	cur = devState.curStep[MOTOR_X];
dfp("curStep=%d\n",(int)devState.curStep[MOTOR_X]);
	switch(option) {
		case ACT_XY_POS:
			switch(Tray) {
				case TRAY_LEFT:
					act = xyzCoord.vialStep_L_X[pos];
					devState.curStep[MOTOR_X] = act;
					devState.curTray = Tray;
					break;
				case TRAY_RIGHT:
					act = xyzCoord.vialStep_R_X[pos];
					devState.curStep[MOTOR_X] = act;
					devState.curTray = Tray;
					break;
				case TRAY_ETC:
					act = xyzCoord.etcStep_X[pos];
					devState.curStep[MOTOR_X] = act;
					devState.curTray = Tray;
					break;
			}
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_X] = act;
			devState.curTray = Tray;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_X] += act;
			devState.curTray = Tray;
			break;
	}

	step = act - cur;

	ret = moveStepSpeed(MOTOR_X, devSet.minSpeed[MOTOR_X], devSet.maxSpeed[MOTOR_X], step);

	if(step > 0 ) usedTime.x += (int)(MM_PER_STEP_X / MICRO_STEP_X * step); 
	else 					usedTime.x += (int)(MM_PER_STEP_X / MICRO_STEP_X * (-step));

	if(ret != MOVE_CMD_OK) {	// Error
		errorFunction(SELF_ERROR_POSITION_Z);
	}
	return ret;

}

int goX_Encoder(int32_t pos, uint32_t Tray, uint32_t option)
{
	int32_t step;
	int32_t cur,act;
	int ret = 0;
	uint32_t encoderCount;
	
#if NEW_BOARD_ENCODER_2020
	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_X);
	devState.mActSet[MOTOR_X].pos = pos;
	devState.mActSet[MOTOR_X].Tray = Tray;
	devState.mActSet[MOTOR_X].option = option;
#endif

	cur = devState.curStep[MOTOR_X];
dfp("curStep=%d\n",(int)devState.curStep[MOTOR_X]);
	switch(option) {
		case ACT_XY_POS:
			switch(Tray) {
				case TRAY_LEFT:
					act = xyzCoord.vialStep_L_X[pos];
					devState.curStep[MOTOR_X] = act;
					devState.curTray = Tray;
					break;
				case TRAY_RIGHT:
					act = xyzCoord.vialStep_R_X[pos];
					devState.curStep[MOTOR_X] = act;
					devState.curTray = Tray;
					break;
				case TRAY_ETC:
					act = xyzCoord.etcStep_X[pos];
					devState.curStep[MOTOR_X] = act;
					devState.curTray = Tray;
					break;
			}
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_X] = act;
			devState.curTray = Tray;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_X] += act;
			devState.curTray = Tray;
			break;
	}

	step = act - cur;

	ret = moveStepSpeed(MOTOR_X, devSet.minSpeed[MOTOR_X], devSet.maxSpeed[MOTOR_X], step);

#if NEW_BOARD_ENCODER_2020
	if(step > 0 ) {
		usedTime.x += (int)(MM_PER_STEP_X / MICRO_STEP_X * step); 
		
//		encoderCount = step * X_ENCODER_COUNTER_FACTOR ;		// ���ڴ� ī��Ʈ ��		
		encoderCount = (step * X_ENCODER_COUNTER_FACTOR + 0.5f);//+ 0.5f; 		// ���ڴ� ī��Ʈ ��				
	}
	else {
		usedTime.x += (int)(MM_PER_STEP_X / MICRO_STEP_X * (-step));
//		encoderCount = -step * X_ENCODER_COUNTER_FACTOR;		
		encoderCount = -(step * X_ENCODER_COUNTER_FACTOR) + 0.5f;		
	}

	if(ret != MOVE_CMD_OK) {	// Error
		errorFunction(SELF_ERROR_POSITION_Z);
	}
	return encoderCount;
#else	
	if(step > 0 ) usedTime.x += (int)(MM_PER_STEP_X / MICRO_STEP_X * step); 
	else 					usedTime.x += (int)(MM_PER_STEP_X / MICRO_STEP_X * (-step));

	if(ret != MOVE_CMD_OK) {	// Error
		errorFunction(SELF_ERROR_POSITION_Z);
	}
	return ret;
#endif

}

int goY(int32_t pos, uint32_t Tray, uint32_t option)
{
	int32_t step;
	int32_t cur,act;
	int ret=0;
	uint32_t encoderCount;
	
#if NEW_BOARD_ENCODER_2020
//	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_Y);
#endif

	cur = devState.curStep[MOTOR_Y];
dfp("curStep=%d\n",(int)devState.curStep[MOTOR_Y]);
	switch(option) {
		case ACT_XY_POS:
			switch(Tray) {
				case TRAY_LEFT:
					act = xyzCoord.vialStep_L_Y[pos];
					devState.curStep[MOTOR_Y] = act;
					break;
				case TRAY_RIGHT:
					act = xyzCoord.vialStep_R_Y[pos];		
					devState.curStep[MOTOR_Y] = act;
					break;
				case TRAY_ETC:
					act = xyzCoord.etcStep_Y[pos];
					devState.curStep[MOTOR_Y] = act;
					break;
			}
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_Y] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_Y] += act;
			break;
	}

// Y���� �������� Tray����� ��.
//					devState.curTray = Tray;
	step = act - cur;

	ret = moveStepSpeed(MOTOR_Y, devSet.minSpeed[MOTOR_Y], devSet.maxSpeed[MOTOR_Y], step);

	if(step > 0 ) usedTime.y += (int)(MM_PER_STEP_Y / MICRO_STEP_Y * step);
	else 					usedTime.y += (int)(MM_PER_STEP_Y / MICRO_STEP_Y * (-step));

	if(ret != MOVE_CMD_OK) {	// Error
		errorFunction(SELF_ERROR_POSITION_Z);
	}
		
	return ret;

}

int goY_Encoder(int32_t pos, uint32_t Tray, uint32_t option)
{
	int32_t step;
	int32_t cur,act;
	int ret=0;
	uint32_t encoderCount;
	int32_t	readCount=0;
	
#if NEW_BOARD_ENCODER_2020
	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_Y);
	devState.mActSet[MOTOR_Y].pos = pos;
	devState.mActSet[MOTOR_Y].Tray = Tray;
	devState.mActSet[MOTOR_Y].option = option;
#endif

	cur = devState.curStep[MOTOR_Y];
dfp("curStep=%d\n",(int)devState.curStep[MOTOR_Y]);
	switch(option) {
		case ACT_XY_POS:
			switch(Tray) {
				case TRAY_LEFT:
					act = xyzCoord.vialStep_L_Y[pos];
					devState.curStep[MOTOR_Y] = act;
					break;
				case TRAY_RIGHT:
					act = xyzCoord.vialStep_R_Y[pos];		
					devState.curStep[MOTOR_Y] = act;
					break;
				case TRAY_ETC:
					act = xyzCoord.etcStep_Y[pos];
					devState.curStep[MOTOR_Y] = act;
					break;
			}
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_Y] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_Y] += act;
			break;
	}

// Y���� �������� Tray����� ��.
//					devState.curTray = Tray;
	step = act - cur;

	ret = moveStepSpeed(MOTOR_Y, devSet.minSpeed[MOTOR_Y], devSet.maxSpeed[MOTOR_Y], step);

#if NEW_BOARD_ENCODER_2020
	if(step > 0 ) {
		usedTime.y += (int)(MM_PER_STEP_Y / MICRO_STEP_Y * step);
//		encoderCount = step * Y_ENCODER_COUNTER_FACTOR;		// ���ڴ� ī��Ʈ ��
		encoderCount = (step * Y_ENCODER_COUNTER_FACTOR + 0.5f);		// ���ڴ� ī��Ʈ ��
	}
	else {
		usedTime.y += (int)(MM_PER_STEP_Y / MICRO_STEP_Y * (-step));
		encoderCount = -(step * Y_ENCODER_COUNTER_FACTOR) + 0.5f;
	}

	if(ret != MOVE_CMD_OK) {	// Error
		errorFunction(SELF_ERROR_POSITION_Z);
	}
		
	return encoderCount;
#else
	if(step > 0 ) usedTime.y += (int)(MM_PER_STEP_Y / MICRO_STEP_Y * step);
	else 					usedTime.y += (int)(MM_PER_STEP_Y / MICRO_STEP_Y * (-step));

	if(ret != MOVE_CMD_OK) {	// Error
		errorFunction(SELF_ERROR_POSITION_Z);
	}
		
	return ret;
#endif

}
	
int goZ(int32_t pos, uint32_t option)
{
	int32_t step;
	int32_t cur,act;
	int ret=0;
	uint32_t encoderCount;
	
#if NEW_BOARD_ENCODER_2020
//	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_Z);
#endif

	cur = devState.curStep[MOTOR_Z];

	switch(option) {
//		case ACT_XY_POS:
		case ACT_Z_POS:			
			act = xyzCoord.step_Z[pos];
			devState.curStep[MOTOR_Z] = act;
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_Z] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_Z] += act;
			break;
	}

	step = act - cur;

	ret = moveStepSpeed(MOTOR_Z, devSet.minSpeed[MOTOR_Z], devSet.maxSpeed[MOTOR_Z], step);

	if(step > 0 ) usedTime.z += (int)(MM_PER_STEP_Z / MICRO_STEP_Z * step);
	else 					usedTime.z += (int)(MM_PER_STEP_Z / MICRO_STEP_Z * (-step));

	return ret;
}

int goZ_Encoder(int32_t pos, uint32_t option)
{
	int32_t step;
	int32_t cur,act;
	int ret=0;
	uint32_t encoderCount;
	
#if NEW_BOARD_ENCODER_2020
	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_Z);
	devState.mActSet[MOTOR_Z].pos = pos;
	devState.mActSet[MOTOR_Z].option = option;
#endif

	cur = devState.curStep[MOTOR_Z];

	switch(option) {
//		case ACT_XY_POS:
		case ACT_Z_POS:			
			act = xyzCoord.step_Z[pos];
			devState.curStep[MOTOR_Z] = act;
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_Z] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_Z] += act;
			break;
	}

	step = act - cur;

	ret = moveStepSpeed(MOTOR_Z, devSet.minSpeed[MOTOR_Z], devSet.maxSpeed[MOTOR_Z], step);

#if NEW_BOARD_ENCODER_2020
	if(step > 0 ) {
		usedTime.z += (int)(MM_PER_STEP_Z / MICRO_STEP_Z * step);
		encoderCount = (step * Z_ENCODER_COUNTER_FACTOR + 0.5f);		// ���ڴ� ī��Ʈ ��
	}
	else {
		usedTime.z += (int)(MM_PER_STEP_Z / MICRO_STEP_Z * (-step));
		encoderCount = -(step * Z_ENCODER_COUNTER_FACTOR) + 0.5f;
	}
	return encoderCount;
#else
	if(step > 0 ) usedTime.z += (int)(MM_PER_STEP_Z / MICRO_STEP_Z * step);
	else 					usedTime.z += (int)(MM_PER_STEP_Z / MICRO_STEP_Z * (-step));

	return ret;
#endif
}

int goPosVial_Z(const VIAL_POS_t *pVial, int32_t z_pos)//, uint32_t z_option)
{
	static int error = NO;
	int32_t	readCount=0;
	static int encoderCount = 0;
	static int encoderCount_X = 0;
	static int encoderCount_Y = 0;
	
#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_gpvz != Menu) {
    dfp("subMenu_gpvz=%d\n",subMenu_gpvz);
    Menu = subMenu_gpvz;
}
#endif

	switch(subMenu_gpvz) {
		case 1:	// ������ġ�� ��ǥ��ġ�� 
			if( ( pVial->uPos.tray == TRAY_LEFT 	
						&& (devState.curStep[MOTOR_X] == xyzCoord.vialStep_L_X[pVial->uPos.x])	
						&& (devState.curStep[MOTOR_Y] == xyzCoord.vialStep_L_Y[pVial->uPos.y]) )
				|| ( pVial->uPos.tray == TRAY_RIGHT
						&& (devState.curStep[MOTOR_X] == xyzCoord.vialStep_R_X[pVial->uPos.x])	
						&& (devState.curStep[MOTOR_Y] == xyzCoord.vialStep_R_Y[pVial->uPos.y]) ) )
			{
				subMenu_gpvz = 5;
			}
			else {
				++subMenu_gpvz;
			}
			break;
		case 2:
//			goZ(POS_Z_TOP,ACT_XY_POS);
			encoderCount = goZ_Encoder(POS_Z_TOP,ACT_Z_POS);
			++subMenu_gpvz;
			break;
		case 3:
			if(readyChk(MOTOR_Z))	{
				readyChkEncoder_Z();					
				encoderCount_X = goX_Encoder(pVial->uPos.x, pVial->uPos.tray, ACT_XY_POS);
				encoderCount_Y = goY_Encoder(pVial->uPos.y, pVial->uPos.tray, ACT_XY_POS);
				++subMenu_gpvz;
			}
			break;
		case 4:
      if(readyChk(MOTOR_X) && readyChk(MOTOR_Y))	{
				readyChkEncoder_X();
				readyChkEncoder_Y();
				++subMenu_gpvz;
      }
			break;
			
		case 5:
      if(readyChk(MOTOR_X) && readyChk(MOTOR_Y))	{
     		encoderCount = goZ_Encoder(z_pos, ACT_Z_POS);
				++subMenu_gpvz;
      }
			break;
		case 6:			
			if(readyChk(MOTOR_Z))	{		
				readyChkEncoder_Z();			
			
				subMenu_gpvz = 1;
				return ACTION_MENU_END;				
			}
			break;
	}
	
	return ACTION_MENU_ING;

}

// Ȯ���� �̵� 
//uint8_t subMenu_gpez	=	1;
int goPosETC_Z(int32_t xy_pos, int32_t z_pos)//, uint32_t z_option)
{
	static int error = NO;
	int32_t	readCount=0;
	static int encoderCount = 0;
	static int encoderCount_X = 0;
	static int encoderCount_Y = 0;
	
#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_gpez != Menu) {
    dfp("subMenu_gpez=%d\n",subMenu_gpez);
    Menu = subMenu_gpez;
}
#endif

	switch(subMenu_gpez) {
		case 1:	// ������ġ�� ��ǥ��ġ�� 
			if(devState.curStep[MOTOR_X] == xyzCoord.etcStep_X[xy_pos] 
				&& devState.curStep[MOTOR_Y] == xyzCoord.etcStep_Y[xy_pos]) 
			{
				subMenu_gpez = 8;
			}
			else {
				++subMenu_gpez;
			}
			break;
		case 2:
			encoderCount = goZ_Encoder(POS_Z_TOP,ACT_Z_POS);
			++subMenu_gpez;
			break;
		case 3:
			if(readyChkRetry_Z(encoderCount)) {
				subMenu_gpez++;
			}
			break;
		case 4:
			encoderCount_X = goX_Encoder(xy_pos, TRAY_ETC, ACT_XY_POS);
			++subMenu_gpez;
			break;

		case 5:
			encoderCount_Y = goY_Encoder(xy_pos, TRAY_ETC, ACT_XY_POS);
			++subMenu_gpez;
			break;
		case 6:
			if(readyChkRetry_X(encoderCount_X)) {
				subMenu_gpez++; 
			}
			break;
 		case 7:
			if(readyChkRetry_Y(encoderCount_Y)) {
				subMenu_gpez++; 
			}
			break;
 		case 8:
//      if(readyChk(MOTOR_X) && readyChk(MOTOR_Y)) {
		  	encoderCount = goZ_Encoder(z_pos, ACT_Z_POS);
				++subMenu_gpez;
//      }
			break;
		case 9:		
			if(readyChkRetry_Z(encoderCount)) {
				subMenu_gpez++;
			}
			break;
		case 10:
			subMenu_gpez = 1;
			return ACTION_MENU_END;
			break;
 	}
	
	return ACTION_MENU_ING;

}

// Ȯ���� �̵� 
// Top���� �̵��� Ȩ üũ �� ���� üũ
// ��ǥ��ġ�� �̵��� üũ(vial,wash,waste,injector)
//uint8_t subMenu_gpezc	=	1;
int goPosETC_Z_check(int32_t xy_pos, int32_t z_pos, int32_t z_chk_pos, int32_t z_chk_upper_pos)
{
	static int error = NO;
	int32_t	readCount=0;
	static int encoderCount = 0;
	static int encoderCount_X = 0;
	static int encoderCount_Y = 0;
	
#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_gpezc != Menu) {
    dfp("subMenu_gpezc=%d\n",subMenu_gpezc);
    Menu = subMenu_gpezc;
}
#endif

	switch(subMenu_gpezc) {
//#if Y_ENCODER_CHECK_ERR_MODIFY
		case 1:	// ������ġ�� ��ǥ��ġ�� 
			if(devState.curStep[MOTOR_X] == xyzCoord.etcStep_X[xy_pos] 
				&& devState.curStep[MOTOR_Y] == xyzCoord.etcStep_Y[xy_pos]) 
			{
				subMenu_gpezc = 5;
			}
			else {
			// POS_Z_TOP�� ���� ���� ����
				encoderCount = goZ_Encoder(POS_Z_TOP,ACT_Z_POS);
				subMenu_gpezc++;
			}
			break;
		case 2:
			if(readyChkRetry_Z(encoderCount)) {
				// �ʿ������?
				if((devState.btMotorState[MOTOR_Z] & 0x30) ==	 0x30) {
					encoderCount_X = goX_Encoder(xy_pos, TRAY_ETC, ACT_XY_POS);
					encoderCount_Y = goY_Encoder(xy_pos, TRAY_ETC, ACT_XY_POS);
					subMenu_gpezc++;
				}
				else {

					if(actRetry_gpezc == 1) {	// Top���� �̵��� �ٽ��ѹ� �õ�
						actRetry_gpezc++;
						subMenu_gpezc = 11;
					}
					else { // ���� ó�� - �޽��� ������ ���´� state_fault��
						actRetry_gpezc = 1;
						// ====================================================================
						// Error - ACT_Z_POS��ġ�� ���� �ʴ�.
						// ���� ó�� - �޽��� ������ ���´� state_fault��
											errorFunction(SELF_ERROR_POSITION_Z);
						// ====================================================================
					}
				}
			}
			break;
		case 3:
			if(readyChkRetry_X(encoderCount_X)) {
				subMenu_gpezc++;
			}
			break;
		case 4:
			if(readyChkRetry_Y(encoderCount_Y)) {
				subMenu_gpezc++;				
			}
			break;
		case 5:
			encoderCount = goZ_Encoder(z_chk_upper_pos, ACT_Z_POS);
			devSet.posCheckEnable = YES;
			devSet.posCheckMotor = MOTOR_Z;
			devSet.posCheck1 = 0x30; // Ȩ��ġ(Ȩ����)(�ϵ�,���̾�) : Ȩ,�ϵ� ���� - �̵���
			devSet.posCheck2 = 0x20; // �̵��� Ȩ��ġ ���        : Ȩ���� �ȵ�,�ϵ� ���� - �̵�
			subMenu_gpezc++;				
			break;
//#endif
		case 6:
			if(readyChkRetry_Z(encoderCount)) {
				devSet.posCheckEnable = NO;		// ������ �˻�����
				encoderCount = goZ_Encoder(z_chk_pos, ACT_Z_POS);
				subMenu_gpezc++;
			}
			break;
		case 7:
			if(readyChkRetry_Z(encoderCount)) {
				if((devState.btMotorState[MOTOR_Z] & 0x30) ==	 0x00) {		// 1��
					encoderCount = goZ_Encoder(z_pos, ACT_Z_POS);
//					subMenu_gpezc++;
					subMenu_gpezc = 10;
	      }
	      else {
						readMotorState();
	dfpErr("===XXXXXXXXXXXXXXXXXX========XXXXXXXXXXXXXXXXXXXXXXX=========devState.btMotorState[MOTOR_Z] = [%x]\n",devState.btMotorState[MOTOR_Z]);
	// �ϰ��� ���伾�� üũ���� �� ����� - ���� �����ؾ� �� ���� �Ǵܵ� - �ƴѰ� ????
					if((devState.btMotorState[MOTOR_Z] & 0x30) ==	 0x00) {		// 2��
						encoderCount = goZ_Encoder(z_pos, ACT_Z_POS);
						subMenu_gpezc = 10;
					}
					else {
						subMenu_gpezc++;	// error - ����� �ٽ� �˻�
					}
				}
			}
			break;
		case 8:
			WAIT_TIME_VALUE_CASE_STATE(subMenu_gpezc, DELAY_Z_ERROR_RECHECK_TIME);
			break;
		case 9:	// ����� �ٽ� �˻� 
			if(readyChkRetry_Z(encoderCount)) {
				if((devState.btMotorState[MOTOR_Z] & 0x30) == 0x00) {			// 3�� �˻�
					encoderCount = goZ_Encoder(z_pos, ACT_Z_POS);
					actRetry_gpezc = 1;
					subMenu_gpezc++;
	      }
	      else {
	      	if(actRetry_gpezc == 1) {	// Top���� �̵��� �ٽ��ѹ� �õ�
	      		actRetry_gpezc++;
	      		subMenu_gpezc = 11;
	        }
	      	else { // ���� ó�� - �޽��� ������ ���´� state_fault��
	      		actRetry_gpezc = 1;
	      		errorFunction(SELF_ERROR_POSITION_Z);
	      	}
				}
			}
			break;
		case 10:
			if(readyChkRetry_Z(encoderCount)) {
				subMenu_gpezc = 1;
				return ACTION_MENU_END;
			}
			break;

		case 11:	// goRetry
	    encoderCount = goZ_Encoder(POS_Z_TOP, ACT_XY_POS);
			subMenu_gpezc++;
			break;
		case 12:
			if(readyChkRetry_Z(encoderCount)) {
				subMenu_gpezc = 1;
			}
			break;
	}
	
	return ACTION_MENU_ING;

}

#if 0
// injector��ġ������ 
int goPosETC_Z_check_inj(int32_t xy_pos, int32_t z_pos, int32_t z_chk_pos)//, uint32_t z_option)
{
	static int error = NO;
	int32_t	readCount=0;
	static int encoderCount = 0;
	static int encoderCount_X = 0;
	static int encoderCount_Y = 0;
	
#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_gpezci != Menu) {
    dfp("subMenu_gpezc=%d\n",subMenu_gpezci);
    Menu = subMenu_gpezci;
}
#endif

	switch(subMenu_gpezci) {
		case 1:	// ������ġ�� ��ǥ��ġ�� 
			if(devState.curStep[MOTOR_X] == xyzCoord.etcStep_X[xy_pos] 
				&& devState.curStep[MOTOR_Y] == xyzCoord.etcStep_Y[xy_pos]) 
			{
				subMenu_gpezci = 5;
			}
			else {
				subMenu_gpezci++;
			}
			break;
		case 2:
			// POS_Z_TOP�� ���� ���� ����
			encoderCount =	goZ_Encoder(POS_Z_TOP,ACT_Z_POS);
			subMenu_gpezci++;
			break;
		case 3:
			if(readyChkRetry_Z(encoderCount)) {
// �ʿ������?
				if((devState.btMotorState[MOTOR_Z] & 0x30) ==	 0x30) {
					encoderCount_X = goX_Encoder(xy_pos, TRAY_ETC, ACT_XY_POS);
					encoderCount_Y = goY_Encoder(xy_pos, TRAY_ETC, ACT_XY_POS);
					subMenu_gpezci++;
				}
				else {
// ====================================================================
// Error - ACT_Z_POS��ġ�� ���� �ʴ�.
// ���� ó�� - �޽��� ������ ���´� state_fault��
					errorFunction(SELF_ERROR_POSITION_Z);
// ====================================================================
				}
			}
			break;
		case 4:
			if(readyChkRetry_X(encoderCount_X)) {
				subMenu_gpezci++;
			}
			break;
		case 5:
			if(readyChkRetry_Y(encoderCount_Y)) {
				encoderCount = goZ_Encoder(z_chk_pos, ACT_Z_POS);
				subMenu_gpezci++;
	    }
			break;
		case 6:
			if(readyChkRetry_Z(encoderCount)) {
				if((devState.btMotorState[MOTOR_Z] & 0x30) ==	 0x00) {
					encoderCount =	goZ_Encoder(z_pos, ACT_Z_POS);
					subMenu_gpezci++;
        }
        else {
// ====================================================================
// Error - z_chk_pos��ġ�� ���� �ʴ�.
// ���� ó�� - �޽��� ������ ���´� state_fault��
					readMotorState();
					if((devState.btMotorState[MOTOR_Z] & 0x30) ==	 0x00) {
						encoderCount =	goZ_Encoder(z_pos, ACT_Z_POS);
						subMenu_gpezci++;
					}
					else {
						errorFunction(SELF_ERROR_POSITION_Z);
					}
// ====================================================================
        }
			}
			break;
		case 7:
#if 1
			if(readyChkRetry_Z(encoderCount)) {
				subMenu_gpezci = 1;
				return ACTION_MENU_END;
			}
#else
			}
			if(readyChk(MOTOR_Z))	{
				readyChkEncoder_Z();								
				subMenu_gpezci = 1;
				return ACTION_MENU_END;
			}
#endif
			break;
	}
	
	return ACTION_MENU_ING;

}
#endif

int goSyr(float pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option)
{
	int32_t step;
	float cur,act;
	int ret=0;
	uint32_t encoderCount;

#if FPGA_VERSION_TRIM
//	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_SYR);
#endif

	cur = devState.curStep[MOTOR_SYR];

	switch(option) {
		case ACT_XY_POS:	// ul
			act = xyzCoord.step_1ul_SYR * pos;
			devState.curStep[MOTOR_SYR] = act;
			break;
		case ACT_XY_POS_ADD:	// ul
			act = xyzCoord.step_1ul_SYR * pos;
			cur = 0;	// ������ġ�� �������� 			
			devState.curStep[MOTOR_SYR] += act;
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_SYR] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;	// ������ġ�� �������� 			
			devState.curStep[MOTOR_SYR] += act;
			break;
	}

	if(act >= cur) step = (int)(act - cur + 0.5);
	else step = (int)(act - cur - 0.5);

dprintf("===	goSyr pos=%f curStep=%f cur=%f act=%f step=%d option=%d\n", pos,devState.curStep[MOTOR_SYR], cur, act, step, option);	

	ret = moveStepSpeed(MOTOR_SYR, minSpeed, maxSpeed, step);

	if(step > 0 ) usedTime.syringe += (int)(MM_PER_STEP_SYR / MICRO_STEP_SYR * step);
	else 					usedTime.syringe += (int)(MM_PER_STEP_SYR / MICRO_STEP_SYR * (-step));

	return ret;
}

int goSyr_Encoder(float pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option)
{
	int32_t step;
	float cur,act;
	int ret=0;
	uint32_t encoderCount;

#if FPGA_VERSION_TRIM
	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_SYR);
#endif

	cur = devState.curStep[MOTOR_SYR];

	switch(option) {
		case ACT_XY_POS:	// ul
			act = xyzCoord.step_1ul_SYR * pos;
			devState.curStep[MOTOR_SYR] = act;
			break;
		case ACT_XY_POS_ADD:	// ul
			act = xyzCoord.step_1ul_SYR * pos;
			cur = 0;	// ������ġ�� �������� 			
			devState.curStep[MOTOR_SYR] += act;
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_SYR] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;	// ������ġ�� �������� 			
			devState.curStep[MOTOR_SYR] += act;
			break;
	}

	if(act >= cur) step = (int)(act - cur + 0.5);
	else step = (int)(act - cur - 0.5);

dprintf("===	goSyr pos=%f curStep=%f cur=%f act=%f step=%d option=%d\n", pos,devState.curStep[MOTOR_SYR], cur, act, step, option);	

	ret = moveStepSpeed(MOTOR_SYR, minSpeed, maxSpeed, step);

#if FPGA_VERSION_TRIM
	if(step > 0 ) {
		usedTime.syringe += (int)(MM_PER_STEP_SYR / MICRO_STEP_SYR * step);
		encoderCount = (step * SYR_ENCODER_COUNTER_FACTOR + 0.5f);		// ���ڴ� ī��Ʈ ��
	}
	else {
		usedTime.syringe += (int)(MM_PER_STEP_SYR / MICRO_STEP_SYR * (-step));
		encoderCount = -(step * SYR_ENCODER_COUNTER_FACTOR) + 0.5f;
	}
	return encoderCount;
#else
	if(step > 0 ) usedTime.syringe += (int)(MM_PER_STEP_SYR / MICRO_STEP_SYR * step);
	else 					usedTime.syringe += (int)(MM_PER_STEP_SYR / MICRO_STEP_SYR * (-step));

	return ret;
#endif
}

// pos
//int goVal(int32_t pos, uint32_t speed, uint32_t option)
int goVal(int32_t pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option)
{
	int32_t step;
	int32_t cur,act;
//	uint32_t encoderCount;
	
#if FPGA_VERSION_TRIM
//	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_VAL);
#endif
	cur = devState.curStep[MOTOR_VAL];

	switch(option) {
		case ACT_XY_POS:	// pos
			act = xyzCoord.valStep[pos];
			devState.curStep[MOTOR_VAL] = act;
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_VAL] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_VAL] += act;
			break;
	}

	step = act - cur;
//dp("step = %d act = %d cur = %d\n",step,act,cur);	
//	moveStepSpeed(MOTOR_VAL, speed, step);
	if(step != 0) {
		usedTime.valve++;	// Valve ���Ƚ�� 
		return moveStepSpeed(MOTOR_VAL, minSpeed, maxSpeed, step);
	}
	else {
		return MOVE_CMD_OK;
	}

}

int goVal_Encoder(int32_t pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option)
{
	int32_t step;
	int32_t cur,act;
//	uint32_t encoderCount;
	
#if FPGA_VERSION_TRIM
	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_VAL);
#endif
	cur = devState.curStep[MOTOR_VAL];

	switch(option) {
		case ACT_XY_POS:	// pos
			act = xyzCoord.valStep[pos];
			devState.curStep[MOTOR_VAL] = act;
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_VAL] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_VAL] += act;
			break;
	}

	step = act - cur;
//dp("step = %d act = %d cur = %d\n",step,act,cur);	
//	moveStepSpeed(MOTOR_VAL, speed, step);
#if FPGA_VERSION_TRIM
	if(step != 0) {
		usedTime.valve++;	// Valve ���Ƚ�� 
		moveStepSpeed(MOTOR_VAL, minSpeed, maxSpeed, step);
		return 1;		
	}
	else {
		return MOVE_CMD_OK;
	}
#else
	if(step != 0) {
		usedTime.valve++;	// Valve ���Ƚ�� 
		return moveStepSpeed(MOTOR_VAL, minSpeed, maxSpeed, step);
	}
	else {
		return MOVE_CMD_OK;
	}
#endif
}

// load / injector
//int goInj(int32_t pos, uint32_t speed, uint32_t option)
int goInj(int32_t pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option)
{
	int32_t step;
	int32_t cur,act;

#if FPGA_VERSION_TRIM
//	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_INJ);
#endif

	cur = devState.curStep[MOTOR_INJ];

	switch(option) {
		case ACT_XY_POS:	// pos
			act = xyzCoord.injStep[pos];
			devState.curStep[MOTOR_INJ] = act;
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_INJ] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_INJ] += act;
			break;
	}

	step = act - cur;

	if(step != 0) {
		usedTime.injector++;	// injector ���Ƚ�� 
		return moveStepSpeed(MOTOR_INJ, minSpeed, maxSpeed, step);
	}
	else {
		return MOVE_CMD_OK;
	}

}

int goInj_Encoder(int32_t pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option)
{
	int32_t step;
	int32_t cur,act;

#if FPGA_VERSION_TRIM
	CMD_ENCODER_CLR_CNT_ENABLE(MOTOR_INJ);
#endif

	cur = devState.curStep[MOTOR_INJ];

	switch(option) {
		case ACT_XY_POS:	// pos
			act = xyzCoord.injStep[pos];
			devState.curStep[MOTOR_INJ] = act;
			break;
		case ACT_MOVE_STEP_ABSOLUTE:	// �������� ���� ��ǥ�̵�, (-)�� ���� (step��)
			act = pos;
			devState.curStep[MOTOR_INJ] = act;
			break;
		case ACT_MOVE_STEP_RELATE:		// ������ġ �������� �̵�(step��), (+)�� home�� �ݴ�� �̵� (step��)
			act = pos;
			cur = 0;
			devState.curStep[MOTOR_INJ] += act;
			break;
	}

	step = act - cur;
//	return moveStepSpeed(MOTOR_INJ, speed, step);
#if FPGA_VERSION_TRIM
	if(step != 0) {
		usedTime.injector++;	// injector ���Ƚ�� 
		moveStepSpeed(MOTOR_INJ, minSpeed, maxSpeed, step);
		return 1;		
	}
	else {
		return MOVE_CMD_OK;
	}
#else
	if(step != 0) {
		usedTime.injector++;	// injector ���Ƚ�� 
		return moveStepSpeed(MOTOR_INJ, minSpeed, maxSpeed, step);
	}
	else {
		return MOVE_CMD_OK;
	}
#endif	
//	return moveStepSpeed(MOTOR_INJ, minSpeed, maxSpeed, step);

}

void initStepDataRam()
{

#if FPGA_VERSION_TRIM
	make_StepData_ISO(MOTOR_X, STEP_FREQUENCY_X, LAST_RPM_X, START_RPM_X, MICRO_STEP_X);		// X
	make_StepData_ISO(MOTOR_Y, STEP_FREQUENCY_Y, LAST_RPM_Y, START_RPM_Y, MICRO_STEP_Y);		// Y
	make_StepData_ISO(MOTOR_Z, STEP_FREQUENCY_Z, LAST_RPM_Z, START_RPM_Z, MICRO_STEP_Z);		// Z
	make_StepData_ISO(MOTOR_SYR, STEP_FREQUENCY_SYR, LAST_RPM_SYR, START_RPM_SYR, MICRO_STEP_SYR);		// syringe
		
	make_StepData(MOTOR_VAL, STEP_FREQUENCY_VAL, TARGET_RPM_VAL, ACC_INTERVAL_VAL, MICRO_STEP_VAL);		// 
	make_StepData(MOTOR_INJ, STEP_FREQUENCY_INJ, TARGET_RPM_INJ, ACC_INTERVAL_INJ, MICRO_STEP_INJ);		// 
#else
	#if 0
	make_StepData(MOTOR_X, STEP_FREQUENCY_X, TARGET_RPM_X, ACC_INTERVAL_X, MICRO_STEP_X);		// X
	make_StepData(MOTOR_Y, STEP_FREQUENCY_Y, TARGET_RPM_Y, ACC_INTERVAL_Y, MICRO_STEP_Y);		// Y
	make_StepData(MOTOR_Z, STEP_FREQUENCY_Z, TARGET_RPM_Z, ACC_INTERVAL_Z, MICRO_STEP_Z);		// Z
	make_StepData(MOTOR_SYR, STEP_FREQUENCY_SYR, TARGET_RPM_SYR, ACC_INTERVAL_SYR, MICRO_STEP_SYR);		// syringe
	
	make_StepData(MOTOR_VAL, STEP_FREQUENCY_VAL, TARGET_RPM_VAL, ACC_INTERVAL_VAL, MICRO_STEP_VAL);		// 
	make_StepData(MOTOR_INJ, STEP_FREQUENCY_INJ, TARGET_RPM_INJ, ACC_INTERVAL_INJ, MICRO_STEP_INJ);		// 
	#endif

	make_StepData_ISO(MOTOR_X, STEP_FREQUENCY_X, LAST_RPM_X, START_RPM_X, MICRO_STEP_X);		// X
	make_StepData_ISO(MOTOR_Y, STEP_FREQUENCY_Y, LAST_RPM_Y, START_RPM_Y, MICRO_STEP_Y);		// Y
	make_StepData_ISO(MOTOR_Z, STEP_FREQUENCY_Z, LAST_RPM_Z, START_RPM_Z, MICRO_STEP_Z);		// Z
	make_StepData_ISO(MOTOR_SYR, STEP_FREQUENCY_SYR, LAST_RPM_SYR, START_RPM_SYR, MICRO_STEP_SYR);		// syringe
		
	make_StepData(MOTOR_VAL, STEP_FREQUENCY_VAL, TARGET_RPM_VAL, ACC_INTERVAL_VAL, MICRO_STEP_VAL);		// 
	make_StepData(MOTOR_INJ, STEP_FREQUENCY_INJ, TARGET_RPM_INJ, ACC_INTERVAL_INJ, MICRO_STEP_INJ);		// 
#endif	
}

// RESET : Active LOW ���
// SLEEP : Active LOW ���
#if FPGA_VERSION_TRIM
void stepCtrlCmd(uint32_t who, uint32_t ctrlCmd)
{
//dfp("stepCtrlCmd=%d, %x\n",who,ctrlCmd);		      
//	*((uint32_t volatile *)(devSet.selMotorAddr[who]))	= who;

	*((uint32_t volatile *)(devSet.ctrlPortAddr[who])) = ctrlCmd;
	devState.curDir[who] = ctrlCmd & 0x00000001;
	devState.bInfinite[who] = ctrlCmd & 0x00000040;
}
#else
void stepCtrlCmd(uint32_t who, uint32_t ctrlCmd)
{
	*((uint32_t volatile *)(devSet.baseAddr[who] + CTRL_PORT)) = ctrlCmd;
	devState.curDir[who] = ctrlCmd & 0x00000001;
	devState.bInfinite[who] = ctrlCmd & 0x00000040;	
}
#endif

//motorSleepCmd()
//	selmotor
//	sleepCMD
	

void motorSleepAll()
{
//=====================================================	
// ���� : READY �����̰ų� BREAK���¿��� �� �������Ѵ�.		
//=====================================================	
#if FPGA_VERSION_TRIM
	motorSleepCmd(MOTOR_X , 	devState.curDir[MOTOR_X]);
	motorSleepCmd(MOTOR_Y , 	devState.curDir[MOTOR_Y]);
	motorSleepCmd(MOTOR_Z , 	devState.curDir[MOTOR_Z]);
	motorSleepCmd(MOTOR_SYR , 	devState.curDir[MOTOR_SYR]);
	motorSleepCmd(MOTOR_VAL , 	devState.curDir[MOTOR_VAL]);
	motorSleepCmd(MOTOR_INJ , 	devState.curDir[MOTOR_INJ]);
#else
	motorSleepCmd(MOTOR_X , 	devState.curDir[MOTOR_X]);
	motorSleepCmd(MOTOR_Y , 	devState.curDir[MOTOR_Y]);
	motorSleepCmd(MOTOR_Z , 	devState.curDir[MOTOR_Z]);
	motorSleepCmd(MOTOR_SYR , 	devState.curDir[MOTOR_SYR]);
	motorSleepCmd(MOTOR_VAL , 	devState.curDir[MOTOR_VAL]);
	motorSleepCmd(MOTOR_INJ , 	devState.curDir[MOTOR_INJ]);
#endif	
}

void motorBreakAll()
{
#if FPGA_VERSION_TRIM
	stepCtrlCmd(MOTOR_X , devState.curDir[MOTOR_X] | CMD_BREAK);
	stepCtrlCmd(MOTOR_Y , devState.curDir[MOTOR_Y] | CMD_BREAK);
	stepCtrlCmd(MOTOR_Z , devState.curDir[MOTOR_Z] | CMD_BREAK);
	stepCtrlCmd(MOTOR_SYR , devState.curDir[MOTOR_SYR] | CMD_BREAK);
	stepCtrlCmd(MOTOR_VAL , devState.curDir[MOTOR_VAL] | CMD_BREAK);
	stepCtrlCmd(MOTOR_INJ , devState.curDir[MOTOR_INJ] | CMD_BREAK);
#else
	stepCtrlCmd(MOTOR_X , devState.curDir[MOTOR_X] | CMD_BREAK);
	stepCtrlCmd(MOTOR_Y , devState.curDir[MOTOR_Y] | CMD_BREAK);
	stepCtrlCmd(MOTOR_Z , devState.curDir[MOTOR_Z] | CMD_BREAK);
	stepCtrlCmd(MOTOR_SYR , devState.curDir[MOTOR_SYR] | CMD_BREAK);
	stepCtrlCmd(MOTOR_VAL , devState.curDir[MOTOR_VAL] | CMD_BREAK);
	stepCtrlCmd(MOTOR_INJ , devState.curDir[MOTOR_INJ] | CMD_BREAK);
#endif
}

void motorResetAll()	// 
{
// ������°� BREAK�����̸� RESET��ȣ�� READY���·� �ȴ�. --> RESET�� ��ġ�� �ʰ� �ٷ� READY ���·� 
// ������°� SLEEP�����̸� 1/8step�� ��� 0x1250/4.6875MHz ����(1msec) reset�� Ready���·� �ȴ�.  -
	motorResetCmd(MOTOR_X);
	motorResetCmd(MOTOR_Y);
	motorResetCmd(MOTOR_Z);
	motorResetCmd(MOTOR_SYR);
	motorResetCmd(MOTOR_VAL);
	motorResetCmd(MOTOR_INJ);
}

void motorReadyAll()	// 
{
// ������°� BREAK�����̸� RESET��ȣ�� READY���·� �ȴ�. --> RESET�� ��ġ�� �ʰ� �ٷ� READY ���·� 
// ������°� SLEEP�����̸� 1/8step�� ��� 0x1250/4.6875MHz ����(1msec) reset�� Ready���·� �ȴ�.  -
#if FPGA_VERSION_TRIM
	motorResetCmd(MOTOR_X);
	motorResetCmd(MOTOR_Y);
	motorResetCmd(MOTOR_Z);
	motorResetCmd(MOTOR_SYR);
	motorResetCmd(MOTOR_VAL);
	motorResetCmd(MOTOR_INJ);
#else
	motorReadyCmd(MOTOR_X);
	motorReadyCmd(MOTOR_Y);
	motorReadyCmd(MOTOR_Z);
	motorReadyCmd(MOTOR_SYR);
	motorReadyCmd(MOTOR_VAL);
	motorReadyCmd(MOTOR_INJ);
#endif
}

void 	initStepCtrlData()
{
	for(int i=0; i< STEP_MOTOR_COUNT ; i++) {	
#if FPGA_VERSION_TRIM
		CMD_SEL_MOTOR(i);
		*((uint32_t volatile *)(devSet.totalStepAddr[i])) =	0x00;
		*((uint32_t volatile *)(devSet.startSpeedAddr[i]))  =	0x00;
		*((uint32_t volatile *)(devSet.accelStepAddr[i])) =	0x00;
		*((uint32_t volatile *)(devSet.deaccStepAddr[i])) =	0x00;
		*((uint32_t volatile *)(devSet.ctrlPortAddr[i])) 		= CMD_RESET;	// ready ���·� �����.
//		*((uint32_t volatile *)(devSet.ctrlPortAddr[i])) 		= CMD_SLEEP;	// 
#else
		*((uint32_t volatile *)(devSet.baseAddr[i] + TOTAL_OFFSET)) =	0x00;
		*((uint32_t volatile *)(devSet.baseAddr[i] + START_SPEED))  =	0x00;
		*((uint32_t volatile *)(devSet.baseAddr[i] + ACCEL_OFFSET)) =	0x00;
		*((uint32_t volatile *)(devSet.baseAddr[i] + DEACC_OFFSET)) =	0x00;
		*((uint32_t volatile *)(devSet.baseAddr[i] + CTRL_PORT)) 		= CMD_RESET;	// ready ���·� �����.
#endif		
	}

printf("	initStepCtrlData()    \n");

}

void initAdjustmentData()
{
	// load adjust data
}
//======================================================================
// 1. Valve home 
// 2. injector
// 3. syr,z 
// 4. x,y
//======================================================================
#define HOME_CHECK_INIT_STEP				1600	// X,Y  (1600 / 8 * 0.048768) = 9.7536mm
#define HOME_CHECK_INIT_STEP_Z			3200	// (3200 / 8 * 0.012192) = 4.8768mm
#if SYRINGE_HOME_FIND_FIX
	#define HOME_CHECK_INIT_STEP_SYR		6400
#else
	#define HOME_CHECK_INIT_STEP_SYR		3200
#endif

#define HOME_CHECK_INIT_STEP_VAL		2000 // 
#define HOME_CHECK_INIT_STEP_INJ		2000 // 

#define HOME_CHECK_STEP				160	//400	//1600	// X Y Z
#define HOME_CHECK_STEP_Z			160	//400	//3200
#define HOME_CHECK_STEP_SYR		160	//400	//3200
//#define HOME_CHECK_STEP_VAL		128//64	// 14.4��
//#define HOME_CHECK_STEP_INJ		400 // 272		// 61.2��

// ==============================================================================
// injector : ���ڴ��� Ȩ�� �д�. (�ո� �κ��� �д�.) - 60���� �ȵǰ� �շ�����.
//    �շ��ִ� �κ��� �� ��Ʈ���̿� �ִ� ��ġ�̰� , ���� �κп� ��Ʈ���� �ִ�.
// ���� �ٸ��� ã�´�. ���� �κ��� ã�´�. 
// �ո���ġ�� �ƴϸ� �ð� �������� �ո���ġ�� ã�� 
// �ٽ� �ð� �������� �����κ��� ã�´�. - �󸶰� �ð�������� ȸ�� -- �� (Ȩ��ġ �Ϸ�) 
// ==============================================================================
// ==============================================================================
// Valve Home : Washing port �� Syringe ���� ��ġ
// CW�������� 90�� ȸ�� : Syringe �� Needle port ���� ��ġ
// injector �� ��ó�� LOAD position ��ġ�Ͽ����Ѵ�.
// ==============================================================================
// �ʱⰪ 
// Injector : LOAD
// Valve : Washing Port
//uint8_t subMenu_ihpv	=	1;


#if FPGA_VERSION_TRIM
// 90(82)�� ������ ������ Ȩ ����
// 1. �����κ�
// 2. 18�� 
// 3. Ȩ��ġ
// 4. 18
// 5. sleep
// 6. �����κ�(Ȩã��)
// 7. adjust

int initHomePos_Val()	// Valve�� ���� Ȩ��ġ ã�� 
{
//	uint32_t who = MOTOR_VAL;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihpv != Menu) {
    dfp("subMenu_ihpv=%d\n",subMenu_ihpv);
    Menu = subMenu_ihpv;
}
#endif

	switch(subMenu_ihpv) {
		case 1:
			if(sleepChkVal(MOTOR_VAL)) {motorReadyCmd(MOTOR_VAL);}
				++subMenu_ihpv;				
			break;
		case 2:
			if(readyChkVal(MOTOR_VAL))
			{	// READY ���°˻� 
				stepControl(MOTOR_VAL, HOME_DIR_VAL | CMD_START, devSet.chkHomeSpeed[MOTOR_VAL], HOME_CHECK_INIT_STEP_VAL);
				++subMenu_ihpv;				
			}
			break;
		case 3:	// 
			if((devState.btMotorState[MOTOR_VAL] & VAL_HOME1_CHECK) == VAL_HOME1_SENSING) {	// Ȩ�� ������ġ
				stepCtrlCmd(MOTOR_VAL , HOME_DIR_VAL | CMD_BREAK);	
				++subMenu_ihpv;
			}
			break;
		case 4:
			if(breakChkVal(MOTOR_VAL)) {
//#if FPGA_VERSION_TRIM_BREAK			
				motorResetCmd(MOTOR_VAL);		
//#else
//				motorReadyCmd(MOTOR_VAL);		
//#endif				
				++subMenu_ihpv;
			}
			break;
		case 5:
			if(readyChkVal(MOTOR_VAL)) {	// 45�� ���� ������.
				stepControl(MOTOR_VAL, HOME_DIR_VAL | CMD_START, devSet.chkHomeSpeed[MOTOR_VAL], STEP_VAL_DEGREE_18);
				++subMenu_ihpv;				
			}
			break;
		case 6:
			if(readyChkVal(MOTOR_VAL)) {	// 
				stepControl(MOTOR_VAL, HOME_DIR_VAL | CMD_START, devSet.chkHomeSpeed[MOTOR_VAL], HOME_CHECK_INIT_STEP_VAL);
				++subMenu_ihpv;								
			}
			break;
		case 7:	// �ո� ��ġ ã��
			if((devState.btMotorState[MOTOR_VAL] & VAL_HOME1_CHECK) != VAL_HOME1_SENSING) {	// �ո� ��ġ ã��
				stepCtrlCmd(MOTOR_VAL , HOME_DIR_VAL | CMD_BREAK);	
				++subMenu_ihpv;
			}
			break;
		case 8:
			if(breakChkVal(MOTOR_VAL)) {
				motorResetCmd(MOTOR_VAL);		
				++subMenu_ihpv;
			}
			break;
		case 9:
			if(readyChkVal(MOTOR_VAL)) {	// 18�� ���� ������.
				stepControl(MOTOR_VAL, HOME_DIR_VAL | CMD_START, devSet.chkHomeSpeed[MOTOR_VAL], STEP_VAL_DEGREE_18);
				++subMenu_ihpv;				
			}
			break;			
		case 10:
			if(readyChkVal(MOTOR_VAL)) {	// �ո� ��ġ 
				// CMD_SLEEP �Ͽ� ���͸� SLEEP���·� �����.
				// ���͸� RESET(�ʱ���·� ������� SLEEP���� RESET����� �����Ͽ����Ѵ�.) - ���Ǹ��� ����Ȩ���·� 
				motorSleepCmd(MOTOR_VAL , HOME_DIR_VAL);
				++subMenu_ihpv;
			}
			break;
		case 11:	// ���� �ʱ�ȭ RESET
			if(sleepChkVal(MOTOR_VAL)) {
				motorReadyCmd(MOTOR_VAL);		
				++subMenu_ihpv;
			}
			break;
		case 12:	// Ȩã���� adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
			if(readyChkVal(MOTOR_VAL)) {	// RESET���� �ٽ� READY���� --> �ٽ� Ȩ��ġ ã�� ���� --> 1.8�� ������ (8pulse)
				readMotorState();
				if((devState.btMotorState[MOTOR_VAL] & VAL_HOME1_CHECK) == VAL_HOME1_SENSING ) {
					// Ȩ��ġ - �����. adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
					// ���� �κ� - STEP_MORE_HOME_INJ(8pulse) �� ���� �����. -��
					moveStepSpeed(MOTOR_VAL, devSet.chkHomeSpeedMin[MOTOR_VAL], devSet.chkHomeSpeed[MOTOR_VAL], devSet.adjust.value[MOTOR_VAL] * ADJUST_STEP_UNIT_VAL);					
					++subMenu_ihpv;
				}
				else { // �ո� �κ� - �����ƾ� �Ѵ�. -- �ٽ� ����
					stepControl(MOTOR_VAL, HOME_DIR_VAL | CMD_START, devSet.chkHomeSpeed[MOTOR_VAL], FIND_HOME_UNIT_STEP_VAL);
				}
			}
			break;
		case 13:
			if(readyChkVal(MOTOR_VAL)) {	//
				// ��꿡 ���� ���� ���� �����ÿ��� ���͸� SLEEP���·� �������.
#if Z_MOTOR_NOISE_FIX				
//				motorSleepCmd(MOTOR_VAL , HOME_DIR_VAL);
#else
				motorSleepCmd(MOTOR_VAL , HOME_DIR_VAL);
#endif
				++subMenu_ihpv;
			}
//dfp("devState.btMotorState[(A)=0x%x\n",devState.btMotorState[MOTOR_VAL]);	
			break;
		case 14: 
//				devState.curPos[MOTOR_VAL] = POS_VAL_WASHING;
				devState.curStep[MOTOR_VAL] = 0;

				subMenu_ihpv = 1;
				devState.homeFindEnd[MOTOR_VAL] = YES;
				return ACTION_MENU_END;
			break;

	}
	
	return ACTION_MENU_ING;
}

// 60(52)�� 
// 1. �����κ�
// 2. 18�� 
// 3. Ȩ��ġ
// 4. 18
// 5. sleep
// 6. �����κ�(Ȩã��)
// 7. adjust
// 8. go POS_INJ_INJECT

// �����ʹ� Ȩ(POS_INJ_LOAD)�� ã���� POS_INJ_INJECT�� ��ȯ�Ͽ� ����Ѵ�.
int initHomePos_Inj()	// �����͸� ���� Ȩ��ġ ã�� 
{
//	uint32_t who = MOTOR_INJ;
//	uint32_t	homeStep;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihpi != Menu) {
    dfp("subMenu_ihpi=%d\n",subMenu_ihpi);
    Menu = subMenu_ihpi;
}
#endif

	switch(subMenu_ihpi) {
		case 1:
			if(sleepChkVal(MOTOR_INJ)) {motorReadyCmd(MOTOR_INJ);}
			++subMenu_ihpi;				
			break;
		case 2:
			if(readyChkVal(MOTOR_INJ))
			{	// READY ���°˻� 
				stepControl(MOTOR_INJ, HOME_DIR_INJ | CMD_START, devSet.chkHomeSpeed[MOTOR_INJ], HOME_CHECK_INIT_STEP_INJ);				
				++subMenu_ihpi;				
			}
			break;
		case 3:	// 
			if((devState.btMotorState[MOTOR_INJ] & INJ_HOME1_CHECK) == INJ_HOME1_SENSING) {	// Ȩ�� ������ġ
				stepCtrlCmd(MOTOR_INJ , HOME_DIR_INJ | CMD_BREAK);	// CMD_STOP�� S2_BREAK���� ����.(ISO ��忡��)
				++subMenu_ihpi;
			}
			break;
		case 4:
			if(breakChkVal(MOTOR_INJ)) {
				motorResetCmd(MOTOR_INJ);		
				++subMenu_ihpi;
			}
			break;
		case 5:
			if(readyChkVal(MOTOR_INJ)) {	// 45�� ���� ������.
				stepControl(MOTOR_INJ, HOME_DIR_INJ | CMD_START, devSet.chkHomeSpeed[MOTOR_INJ], STEP_INJ_DEGREE_18);
				++subMenu_ihpi;				
			}
			break;			
		case 6:
			if(readyChkVal(MOTOR_INJ))
			{	// READY ���°˻� 
				stepControl(MOTOR_INJ, HOME_DIR_INJ | CMD_START, devSet.chkHomeSpeed[MOTOR_INJ], HOME_CHECK_INIT_STEP_INJ);				
				++subMenu_ihpi;				
			}
			break;
		case 7:	// 
			if((devState.btMotorState[MOTOR_INJ] & INJ_HOME1_CHECK) != INJ_HOME1_SENSING) {	// Ȩ�� ���κκ�(��ġ) �ΰ�?
				stepCtrlCmd(MOTOR_INJ , HOME_DIR_INJ | CMD_BREAK);	// CMD_STOP�� S2_BREAK���� ����.(ISO ��忡��)
				++subMenu_ihpi;
			}
			break;
		case 8:
			if(breakChkVal(MOTOR_INJ)) {
				motorResetCmd(MOTOR_INJ);		
				++subMenu_ihpi;
			}
			break;
		case 9:
			if(readyChkVal(MOTOR_INJ)) {	// 45�� ���� ������.
				stepControl(MOTOR_INJ, HOME_DIR_INJ | CMD_START, devSet.chkHomeSpeed[MOTOR_INJ], STEP_INJ_DEGREE_18);
				++subMenu_ihpi;				
			}
			break;			
		case 10:
			if(readyChkVal(MOTOR_INJ)) {	// �ո� ��ġ 
				// CMD_SLEEP �Ͽ� ���͸� SLEEP���·� �����.
				// ���͸� RESET(�ʱ���·� ������� SLEEP���� RESET����� �����Ͽ����Ѵ�.) - ���Ǹ��� ����Ȩ���·� 
				motorSleepCmd(MOTOR_INJ , HOME_DIR_INJ);
				++subMenu_ihpi;
			}
			break;
		case 11:	// ���� �ʱ�ȭ RESET
			if(sleepChkVal(MOTOR_INJ)) {
				motorReadyCmd(MOTOR_INJ);		
				++subMenu_ihpi;
			}			
			break;
		case 12:	// Ȩã���� adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
			if(readyChkVal(MOTOR_INJ)) {	// RESET���� �ٽ� READY���� --> �ٽ� Ȩ��ġ ã�� ���� --> 1.8�� ������ (8pulse)
				readMotorState();
				if((devState.btMotorState[MOTOR_INJ] & INJ_HOME1_CHECK) == INJ_HOME1_SENSING ) {
					// Ȩ��ġ - �����. adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
					// ���� �κ� - STEP_MORE_HOME_INJ(8pulse) �� ���� �����. -��
					moveStepSpeed(MOTOR_INJ, devSet.chkHomeSpeedMin[MOTOR_INJ], devSet.chkHomeSpeed[MOTOR_INJ], devSet.adjust.value[MOTOR_INJ] * ADJUST_STEP_UNIT_INJ);					
					++subMenu_ihpi;
				}
				else { // �ո� �κ� - �����ƾ� �Ѵ�. -- �ٽ� ����
					stepControl(MOTOR_INJ, HOME_DIR_INJ | CMD_START, devSet.chkHomeSpeed[MOTOR_INJ], FIND_HOME_UNIT_STEP_INJ);
				}
			}
			break;
		case 13:
			if(readyChkVal(MOTOR_INJ)) {	//
				devState.curStep[MOTOR_INJ] = 0;
// ���⿡ ������ Home Find�� ���ߴϱ� �� �������� �־�� �Ѵ�.				
//				devState.homeFindEnd[MOTOR_INJ] = YES;
				++subMenu_ihpi;				
			}
			break;
		case 14:
			if(readyChkVal(MOTOR_INJ)) {	//
				goInj(POS_INJ_INJECT,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);
				++subMenu_ihpi;
			}
			break;
		case 15:
			if(readyChkVal(MOTOR_INJ)) {	//
				// �����Ϳ� ���� ���� ���� �����ÿ��� ���͸� SLEEP���·� �������.
//				motorSleepCmd(MOTOR_INJ , HOME_DIR_INJ);
#if Z_MOTOR_NOISE_FIX
//				motorSleepCmd(MOTOR_INJ , R_HOME_DIR_INJ);
#else
				motorSleepCmd(MOTOR_INJ , R_HOME_DIR_INJ);
#endif
				++subMenu_ihpi;
			}
			break;
		case 16: 
//				devState.curStep[MOTOR_INJ] = xyzCoord.injStep[POS_INJ_INJECT]
				devState.homeFindEnd[MOTOR_INJ] = YES;
				subMenu_ihpi = 1;
				return ACTION_MENU_END;
			break;
	}

	return ACTION_MENU_ING;			
}
#else
int initHomePos_Val()	// Valve�� ���� Ȩ��ġ ã�� 
{
	uint32_t who = MOTOR_VAL;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihpv != Menu) {
    dfp("subMenu_ihpv=%d\n",subMenu_ihpv);
    Menu = subMenu_ihpv;
}
#endif

	switch(subMenu_ihpv) {
		case 1:
			if(sleepChkVal(who)) {motorReadyCmd(who);}
				++subMenu_ihpv;				
			break;
		case 2:
			if(readyChkVal(who))
			{	// READY ���°˻� 
				stepControl(who, HOME_DIR_VAL | CMD_START, devSet.chkHomeSpeed[who], HOME_CHECK_INIT_STEP_VAL);
				++subMenu_ihpv;				
			}
			break;
		case 3:	// 
			if((devState.btMotorState[who] & VAL_HOME1_CHECK) == VAL_HOME1_SENSING) {	// Ȩ�� ������ġ
				stepCtrlCmd(who , HOME_DIR_VAL | CMD_BREAK);	
				++subMenu_ihpv;
			}
			break;
		case 4:
			if(breakChkVal(who)) {
				motorResetCmd(who);		
				++subMenu_ihpv;
			}
			break;
		case 5:
			if(readyChkVal(who)) {	// 45�� ���� ������.
				stepControl(who, HOME_DIR_VAL | CMD_START, devSet.chkHomeSpeed[who], STEP_VAL_DEGREE_45);
				++subMenu_ihpv;				
			}
			break;
		case 6:
			if(readyChkVal(who)) {	// 
				stepControl(who, HOME_DIR_VAL | CMD_START, devSet.chkHomeSpeed[who], HOME_CHECK_INIT_STEP_VAL);
				++subMenu_ihpv;								
			}
			break;
		case 7:	// �ո� ��ġ ã��
			if((devState.btMotorState[who] & VAL_HOME1_CHECK) != VAL_HOME1_SENSING) {	// �ո� ��ġ ã��
				stepCtrlCmd(who , HOME_DIR_VAL | CMD_BREAK);	
				++subMenu_ihpv;
			}
			break;
		case 8:
			if(breakChkVal(who)) {
				motorResetCmd(who);		
				++subMenu_ihpv;
			}
			break;
		case 9:
			if(readyChkVal(who)) {	// �ո� ��ġ 
				// CMD_SLEEP �Ͽ� ���͸� SLEEP���·� �����.
				// �����Ϳ� ���� ���� ���� �����ÿ��� ���͸� SLEEP���·� �������.
				// ���͸� RESET(�ʱ���·� ������� SLEEP���� RESET����� �����Ͽ����Ѵ�.) - ���Ǹ��� ����Ȩ���·� 
				motorSleepCmd(who , HOME_DIR_VAL);
				++subMenu_ihpv;
			}
			break;
		case 10:	// ���� �ʱ�ȭ RESET
			if(sleepChkVal(who)) {
				motorReadyCmd(who);		
				++subMenu_ihpv;
			}
			break;
		case 11:	// Ȩã���� adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
			if(readyChkVal(who)) {	// RESET���� �ٽ� READY���� --> �ٽ� Ȩ��ġ ã�� ���� --> 1.8�� ������ (8pulse)
				readMotorState();
				if((devState.btMotorState[who] & VAL_HOME1_CHECK) == VAL_HOME1_SENSING ) {
					// Ȩ��ġ - �����. adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
					// ���� �κ� - STEP_MORE_HOME_INJ(8pulse) �� ���� �����. -��
					moveStepSpeed(who, devSet.chkHomeSpeedMin[who], devSet.chkHomeSpeed[who], devSet.adjust.value[who] * ADJUST_STEP_UNIT_VAL);					
					++subMenu_ihpv;
				}
				else { // �ո� �κ� - �����ƾ� �Ѵ�. -- �ٽ� ����
					stepControl(who, HOME_DIR_VAL | CMD_START, devSet.chkHomeSpeed[who], FIND_HOME_UNIT_STEP_VAL);
				}
			}
			break;
		case 12:
			if(readyChkVal(who)) {	//
				// �����Ϳ� ���� ���� ���� �����ÿ��� ���͸� SLEEP���·� �������.
				motorSleepCmd(who , HOME_DIR_VAL);
				++subMenu_ihpv;
			}
//dfp("devState.btMotorState[(A)=0x%x\n",devState.btMotorState[MOTOR_VAL]);	
			break;
		case 13: 
//				devState.curPos[who] = POS_VAL_WASHING;
				devState.curStep[who] = 0;

				subMenu_ihpv = 1;
				devState.homeFindEnd[who] = YES;
				return ACTION_MENU_END;
			break;

	}
	
	return ACTION_MENU_ING;
}

// �����ʹ� Ȩ(POS_INJ_LOAD)�� ã���� POS_INJ_INJECT�� ��ȯ�Ͽ� ����Ѵ�.
int initHomePos_Inj()	// �����͸� ���� Ȩ��ġ ã�� 
{
	uint32_t who = MOTOR_INJ;
	
#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihpi != Menu) {
    dfp("subMenu_ihpi=%d\n",subMenu_ihpi);
    Menu = subMenu_ihpi;
}
#endif

	switch(subMenu_ihpi) {
		case 1:
			if(sleepChkVal(who)) {motorReadyCmd(who);}
			++subMenu_ihpi;				
			break;
		case 2:
			if(readyChkVal(who))
			{	// READY ���°˻� 
				stepControl(who, HOME_DIR_INJ | CMD_START, devSet.chkHomeSpeed[who], HOME_CHECK_INIT_STEP_INJ);				
				++subMenu_ihpi;				
			}
			break;
		case 3:	// 
			if((devState.btMotorState[who] & INJ_HOME1_CHECK) == INJ_HOME1_SENSING) {	// Ȩ�� ������ġ
				stepCtrlCmd(who , HOME_DIR_INJ | CMD_BREAK);	// CMD_STOP�� S2_BREAK���� ����.(ISO ��忡��)
				++subMenu_ihpi;
			}
			break;
		case 4:
			if(breakChkVal(who)) {
				motorResetCmd(who);		
				++subMenu_ihpi;
			}
			break;
		case 5:
			if(readyChkVal(who)) {	// 45�� ���� ������.
				stepControl(who, HOME_DIR_INJ | CMD_START, devSet.chkHomeSpeed[who], STEP_INJ_DEGREE_45);
				++subMenu_ihpi;				
			}
			break;			
		case 6:
			if(readyChkVal(who))
			{	// READY ���°˻� 
				stepControl(who, HOME_DIR_INJ | CMD_START, devSet.chkHomeSpeed[who], HOME_CHECK_INIT_STEP_INJ);				
				++subMenu_ihpi;				
			}
			break;
		case 7:	// 
			if((devState.btMotorState[who] & INJ_HOME1_CHECK) != INJ_HOME1_SENSING) {	// Ȩ�� ���κκ�(��ġ) �ΰ�?
				stepCtrlCmd(who , HOME_DIR_INJ | CMD_BREAK);	// CMD_STOP�� S2_BREAK���� ����.(ISO ��忡��)
				++subMenu_ihpi;
			}
			break;
		case 8:
			if(breakChkVal(who)) {
				motorResetCmd(who);		
				++subMenu_ihpi;
			}
			break;
		case 9:
			if(readyChkVal(who)) {	// �ո� ��ġ 
				// CMD_SLEEP �Ͽ� ���͸� SLEEP���·� �����.
				// �����Ϳ� ���� ���� ���� �����ÿ��� ���͸� SLEEP���·� �������.
				// ���͸� RESET(�ʱ���·� ������� SLEEP���� RESET����� �����Ͽ����Ѵ�.) - ���Ǹ��� ����Ȩ���·� 
				motorSleepCmd(who , HOME_DIR_INJ);
				++subMenu_ihpi;
			}
			break;
		case 10:	// ���� �ʱ�ȭ RESET
			if(sleepChkVal(who)) {
				motorReadyCmd(who);		
				++subMenu_ihpi;
			}			
			break;
		case 11:	// Ȩã���� adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
			if(readyChkVal(who)) {	// RESET���� �ٽ� READY���� --> �ٽ� Ȩ��ġ ã�� ���� --> 1.8�� ������ (8pulse)
				readMotorState();
				if((devState.btMotorState[who] & INJ_HOME1_CHECK) == INJ_HOME1_SENSING ) {
					// Ȩ��ġ - �����. adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
					// ���� �κ� - STEP_MORE_HOME_INJ(8pulse) �� ���� �����. -��
					moveStepSpeed(who, devSet.chkHomeSpeedMin[who], devSet.chkHomeSpeed[who], devSet.adjust.value[who] * ADJUST_STEP_UNIT_INJ);					
					++subMenu_ihpi;
				}
				else { // �ո� �κ� - �����ƾ� �Ѵ�. -- �ٽ� ����
					stepControl(who, HOME_DIR_INJ | CMD_START, devSet.chkHomeSpeed[who], FIND_HOME_UNIT_STEP_INJ);
				}
			}
			break;
		case 12:
			if(readyChkVal(who)) {	//
				devState.curStep[who] = 0;
// ���⿡ ������ Home Find�� ���ߴϱ� �� �������� �־�� �Ѵ�.				
//				devState.homeFindEnd[who] = YES;
				++subMenu_ihpi;				
			}
			break;
		case 13:
			if(readyChkVal(who)) {	//
				goInj(POS_INJ_INJECT,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);
				++subMenu_ihpi;
			}
			break;
		case 14:
			if(readyChkVal(who)) {	//
				// �����Ϳ� ���� ���� ���� �����ÿ��� ���͸� SLEEP���·� �������.
//				motorSleepCmd(who , HOME_DIR_INJ);
				motorSleepCmd(who , R_HOME_DIR_INJ);
				++subMenu_ihpi;
			}
			break;
		case 15: 
//				devState.curStep[who] = xyzCoord.injStep[POS_INJ_INJECT]
				devState.homeFindEnd[who] = YES;
				subMenu_ihpi = 1;
				return ACTION_MENU_END;
			break;
	}

	return ACTION_MENU_ING;			
}


#endif

#if SYRINGE_HOME_FIND_FIX
int initHomePos_Syr()	// 
{
	int32_t homeStep;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihps != Menu) {
    dfp("subMenu_ihps=%d\n",subMenu_ihps);
    Menu = subMenu_ihps;
}
#endif

//static uint32_t homeChk_retryCnt = 0;

	switch(subMenu_ihps) {
		case 1:
			if(readyChk(MOTOR_SYR))
			{	// READY ���°˻� 
				++subMenu_ihps;
			}
			break;
		case 2:
			if((devState.btMotorState[MOTOR_SYR] & SYR_HOME1_CHECK) == SYR_HOME1_SENSING) {	// ���� Ȩ��ġ�� �ִ°�� - �ݴ�� ������ �ٽ� ����
				stepControl(MOTOR_SYR, R_HOME_DIR_SYR| CMD_START, devSet.chkHomeSpeed[MOTOR_SYR], HOME_CHECK_INIT_STEP_SYR);
			}
			++subMenu_ihps;
			break;
		case 3:	// Ȩ��ġ ã��
			if(readyChk(MOTOR_SYR)) {	// ���� �Ϸ�
				if((devState.btMotorState[MOTOR_SYR] & SYR_HOME1_CHECK) == SYR_HOME1_SENSING) {	
					// error
					errorFunction(SELF_ERROR_HOME_CHK_SYR);
				}
				else {
					devSet.ucChkHomeMethod[MOTOR_SYR] = CMD_BREAK;
					goSensorHome(MOTOR_SYR, devSet.chkHomeSpeed[MOTOR_SYR], devSet.chkHomeStepCnt[MOTOR_SYR]);
					++subMenu_ihps;
				}
			}
			break;
		case 4:
			if(homeChkEnd(MOTOR_SYR)) { // breakChk(Z)
				motorResetCmd(MOTOR_SYR);	// ���������� break�̹Ƿ� ready(reset)��ȣ �ԷµǾ�� �Ѵ�. - reset)��ȣ �ԷµǸ� ready ��						
				++subMenu_ihps;
			}
			break;
		case 5:	// Ȩ��ġ ã���� �ٽ� �������� �ణ �̵� 
			stepControl(MOTOR_SYR, R_HOME_DIR_SYR| CMD_START, devSet.chkHomeSpeed[MOTOR_SYR], HOME_CHECK_STEP_SYR);
			++subMenu_ihps;
			break;
		case 6:
			if(readyChk(MOTOR_SYR)) {	// ���� �Ϸ�
				// ���͸� RESET(�ʱ���·� ������� SLEEP���� RESET����� �����Ͽ����Ѵ�.) - ���Ǹ��� ����Ȩ���·� 
				motorSleepCmd(MOTOR_SYR , HOME_DIR_SYR);
				++subMenu_ihps;
			}
			break;
		case 7:	// ���� �ʱ�ȭ RESET
			if(sleepChk(MOTOR_SYR)) {
				motorReadyCmd(MOTOR_SYR);		
				++subMenu_ihps;
			}
			break;
		case 8: // Ȩã���� adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
			if(readyChk(MOTOR_SYR)) {	// RESET���� �ٽ� READY���� --> �ٽ� Ȩ��ġ ã�� ���� --> 16pulse)
				readMotorState();
				if((devState.btMotorState[MOTOR_SYR] & SYR_HOME1_CHECK) == SYR_HOME1_SENSING ) {
					// Ȩ��ġ - �����. adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
					// STEP_MORE_HOME_SYR(16pulse) �� ���� �����. -�� 
					// �ⱸ���� ������ Ȩ üũ �Ҿ����ϴ� Ȩ��ġ���� ���ݸ� �� �̵��Ͽ� ���� �Ҿ��� �ؼҸ���
					homeStep = devSet.adjust.value[MOTOR_SYR] * ADJUST_STEP_UNIT_SYR - STEP_MORE_HOME_SYR;	// Ȩ�������� �̵��ϸ�(-) / Ȩ�� �ݴ�� �̵�(+)
					
					if(homeStep > 0) { 
						stepControl(MOTOR_SYR, R_HOME_DIR_SYR | CMD_START, devSet.chkHomeSpeed[MOTOR_SYR], homeStep);
					}
					else if(homeStep < 0) {
						stepControl(MOTOR_SYR, HOME_DIR_SYR | CMD_START, devSet.chkHomeSpeed[MOTOR_SYR], -homeStep);
					}
					++subMenu_ihps;
				}
				else { // �����ƾ� �Ѵ�. -- �ٽ� ����
					stepControl(MOTOR_SYR, HOME_DIR_SYR | CMD_START, devSet.chkHomeSpeed[MOTOR_SYR], FIND_HOME_UNIT_STEP_SYR);
				}
			}
			break;		
		case 9:
			if(readyChk(MOTOR_SYR)) {	//
				// �����ÿ��� ���͸� SLEEP���·� �������. - �����ͳ� ��길 Sleep���·� 
//					motorSleepCmd(MOTOR_SYR , HOME_DIR_SYR);
				++subMenu_ihps;
			}
			break;
		case 10: 
			devState.curStep[MOTOR_SYR] = 0;

			subMenu_ihps = 1;
			devState.homeFindEnd[MOTOR_SYR] = YES;
			return ACTION_MENU_END;
			break;
	}

	return ACTION_MENU_ING;			
}
#else

int initHomePos_Syr()	// 
{
	uint32_t who = MOTOR_SYR;
	int32_t homeStep;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihps != Menu) {
    dfp("subMenu_ihps=%d\n",subMenu_ihps);
    Menu = subMenu_ihps;
}
#endif

	switch(subMenu_ihps) {
		case 1:
			if(readyChk(who))
			{	// READY ���°˻� 
				++subMenu_ihps;
			}
			break;
		case 2:
			if((devState.btMotorState[who] & SYR_HOME1_CHECK) == SYR_HOME1_SENSING) {	// ���� Ȩ��ġ�� �ִ°�� - �ݴ�� ������ �ٽ� ����
				stepControl(who, R_HOME_DIR_SYR| CMD_START, devSet.chkHomeSpeed[who], HOME_CHECK_INIT_STEP_SYR);
			}
			++subMenu_ihps;
			break;
		case 3:	// Ȩ��ġ ã��
			if(readyChk(who)) {	// ���� �Ϸ�
				devSet.ucChkHomeMethod[who] = CMD_BREAK;
				goSensorHome(who, devSet.chkHomeSpeed[who], devSet.chkHomeStepCnt[who]);
				++subMenu_ihps;
			}
			break;
		case 4:
			if(homeChkEnd(who)) { // breakChk(Z)
				motorResetCmd(who);	// ���������� break�̹Ƿ� ready(reset)��ȣ �ԷµǾ�� �Ѵ�. - reset)��ȣ �ԷµǸ� ready ��						
				++subMenu_ihps;
			}
			break;
		case 5:	// Ȩ��ġ ã���� �ٽ� �������� �ణ �̵� 
			stepControl(who, R_HOME_DIR_SYR| CMD_START, devSet.chkHomeSpeed[who], HOME_CHECK_STEP_SYR);
			++subMenu_ihps;
			break;
		case 6:
			if(readyChk(who)) {	// ���� �Ϸ�
				// ���͸� RESET(�ʱ���·� ������� SLEEP���� RESET����� �����Ͽ����Ѵ�.) - ���Ǹ��� ����Ȩ���·� 
				motorSleepCmd(who , HOME_DIR_SYR);
				++subMenu_ihps;
			}
			break;
		case 7:	// ���� �ʱ�ȭ RESET
			if(sleepChk(who)) {
				motorReadyCmd(who);		
				++subMenu_ihps;
			}
			break;
		case 8: // Ȩã���� adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
			if(readyChk(who)) {	// RESET���� �ٽ� READY���� --> �ٽ� Ȩ��ġ ã�� ���� --> 16pulse)
				readMotorState();
				if((devState.btMotorState[who] & SYR_HOME1_CHECK) == SYR_HOME1_SENSING ) {
					// Ȩ��ġ - �����. adjust(����)�� ��ŭ �̵� �Ѵ�.  -��
					// STEP_MORE_HOME_SYR(16pulse) �� ���� �����. -�� 
					// �ⱸ���� ������ Ȩ üũ �Ҿ����ϴ� Ȩ��ġ���� ���ݸ� �� �̵��Ͽ� ���� �Ҿ��� �ؼҸ���
					homeStep = devSet.adjust.value[who] * ADJUST_STEP_UNIT_SYR - STEP_MORE_HOME_SYR;	// Ȩ�������� �̵��ϸ�(-) / Ȩ�� �ݴ�� �̵�(+)
					
					if(homeStep > 0) { 
						stepControl(who, R_HOME_DIR_SYR | CMD_START, devSet.chkHomeSpeed[who], homeStep);
					}
					else if(homeStep < 0) {
						stepControl(who, HOME_DIR_SYR | CMD_START, devSet.chkHomeSpeed[who], -homeStep);
					}
					++subMenu_ihps;
				}
				else { // �����ƾ� �Ѵ�. -- �ٽ� ����
					stepControl(who, HOME_DIR_SYR | CMD_START, devSet.chkHomeSpeed[who], FIND_HOME_UNIT_STEP_SYR);
				}
			}
			break;		
		case 9:
			if(readyChk(who)) {	//
				// �����ÿ��� ���͸� SLEEP���·� �������. - �����ͳ� ��길 Sleep���·� 
//					motorSleepCmd(who , HOME_DIR_SYR);
				++subMenu_ihps;
			}
			break;
		case 10: 
			devState.curStep[who] = 0;	// ������ ��ġ�� �������� (adjust) 

			subMenu_ihps = 1;
			devState.homeFindEnd[who] = YES;
			return ACTION_MENU_END;
			break;
	}

	return ACTION_MENU_ING;			
}

#endif

int initHomePos_Z()	// 
{

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihpz != Menu) {
    dfp("subMenu_ihpz=%d\n",subMenu_ihpz);
    Menu = subMenu_ihpz;
}
#endif

static uint32_t homeChk_retryCnt = 0;

	switch(subMenu_ihpz) {
		case 1:
			if(readyChk(MOTOR_Z))
			{	// READY ���°˻� 
				++subMenu_ihpz;
homeChk_retryCnt = 0;				
			}
			break;
		case 2:
			if((devState.btMotorState[MOTOR_Z] & Z_HOME1_CHECK) == Z_HOME1_SENSING) {
//				stepControl(MOTOR_Z, R_HOME_DIR_Z | CMD_START, devSet.chkHomeSpeed[MOTOR_Z], HOME_CHECK_INIT_STEP_Z);
// ���� Ȩ��ġ�� �ִ°�� - �ݴ�� �����ϸ鼭 Ȩ���� ����� ������ �̵� 
				stepControl(MOTOR_Z, R_HOME_DIR_Z | CMD_START, devSet.chkHomeSpeed[MOTOR_Z], devSet.chkHomeStepCnt[MOTOR_Z]);
				devSet.ucChkHome_R[MOTOR_Z] = YES;
#if HOME_FIND_TIME_CHECK_Z	// �����ð����� homeChkEnd �ȵǸ� �����߰�
				check_time_cnt = System_ticks;
#endif
				subMenu_ihpz = 7;
			}
			else {
				devSet.ucChkHomeMethod[MOTOR_Z] = CMD_BREAK;
				goSensorHome(MOTOR_Z, devSet.chkHomeSpeed[MOTOR_Z], devSet.chkHomeStepCnt[MOTOR_Z]);
#if HOME_FIND_TIME_CHECK_Z	// �����ð����� homeChkEnd �ȵǸ� �����߰�
				check_time_cnt = System_ticks;
#endif
				subMenu_ihpz++;
			}
			break;
		case 3:
			if(homeChkEnd(MOTOR_Z)) { // breakChk(Z)
				// ������ üũ�ؼ� �극��ũ �ɸ� �����̴�.
				// �̻��¿��� ������ ������ ������ �ణ �����մ� ���¶� ������ �߻��� �� �մ�.
					motorResetCmd(MOTOR_Z);	// ���������� break�̹Ƿ� ready(reset)��ȣ �ԷµǾ�� �Ѵ�. - reset)��ȣ �ԷµǸ� ready ��						
					init_ticks(&nextCaseDelay);	// ���� ������ �����̸� ���ؼ� �ʱ�ȭ 				
					++subMenu_ihpz;
			}
#if HOME_FIND_TIME_CHECK_Z
			else {
				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_Z) {
					errorFunction(SELF_ERROR_HOME_CHK_Z); // �ð� �ʰ� 
				}
			}
#endif			
			break;
		case 4:
			if(readyChk(MOTOR_Z)) {	// RESET���� �ٽ� READY���� 
				GO_NEXT_CASE_TIME(nextCaseDelay, WAIT_HOME_FIND_INTERVAL, subMenu_ihpz);		// 							
			}
			break;
		case 5:
			stepControl(MOTOR_Z, HOME_DIR_Z | CMD_START, devSet.chkHomeSpeed[MOTOR_Z], STEP_MORE_HOME_Z);			
#if HOME_FIND_TIME_CHECK_Z
				check_time_cnt = System_ticks;
#endif
			++subMenu_ihpz;			
			break;
		case 6:	// Ȩ��ġ ã��
			if(readyChk(MOTOR_Z)) {
				stepControl(MOTOR_Z, R_HOME_DIR_Z | CMD_START, devSet.chkHomeSpeed[MOTOR_Z], devSet.chkHomeStepCnt[MOTOR_Z]);
				devSet.ucChkHome_R[MOTOR_Z] = YES;
#if HOME_FIND_TIME_CHECK_Z
				check_time_cnt = System_ticks;
#endif
				++subMenu_ihpz;							
			}			
#if HOME_FIND_TIME_CHECK_Z
			else {
				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_Z) {
					errorFunction(SELF_ERROR_HOME_CHK_Z); // �ð� �ʰ� 
				}
			}
#endif			
			break;
		case 7:
			if(homeChkEnd(MOTOR_Z)) { // breakChk(Z)
				motorResetCmd(MOTOR_Z);	// ���������� break�̹Ƿ� ready(reset)��ȣ �ԷµǾ�� �Ѵ�. - reset)��ȣ �ԷµǸ� ready ��						
				init_ticks(&nextCaseDelay);	// ���� ������ �����̸� ���ؼ� �ʱ�ȭ 				
				++subMenu_ihpz;
			}
#if HOME_FIND_TIME_CHECK_Z
			else {
				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_Z) {
					errorFunction(SELF_ERROR_HOME_CHK_Z); // �ð� �ʰ� 
				}
			}
#endif			
			break;
		case 8:
			if(readyChk(MOTOR_Z)) {	// RESET���� �ٽ� READY���� --> �ٽ� Ȩ��ġ ã�� ���� --> 16pulse)
				GO_NEXT_CASE_TIME(nextCaseDelay, WAIT_HOME_FIND_INTERVAL, subMenu_ihpz);		// 
			}
			break;


			
		case 9:	
// Ȩ��ġ ã���� �ٽ� �������� �ణ �̵� 
			stepControl(MOTOR_Z, HOME_DIR_Z | CMD_START, devSet.chkHomeSpeed[MOTOR_Z], STEP_MORE_HOME_Z);
//#if HOME_FIND_TIME_CHECK_Z
//				check_time_cnt = System_ticks;
//#endif
			++subMenu_ihpz;
			break;
		case 10:
			if(readyChk(MOTOR_Z)) {	//
				if((devState.btMotorState[MOTOR_Z] & Z_HOME1_CHECK) == Z_HOME1_SENSING) { // ���� Ȩ��ġ�� �վ�� �Ѵ�.
				// �����ÿ��� ���͸� SLEEP���·� �������. - �����ͳ� ��길 Sleep���·� 
					++subMenu_ihpz;
				}
				else {
if(++homeChk_retryCnt >= HOME_CHECK_RETRY_COUNT) {					
					errorFunction(SELF_ERROR_HOME_CHK_Z);
}
else {
subMenu_ihpz = 2;
}
				}
			}
//#if HOME_FIND_TIME_CHECK_Z
//			else {
//				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_Z) {
//					errorFunction(SELF_ERROR_HOME_CHK_Z); // �ð� �ʰ� 
//				}
//			}
//#endif			
			break;
		case 11: 
			devState.curStep[MOTOR_Z] = 0;
			subMenu_ihpz = 1;
			devState.homeFindEnd[MOTOR_Z] = YES;
			return ACTION_MENU_END;
			break;
	}

	return ACTION_MENU_ING;
}

// 0. Ȩ��ġ�� Ȩ��ġ�� �ݴ�� �ణ�̵� 3200
// 1. Ȩ��ġ ã��
// 2. Ȩ��ġã���� - motorReadyCmd() : Reset
// Ȩ��ġ�� �ݴ�� �ణ�̵� 3200
// 3. �̵��Ϸ��� Sleep
// 4. motorReadyCmd() : motor�ʱ�ȭ Reset
// 5. 16pulse ������ �̵��ϸ鼭 Ȩã�� �� 16pulse���̵� 
//uint8_t subMenu_ihpx	=	1;
int initHomePos_X()	// 
{
	
#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihpx != Menu) {
    dfp("subMenu_ihpx=%d\n",subMenu_ihpx);
    Menu = subMenu_ihpx;
}
#endif

static uint32_t homeChk_retryCnt = 0;

	switch(subMenu_ihpx) {
		case 1:
			if(readyChk(MOTOR_X))
			{	// READY ���°˻� 
				++subMenu_ihpx;
homeChk_retryCnt = 0;				
			}
			break;
		case 2:
			if((devState.btMotorState[MOTOR_X] & X_HOME1_CHECK) == X_HOME1_SENSING) {	// ���� Ȩ��ġ�� �ִ°�� - �ݴ�� ������ �ٽ� ����
				stepControl(MOTOR_X, R_HOME_DIR_X | CMD_START, devSet.chkHomeSpeed[MOTOR_X], devSet.chkHomeStepCnt[MOTOR_X]); 
				devSet.ucChkHome_R[MOTOR_X] = YES;
#if HOME_FIND_TIME_CHECK_X	// �����ð����� homeChkEnd �ȵǸ� �����߰�
				check_time_cnt = System_ticks;
#endif
				subMenu_ihpx = 9;				
			}
			else {
				devSet.ucChkHomeMethod[MOTOR_X] = CMD_BREAK;
				goSensorHome(MOTOR_X, devSet.chkHomeSpeed[MOTOR_X], devSet.chkHomeStepCnt[MOTOR_X]);
#if HOME_FIND_TIME_CHECK_X	// �����ð����� homeChkEnd �ȵǸ� �����߰�
				check_time_cnt = System_ticks;
#endif
				++subMenu_ihpx;
			}
			break;
		case 3:
			if(homeChkEnd(MOTOR_X)) {
					motorResetCmd(MOTOR_X);	// ���������� break�̹Ƿ� ready(reset)��ȣ �ԷµǾ�� �Ѵ�. - reset)��ȣ �ԷµǸ� ready ��						
					init_ticks(&nextCaseDelay);	// ���� ������ �����̸� ���ؼ� �ʱ�ȭ 				
					++subMenu_ihpx;
			}
#if HOME_FIND_TIME_CHECK_X
			else {
				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_X) {
					errorFunction(SELF_ERROR_HOME_CHK_X); // �ð� �ʰ� 
				}
			}
#endif			
			break;
		case 4:
			if(readyChk(MOTOR_X)) {	// RESET���� �ٽ� READY���� 
				GO_NEXT_CASE_TIME(nextCaseDelay, WAIT_HOME_FIND_INTERVAL, subMenu_ihpx);				
			}
			break;
		case 5:	
			stepControl(MOTOR_X, HOME_DIR_X | CMD_START, devSet.chkHomeSpeed[MOTOR_X], STEP_MORE_HOME_X);
#if HOME_FIND_TIME_CHECK_X
			check_time_cnt = System_ticks;
#endif			
			subMenu_ihpx++;
			break;
		case 6:
			if(readyChk(MOTOR_X)) {	// ���� �Ϸ�
//				stepControl(MOTOR_X, R_HOME_DIR_X | CMD_START, devSet.chkHomeSpeed[MOTOR_X], devSet.chkHomeStepCnt[MOTOR_X]); 
//				devSet.ucChkHome_R[MOTOR_X] = YES;
				init_ticks(&nextCaseDelay);	// ���� ������ �����̸� ���ؼ� �ʱ�ȭ 				
				subMenu_ihpx++;				
			}
#if HOME_FIND_TIME_CHECK_X
			else {
				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_X) {
					errorFunction(SELF_ERROR_HOME_CHK_X); // �ð� �ʰ� 
				}
			}
#endif			
			break;
		case 7:
			GO_NEXT_CASE_TIME(nextCaseDelay, WAIT_HOME_FIND_INTERVAL, subMenu_ihpx);		
			break;
		case 8:
			stepControl(MOTOR_X, R_HOME_DIR_X | CMD_START, devSet.chkHomeSpeed[MOTOR_X], devSet.chkHomeStepCnt[MOTOR_X]); 
			devSet.ucChkHome_R[MOTOR_X] = YES;
#if HOME_FIND_TIME_CHECK_X
			check_time_cnt = System_ticks;
#endif			
			subMenu_ihpx++;				
			break;
 		case 9:
			if(homeChkEnd(MOTOR_X)) {
				motorResetCmd(MOTOR_X);	// ���������� break�̹Ƿ� ready(reset)��ȣ �ԷµǾ�� �Ѵ�. - reset)��ȣ �ԷµǸ� ready ��						
				init_ticks(&nextCaseDelay);	// ���� ������ �����̸� ���ؼ� �ʱ�ȭ 			
				++subMenu_ihpx;
			}
#if HOME_FIND_TIME_CHECK_X
			else {
				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_X) {
					errorFunction(SELF_ERROR_HOME_CHK_X); // �ð� �ʰ� 
				}
			}
#endif			
			break;
		case 10:
			if(readyChk(MOTOR_X)) {	// RESET���� �ٽ� READY����
				GO_NEXT_CASE_TIME(nextCaseDelay, WAIT_HOME_FIND_INTERVAL, subMenu_ihpx);
			}
			break;

			
		case 11:
			stepControl(MOTOR_X, HOME_DIR_X | CMD_START, devSet.chkHomeSpeed[MOTOR_X], STEP_MORE_HOME_X);
//#if HOME_FIND_TIME_CHECK_X
//			check_time_cnt = System_ticks;
//#endif			
			subMenu_ihpx++;
			break;
		case 12:
			if(readyChk(MOTOR_X)) {	
				if((devState.btMotorState[MOTOR_X] & X_HOME1_CHECK) == X_HOME1_SENSING) { 
					subMenu_ihpx++;				
				}
				else {
if(++homeChk_retryCnt >= HOME_CHECK_RETRY_COUNT) {					
					errorFunction(SELF_ERROR_HOME_CHK_X);
}
else {
subMenu_ihpx = 2;
}	
				}
			}
//#if HOME_FIND_TIME_CHECK_X
//			else {
//				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_X) {
//					errorFunction(SELF_ERROR_HOME_CHK_X); // �ð� �ʰ� 
//				}
//			}
//#endif			
			break;
		case 13:
			devState.curTray = TRAY_ETC;
//			devState.curStep[MOTOR_X] = STEP_MORE_HOME_X;
			devState.curStep[MOTOR_X] = 0;
			
			subMenu_ihpx = 1;
			devState.homeFindEnd[MOTOR_X] = YES;
			return ACTION_MENU_END;
			break;
	}

	return ACTION_MENU_ING;			
}

int initHomePos_Y()	// 
{

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihpy != Menu) {
    dfp("subMenu_ihpy=%d\n",subMenu_ihpy);
    Menu = subMenu_ihpy;
}
#endif

static uint32_t homeChk_retryCnt = 0;

	switch(subMenu_ihpy) {
		case 1:
			if(readyChk(MOTOR_Y))
			{	// READY ���°˻� 
				++subMenu_ihpy;
homeChk_retryCnt = 0;
			}
			break;
		case 2:
			if((devState.btMotorState[MOTOR_Y] & Y_HOME1_CHECK) == Y_HOME1_SENSING) {	// ���� Ȩ��ġ�� �ִ°�� - �ݴ�� ������ �ٽ� ����
				stepControl(MOTOR_Y, R_HOME_DIR_Y | CMD_START, devSet.chkHomeSpeed[MOTOR_Y], devSet.chkHomeStepCnt[MOTOR_Y]);
				devSet.ucChkHome_R[MOTOR_Y] = YES;
#if HOME_FIND_TIME_CHECK_Y	// �����ð����� homeChkEnd �ȵǸ� �����߰�
				check_time_cnt = System_ticks;
#endif
				subMenu_ihpy = 9;				
			}
			else { // Ȩ ã�� 
				devSet.ucChkHomeMethod[MOTOR_Y] = CMD_BREAK;
				goSensorHome(MOTOR_Y, devSet.chkHomeSpeed[MOTOR_Y], devSet.chkHomeStepCnt[MOTOR_Y]);
#if HOME_FIND_TIME_CHECK_Y	// �����ð����� homeChkEnd �ȵǸ� �����߰�
				check_time_cnt = System_ticks;
#endif
				++subMenu_ihpy;
			}
			break;
		case 3:
//dfpErr("devState.btMotorState[Y]=x[%x] y[%x] z[%x]\n",devState.btMotorState[MOTOR_X],devState.btMotorState[MOTOR_Y],devState.btMotorState[MOTOR_Z]);				
			if(homeChkEnd(MOTOR_Y)) {
					motorResetCmd(MOTOR_Y);	// ���������� break�̹Ƿ� ready(reset)��ȣ �ԷµǾ�� �Ѵ�. - reset)��ȣ �ԷµǸ� ready ��						
					init_ticks(&nextCaseDelay);	// ���� ������ �����̸� ���ؼ� �ʱ�ȭ 				
					++subMenu_ihpy;
			}
#if HOME_FIND_TIME_CHECK_Y
			else {
				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_Y) {
					errorFunction(SELF_ERROR_HOME_CHK_Y); // �ð� �ʰ� 
				}
			}
#endif
			break;
		case 4:
			if(readyChk(MOTOR_Y)) {	// RESET���� �ٽ� READY���� 
				GO_NEXT_CASE_TIME(nextCaseDelay, WAIT_HOME_FIND_INTERVAL, subMenu_ihpy);
			}
			break;
		case 5:	// Ȩ
			stepControl(MOTOR_Y, HOME_DIR_Y | CMD_START, devSet.chkHomeSpeed[MOTOR_Y], STEP_MORE_HOME_Y);
#if HOME_FIND_TIME_CHECK_Y
			check_time_cnt = System_ticks;
#endif
			subMenu_ihpy++;
			break;
		case 6:
			if(readyChk(MOTOR_Y)) {	// ���� �Ϸ�
					init_ticks(&nextCaseDelay);	// ���� ������ �����̸� ���ؼ� �ʱ�ȭ 
					++subMenu_ihpy;
			}
#if HOME_FIND_TIME_CHECK_Y
			else {
				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_Y) {
					errorFunction(SELF_ERROR_HOME_CHK_Y); // �ð� �ʰ� 
				}
			}
#endif			
			break;
		case 7:
			GO_NEXT_CASE_TIME(nextCaseDelay, WAIT_HOME_FIND_INTERVAL, subMenu_ihpy);
			break;
		case 8:	// Ȩ �ݴ�� �̵��ϸ鼭 Ȩ���� ����� ������ �̵��Ͽ� ���� 
			stepControl(MOTOR_Y, R_HOME_DIR_Y | CMD_START, devSet.chkHomeSpeed[MOTOR_Y], devSet.chkHomeStepCnt[MOTOR_Y]);
			devSet.ucChkHome_R[MOTOR_Y] = YES;
#if HOME_FIND_TIME_CHECK_Y
			check_time_cnt = System_ticks;
#endif
			++subMenu_ihpy;
			break;
		case 9:
			if(homeChkEnd(MOTOR_Y)) {		
					motorResetCmd(MOTOR_Y);	// ���������� break�̹Ƿ� ready(reset)��ȣ �ԷµǾ�� �Ѵ�. - reset)��ȣ �ԷµǸ� ready ��						
					init_ticks(&nextCaseDelay);	// ���� ������ �����̸� ���ؼ� �ʱ�ȭ 				
					++subMenu_ihpy;
			}
#if HOME_FIND_TIME_CHECK_Y
			else {
				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_Y) {
					errorFunction(SELF_ERROR_HOME_CHK_Y); // �ð� �ʰ� 
				}
			}
#endif			
			break;		
		case 10:
			if(readyChk(MOTOR_Y)) {	// RESET���� �ٽ� READY����
				GO_NEXT_CASE_TIME(nextCaseDelay, WAIT_HOME_FIND_INTERVAL, subMenu_ihpy);
			}
			break;



			
		case 11:  // Ȩ������ �̵� 
			stepControl(MOTOR_Y, HOME_DIR_Y | CMD_START, devSet.chkHomeSpeed[MOTOR_Y], STEP_MORE_HOME_Y);
//#if HOME_FIND_TIME_CHECK_Y
//			check_time_cnt = System_ticks;
//#endif
			subMenu_ihpy++;
			break;
		case 12:
			if(readyChk(MOTOR_Y)) {	// ���� üũ 
				if((devState.btMotorState[MOTOR_Y] & Y_HOME1_CHECK) == Y_HOME1_SENSING) {
					subMenu_ihpy++;				
				}
				else {
if(++homeChk_retryCnt >= HOME_CHECK_RETRY_COUNT) {
dfpErr("devState.btMotorState[Y]=x[%x] y[%x] z[%x]\n",devState.btMotorState[MOTOR_X],devState.btMotorState[MOTOR_Y],devState.btMotorState[MOTOR_Z]);				
					errorFunction(SELF_ERROR_HOME_CHK_Y);
}
else {
subMenu_ihpy = 2;
}	
				}
			}
			
//#if HOME_FIND_TIME_CHECK_Y
//			else {
//				if((System_ticks - check_time_cnt) >  HOME_FINE_CHECK_MAX_TIME_Y) {
//					errorFunction(SELF_ERROR_HOME_CHK_Y); // �ð� �ʰ� 
//				}
//			}
//#endif			
			break;
		case 13:
			devState.curTray = TRAY_ETC;
//			devState.curStep[MOTOR_Y] = STEP_MORE_HOME_Y;
			devState.curStep[MOTOR_Y] = 0;
		
			subMenu_ihpy = 1;
			devState.homeFindEnd[MOTOR_Y] = YES;
			return ACTION_MENU_END;
			break;
	}

	return ACTION_MENU_ING;			
}

// ó�� ���� �µǸ� �ҿ��� ���� ���� ���� �������� ���� 
int initHomePos_All()
{
#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ihpa != Menu) {
    dfp("subMenu_ihpa=%d\n",subMenu_ihpa);
    Menu = subMenu_ihpa;
}
#endif

	switch(subMenu_ihpa) {
		case 1:
/*		
			if(sleepChk(MOTOR_X)) { 
				motorReadyCmd(MOTOR_X);
			}
			if(sleepChk(MOTOR_Y)) { 
				motorReadyCmd(MOTOR_Y);
			}
			if(sleepChk(MOTOR_Z)) { 
				motorReadyCmd(MOTOR_Z);
			}
			if(sleepChk(MOTOR_SYR)) { 
				motorReadyCmd(MOTOR_SYR);
			}
*/			
			if(sleepChkVal(MOTOR_VAL)) { 
				motorReadyCmd(MOTOR_VAL);
//dfp("motorReadyCmd(MOTOR_VAL) \n");
			}
			if(sleepChkVal(MOTOR_INJ)) { 
				motorReadyCmd(MOTOR_INJ);
//dfp("motorReadyCmd(MOTOR_INJ) \n");
			}

			++subMenu_ihpa;
			break;
		case 2:
//			if(readyChk(MOTOR_X) &&	readyChk(MOTOR_Y)	&& readyChk(MOTOR_Z) && readyChk(MOTOR_SYR) && readyChkVal(MOTOR_VAL) &&	readyChkVal(MOTOR_INJ)){
			if(readyChkVal(MOTOR_VAL) &&	readyChkVal(MOTOR_INJ)){
				++subMenu_ihpa;
			}
			break;
		case 3:
			if(devState.homeFindEnd[MOTOR_VAL] == NO) {
				initHomePos_Val();
			}
			else subMenu_ihpa++;
			break;
		case 4:
			if(devState.homeFindEnd[MOTOR_INJ] == NO) {
				initHomePos_Inj();
			}
			else subMenu_ihpa++;
			break;
		case 5:
			if(sleepChk(MOTOR_Z)) {
				motorReadyCmd(MOTOR_Z);
			}
			subMenu_ihpa++;			
			break;
		case 6:
			if(readyChk(MOTOR_Z)){
//				init_ticks(&nextCaseDelay);	// ���� ������ �����̸� ���ؼ� �ʱ�ȭ 
				++subMenu_ihpa;
			}
			break;
		case 7:
			if(devState.homeFindEnd[MOTOR_Z] == NO) {
				initHomePos_Z();
			}
			else subMenu_ihpa++;
			break;
		case 8:
			if(sleepChk(MOTOR_X)) { 
				motorReadyCmd(MOTOR_X);
			}
			subMenu_ihpa++;			
			break;
		case 9:
			if(readyChk(MOTOR_X)){
				++subMenu_ihpa;
			}
			break;
		case 10://:
			if(devState.homeFindEnd[MOTOR_X] == NO) {
				initHomePos_X();
			}
			else subMenu_ihpa++;//;
			break;
		case 11:
			if(sleepChk(MOTOR_Y)) { 
				motorReadyCmd(MOTOR_Y);
			}
			subMenu_ihpa++;			
			break;
		case 12:
			if(readyChk(MOTOR_Y)){
				++subMenu_ihpa;
			}
			break;
		case 13://:	// ����(����)������ X�� ���� ����
			if( devState.homeFindEnd[MOTOR_Y] == NO) {
				initHomePos_Y();
			}
			else {
				subMenu_ihpa++;//;
			}
			break;
#if 0			
		case 14:
			if(sleepChk(MOTOR_SYR)) { 
				motorReadyCmd(MOTOR_SYR);
			}
			subMenu_ihpa++;
		case 15:
			if(readyChk(MOTOR_SYR)){
				++subMenu_ihpa;
			}
			break;
		case 16://:		// �ø��� ���� ã�� ������ ���ؼ� waste��Ʈ�� �̵� 
			if(goPosWastePort()) {	subMenu_ihpa++;	}
//			if(goPosWashingPort()) {	subMenu_ihpa++;	}
			break;
#else
		case 14://:		// �ø��� ���� ã�� ������ ���ؼ� waste��Ʈ�� �̵� 
			if(goPosWastePort()) {	subMenu_ihpa++;	}
//			if(goPosWashingPort()) {	subMenu_ihpa++;	}
			break;
		case 15:
			if(sleepChk(MOTOR_SYR)) { 
				motorReadyCmd(MOTOR_SYR);
			}
			subMenu_ihpa++;
			break;
		case 16:
			if(readyChk(MOTOR_SYR)){
				++subMenu_ihpa;
			}
			break;

#endif
		case 17://:
			if(devState.homeFindEnd[MOTOR_SYR] == NO) {
				initHomePos_Syr();
			}
			else {
				subMenu_ihpa++;//;
			}
			break;
		case 18://:
			if(goPosHome()) { 
				subMenu_ihpa = 1;//; 
				return ACTION_MENU_END;				
			}
			break;
	}				

	return ACTION_MENU_ING;
}


/* =============================================================
// ������ Tray���� ���� 
1. TRAY_VIALS_40 pos (D,10) , COORD_Z_VIAL_40_DETECT ?
	YES : TRAY_VIALS_40Ȯ�� (Vial ��) -- Go 7
	NO : Go 2
2. TRAY_VIALS_60 pos (E,12) , COORD_Z_VIAL_60_DETECT ?
	YES : Go 3 - 40���� 60���� ���� �ȵ�.
	NO : Go 5
3. TRAY_VIALS_40 pos (D,10) , COORD_Z_VIAL_60_DETECT ? ���� 60 Vial Detection
	40vial ��ġ���� ���̾� ������ �˻��Ͽ� �������� �����Ǹ� 40vial 
	YES : TRAY_VIALS_60Ȯ�� (Vial ��) -- Go 7 
	NO : Go 4
4. Z DOWN Detect ? COORD_Z_VIAL_60_DETECT + COORD_Z_VIAL_40_TRAY_DETECT =(4mm)
	YES : TRAY_VIALS_40Ȯ�� (Vial ��) -- Go 7
	NO : Go 5
5. Z DOWN Detect ? COORD_Z_VIAL_60_DETECT + COORD_Z_VIAL_60_TRAY_DETECT =(11.7mm)
	YES : TRAY_VIALS_60Ȯ�� (Vial ��) -- Go 7
	NO : Go 6
6. WELLPALTE_96 pos (12,A) , COORD_Z_WELLPALTE_96_DETECT ?
	YES : WELLPALTE_96Ȯ��   -- Go 12
	NO : Tray ��
	
// ���� Tray ���� 
7.TRAY_VIALS_40 pos (A,10) , COORD_Z_VIAL_40_DETECT ?
	YES : TRAY_VIALS_40Ȯ�� (Vial ��) - end
	NO : Go 8
8. TRAY_VIALS_60 pos (A,12) , COORD_Z_VIAL_60_DETECT ?
	YES : Go 9 - 40���� 60���� ���� �ȵ�.
	NO : Go 11
9. TRAY_VIALS_40 pos (D,10) , COORD_Z_VIAL_60_DETECT ? ���� 60 Vial Detection
	40vial ��ġ���� ���̾� ������ �˻��Ͽ� �������� �����Ǹ� 40vial 
	YES : TRAY_VIALS_60Ȯ�� (Vial ��) - end
	NO : Go 10
10. Z DOWN Detect ? COORD_Z_VIAL_60_DETECT + COORD_Z_VIAL_40_TRAY_DETECT =(4mm)
	YES : TRAY_VIALS_40Ȯ�� (Vial ��) - end 
	NO : Go 11
11. Z DOWN Detect ? COORD_Z_VIAL_60_DETECT + COORD_Z_VIAL_60_TRAY_DETECT =(11.7mm)
	YES : TRAY_VIALS_60Ȯ�� (Vial ��) - end
	NO : Go 12
12. WELLPALTE_96 pos (12,A) , COORD_Z_WELLPALTE_96_DETECT ?
	YES : WELLPALTE_96Ȯ��   - end
	NO : Tray ��
		
	
================================================================*/	

int adjustMotorInit()
{

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ami != Menu) {
    dfp("subMenu_ami=%d\n",subMenu_ami);
    Menu = subMenu_ami;
}
#endif

//	VIAL_POS_t *pVial;	// �ʱ�ȭ ��Ų�� ����ؾ� �Ѵ�.
	VIAL_POS_t Vial;
	static int32_t zPos;

  switch(subMenu_ami) {
		case 1:
			switch(devSet.adjustCurMotor) {
#if	MECH_VER == 6				
				case ADJUST_NEEDLE_XY:
					if(goPosETC_Z(POS_XY_INJECTOR, POS_Z_INJECTOR_TOP_ADJUST)) {	// ��ó�� ��ǰ					
						subMenu_ami = 8;
					}
					break;					
				case ADJUST_NEEDLE_HEIGHT:
					if(goPosETC_Z(POS_XY_INJECTOR, POS_Z_INJECTOR_TOP)) {				// ��ó�� ��ǰ				
						subMenu_ami = 8;
					}
					break;
#elif	MECH_VER == 7				
				case ADJUST_NEEDLE_XY:
					if(goPosETC_Z(POS_XY_INJECTOR, POS_Z_INJECTOR_TOP_ADJUST)) {	// ��ó�� ��ǰ					
						subMenu_ami = 8;
					}
					break;					
				case ADJUST_NEEDLE_HEIGHT:
//					if(goPosETC_Z(POS_XY_EXCHANGE, POS_Z_INJECTOR_TOP)) {
					if(goPosETC_Z(POS_XY_EXCHANGE, POS_Z_NEEDLE_DEPTH_ADJUST)) {
						subMenu_ami = 8;
					}
					break;
#else
				case ADJUST_NEEDLE_XY:
					if(goPosETC_Z(POS_XY_INJECTOR, POS_Z_INJECTOR_TOP_ADJUST)) {
						subMenu_ami = 8;
					}
					break;					
				case ADJUST_NEEDLE_HEIGHT:
					if(goPosETC_Z(POS_XY_INJECTOR, POS_Z_INJECTOR_TOP)) {
						subMenu_ami = 8;
					}
					break;
#endif
				case ADJUST_SYRINGE:
					if(goPosETC_Z(POS_XY_WASTE, POS_Z_TOP)) {
						subMenu_ami = 8;
					}
					break;
				case ADJUST_VALVE_POS:
					if(sleepChkVal(MOTOR_VAL)) {
						motorReadyCmd(MOTOR_VAL);
					}
					subMenu_ami = 2;
					break;
				case ADJUST_INJECTOR_POS:
					if(sleepChkVal(MOTOR_INJ)) {
						motorReadyCmd(MOTOR_INJ);
					}
					subMenu_ami = 5;
					break;
				case ADJUST_WASHING_PORT:
					if(goPosETC_Z(POS_XY_WASH, POS_Z_WASH_ADJUST)) {
						subMenu_ami = 8;
					}
					break;
				case ADJUST_WASTE_PORT:
					if(goPosETC_Z(POS_XY_WASTE, POS_Z_WASH_ADJUST)) {
						subMenu_ami = 8;
					}
					break;					
				case ADJUST_VIAL_LEFT:
//				case ADJUST_VIAL_LEFT_1:
//				case ADJUST_VIAL_LEFT_2:
					Vial.uPos.tray = TRAY_LEFT;
					Vial.uPos.etc = 0;
					Vial.uPos.x = 0;
					Vial.uPos.y = 0;
//					if(goPosVial_Z(&Vial,POS_Z_VIAL_LEFT_TOP)) {
					if(goPosVial_Z(&Vial,POS_Z_VIAL_LEFT_ADJUST)) {
						subMenu_ami = 8;
					}
					break;
				case ADJUST_VIAL_RIGHT:
//				case ADJUST_VIAL_RIGHT_1:
//				case ADJUST_VIAL_RIGHT_2:
					Vial.uPos.tray = TRAY_RIGHT;
					Vial.uPos.etc = 0;
					Vial.uPos.x = 0;
					Vial.uPos.y = 0;
//					if(goPosVial_Z(&Vial,POS_Z_VIAL_RIGHT_TOP)) {
					if(goPosVial_Z(&Vial,POS_Z_VIAL_RIGHT_ADJUST)) {
						subMenu_ami = 8;
					}
					break;
				case ADJUST_VIAL_HEIGHT:
//					if(goPosETC_Z(POS_XY_INJECTOR, POS_Z_VIAL_HEIGHT_ADJUST)) {					
					if(goPosETC_Z(POS_XY_EXCHANGE, POS_Z_VIAL_HEIGHT_ADJUST)) {					
						subMenu_ami = 8;
					}
					break;
#if VIAL_POS_MODIFY
				case ADJUST_VIAL_HEIGHT_XY:
					if(devSet.adjust.saveVL[sysConfig.trayLeft] == DATA_SAVE_FLAG) {
						Vial.uPos.tray = TRAY_LEFT;
						Vial.uPos.etc = 0;
						switch(sysConfig.trayLeft) {
							case TRAY_VIALS_40:
								Vial.uPos.x = 3;
								Vial.uPos.y = 9;
								break;
							case TRAY_VIALS_60:
								Vial.uPos.x = 4;
								Vial.uPos.y = 11;
								break;
							case WELLPALTE_96:
								Vial.uPos.x = 0;
								Vial.uPos.y = 0;
								break;
							default:
								Vial.uPos.x = 0;
								Vial.uPos.y = 0;
								break;
						}
						if(goPosVial_Z(&Vial,POS_Z_VIAL_LEFT_ADJUST)) {
							subMenu_ami = 8;
						}
					}
					else {
						send_SelfMessage(SELF_COMMAND_REJECT,0);
					}
					break;
#endif
				case ADJUST_MICRO_PUMP_VOLTAGE:
					subMenu_ami = 8;
					break;
			}			
			break;
			
		case 2:
			if(readyChkVal(MOTOR_VAL)) {					
				goVal(POS_VAL_WASHING,	0,ACT_SPEED_MAX_VAL, ACT_XY_POS);
				++subMenu_ami;
			}
			break;
		case 3:
			if(readyChkVal(MOTOR_VAL)) {
				++subMenu_ami;
			}
			break;
		case 4:
			if(goPosETC_Z(POS_XY_WASTE, POS_Z_TOP)) {
				subMenu_ami = 8;
			}
			break;
			
		case 5:
			if(readyChkVal(MOTOR_INJ)) {	
				goInj(POS_INJ_LOAD,	0,ACT_SPEED_MAX_INJ, ACT_XY_POS);
				++subMenu_ami;
			}
			break;
		case 6:
			if(readyChkVal(MOTOR_INJ)) {
				++subMenu_ami;
			}
			break;
		case 7:
			if(goPosETC_Z(POS_XY_WASTE, POS_Z_TOP)) {
				subMenu_ami = 8;
			}
			break;

		case 8:
			if(readyChk(MOTOR_Z))	{
				++subMenu_ami;
			}
			break;
		case 9:	// �����.
			if(sleepChkVal(MOTOR_VAL)) {
				motorReadyCmd(MOTOR_VAL);
			}
			++subMenu_ami;
			break;
		case 10:
			if(readyChkVal(MOTOR_VAL)) {					
				if(sleepChkVal(MOTOR_INJ)) {
					motorReadyCmd(MOTOR_INJ);
				}
				++subMenu_ami;
			}
			break;
		case 11:
			if(readyChkVal(MOTOR_INJ)) {
				subMenu_ami = 1;
				
		    service.command = SERVICE_ADJUST;
		    service.subCommand1 = devSet.adjustCurMotor;
		    service.subCommand2 = ADJUST_INIT_POS_END;
				SEND_LCAS_PACKET(PACKCODE_LCAS_SERVICE, service, service);

				return ACTION_MENU_END;
			}
			break;
  }
	return ACTION_MENU_ING;		
	
}

// =================================================================================
// =================================================================================
int adjustNeedleExchange()
{
//	VIAL_POS_t *pVial;	// �ʱ�ȭ ��Ų�� ����ؾ� �Ѵ�.
	VIAL_POS_t Vial;
	static int32_t zPos;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ane != Menu) {
    dfp("subMenu_ane=%d\n",subMenu_ane);
    Menu = subMenu_ane;
}
#endif

  switch(subMenu_ane) {
		case 1:
			if( devSet.adjustAct == ADJUST_ACT_NOT_READY) {		
				++subMenu_ane;	
			}
			else subMenu_ane = 3;
			break;
		case 2:
			if(initHomePos_All())	{
				devSet.adjustAct = ADJUST_ACT_READY;
				++subMenu_ane;			
			}
			break;
		case 3:
			goZ(POS_Z_TOP,ACT_Z_POS);
			++subMenu_ane;
			break;
		case 4:
			if(readyChk(MOTOR_Z))	{
				goX(POS_XY_EXCHANGE, TRAY_ETC, ACT_XY_POS);
				goY(POS_XY_EXCHANGE, TRAY_ETC, ACT_XY_POS);
				++subMenu_ane;
			}
			break;
		case 5:
      if(readyChk(MOTOR_X) && readyChk(MOTOR_Y))	{
        goZ(POS_Z_NEEDLE_EXCHANGE, ACT_Z_POS);
				++subMenu_ane;
      }
			break;
		case 6:
			if(readyChk(MOTOR_Z))	{
				subMenu_ane = 1;
				return ACTION_MENU_END;
			}
			break;

  }
	
	return ACTION_MENU_ING;
	
}

// =================================================================================
// =================================================================================
int adjustSyringeExchange()
{
	VIAL_POS_t Vial;
	static int32_t zPos;

#if DEBUG_MOTORCTRL_SUBMENU_NO
static uint32_t Menu = 0;
if(subMenu_ase != Menu) {
    dfp("subMenu_ase=%d\n",subMenu_ase);
    Menu = subMenu_ase;
}
#endif

  switch(subMenu_ase) {
		case 1:
			if( devSet.adjustAct == ADJUST_ACT_NOT_READY) {		
				++subMenu_ase;	
			}
			else subMenu_ase = 3;
			break;
		case 2:
			if(initHomePos_All())	{
				devSet.adjustAct = ADJUST_ACT_READY;
				++subMenu_ase;			
			}
			break;
		case 3:
			goZ(POS_Z_TOP,ACT_Z_POS);
			++subMenu_ase;
			break;
		case 4:
			if(sleepChkVal(MOTOR_VAL)) {
				motorReadyCmd(MOTOR_VAL);
			}
			++subMenu_ase;
			break;
		case 5:			
			if(readyChkVal(MOTOR_VAL)) {
				goVal(POS_VAL_WASHING, 0,ACT_SPEED_MAX_VAL, ACT_XY_POS);
				++subMenu_ase;
			}
			break;
		case 6:
			goSyr((float)(devSet.syrExchangeHeight), 0, WASH_SPEED_UP_SYR, ACT_XY_POS);
			++subMenu_ase;
			break;
		case 7:
			if(readyChk(MOTOR_SYR))	{
				subMenu_ase = 1;
				return ACTION_MENU_END;
			}
			break;
  }
	
	return ACTION_MENU_ING;
	
}
// ====================================================================
//  ������ 
// Pulse������  x,y,z,syr : 16pulse
// 
int initCalibration()
{

}

//================================================================================================
// 1. Z�� HOME
// 2. Y�� Home
// 3. X�� Home
int adjustment_X()
{
}

// X coordinate

int initCoordinate()
{
	xyzCoord.etcStep_X[POS_XY_INJECTOR] 	= (int)(COORD_X_INJECTOR / MM_PER_STEP_X + 0.5) * (int)(MICRO_STEP_X);
		xyzCoord.etcStep_X[POS_XY_INJECTOR] +=	devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] * ADJUST_STEP_UNIT_X;
		
	xyzCoord.etcStep_X[POS_XY_WASTE] 		= (int)(COORD_X_WASTE / MM_PER_STEP_X + 0.5) * (int)(MICRO_STEP_X);
		xyzCoord.etcStep_X[POS_XY_WASTE] += devSet.adjust.waste_x * ADJUST_STEP_UNIT_X;

	xyzCoord.etcStep_X[POS_XY_WASH] 			= (int)(COORD_X_WASH / MM_PER_STEP_X + 0.5) * (int)(MICRO_STEP_X);
		xyzCoord.etcStep_X[POS_XY_WASH] += devSet.adjust.wash_x * ADJUST_STEP_UNIT_X;
		
	xyzCoord.etcStep_X[POS_XY_EXCHANGE] 			= (int)(COORD_X_EXCHANGE / MM_PER_STEP_X + 0.5) * (int)(MICRO_STEP_X);
		xyzCoord.etcStep_X[POS_XY_EXCHANGE] += devSet.adjust.value[ADJUST_INJ_PORT_VALUE_X] * ADJUST_STEP_UNIT_X;
	
	xyzCoord.etcStep_X[POS_XY_HOME] 			= (int)(COORD_X_HOME / MM_PER_STEP_X + 0.5) * (int)(MICRO_STEP_X);
	
	xyzCoord.etcStep_Y[POS_XY_INJECTOR] 	= (int)(COORD_Y_INJECTOR / MM_PER_STEP_Y + 0.5) * (int)(MICRO_STEP_Y);
		xyzCoord.etcStep_Y[POS_XY_INJECTOR] += devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] * ADJUST_STEP_UNIT_Y;
		
	xyzCoord.etcStep_Y[POS_XY_WASTE] 		= (int)(COORD_Y_WASTE / MM_PER_STEP_Y + 0.5) * (int)(MICRO_STEP_Y);
		xyzCoord.etcStep_Y[POS_XY_WASTE] += devSet.adjust.waste_y * ADJUST_STEP_UNIT_Y;	
		
	xyzCoord.etcStep_Y[POS_XY_WASH] 			= (int)(COORD_Y_WASH / MM_PER_STEP_Y + 0.5) * (int)(MICRO_STEP_Y);
		xyzCoord.etcStep_Y[POS_XY_WASH] += devSet.adjust.wash_y * ADJUST_STEP_UNIT_Y;	
		
	xyzCoord.etcStep_Y[POS_XY_EXCHANGE] 			= (int)(COORD_Y_EXCHANGE / MM_PER_STEP_Y + 0.5) * (int)(MICRO_STEP_Y);
		xyzCoord.etcStep_Y[POS_XY_EXCHANGE] += devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Y] * ADJUST_STEP_UNIT_Y;
		
	xyzCoord.etcStep_Y[POS_XY_HOME] 			= (int)(COORD_Y_HOME / MM_PER_STEP_Y + 0.5) * (int)(MICRO_STEP_Y);

int AdjustInjectionStep_Z;	// injection ���� ��ġ
	AdjustInjectionStep_Z = devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] * ADJUST_STEP_UNIT_Z;

int AdjustVialHeight_Z;
	AdjustVialHeight_Z = devSet.adjust.vialHeight * ADJUST_STEP_UNIT_Z;
// Z���� ���⼭ adjust���� �����Ѵ�.
	xyzCoord.step_Z[POS_Z_TOP] 			  = (int)(COORD_Z_TOP 		/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);

												// AdjustVialHeight_Z
													xyzCoord.step_Z[POS_Z_BOTTOM] 		= (int)(COORD_Z_BOTTOM 	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
														xyzCoord.step_Z[POS_Z_BOTTOM] += AdjustVialHeight_Z;

// AdjustInjectionStep_Z
	xyzCoord.step_Z[POS_Z_INJECTION] 	= (int)(COORD_Z_INJECTION/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_INJECTION] +=	AdjustInjectionStep_Z;

#if VIAL_BOTTOM_CAL_FIX
	xyzCoord.step_Z[POS_Z_WASTE] 		  = (int)(COORD_Z_WASTE 	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_WASTE] +=	AdjustVialHeight_Z;
	xyzCoord.step_Z[POS_Z_WASH] 			= (int)(COORD_Z_WASH 		/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_WASH] +=	AdjustVialHeight_Z;
#else		
	xyzCoord.step_Z[POS_Z_WASTE] 		  = (int)(COORD_Z_WASTE 	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_WASTE] +=	AdjustInjectionStep_Z;
	xyzCoord.step_Z[POS_Z_WASH] 			= (int)(COORD_Z_WASH 		/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_WASH] +=	AdjustInjectionStep_Z;
#endif

// AdjustVialHeight_Z
	xyzCoord.step_Z[POS_Z_WASH_ADJUST] 		= (int)(COORD_Z_WASH_ADJUST 		/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_WASH_ADJUST] +=	AdjustVialHeight_Z;		

	xyzCoord.step_Z[POS_Z_VIAL_HEIGHT_ADJUST] = (int)(COORD_Z_INJECTOR_TOP / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
			xyzCoord.step_Z[POS_Z_VIAL_HEIGHT_ADJUST] +=	AdjustVialHeight_Z;

#if MECH_VER == 7
// Needle adjust position
	xyzCoord.step_Z[POS_Z_NEEDLE_DEPTH_ADJUST] = (int)(COORD_Z_NEEDLE_DEPTH_ADJUST / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_NEEDLE_DEPTH_ADJUST] +=	AdjustInjectionStep_Z;				

	#if VIAL_BOTTOM_CAL_FIX
		xyzCoord.step_Z[POS_Z_INJECTOR_TOP] = (int)(COORD_Z_INJECTOR_TOP / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
			xyzCoord.step_Z[POS_Z_INJECTOR_TOP] +=	AdjustVialHeight_Z;				
	#else
		xyzCoord.step_Z[POS_Z_INJECTOR_TOP] = (int)(COORD_Z_INJECTOR_TOP / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
			xyzCoord.step_Z[POS_Z_INJECTOR_TOP] +=	AdjustInjectionStep_Z;				
	#endif
#else
	xyzCoord.step_Z[POS_Z_INJECTOR_TOP] = (int)(COORD_Z_INJECTOR_TOP / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_INJECTOR_TOP] +=	AdjustInjectionStep_Z;				
#endif
	xyzCoord.step_Z[POS_Z_INJECTOR_TOP_ADJUST] = (int)(COORD_Z_INJECTOR_TOP_ADJUST / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_INJECTOR_TOP_ADJUST] +=	AdjustInjectionStep_Z;

// AdjustVialHeight_Z		
	xyzCoord.step_Z[POS_Z_WASH_TOP] = (int)(COORD_Z_WASH_TOP / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_WASH_TOP] += AdjustVialHeight_Z;				

	xyzCoord.step_Z[POS_Z_NEEDLE_EXCHANGE] = (int)(COORD_Z_NEEDLE_EXCHANGE / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_NEEDLE_EXCHANGE] += AdjustVialHeight_Z;

	xyzCoord.step_Z[POS_Z_SLEEP] = (int)(COORD_Z_SLEEP_STEP) * (int)(MICRO_STEP_Z);

int portDetectDepthStep;

	portDetectDepthStep = (int)(INJ_PORT_DETECT_DEPTH / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
	xyzCoord.step_Z[POS_Z_INJECTION_PORT_DETECT] = xyzCoord.step_Z[POS_Z_INJECTOR_TOP] + portDetectDepthStep;

// AdjustVialHeight_Z
	#if MECH_VER == 6 || MECH_VER == 7				
		xyzCoord.step_Z[POS_Z_INJECTION_PORT_UPPER_DETECT] = (int)((COORD_Z_INJECTOR_TOP - 1) / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
			xyzCoord.step_Z[POS_Z_INJECTION_PORT_UPPER_DETECT] += AdjustVialHeight_Z;				
	#else
		xyzCoord.step_Z[POS_Z_INJECTION_PORT_UPPER_DETECT] = xyzCoord.step_Z[POS_Z_INJECTOR_TOP];
	#endif
	
	portDetectDepthStep = (int)(WASH_PORT_DETECT_DEPTH / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);	
	xyzCoord.step_Z[POS_Z_WASH_DETECT] = xyzCoord.step_Z[POS_Z_WASH_TOP] + portDetectDepthStep;
	
xyzCoord.step_Z[POS_Z_WASH_UPPER_DETECT] = xyzCoord.step_Z[POS_Z_WASH_TOP];	

	portDetectDepthStep = (int)(WASTE_PORT_DETECT_DEPTH / MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);		
	xyzCoord.step_Z[POS_Z_WASTE_DETECT] = xyzCoord.step_Z[POS_Z_WASH_TOP] + portDetectDepthStep;

xyzCoord.step_Z[POS_Z_WASTE_UPPER_DETECT] = xyzCoord.step_Z[POS_Z_WASH_TOP];	

	xyzCoord.valStep[POS_VAL_NEEDLE] = 0;	// 0 * (int)(MICRO_STEP_VAL);	// home
	xyzCoord.valStep[POS_VAL_WASHING] = UNIT_STEP_VAL * (int)(MICRO_STEP_VAL);		// 90�� 

	xyzCoord.injStep[POS_INJ_LOAD] = 0;	// 0 * (int)(MICRO_STEP_INJ);	// home
	xyzCoord.injStep[POS_INJ_INJECT] = (int)(UNIT_STEP_INJ * (int)(MICRO_STEP_INJ));

 initVialCoordinate(AdjustInjectionStep_Z,AdjustVialHeight_Z);
}

int initVialCoordinate(int AdjustStep_Z, int AdjustVial_Z)
{
	int count_X,count_Y;
	float R_start_X,L_start_X,start_Y;
	float interval_X,interval_Y;
	float	vial_top, vial_adjust_z, vial_detect_z, vial_bottom;
//	int adjust_value_xy;
	
	int i;	
int adjust_value_x, adjust_value_y;
int AdjustVialHeight_x, AdjustVialHeight_y;

float vial_adjust_length_mm_x,vial_adjust_length_mm_y;
	
// �ʱ�ȭ 
	for(i = 0 ; i < TRAY_COUNT_MAX_X ; i++) {
		xyzCoord.vialStep_L_X[i] = 0;
		xyzCoord.vialStep_R_X[i] = 0;
	}
	for(i = 0 ; i < TRAY_COUNT_MAX_Y ; i++) {
		xyzCoord.vialStep_L_Y[i] = 0;
		xyzCoord.vialStep_R_Y[i] = 0;
	}

	switch(sysConfig.trayLeft) {	// Left , Back
		case TRAY_VIALS_40:
			count_X = TRAY40_COUNT_X;
			count_Y = TRAY40_COUNT_Y;
			R_start_X = TRAY40_RIGHT_START_X;
			L_start_X = TRAY40_LEFT_START_X;
			start_Y = TRAY40_START_Y;
			interval_X = TRAY40_INTERVAL_X;
			interval_Y = TRAY40_INTERVAL_Y;

			vial_top = COORD_Z_VIAL_40_TOP;
			vial_adjust_z = COORD_Z_VIAL_40_ADJUST;
			vial_detect_z = COORD_Z_VIAL_40_DETECT;

#if VIAL_POS_MODIFY
			vial_bottom = COORD_Z_VIAL_40_BOTTOM;
#endif		
			break;
		case TRAY_VIALS_60:
			count_X = TRAY60_COUNT_X;
			count_Y = TRAY60_COUNT_Y;
			R_start_X = TRAY60_RIGHT_START_X;
			L_start_X = TRAY60_LEFT_START_X;
			start_Y = TRAY60_START_Y;
			interval_X = TRAY60_INTERVAL_X;
			interval_Y = TRAY60_INTERVAL_Y;	

			vial_top = COORD_Z_VIAL_60_TOP;
			vial_adjust_z = COORD_Z_VIAL_60_ADJUST;
			vial_detect_z = COORD_Z_VIAL_60_DETECT;

#if VIAL_POS_MODIFY
			vial_bottom = COORD_Z_VIAL_60_BOTTOM;
#endif	
			break;
		case WELLPALTE_96:
			count_X = WELLPALTE_96_COUNT_X;	// 1-12
			count_Y = WELLPALTE_96_COUNT_Y;	// a-h
			R_start_X = TRAY96_RIGHT_START_X;
			L_start_X = TRAY96_LEFT_START_X;
//			start_Y = TRAY96_START_Y;	

			start_Y = TRAY96_BACK_START_Y;

			interval_X = TRAY96_INTERVAL_X;
			interval_Y = TRAY96_INTERVAL_Y;		

			vial_top = COORD_Z_WELLPALTE_96_TOP;
			vial_adjust_z = COORD_Z_WELLPALTE_96_ADJUST;
			vial_detect_z = COORD_Z_WELLPALTE_96_DETECT;

#if VIAL_POS_MODIFY
			vial_bottom = COORD_Z_WELLPALTE_96_BOTTOM;
#endif
			break;
		default:
			count_X = TRAY60_COUNT_X;
			count_Y = TRAY60_COUNT_Y;
			R_start_X = TRAY60_RIGHT_START_X;
			L_start_X = TRAY60_LEFT_START_X;
			start_Y = TRAY60_START_Y;
			interval_X = TRAY60_INTERVAL_X;
			interval_Y = TRAY60_INTERVAL_Y;	

			vial_top = COORD_Z_VIAL_60_TOP;
			vial_adjust_z = COORD_Z_VIAL_60_ADJUST;
			vial_detect_z = COORD_Z_VIAL_60_DETECT;
			
#if VIAL_POS_MODIFY
			vial_bottom = COORD_Z_VIAL_60_BOTTOM;
#endif			
			break;
	}

// AdjustVial_Z		
	xyzCoord.step_Z[POS_Z_VIAL_LEFT_TOP] =		 (int)(vial_top	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_VIAL_LEFT_TOP] += AdjustVial_Z;				
	xyzCoord.step_Z[POS_Z_VIAL_LEFT_ADJUST] =		 (int)(vial_adjust_z	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_VIAL_LEFT_ADJUST] += AdjustVial_Z;				
	xyzCoord.step_Z[POS_Z_VIAL_LEFT_DETECT] =	 (int)(vial_detect_z	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_VIAL_LEFT_DETECT] += AdjustVial_Z;			

	#if	MECH_VER == 6 || MECH_VER == 7
		xyzCoord.step_Z[POS_Z_VIAL_LEFT_UPPER_DETECT] =		 (int)((vial_top-1)	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
			xyzCoord.step_Z[POS_Z_VIAL_LEFT_UPPER_DETECT] += AdjustVial_Z;				
	#else
		xyzCoord.step_Z[POS_Z_VIAL_LEFT_UPPER_DETECT] = xyzCoord.step_Z[POS_Z_VIAL_LEFT_TOP];
	#endif

#if VIAL_POS_MODIFY
	adjust_value_x = devSet.adjust.leftVial_x[sysConfig.trayLeft] * (int)(ADJUST_STEP_UNIT_X);
	adjust_value_y = devSet.adjust.leftVial_y[sysConfig.trayLeft] * (int)(ADJUST_STEP_UNIT_Y);
	AdjustVialHeight_x = devSet.adjust.vialHeight_x * ADJUST_STEP_UNIT_X;
	AdjustVialHeight_y = devSet.adjust.vialHeight_y * ADJUST_STEP_UNIT_Y;

/*
			vial_adjust_length_x = interval_X * (count_X - 1); 
			vial_adjust_length_x = AdjustVialHeight_x / (count_X - 1);
			vial_adjust_length_y = interval_Y * (count_Y - 1);
			vial_adjust_length_y = AdjustVialHeight_y / (count_X - 1);			
*/
	vial_adjust_length_mm_x = AdjustVialHeight_x / MICRO_STEP_X * MM_PER_STEP_X / (count_X - 1);
	vial_adjust_length_mm_y = AdjustVialHeight_y / MICRO_STEP_Y * MM_PER_STEP_Y / (count_Y - 1);			
// ===== Left Y
	if(sysConfig.trayLeft == TRAY_VIALS_40 || sysConfig.trayLeft == TRAY_VIALS_60) {
			for(i = 0 ; i < count_Y; i++) {
				xyzCoord.vialStep_L_Y[count_Y - i - 1] = 
					((int)(((start_Y + interval_Y * i) / MM_PER_STEP_Y) * 2 + 0.5) * (int)(MICRO_STEP_Y)) / 2
					+ adjust_value_y;
//dprintf("xyzCoord.vialStep_L_Y[%d] = %d \n",count_Y - i - 1,xyzCoord.vialStep_L_Y[count_Y - i - 1]);

				xyzCoord.vialStep_L_Y[count_Y - i - 1] = 
					((int)(((start_Y + (interval_Y * i) + vial_adjust_length_mm_y * (count_Y - i - 1)) / MM_PER_STEP_Y) * 2 + 0.5) * (int)(MICRO_STEP_Y)) / 2					
					+ adjust_value_y;
//dprintf("xyzCoord.vialStep_L_Y[%d] = %d , %f %f \n",count_Y - i - 1,xyzCoord.vialStep_L_Y[count_Y - i - 1],vial_adjust_length_mm_x,vial_adjust_length_mm_y*i);
			}
	}
	else {
			for(i = 0 ; i < count_Y; i++) {
				xyzCoord.vialStep_L_Y[i] = 
					((int)(((start_Y + interval_Y * i) / MM_PER_STEP_Y) * 2 + 0.5) * (int)(MICRO_STEP_Y)) / 2
					+ adjust_value_y;
//dfp("xyzCoord.vialStep_L_Y[%d] = %d\n",i,xyzCoord.vialStep_L_Y[i]);
			}
	}
// ===== Left X
	if(sysConfig.trayLeft < TRAY_NONE) {
//	dfp("count_X = %d | count_Y = %d\n",count_X,count_Y);	
		for(i = 0 ; i < count_X; i++) {
			xyzCoord.vialStep_L_X[count_X - i - 1] = 
				((int)(((L_start_X + interval_X * i) / MM_PER_STEP_X) * 2 + 0.5) * (int)(MICRO_STEP_X)) / 2
				+ adjust_value_x;
//dfp("xyzCoord.vialStep_L_X[%d] = %d\n",count_X - i - 1,xyzCoord.vialStep_L_X[count_X - i - 1]);

			xyzCoord.vialStep_L_X[count_X - i - 1] = 
				((int)(((L_start_X + (interval_X * i) + vial_adjust_length_mm_x * (count_X - i - 1)) / MM_PER_STEP_X) * 2 + 0.5) * (int)(MICRO_STEP_X)) / 2
				+ adjust_value_x;
//dprintf("xyzCoord.vialStep_L_X[%d] = %d %f \n",count_X - i - 1,xyzCoord.vialStep_L_X[count_X - i - 1],vial_adjust_length_mm_x);
		}
	}
#else	
	if(sysConfig.trayLeft == TRAY_VIALS_40 || sysConfig.trayLeft == TRAY_VIALS_60) {
			for(i = 0 ; i < count_Y; i++) {
				xyzCoord.vialStep_L_Y[count_Y - i - 1] = 
					(int)((start_Y + interval_Y * i) / MM_PER_STEP_Y + 0.5) * (int)(MICRO_STEP_Y)
					+ adjust_value_y;
		//dfp("xyzCoord.vialStep_L_Y[%d] = %d\n",count_Y - i - 1,xyzCoord.vialStep_L_Y[count_Y - i - 1]);
			}
	}
	else {
			for(i = 0 ; i < count_Y; i++) {
				xyzCoord.vialStep_L_Y[i] = 
					(int)((start_Y + interval_Y * i) / MM_PER_STEP_Y + 0.5) * (int)(MICRO_STEP_Y)
					+ adjust_value_y;
		//dfp("xyzCoord.vialStep_L_Y[%d] = %d\n",i,xyzCoord.vialStep_L_Y[i]);
			}
	}

	if(sysConfig.trayLeft < TRAY_NONE) {
//	dfp("count_X = %d | count_Y = %d\n",count_X,count_Y);	
		for(i = 0 ; i < count_X; i++) {
			xyzCoord.vialStep_L_X[count_X - i - 1] = 
				(int)((L_start_X + interval_X * i) / MM_PER_STEP_X + 0.5) * (int)(MICRO_STEP_X)
				+ adjust_value_x;
//dfp("xyzCoord.vialStep_L_X[%d] = %d\n",count_X - i - 1,xyzCoord.vialStep_L_X[count_X - i - 1]);
		}
	}
#endif

//			devSet.left_z_bottom = COORD_Z_WELLPALTE_96_BOTTOM + (devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] * (ADJUST_STEP_UNIT_Z / MICRO_STEP_Z) * MM_PER_STEP_Z);
#if VIAL_BOTTOM_CAL_FIX
			devSet.left_z_bottom = vial_bottom + AdjustVial_Z / MICRO_STEP_Z * MM_PER_STEP_Z;
#else
			devSet.left_z_bottom = vial_bottom + AdjustStep_Z / MICRO_STEP_Z * MM_PER_STEP_Z;
#endif

//===========================================================================================================
//===========================================================================================================
	switch(sysConfig.trayRight) {	// Right , Front
		case TRAY_VIALS_40:
			count_X = TRAY40_COUNT_X;
			count_Y = TRAY40_COUNT_Y;
			R_start_X = TRAY40_RIGHT_START_X;
			L_start_X = TRAY40_LEFT_START_X;
			start_Y = TRAY40_START_Y;
			interval_X = TRAY40_INTERVAL_X;
			interval_Y = TRAY40_INTERVAL_Y;

			vial_top = COORD_Z_VIAL_40_TOP;
			vial_adjust_z = COORD_Z_VIAL_40_ADJUST;
			vial_detect_z = COORD_Z_VIAL_40_DETECT;

#if VIAL_POS_MODIFY
			vial_bottom = COORD_Z_VIAL_40_BOTTOM;
#endif
			break;
		case TRAY_VIALS_60:
			count_X = TRAY60_COUNT_X;
			count_Y = TRAY60_COUNT_Y;
			R_start_X = TRAY60_RIGHT_START_X;
			L_start_X = TRAY60_LEFT_START_X;
			start_Y = TRAY60_START_Y;
			interval_X = TRAY60_INTERVAL_X;
			interval_Y = TRAY60_INTERVAL_Y;
			
			vial_top = COORD_Z_VIAL_60_TOP;
			vial_adjust_z = COORD_Z_VIAL_60_ADJUST;
			vial_detect_z = COORD_Z_VIAL_60_DETECT;

#if VIAL_POS_MODIFY
			vial_bottom = COORD_Z_VIAL_60_BOTTOM;
#endif
			break;
		case WELLPALTE_96:
			count_X = WELLPALTE_96_COUNT_X;	//TRAY96_COUNT_X;
			count_Y = WELLPALTE_96_COUNT_Y;	//TRAY96_COUNT_Y;
			R_start_X = TRAY96_RIGHT_START_X;
			L_start_X = TRAY96_LEFT_START_X;
//			start_Y = TRAY96_START_Y;
			start_Y = TRAY96_FRONT_START_Y;			

			interval_X = TRAY96_INTERVAL_X;
			interval_Y = TRAY96_INTERVAL_Y;		

			vial_top = COORD_Z_WELLPALTE_96_TOP;
			vial_adjust_z = COORD_Z_WELLPALTE_96_ADJUST;
			vial_detect_z = COORD_Z_WELLPALTE_96_DETECT;

#if VIAL_POS_MODIFY
			vial_bottom = COORD_Z_WELLPALTE_96_BOTTOM;
#endif
			break;
		default:
			count_X = TRAY60_COUNT_X;
			count_Y = TRAY60_COUNT_Y;
			R_start_X = TRAY60_RIGHT_START_X;
			L_start_X = TRAY60_LEFT_START_X;
			start_Y = TRAY60_START_Y;
			interval_X = TRAY60_INTERVAL_X;
			interval_Y = TRAY60_INTERVAL_Y;
			
			vial_top = COORD_Z_VIAL_60_TOP;
			vial_adjust_z = COORD_Z_VIAL_60_ADJUST;
			vial_detect_z = COORD_Z_VIAL_60_DETECT;

#if VIAL_POS_MODIFY
			vial_bottom = COORD_Z_VIAL_60_BOTTOM;
#endif			
			break;			
	}

// AdjustVial_Z		
	xyzCoord.step_Z[POS_Z_VIAL_RIGHT_TOP] =		 (int)(vial_top	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_VIAL_RIGHT_TOP] += AdjustVial_Z;
	xyzCoord.step_Z[POS_Z_VIAL_RIGHT_ADJUST] =		 (int)(vial_adjust_z	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_VIAL_RIGHT_ADJUST] += AdjustVial_Z;
	xyzCoord.step_Z[POS_Z_VIAL_RIGHT_DETECT] =	 (int)(vial_detect_z	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
		xyzCoord.step_Z[POS_Z_VIAL_RIGHT_DETECT] += AdjustVial_Z;

	#if	MECH_VER == 6 || MECH_VER == 7
		xyzCoord.step_Z[POS_Z_VIAL_RIGHT_UPPER_DETECT] =		 (int)((vial_top-1)	/ MM_PER_STEP_Z + 0.5) * (int)(MICRO_STEP_Z);
			xyzCoord.step_Z[POS_Z_VIAL_RIGHT_UPPER_DETECT] += AdjustVial_Z;				
	#else
		xyzCoord.step_Z[POS_Z_VIAL_RIGHT_UPPER_DETECT] = xyzCoord.step_Z[POS_Z_VIAL_RIGHT_TOP];	
	#endif

#if VIAL_POS_MODIFY
	adjust_value_x = devSet.adjust.rightVial_x[sysConfig.trayRight] * (int)(ADJUST_STEP_UNIT_X);
	adjust_value_y = devSet.adjust.rightVial_y[sysConfig.trayRight] * (int)(ADJUST_STEP_UNIT_Y);
// === right Y
	if(sysConfig.trayRight == TRAY_VIALS_40 || sysConfig.trayRight == TRAY_VIALS_60) {
			for(i = 0 ; i < count_Y; i++) {		// Y : 0���� ���� -- 0.5 step ���� ���� : 0.9�� ���� : 4 pulse
				xyzCoord.vialStep_R_Y[count_Y - i - 1 ] = 
					((int)(((start_Y + interval_Y * i) / MM_PER_STEP_Y) * 2 + 0.5) * (int)(MICRO_STEP_Y)) / 2
					+ adjust_value_y;
//dprintf("xyzCoord.vialStep_R_Y[%d] = %d start_Y=%f interval_Y=%f \n",count_Y - i - 1,xyzCoord.vialStep_R_Y[count_Y - i - 1],start_Y,interval_Y);

				xyzCoord.vialStep_R_Y[count_Y - i - 1] = 
					((int)(((start_Y + (interval_Y * i) + vial_adjust_length_mm_y * (count_Y - i - 1)) / MM_PER_STEP_Y) * 2 + 0.5) * (int)(MICRO_STEP_Y)) / 2					
					+ adjust_value_y;
//dprintf("xyzCoord.vialStep_R_Y[%d] = %d , %f %f \n",count_Y - i - 1,xyzCoord.vialStep_R_Y[count_Y - i - 1],vial_adjust_length_mm_x,vial_adjust_length_mm_y*i);
			}
	}
	else {
			for(i = 0 ; i < count_Y; i++) {		// Y : 0���� ���� 
				xyzCoord.vialStep_R_Y[i] = 
					((int)(((start_Y + interval_Y * i) / MM_PER_STEP_Y) * 2 + 0.5) * (int)(MICRO_STEP_Y)) / 2
					+ adjust_value_y;
		//dfp("xyzCoord.vialStep_R_Y[%d] = %d\n",i,xyzCoord.vialStep_R_Y[i]);
			}
	}
// === right X	
	if(sysConfig.trayRight < TRAY_NONE) {
//dfp("count_X = %d | count_Y = %d\n",count_X,count_Y);		
		for(i = 0 ; i < count_X; i++) {		// X : 0���� ���� 
			xyzCoord.vialStep_R_X[count_X - i - 1] = 
				((int)(((R_start_X + interval_X * i) / MM_PER_STEP_X) * 2 + 0.5) * (int)(MICRO_STEP_X)) / 2
				+ adjust_value_x;
//dfp("xyzCoord.vialStep_R_X[%d] = %d\n",count_X - i - 1,xyzCoord.vialStep_R_X[count_X - i - 1]);

			xyzCoord.vialStep_R_X[count_X - i - 1] = 
				((int)(((R_start_X + interval_X * i + vial_adjust_length_mm_x * (count_X - i - 1)) / MM_PER_STEP_X) * 2 + 0.5) * (int)(MICRO_STEP_X)) / 2
				+ adjust_value_x;
//dfp("xyzCoord.vialStep_R_X[%d] = %d\n",count_X - i - 1,xyzCoord.vialStep_R_X[count_X - i - 1]);

		}
	}
	
#else
	if(sysConfig.trayRight == TRAY_VIALS_40 || sysConfig.trayRight == TRAY_VIALS_60) {
			for(i = 0 ; i < count_Y; i++) {		// Y : 0���� ���� 
				xyzCoord.vialStep_R_Y[count_Y - i - 1 ] = 
					(int)((start_Y + interval_Y * i) / MM_PER_STEP_Y + 0.5) * (int)(MICRO_STEP_Y)
					+ adjust_value_y;
//dfp("xyzCoord.vialStep_R_Y[%d] = %d\n",count_Y - i - 1,xyzCoord.vialStep_R_Y[count_Y - i - 1]);
			}
	}
	else {
			for(i = 0 ; i < count_Y; i++) {		// Y : 0���� ���� 
				xyzCoord.vialStep_R_Y[i] = 
					(int)((start_Y + interval_Y * i) / MM_PER_STEP_Y + 0.5) * (int)(MICRO_STEP_Y)
					+ adjust_value_y;
		//dfp("xyzCoord.vialStep_R_Y[%d] = %d\n",i,xyzCoord.vialStep_R_Y[i]);
			}
	}
		
	if(sysConfig.trayRight < TRAY_NONE) {
//dfp("count_X = %d | count_Y = %d\n",count_X,count_Y);		
		for(i = 0 ; i < count_X; i++) {		// X : 0���� ���� 
			xyzCoord.vialStep_R_X[count_X - i - 1] = 
				(int)((R_start_X + interval_X * i) / MM_PER_STEP_X + 0.5) * (int)(MICRO_STEP_X)
				+ adjust_value_x;
//dfp("xyzCoord.vialStep_R_X[%d] = %d\n",count_X - i - 1,xyzCoord.vialStep_R_X[count_X - i - 1]);
		}
	}
#endif

//			devSet.right_z_bottom = COORD_Z_VIAL_60_BOTTOM + (devSet.adjust.value[ADJUST_INJ_PORT_VALUE_Z] * (ADJUST_STEP_UNIT_Z / MICRO_STEP_Z) * MM_PER_STEP_Z);	
#if VIAL_BOTTOM_CAL_FIX
			devSet.right_z_bottom = vial_bottom + AdjustVial_Z / MICRO_STEP_Z * MM_PER_STEP_Z;
#else
			devSet.right_z_bottom = vial_bottom + AdjustStep_Z / MICRO_STEP_Z * MM_PER_STEP_Z;
#endif
}

void initStepCtrlValue()
{

//#if BOARD_TEST_PRGM
#if BOARD_TEST_PRGM || MACHINE_TEST_PRGM
	initMotorCtrlSet();
#endif

#if FPGA_VERSION_TRIM
	devSet.baseAddr[MOTOR_X] = BASEADDR_MOTOR_X_Z;
	devSet.baseAddr[MOTOR_Y] = BASEADDR_MOTOR_Y_SYR;
	devSet.baseAddr[MOTOR_Z] = BASEADDR_MOTOR_X_Z;
	devSet.baseAddr[MOTOR_SYR] = BASEADDR_MOTOR_Y_SYR;
	devSet.baseAddr[MOTOR_VAL] = BASEADDR_MOTOR_VAL_INJ;
	devSet.baseAddr[MOTOR_INJ] = BASEADDR_MOTOR_VAL_INJ;

	devSet.accelStepAddr[MOTOR_X] = 	BASEADDR_MOTOR_X_Z 		+ ACCEL_OFFSET;
	devSet.accelStepAddr[MOTOR_Y] = 	BASEADDR_MOTOR_Y_SYR 	+ ACCEL_OFFSET;
	devSet.accelStepAddr[MOTOR_Z] = 	BASEADDR_MOTOR_X_Z 		+ ACCEL_OFFSET;
	devSet.accelStepAddr[MOTOR_SYR] = BASEADDR_MOTOR_Y_SYR 	+ ACCEL_OFFSET;
	devSet.accelStepAddr[MOTOR_VAL] = BASEADDR_MOTOR_VAL_INJ + ACCEL_OFFSET;
	devSet.accelStepAddr[MOTOR_INJ] = BASEADDR_MOTOR_VAL_INJ + ACCEL_OFFSET;

	devSet.deaccStepAddr[MOTOR_X] = 	BASEADDR_MOTOR_X_Z 		+ DEACC_OFFSET;
	devSet.deaccStepAddr[MOTOR_Y] = 	BASEADDR_MOTOR_Y_SYR 	+ DEACC_OFFSET;
	devSet.deaccStepAddr[MOTOR_Z] = 	BASEADDR_MOTOR_X_Z 		+ DEACC_OFFSET;
	devSet.deaccStepAddr[MOTOR_SYR] = BASEADDR_MOTOR_Y_SYR 	+ DEACC_OFFSET;
	devSet.deaccStepAddr[MOTOR_VAL] = BASEADDR_MOTOR_VAL_INJ + DEACC_OFFSET;
	devSet.deaccStepAddr[MOTOR_INJ] = BASEADDR_MOTOR_VAL_INJ + DEACC_OFFSET;

	devSet.totalStepAddr[MOTOR_X] = 	BASEADDR_MOTOR_X_Z 		+ TOTAL_OFFSET;
	devSet.totalStepAddr[MOTOR_Y] = 	BASEADDR_MOTOR_Y_SYR 	+ TOTAL_OFFSET;
	devSet.totalStepAddr[MOTOR_Z] = 	BASEADDR_MOTOR_X_Z 		+ TOTAL_OFFSET;
	devSet.totalStepAddr[MOTOR_SYR] = BASEADDR_MOTOR_Y_SYR 	+ TOTAL_OFFSET;
	devSet.totalStepAddr[MOTOR_VAL] = BASEADDR_MOTOR_VAL_INJ + TOTAL_OFFSET;
	devSet.totalStepAddr[MOTOR_INJ] = BASEADDR_MOTOR_VAL_INJ + TOTAL_OFFSET;

	devSet.startSpeedAddr[MOTOR_X] = 	BASEADDR_MOTOR_X_Z 		+ START_SPEED;
	devSet.startSpeedAddr[MOTOR_Y] = 	BASEADDR_MOTOR_Y_SYR 	+ START_SPEED;
	devSet.startSpeedAddr[MOTOR_Z] = 	BASEADDR_MOTOR_X_Z 		+ START_SPEED;
	devSet.startSpeedAddr[MOTOR_SYR] = BASEADDR_MOTOR_Y_SYR + START_SPEED;
	devSet.startSpeedAddr[MOTOR_VAL] = BASEADDR_MOTOR_VAL_INJ + START_SPEED;
	devSet.startSpeedAddr[MOTOR_INJ] = BASEADDR_MOTOR_VAL_INJ + START_SPEED;

	devSet.ctrlPortAddr[MOTOR_X] = CTRL_PORT_ADDR_MOTOR_X_Z;
	devSet.ctrlPortAddr[MOTOR_Y] = CTRL_PORT_ADDR_MOTOR_Y_SYR;
	devSet.ctrlPortAddr[MOTOR_Z] = CTRL_PORT_ADDR_MOTOR_X_Z;
	devSet.ctrlPortAddr[MOTOR_SYR] = CTRL_PORT_ADDR_MOTOR_Y_SYR;
	devSet.ctrlPortAddr[MOTOR_VAL] = CTRL_PORT_ADDR_MOTOR_VAL_INJ;
	devSet.ctrlPortAddr[MOTOR_INJ] = CTRL_PORT_ADDR_MOTOR_VAL_INJ;

	devSet.selMotorAddr[MOTOR_X] = SEL_MOTOR_ADDR_MOTOR_X_Z;
	devSet.selMotorAddr[MOTOR_Y] = SEL_MOTOR_ADDR_MOTOR_Y_SYR;
	devSet.selMotorAddr[MOTOR_Z] = SEL_MOTOR_ADDR_MOTOR_X_Z;
	devSet.selMotorAddr[MOTOR_SYR] = SEL_MOTOR_ADDR_MOTOR_Y_SYR;
	devSet.selMotorAddr[MOTOR_VAL] = SEL_MOTOR_ADDR_MOTOR_VAL_INJ;
	devSet.selMotorAddr[MOTOR_INJ] = SEL_MOTOR_ADDR_MOTOR_VAL_INJ;

	devSet.encoderCtrlAddr[MOTOR_X] = ENCODER_CTRL_ADDR_MOTOR_X_Z;
	devSet.encoderCtrlAddr[MOTOR_Y] = ENCODER_CTRL_ADDR_MOTOR_Y_SYR;
	devSet.encoderCtrlAddr[MOTOR_Z] = ENCODER_CTRL_ADDR_MOTOR_X_Z;
	devSet.encoderCtrlAddr[MOTOR_SYR] = ENCODER_CTRL_ADDR_MOTOR_Y_SYR;
	devSet.encoderCtrlAddr[MOTOR_VAL] = ENCODER_CTRL_ADDR_MOTOR_VAL_INJ;
	devSet.encoderCtrlAddr[MOTOR_INJ] = ENCODER_CTRL_ADDR_MOTOR_VAL_INJ;

	devSet.selStateAddr[MOTOR_X] = SEL_STATE_ADDR_MOTOR_X_Z;
	devSet.selStateAddr[MOTOR_Y] = SEL_STATE_ADDR_MOTOR_Y_SYR;
	devSet.selStateAddr[MOTOR_Z] = SEL_STATE_ADDR_MOTOR_X_Z;
	devSet.selStateAddr[MOTOR_SYR] = SEL_STATE_ADDR_MOTOR_Y_SYR;
	devSet.selStateAddr[MOTOR_VAL] = SEL_STATE_ADDR_MOTOR_VAL_INJ;
	devSet.selStateAddr[MOTOR_INJ] = SEL_STATE_ADDR_MOTOR_VAL_INJ;

/*
	devSet.selMotor[MOTOR_X] = 0x0;
	devSet.selMotor[MOTOR_Y] = 0x1;
	devSet.selMotor[MOTOR_Z] = 0x1;
	devSet.selMotor[MOTOR_SYR] = 0x1;
	devSet.selMotor[MOTOR_VAL] = 0x0;
	devSet.selMotor[MOTOR_INJ] = 0x1;		
*/	
#else
	devSet.baseAddr[MOTOR_X] = BASEADDR_MOTOR_X;
	devSet.baseAddr[MOTOR_Y] = BASEADDR_MOTOR_Y;
	devSet.baseAddr[MOTOR_Z] = BASEADDR_MOTOR_Z;
	devSet.baseAddr[MOTOR_SYR] = BASEADDR_MOTOR_SYR;
	devSet.baseAddr[MOTOR_VAL] = BASEADDR_MOTOR_VAL;
	devSet.baseAddr[MOTOR_INJ] = BASEADDR_MOTOR_INJ;
#endif

	devState.homeFindEnd[MOTOR_X] = NO;
	devState.homeFindEnd[MOTOR_Y] = NO;
	devState.homeFindEnd[MOTOR_Z] = NO;
	devState.homeFindEnd[MOTOR_SYR] = NO;
	devState.homeFindEnd[MOTOR_VAL] = NO;
	devState.homeFindEnd[MOTOR_INJ] = NO;
	
	devSet.ucChkHome[MOTOR_X] = NO;
	devSet.ucChkHome[MOTOR_Y] = NO;
	devSet.ucChkHome[MOTOR_Z] = NO;
	devSet.ucChkHome[MOTOR_SYR] = NO;
	devSet.ucChkHome[MOTOR_VAL] = NO;
	devSet.ucChkHome[MOTOR_INJ] = NO;

	devSet.ucChkHome_R[MOTOR_X] = NO;
	devSet.ucChkHome_R[MOTOR_Y] = NO;
	devSet.ucChkHome_R[MOTOR_Z] = NO;
	devSet.ucChkHome_R[MOTOR_SYR] = NO;
	devSet.ucChkHome_R[MOTOR_VAL] = NO;
	devSet.ucChkHome_R[MOTOR_INJ] = NO;

	devSet.ucChkHomeMethod[MOTOR_X] = CMD_BREAK;
	devSet.ucChkHomeMethod[MOTOR_Y] = CMD_BREAK;
	devSet.ucChkHomeMethod[MOTOR_Z] = CMD_BREAK;
	devSet.ucChkHomeMethod[MOTOR_SYR] = CMD_BREAK;
	devSet.ucChkHomeMethod[MOTOR_VAL] = CMD_BREAK;
	devSet.ucChkHomeMethod[MOTOR_INJ] = CMD_BREAK;
	
	devSet.chkHomeSpeedMin[MOTOR_X] = HOMECHK_SPEEDMIN_MOTOR_X;
	devSet.chkHomeSpeedMin[MOTOR_Y] = HOMECHK_SPEEDMIN_MOTOR_Y;
	devSet.chkHomeSpeedMin[MOTOR_Z] = HOMECHK_SPEEDMIN_MOTOR_Z;
	devSet.chkHomeSpeedMin[MOTOR_SYR] = HOMECHK_SPEEDMIN_MOTOR_SYR;
	devSet.chkHomeSpeedMin[MOTOR_VAL] = HOMECHK_SPEEDMIN_MOTOR_VAL;
	devSet.chkHomeSpeedMin[MOTOR_INJ] = HOMECHK_SPEEDMIN_MOTOR_INJ;	

	devSet.chkHomeSpeed[MOTOR_X] = HOMECHK_SPEEDMAX_MOTOR_X;
	devSet.chkHomeSpeed[MOTOR_Y] = HOMECHK_SPEEDMAX_MOTOR_Y;
	devSet.chkHomeSpeed[MOTOR_Z] = HOMECHK_SPEEDMAX_MOTOR_Z;
	devSet.chkHomeSpeed[MOTOR_SYR] = HOMECHK_SPEEDMAX_MOTOR_SYR;
	devSet.chkHomeSpeed[MOTOR_VAL] = HOMECHK_SPEEDMAX_MOTOR_VAL;
	devSet.chkHomeSpeed[MOTOR_INJ] = HOMECHK_SPEEDMAX_MOTOR_INJ;	

/*
	devSet.findHomeUnitStep[MOTOR_X] = MICRO_STEP_X * 2;
	devSet.findHomeUnitStep[MOTOR_Y] = MICRO_STEP_Y * 2;
	devSet.findHomeUnitStep[MOTOR_Z] = MICRO_STEP_Z * 2;
	devSet.findHomeUnitStep[MOTOR_SYR] = MICRO_STEP_SYR * 2;
	devSet.findHomeUnitStep[MOTOR_VAL] = MICRO_STEP_VAL;
	devSet.findHomeUnitStep[MOTOR_INJ] = MICRO_STEP_INJ;	
*/
	devSet.chkHomeStepCnt[MOTOR_X] = HOMECHK_CNT_MOTOR_X;
	devSet.chkHomeStepCnt[MOTOR_Y] = HOMECHK_CNT_MOTOR_Y;
	devSet.chkHomeStepCnt[MOTOR_Z] = HOMECHK_CNT_MOTOR_Z;
	devSet.chkHomeStepCnt[MOTOR_SYR] = HOMECHK_CNT_MOTOR_SYR;
	devSet.chkHomeStepCnt[MOTOR_VAL] = HOMECHK_CNT_MOTOR_VAL;
	devSet.chkHomeStepCnt[MOTOR_INJ] = HOMECHK_CNT_MOTOR_INJ;

	devSet.minSpeed[MOTOR_X] = ACT_SPEED_MIN_X;
	devSet.minSpeed[MOTOR_Y] = ACT_SPEED_MIN_Y;
	devSet.minSpeed[MOTOR_Z] = ACT_SPEED_MIN_Z;
	devSet.minSpeed[MOTOR_SYR] = ACT_SPEED_MIN_SYR;
	devSet.minSpeed[MOTOR_VAL] = ACT_SPEED_MIN_VAL;
	devSet.minSpeed[MOTOR_INJ] = ACT_SPEED_MIN_INJ;

	devSet.maxSpeed[MOTOR_X] = ACT_SPEED_MAX_X;
	devSet.maxSpeed[MOTOR_Y] = ACT_SPEED_MAX_Y;
	devSet.maxSpeed[MOTOR_Z] = ACT_SPEED_MAX_Z;
	devSet.maxSpeed[MOTOR_SYR] = ACT_SPEED_MAX_SYR;
	devSet.maxSpeed[MOTOR_VAL] = ACT_SPEED_MAX_VAL;
	devSet.maxSpeed[MOTOR_INJ] = ACT_SPEED_MAX_INJ;

	devSet.syrSpeedInj[0] = SYRINGE_SPEED_INJ_1;
	devSet.syrSpeedInj[1] = SYRINGE_SPEED_INJ_2;
	devSet.syrSpeedInj[2] = SYRINGE_SPEED_INJ_3;
	devSet.syrSpeedInj[3] = SYRINGE_SPEED_INJ_4;
	devSet.syrSpeedInj[4] = SYRINGE_SPEED_INJ_5;
	devSet.syrSpeedInj[5] = SYRINGE_SPEED_INJ_6;
	devSet.syrSpeedInj[6] = SYRINGE_SPEED_INJ_7;
	devSet.syrSpeedInj[7] = SYRINGE_SPEED_INJ_8;
	devSet.syrSpeedInj[8] = SYRINGE_SPEED_INJ_9;	
	devSet.syrSpeedInj[9] = SYRINGE_SPEED_INJ_10;

	devSet.syrSpeedWash[0] = SYRINGE_SPEED_1;
	devSet.syrSpeedWash[1] = SYRINGE_SPEED_2;
	devSet.syrSpeedWash[2] = SYRINGE_SPEED_3;
	devSet.syrSpeedWash[3] = SYRINGE_SPEED_4;
	devSet.syrSpeedWash[4] = SYRINGE_SPEED_5;
	devSet.syrSpeedWash[5] = SYRINGE_SPEED_6;
	devSet.syrSpeedWash[6] = SYRINGE_SPEED_7;
	devSet.syrSpeedWash[7] = SYRINGE_SPEED_8;
	devSet.syrSpeedWash[8] = SYRINGE_SPEED_9;	
	devSet.syrSpeedWash[9] = SYRINGE_SPEED_10;

	devSet.posCheckEnable = NO;
	devSet.posCheckMotor = MOTOR_Z; // MOTOR_X;
//	devSet.posCheck1 = 0x30; // ���� x - �̵��� 
//	devSet.posCheck2 = 0x20; // �ϵ� �̵� ���� 
	devSet.posCheck1 = 0x30; // Ȩ��ġ(Ȩ����)(�ϵ�,���̾�) : Ȩ,�ϵ� ���� - �̵���
	devSet.posCheck2 = 0x20; // �̵��� Ȩ��ġ ���        : Ȩ���� �ȵ�,�ϵ� ���� - �̵�

	devState.curStep[MOTOR_X] = 0;
	devState.curStep[MOTOR_Y] = 0;
	devState.curStep[MOTOR_Z] = 0;
	devState.curStep[MOTOR_SYR] = 0;
	devState.curStep[MOTOR_VAL] = 0;
	devState.curStep[MOTOR_INJ] = 0;		
	
// �ʱ�ȭ ���� ���� 
//	devState.curPos[MOTOR_X] = 0;
//	devState.curPos[MOTOR_Y] = 0;
//	devState.curPos[MOTOR_Z] = POS_Z_TOP; //0;
//	devState.curPos[MOTOR_SYR] = 0;
//	devState.curPos[MOTOR_VAL] = POS_VAL_WASHING;
//	devState.curPos[MOTOR_INJ] = POS_INJ_LOAD;

	devState.curTray = TRAY_ETC;

	devSet.homeDir[MOTOR_X] = HOME_DIR_X;
	devSet.homeDir[MOTOR_Y] = HOME_DIR_Y;
	devSet.homeDir[MOTOR_Z] = HOME_DIR_Z;
	devSet.homeDir[MOTOR_SYR] = HOME_DIR_SYR;
	devSet.homeDir[MOTOR_VAL] = HOME_DIR_VAL;
	devSet.homeDir[MOTOR_INJ] = HOME_DIR_INJ;
	
	devState.curDir[MOTOR_X] = HOME_DIR_X;
	devState.curDir[MOTOR_Y] = HOME_DIR_Y;
	devState.curDir[MOTOR_Z] = HOME_DIR_Z;
	devState.curDir[MOTOR_SYR] = HOME_DIR_SYR;
	devState.curDir[MOTOR_VAL] = HOME_DIR_VAL;
	devState.curDir[MOTOR_INJ] = HOME_DIR_INJ;

	devState.bInfinite[MOTOR_X] = 0;
	devState.bInfinite[MOTOR_Y] = 0;
	devState.bInfinite[MOTOR_Z] = 0;
	devState.bInfinite[MOTOR_SYR] = 0;
	devState.bInfinite[MOTOR_VAL] = 0;
	devState.bInfinite[MOTOR_INJ] = 0;	

// 20200218
//motorReadyAll();

//motorResetAll();

	initStepDataRam();


	initStepCtrlData();

	initAdjustmentData();

/*	
	readMotorState();
	checkAdjustData();
	initCoordinate();
*/
}

void readMotorState()
{
//	static uint32_t state[6] = {0x00};

//	if(devState.command == COMMAND_STOP) {
	if(devState.command == COMMAND_ABORT) {
// ��� �ߴ� - COMMAND_INIT(COMMAND_RESET) ������� �ʱ�ȭ 		
// �߰��� ����Ǹ� �ʱ�ȭ �ؾ� �� �͵��� ���� ������ Ȯ���϶�
		stepCtrlCmd(MOTOR_X, HOME_DIR_X | 	CMD_BREAK);
		stepCtrlCmd(MOTOR_Y, HOME_DIR_Y | 	CMD_BREAK);
		stepCtrlCmd(MOTOR_Z, HOME_DIR_Z | 	CMD_BREAK);
		stepCtrlCmd(MOTOR_SYR, HOME_DIR_SYR | 	CMD_BREAK);
		stepCtrlCmd(MOTOR_VAL, HOME_DIR_VAL | 	CMD_BREAK);
		stepCtrlCmd(MOTOR_INJ, HOME_DIR_INJ | 	CMD_BREAK);

//send_SelfMessage(SELF_STOP,0);
send_SelfMessage(SELF_ABORT,0);

stateChange(STATE_FAULT);
devState.command = COMMAND_NONE;

// 20200526
//==================================================================================
// �ʱ�ȭ�� ��ģ�� sleepModeAll() ���� devState.initCommand = NO;�� �����.
// COMMAND_ABORT������ε� ���� �� �� �ִ�. devState.initCommand = NO;�� �����.
//==================================================================================
devState.initCommand = NO;

dfp("devState.command == COMMAND_ABORT\n");
/*
	devState.curNdlWashBeforeCnt = 0;			// ��� �����ߴ°�?
	devState.curInjWashBeforeCnt = 0;			// ��� �����ߴ°�?
	devState.curNdlWashAfterCnt = 0;			// ��� �����ߴ°�?
	devState.curInjWashAfterCnt = 0;			// ��� �����ߴ°�?
*/		
	}

#if FPGA_VERSION_TRIM
	CMD_SEL_STATE(MOTOR_X);
	devState.btMotorState[MOTOR_X] = *((uint32_t volatile *)(BASEADDR_MOTOR_X_Z));
	CMD_SEL_STATE(MOTOR_Y);	
	devState.btMotorState[MOTOR_Y] = *((uint32_t volatile *)(BASEADDR_MOTOR_Y_SYR));
	CMD_SEL_STATE(MOTOR_Z);
	devState.btMotorState[MOTOR_Z] = *((uint32_t volatile *)(BASEADDR_MOTOR_X_Z));
	CMD_SEL_STATE(MOTOR_SYR);	
	devState.btMotorState[MOTOR_SYR] = *((uint32_t volatile *)(BASEADDR_MOTOR_Y_SYR));
	CMD_SEL_STATE(MOTOR_VAL);	
	devState.btMotorState[MOTOR_VAL] = *((uint32_t volatile *)(BASEADDR_MOTOR_VAL_INJ));
//	CMD_SEL_STATE(MOTOR_VAL);	
	CMD_SEL_STATE(MOTOR_INJ);	
	devState.btMotorState[MOTOR_INJ] = *((uint32_t volatile *)(BASEADDR_MOTOR_VAL_INJ));
#else
	devState.btMotorState[MOTOR_X] = *((uint32_t volatile *)(BASEADDR_MOTOR_X));
	devState.btMotorState[MOTOR_Y] = *((uint32_t volatile *)(BASEADDR_MOTOR_Y));
	devState.btMotorState[MOTOR_Z] = *((uint32_t volatile *)(BASEADDR_MOTOR_Z));
	devState.btMotorState[MOTOR_SYR] = *((uint32_t volatile *)(BASEADDR_MOTOR_SYR));
	devState.btMotorState[MOTOR_VAL] = *((uint32_t volatile *)(BASEADDR_MOTOR_VAL));
	devState.btMotorState[MOTOR_INJ] = *((uint32_t volatile *)(BASEADDR_MOTOR_INJ));
#endif


#if 0
dfp(" devState.btMotorState[%d]=%x\n",MOTOR_X,devState.btMotorState[MOTOR_X]);
dfp(" devState.btMotorState[%d]=%x\n",MOTOR_Y,devState.btMotorState[MOTOR_Y]);
dfp(" devState.btMotorState[%d]=%x\n",MOTOR_Z,devState.btMotorState[MOTOR_Z]);
dfp(" devState.btMotorState[%d]=%x\n",MOTOR_SYR,devState.btMotorState[MOTOR_SYR]);
dfp(" devState.btMotorState[%d]=%x\n",MOTOR_VAL,devState.btMotorState[MOTOR_VAL]);
dfp(" devState.btMotorState[%d]=%x\n",MOTOR_INJ,devState.btMotorState[MOTOR_INJ]);
#endif

	if(devSet.ucChkHome[MOTOR_X] == YES) {
		if((devState.btMotorState[MOTOR_X] & X_HOME1_CHECK) == X_HOME1_SENSING ) {
			stepCtrlCmd(MOTOR_X, HOME_DIR_X | devSet.ucChkHomeMethod[MOTOR_X]);
			devSet.ucChkHome[MOTOR_X] = NO;
		}
	}
	else if(devSet.ucChkHome_R[MOTOR_X] == YES) {
		if((devState.btMotorState[MOTOR_X] & X_HOME1_CHECK) == X_HOME1_NOT_SENSING ) {
			stepCtrlCmd(MOTOR_X, HOME_DIR_X | devSet.ucChkHomeMethod[MOTOR_X]);
			devSet.ucChkHome_R[MOTOR_X] = NO;
		}
	}
	
	if(devSet.ucChkHome[MOTOR_Y] == YES) {
		if((devState.btMotorState[MOTOR_Y] & Y_HOME1_CHECK) == Y_HOME1_SENSING ) {
			stepCtrlCmd(MOTOR_Y, HOME_DIR_Y | devSet.ucChkHomeMethod[MOTOR_Y]);
			devSet.ucChkHome[MOTOR_Y] = NO;
		}
	}
	else if(devSet.ucChkHome_R[MOTOR_Y] == YES) {
		if((devState.btMotorState[MOTOR_Y] & Y_HOME1_CHECK) == Y_HOME1_NOT_SENSING ) {
			stepCtrlCmd(MOTOR_Y, HOME_DIR_Y | devSet.ucChkHomeMethod[MOTOR_Y]);
			devSet.ucChkHome_R[MOTOR_Y] = NO;
		}
	}
	
	if(devSet.ucChkHome[MOTOR_Z] == YES) {
		if((devState.btMotorState[MOTOR_Z] & Z_HOME1_CHECK) == Z_HOME1_SENSING ) {
			stepCtrlCmd(MOTOR_Z, HOME_DIR_Z | devSet.ucChkHomeMethod[MOTOR_Z]);
			devSet.ucChkHome[MOTOR_Z] = NO;
		}
	}
	else if(devSet.ucChkHome_R[MOTOR_Z] == YES) {
		if((devState.btMotorState[MOTOR_Z] & Z_HOME1_CHECK) == Z_HOME1_NOT_SENSING ) {
			stepCtrlCmd(MOTOR_Z, HOME_DIR_Z | devSet.ucChkHomeMethod[MOTOR_Z]);
			devSet.ucChkHome_R[MOTOR_Z] = NO;
		}
	}

	if(devSet.ucChkHome[MOTOR_SYR] == YES) {
		if((devState.btMotorState[MOTOR_SYR] & SYR_HOME1_CHECK) == SYR_HOME1_SENSING ) {
			stepCtrlCmd(MOTOR_SYR, HOME_DIR_SYR | devSet.ucChkHomeMethod[MOTOR_SYR]);
			devSet.ucChkHome[MOTOR_SYR] = NO;
		}
	}
	else if(devSet.ucChkHome_R[MOTOR_SYR] == YES) {
		if((devState.btMotorState[MOTOR_SYR] & SYR_HOME1_CHECK) == SYR_HOME1_NOT_SENSING ) {
			stepCtrlCmd(MOTOR_SYR, HOME_DIR_SYR | devSet.ucChkHomeMethod[MOTOR_SYR]);
			devSet.ucChkHome_R[MOTOR_SYR] = NO;
		}
	}
	
	if(devSet.ucChkHome[MOTOR_VAL] == YES) {
		if((devState.btMotorState[MOTOR_VAL] & VAL_HOME1_CHECK) == VAL_HOME1_SENSING ) {
			stepCtrlCmd(MOTOR_VAL, HOME_DIR_VAL | devSet.ucChkHomeMethod[MOTOR_VAL]);
			devSet.ucChkHome[MOTOR_VAL] = NO;
		}
	}

// ==============================================================================
// injector : ���ڴ��� Ȩ�� �д�. (�ո� �κ��� �д�.) - 60���� �ȵǰ� �շ�����.
//    �շ��ִ� �κ��� �� ��Ʈ���̿� �ִ� ��ġ�̰� , ���� �κп� ��Ʈ���� �ִ�.
// ���� �ٸ��� ã�´�. ���� �κ��� ã�´�. 
// Ȩ��ġ�� �ƴϸ� �ð� �������� Ȩ��ġ�� ã�� 
// �ٽ� �ð� �������� Ȩ�ƴ� �κ��� ã�´�. - �󸶰� �ð�������� ȸ�� -- ��
// ==============================================================================
	if(devSet.ucChkHome[MOTOR_INJ] == YES) {
		if((devState.btMotorState[MOTOR_INJ] & INJ_HOME1_CHECK) == INJ_HOME1_SENSING ) {
			stepCtrlCmd(MOTOR_INJ, HOME_DIR_INJ | devSet.ucChkHomeMethod[MOTOR_INJ]);
			devSet.ucChkHome[MOTOR_INJ] = NO;
		}
	}

}

void checkPosition()
{
	//devSet.posCheck1 = 0x30; // ���� x - �̵���
	//devSet.posCheck2 = 0x20; // �ϵ� �̵� ����
	unsigned char btMotorState;
	
// RYU_SF2
#if 1
	unsigned char who;

	who = devSet.posCheckMotor;
	CMD_SEL_STATE(who);

	btMotorState = (*((uint32_t volatile *)(devSet.baseAddr[who])) & Z_POS_CHECK_MASK);
	
#if CHECK_POSITION_MODIFY	
	if(btMotorState != devSet.posCheck2) {
#else
	if(btMotorState != devSet.posCheck1 && btMotorState != devSet.posCheck2) {
#endif
		stepCtrlCmd(who, HOME_DIR_X | CMD_BREAK);
		motorBreakAll();
		motorSleepAll();
		errorFunction(SELF_ERROR_POSITION_XY);
		devSet.posCheckEnable = NO;
iprintf("======btMotorState[%d]=%x\n",who,btMotorState);
	}
#else
	btMotorState = (*((uint32_t volatile *)(devSet.baseAddr[devSet.posCheckMotor])) & 0x30);

	if(btMotorState != devSet.posCheck1 && btMotorState != devSet.posCheck2) {
		stepCtrlCmd(devSet.posCheckMotor, HOME_DIR_X | CMD_BREAK);
		motorBreakAll();		
		motorSleepAll();
		errorFunction(SELF_ERROR_POSITION_XY);
devSet.posCheckEnable = NO;		
	}
#endif

}


