 /**
  ******************************************************************************
  * @file    net_AS.h
  * @date    01/20/2016
  * @brief   This file contains all the functions prototypes for the net_AS.c
  *          file.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCAS_ACTION_TASK_H
#define __LCAS_ACTION_TASK_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/**************************************************************************/
/* Driver Includes */
/**************************************************************************/

void init_analysisTimetick();

#if 0
int WashSyringe(int repeat);
int WashInternal(int repeat);
int WashExternal(int repeat);
#endif

int Washing(int repeat, int *curRepeat, int port);

int endSequence();	// ����������� �������� ������.

int	sleepModeAll();
int wakeup_XYZ_checkHome();		// sleep mode���� �����

int startInjectionFunction();
int endAnalysisFunction();
int startLoadingCommand();

int stopFunction();

int stateChange(int state);

int goHomeXY();
int	goHomeCheckXYZ();

int chkNextInjectionMix();
int	chkNextInjectionRun();
int checkWashBetween();

int aspirateSample(const VIAL_POS_t *pVial, float vol, uint32_t speed);
//int dispenseSample(const VIAL_POS_t *pVial, int32_t vol, uint32_t speed);

int (*pLoop_Injection_Fn) (const VIAL_POS_t *pVial); // �Լ������� ����
int full_LoopInjection(/*int repeat,*/ const VIAL_POS_t *pVial);
int partial_LoopInjection(const VIAL_POS_t *pVial);
int micro_pick_Injection(const VIAL_POS_t *pVial);

int	checkAdjustData();

int nextSample(VIAL_POS_t *pVial);
int nextVial(VIAL_POS_t *pVial);

int	mix_ProgramNew();
int mix_addPrgmNew(unsigned int order);
int mix_mixPrgmNew(unsigned int order);
int mix_waitPrgmNew(unsigned int order);

int mix_Program();
int mix_addPrgm(unsigned int order);
int mix_mixPrgm(unsigned int order);
int mix_waitPrgm(unsigned int order);

int (*pTime_Event_Fn)();	// �Լ������� ����
int time_Event();	// �Լ������� ����
int time_Event_Blank();	// �Լ������� ����

//int errorFunction(unsigned char err);
int errFunction(unsigned char err);

//int (*set[3])(int *, int,int *, int,int *); //�Լ� ������ �迭 ���� ����
int (*pAction_Fn[AS_ACTION_FUNC_MAX+1]) (int); // �Լ������� ���� : ȣ�� -  pAction_Fn[0](int);
int actionNone(int value);
int actionInitilize(int value);	// AS_ACTION_INITILIZE
int actionReady(int value);
int actionMix(int value);
int actionRun(int value);
int actionFault(int value);
int actionStandby(int value);
int actionDiagnostics(int value);
//	int (*pDiag_Fn[4])(int);
	int actionDiagNone(int value);
	int actionDiagHeaterCheck(int value);
	int actionDiagCoolerCheck(int value);
	int actionDiagTempsenCheck(int value);

int actionAdjust(int value);
int actionServiceWash(int value);
int actionStopFunction(int value);

int Action();

//void timer_100ms();

void initSystemInform();
void saveSystemInform(int who);
void initSubmenuValue();
void initValue();

void applySystemConfig();
void initSystemConfig();
//void initRemoteConfig();

void initRunSetup();
void initSequence();

void initAuxEvntFn();
void initAuxEvntValue();
	
void initMixTable();
void initMixSetup();

void initDiagData();
void initSpecial(unsigned char load);
int usedTimeCheck(unsigned char act);
int usedTimeSaveFunc();

int encoderCountCheck(int encoderCount, int readCount);

//void remoteControl();
void remoteControl(uint32_t signalIn);
void CheckPowerBtn(uint32_t signalIn);

void CheckDeviceState();
	
#ifdef __cplusplus
}
#endif

#endif //__LCAS_ACTION_TASK_H

/******************* (C) COPYRIGHT 2016 stein507 *****END OF FILE****/

