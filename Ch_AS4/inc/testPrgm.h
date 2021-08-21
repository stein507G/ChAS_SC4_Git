 /**
  ******************************************************************************
  * @file    net_AS.h
  * @date    01/20/2016
  * @brief   This file contains all the functions prototypes for the net_AS.c
  *          file.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TEST_PRGM_H
#define __TEST_PRGM_H

#ifdef __cplusplus
 extern "C" {
#endif


#define TEST_MODE_X 		0x0000	// ON,ON,ON(000)
#define TEST_MODE_Y 		0x0001	// ON,ON,OFF(001)
#define TEST_MODE_Z 		0x0002	// ON,OFF,ON(010)
#define TEST_MODE_SYR 		0x0003	// ON,OFF,OFF(011)
#define TEST_MODE_VAL 		0x0004	// OFF,ON,ON(100)
#define TEST_MODE_INJ 		0x0005	// OFF,ON,OFF(101)


//#define TEST_MODE_BOARD_ALL		0x0006 	// OFF,OFF,ON(110)
//#define TEST_MODE_VAL_SYR		0x0007 // OFF,OFF,OFF(111)

#define TEST_MODE_VAL_SYR			0x003B 		// sw6~sw1 : OFF,OFF,OFF,ON,OFF,OFF(111111 (0x3b))
#define TEST_MODE_X_Y_Z 			0x003D		// sw6~sw1 : OFF,OFF,OFF,OFF,ON,OFF(111111 (0x3d))
#define TEST_MODE_BOARD_ALL		0x003F 		// sw6~sw1 : OFF,OFF,OFF,OFF,OFF,OFF(111111 (0x3f))

// Switch input
#define TEST_MODE_NONE 		0x3f

#define TEST_MODE_SW1 		0x3e
#define TEST_MODE_SW1_SW2 		0x3c
#define TEST_MODE_SW1_SW3 		0x3a
#define TEST_MODE_SW1_SW2_SW3 		0x38

#define TEST_MODE_SW2 		0x3d

#define TEST_MODE_SW3 		0x3b

#define TEST_MODE_SW2_SW3 		0x39

#define TEST_MODE_SW4 		0x37
#define TEST_MODE_SW4_SW2 		0x35
#define TEST_MODE_SW4_SW3 		0x33
#define TEST_MODE_SW4_SW2_SW3 		0x31

#define TEST_MODE_SW5 		0x2f
#define TEST_MODE_SW5_SW2 		0x2d
#define TEST_MODE_SW5_SW3 		0x2b
#define TEST_MODE_SW5_SW2_SW3 		0x29

#define TEST_MODE_SW6 		0x1f
#define TEST_MODE_SW6_SW2 		0x1d
#define TEST_MODE_SW6_SW3 		0x1b
#define TEST_MODE_SW6_SW2_SW3 		0x19



/* Includes ------------------------------------------------------------------*/



/**************************************************************************/
/* Driver Includes */
/**************************************************************************/
void initMotorCtrlSet();

void CheckBoardState();

void motorMoving();



//void machineTest_Mode();

int send_BoardState();

void motorControlWho(uint32_t who);
void motorControlFn();

int	netBoardCmd(char *pData);

//#if BOARD_TEST_PRGM
int (*pTest_Fn[2]) (int);

int testMotor(int value);
int testSensor(int value);

void boardTest_Function_Set();
//#endif

//#if MACHINE_TEST_PRGM
void machineTest_Function_Set();
/*
//int (*pMachineTest_Fn) (int);
int machineTest_Mode_Blank(int value);
int machineTest_Mode1(int value);
int machineTest_Mode2(int value);
int machineTest_Mode3(int value);
int machineTest_Mode4(int value);
int machineTest_Mode5(int value);
*/
int (*pMachineTest_Fn) (int);
//int (*pMachineTest_Fn[8]) (int);
int machineTest_Mode_X(int value);
int machineTest_Mode_Y(int value);
int machineTest_Mode_Z(int value);
int machineTest_Mode_SYR(int value);
int machineTest_Mode_VAL(int value);
int machineTest_Mode_INJ(int value);

int machineTest_Mode_X_Y_Z(int value);
int mTest_X_Y_Z_Act(int func);
int mTest_Mode_XYZ_1Cycle(int func);
	
int machineTest_Mode_VAL_SYR(int value);
int mTest_VAL_SYR_Act(int func);
int mTest_VAL_SYR_Act_1Cycle(int func);	

int machineTest_Mode_BOARD_ALL(int value);
//#endif

//int nvm_main(void);
//void test_eeprom();


#ifdef __cplusplus
}
#endif

#endif //__LCAS_ACTION_TASK_H

/******************* (C) COPYRIGHT 2016 stein507 *****END OF FILE****/

