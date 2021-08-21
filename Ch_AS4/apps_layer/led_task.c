/*************************************************************************
 * (c) Copyright 2009 Actel Corporation.  All rights reserved.
 *
 *  Application demo for Smartfusion
 *
 *
 * Author : Actel Application Team
 * Rev     : 1.0.0.3
 *
 **************************************************************************/

/**************************************************************************/
/* Standard Includes */
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>

/**************************************************************************/
/* Driver Includes */
/**************************************************************************/

//#include "../drivers/mss_gpio/mss_gpio.h"

#include "fpga_ChAS.h"

#include "def_com.h"
#include "define_AS.h"
#include "debug_inform.h"

#include "../common_inc/Def_stepCon.h"
#include "../common_inc/protocolType_AS.h"
#include "../common_inc/comType_AS.h"
#include "led_task.h"

/**************************************************************************/
/* RTOS Includes */
/**************************************************************************/

// ryu #include "FreeRTOS.h"
// ryu #include "semphr.h"

/**************************************************************************/
/* Extern Declarations */
/**************************************************************************/
extern LCAS_DEV_STATE_t devState;
extern LCAS_STATE_t LcasState;

void led_Control()
{
	static uint32_t cntInit = 0;
	static uint32_t cnt = 0;
	static uint32_t cntConnect = 0;	
	static uint32_t oldActionMenu = 0;
	uint32_t menuChg = NO;

	if(cnt++ == 5) cnt = 0;		// 50msec

	if(oldActionMenu != devState.actionMenu) {
dfp("old=%d new=%d\n",oldActionMenu,devState.actionMenu);
		oldActionMenu = devState.actionMenu;
		menuChg = YES;
	}

	if(devState.powerOffLed == ON) {
		PowerOff_LED();
		return;
	}

	switch( devState.actionMenu ) {
		case AS_ACTION_NONE:
			LED_READY_RUN_OFF;
			LED_ERROR_OFF;
			if(menuChg) LED_Act_None();
			break;
		case AS_ACTION_INITILIZE:
			if(++cntInit == 2) {
				cntInit = 0;
				LED_READY_RUN_BLINK;
				LED_ERROR_OFF;				
			}
			if(menuChg) LED_Act_Initiaize();
			break;
		case AS_ACTION_READY:
			if(cnt == 5) {		
				LED_READY_RUN_ON;
				LED_ERROR_OFF;
			}
			if(menuChg) LED_Act_Ready();
			break;
		case AS_ACTION_RUN:
			if(cnt == 5) {
				LED_READY_RUN_BLINK;
				LED_ERROR_OFF;
			}
			if(menuChg) LED_Act_Run();
			break;
		case AS_ACTION_FAULT:
			LED_READY_RUN_OFF;
			LED_ERROR_BLINK;
			if(menuChg) {
				if(devState.LeakError == YES) {
					LED_Act_Fault();		// leak
				}
				else {
					LED_Act_Fault_Blink();		// blink
				}
			}
			break;
		case AS_ACTION_STANDBY:
			LED_READY_RUN_OFF;
			LED_ERROR_OFF;
			if(menuChg) LED_Act_Standby();
			break;
		case AS_ACTION_DIAGNOSTICS:
			if(cnt == 5) {		
				LED_READY_RUN_BLINK;
				LED_ERROR_OFF;
			}
			if(menuChg) LED_Act_Diagnostics();
			break;
		case AS_ACTION_ADJUST:
			if(cnt == 5) {		
				LED_READY_RUN_BLINK;
				LED_ERROR_BLINK;
			}
			if(menuChg) LED_Act_Adjust();
			break;
	}

	if(++cntConnect != 5) return;

	cntConnect = 0;

}

void led_Control_Blank()
{

}

/*
void InitialLED(void)
{

// LED 설정
// time : 100ms
// Green / red / blue


#define BUZZER_CON_OFF			FPGA_WRITE_WORD(W3_BUZZER_CON, 0x0)
#define BUZZER_CON_ON				FPGA_WRITE_WORD(W3_BUZZER_CON, 0x1)

	#define  W3_BUZZER_CON	 				(COMMON_CONTROL + 0x00000003)	//					(COMMON_CTRL_BASE_ADDR + (0x00000003 << 2))


#define FPGA_BASE_ADDR   		(0x50000000U)		// #define COREMEMCTRL_0     0x50000000U
#define LED_SWITCH								0x1000U			// sel[14:12] = "001"
#define  W6_FLED_COLOR_TIME1		(LED_SWITCH + 0x00000006)

#define FLED_1_CONTROL(X)		*((uint32_t volatile *)(W6_FLED_COLOR_TIME1)) = (X)


		FLED_1_CONTROL(FLED_COLOR1 | (FLED_COLOR_TIME1 << 24));
		FLED_2_CONTROL(FLED_COLOR2 | (FLED_COLOR_TIME2 << 24));
		FLED_3_CONTROL(FLED_COLOR3 | (FLED_COLOR_TIME3 << 24));		
//		FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
		FLED_CFG_LOAD_TOUCH();

}

#define FPGA_WRITE_WORD(X,Y)		 *(volatile unsigned int *) (FPGA_BASE_ADDR + ((X)<<2)) = (Y)
*/
void InitialLED(void)
{
// LED 설정
// time : 100ms
// Green / red / blue
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, FLED_COLOR1 | (FLED_COLOR_TIME1 << 24));
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, FLED_COLOR2 | (FLED_COLOR_TIME2 << 24));
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, FLED_COLOR3 | (FLED_COLOR_TIME3 << 24));	

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
}




void PowerOff_LED(void)	// white
{
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x04FFFFFF);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05FFFFFF);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x01FFFFFF);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
}

void LED_Act_None()
{

	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x04FF0000);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05FF0000);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x01FF0000);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);		
}
void LED_Act_Initiaize()	
{
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x04FFFFFF);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05FFFFFF);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x01FFFFFF);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
}
void LED_Act_Ready()
{
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x04FF0000);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05FF0000);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x01FF0000);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);		
}
void LED_Act_Run()
{
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x040000FF);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x050000FF);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x010000FF);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
}
void LED_Act_Fault()
{
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x0400FF00);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x0500FF00);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x0100FF00);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
}

void LED_Act_Fault_Blink()
{
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x0400FF00);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05FFFFFF);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x0100FF00);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
}

void LED_Act_Standby()
{
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x04FF0000);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05FF0000);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x01FF0000);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);		
}
void LED_Act_Diagnostics()
{
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x04FF0000);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05FF0000);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x01FF0000);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);		
}
void LED_Act_Adjust()
{
	FPGA_WRITE_WORD(W6_FLED_COLOR_TIME1, 0x04FFFFFF);
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05FFFFFF);
	FPGA_WRITE_WORD(W8_FLED_COLOR_TIME3, 0x01FFFFFF);

	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
}

void LED_Act_Connect()
{
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05FF0000);
	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
}

void LED_Act_NotConnect()
{
	FPGA_WRITE_WORD(W7_FLED_COLOR_TIME2, 0x05000000);	
	FPGA_WRITE_WORD(T5_FLED_CONFIG_LOAD, 1);
}











