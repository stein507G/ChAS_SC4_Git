 /**
  ******************************************************************************
  * @file    net_AS.h
  * @date    01/20/2016
  * @brief   This file contains all the functions prototypes for the net_AS.c
  *          file.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOTOR_CTRL_TASK_H
#define __MOTOR_CTRL_TASK_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/**************************************************************************/
/* Driver Includes */
/**************************************************************************/

// Z축 상태 때문에 움직일 수 없는 경우 
#define MOVE_CMD_OK						0
#define MOVE_ERR_NDLCHK				1				// Z축이 내려와 있음
#define MOVE_ERR_VIALCHK			2				// Z축 Vial 검출 상태
#define MOVE_ERR_BOTH					3				// Z축 둘다 검출

// 20200516
#if GO_POS_ETC_USE

#if 1
#define goPosWashingPort()		goPosETC_Z_check(POS_XY_WASH,		POS_Z_WASH,		POS_Z_WASH_DETECT,	POS_Z_WASH_UPPER_DETECT)
#define goPosWastePort()			goPosETC_Z_check(POS_XY_WASTE,	POS_Z_WASTE,	POS_Z_WASTE_DETECT,	POS_Z_WASTE_UPPER_DETECT)

//#define goPosInjection()			goPosETC_Z_check_inj(POS_XY_INJECTOR,		POS_Z_INJECTION,		POS_Z_INJECTION_PORT_DETECT)
#define goPosInjection()			goPosETC_Z_check(POS_XY_INJECTOR,		POS_Z_INJECTION,		POS_Z_INJECTION_PORT_DETECT,  POS_Z_INJECTION_PORT_UPPER_DETECT)

//POS_Z_INJECTION_PORT_UPPER_DETECT
//POS_Z_INJECTION_PORT_DETECT
#else
#define goPosWashingPort()		goPosETC_Z(POS_XY_WASH,POS_Z_WASH)
#define goPosWastePort()		goPosETC_Z(POS_XY_WASTE,POS_Z_WASTE)
#define goPosInjection()		goPosETC_Z(POS_XY_INJECTOR,POS_Z_INJECTION)
#endif
#else
#define goPosWashingPort()		goPosETC_Z_check(POS_XY_WASH,POS_Z_WASH,POS_Z_WASH_DETECT,POS_Z_WASH_UPPER_DETECT)
#define goPosWastePort()		goPosETC_Z_check(POS_XY_WASTE,POS_Z_WASTE,POS_Z_WASTE_DETECT,POS_Z_WASTE_UPPER_DETECT)
#define goPosInjection()		goPosETC_Z_check_inj(POS_XY_INJECTOR,POS_Z_INJECTION,POS_Z_INJECTION_PORT_DETECT)
#endif

#define goPosHome()					goPosETC_Z(POS_XY_HOME,POS_Z_TOP)


#if FPGA_VERSION_TRIM
	#define CtrlCmd_SelMotor_X()  *((uint32_t volatile *)(CTRL_PORT_ADDR_MOTOR_X)) = ctrlCmd;
#endif


void load_StepDataAddr(uint32_t who, uint32_t addr, uint32_t data);

//void make_StepData(uint32_t who, uint32_t rpm, uint32_t interval, uint32_t microStep);
//void make_StepData(uint32_t who, uint32_t rpm, float interval, uint32_t microStep);
void make_StepData(uint32_t who, uint32_t Freq, uint32_t rpm, float interval, uint32_t microStep);

int moveStep(uint32_t who,int32_t tStep);

int stepControl(uint32_t who, uint32_t ctrlCmd, uint32_t Speed, uint32_t totalStep);

int moveStepSpeed(uint32_t who, uint32_t minSpeed, uint32_t maxSpeed, int32_t tStep);

// initialize 할  홈찾기 에서만 사용함. 
int goSensorHome(uint32_t who, uint32_t speed, uint32_t totalStep);

int goPosETC(unsigned char pos);
int goPosVial(const VIAL_POS_t *vial);

int goX(int32_t pos, uint32_t Tray, uint32_t option);
int goY(int32_t pos, uint32_t Tray, uint32_t option);
int goZ(int32_t pos, uint32_t option);
int goX_Encoder(int32_t pos, uint32_t Tray, uint32_t option);
int goY_Encoder(int32_t pos, uint32_t Tray, uint32_t option);
int goZ_Encoder(int32_t pos, uint32_t option);

int goSyr(float pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option);
int goVal(int32_t pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option);
int goInj(int32_t pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option);
int goSyr_Encoder(float pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option);
int goVal_Encoder(int32_t pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option);
int goInj_Encoder(int32_t pos, uint32_t minSpeed, uint32_t maxSpeed, uint32_t option);

int goPosVial_Z(const VIAL_POS_t *pVial, int32_t z_pos);//, uint32_t z_option)
int goPosETC_Z(int32_t xy_pos, int32_t z_pos);//, uint32_t z_option)
int goPosETC_Z_check(int32_t xy_pos, int32_t z_pos, int32_t z_chk_pos, int32_t z_chk_upper_pos);

void initStepDataRam();

// RESET : Active LOW 출력
// SLEEP : Active LOW 출력
void stepCtrlCmd(uint32_t who, uint32_t ctrlCmd);

void motorSleepAll();

void motorBreakAll();
void motorResetAll();
void motorReadyAll();

void initStepCtrlData();

int initHomePos_Val();	// Valve만 따로 홈위치 찾기 
int initHomePos_Inj();	// 인젝터만 따로 홈위치 찾기 
int initHomePos_Syr();	// 
int initHomePos_Z();	// 
int initHomePos_X();// 
int initHomePos_Y();// 
int initHomePos_All();

int checkTray(unsigned char vialType, VIAL_POS_t *pVial);
int checkVialPlate();

int adjustMotorInit();
int adjustNeedleExchange();
int adjustSyringeExchange();
int initCalibration();
int adjustment_X();

int initCoordinate();
int initVialCoordinate(int AdjustStep_Z, int AdjustVial_Z);

void initStepCtrlValue();

void readMotorState();

void (*pCheckHome_Fn) (); // 함수포인터 선언

void checkPosition();

int ChkEncoder_X(int encoderCount);
int ChkEncoder_Y(int encoderCount);
int ChkEncoder_Z(int encoderCount);

int readyChkRetry_X(int encoderCount);
int readyChkRetry_Y(int encoderCount);
int readyChkRetry_Z(int encoderCount);

//int goRetryPos(int32_t pos, uint32_t Tray, uint32_t option);
int goRetryPos_X(int32_t pos, uint32_t Tray, uint32_t option);
int goRetryPos_Y(int32_t pos, uint32_t Tray, uint32_t option);
int goRetryPos_Z(int32_t pos, uint32_t Tray, uint32_t option);

#ifdef __cplusplus
}
#endif

#endif //__MOTOR_CTRL_TASK_H

/******************* (C) COPYRIGHT 2016 stein507 *****END OF FILE****/

