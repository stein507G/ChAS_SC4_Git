#ifndef LED_TASK_H_
#define LED_TASK_H_

//#include "LCAS_SOC_2016_hw_platform.h"
#include "Chrozen_AS_hw_platform.h"

//=======================================================================================================================
// LED Control
// CONNECTED : 연결전 BLINK		0.5초
//					 : 연결후 ON
//					 : 끊김 	OFF
// READY/RUN : 초기화 BLINK
//					 : ADJUST BLINK 빠르게
//					 : READY(대기) : ON
// ERROR     : 초기화 BLINK
//					 : ADJUST BLINK 빠르게
//					 : READY(대기) : ON
// LED_S1 - GPIO_8
// LED_S2 - GPIO_9
// LED_S3 - GPIO_10
//=======================================================================================================================
//#define GPIO_OUT_REG_OFFSET				COREGPIO_BASE_ADDR + GPIO_OUT_REG_OFFSET
//HAL_set_32bit_reg( this_gpio->base_addr, GPIO_OUT, value )

#if 0
#define LED_S1_ON		Core_gpio_pattern &= 0xfffffeff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define LED_S2_ON		Core_gpio_pattern &= 0xfffffdff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define LED_S3_ON		Core_gpio_pattern &= 0xfffffbff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)

#define LED_S1_OFF	Core_gpio_pattern |= 0x00000100;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define LED_S2_OFF	Core_gpio_pattern |= 0x00000200;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define LED_S3_OFF	Core_gpio_pattern |= 0x00000400;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)

#define LED_S1_BLINK	Core_gpio_pattern ^= 0x00000100;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define LED_S2_BLINK	Core_gpio_pattern ^= 0x00000200;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define LED_S3_BLINK	Core_gpio_pattern ^= 0x00000400;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)

#define LED_CONNECTED_ON			LED_S3_ON
#define LED_READY_RUN_ON			LED_S2_ON
#define LED_ERROR_ON					LED_S1_ON

#define LED_CONNECTED_OFF			LED_S3_OFF
#define LED_READY_RUN_OFF			LED_S2_OFF
#define LED_ERROR_OFF					LED_S1_OFF    //LED_S1_OFF

#define LED_CONNECTED_BLINK		LED_S3_BLINK
#define LED_READY_RUN_BLINK		LED_S2_BLINK
#define LED_ERROR_BLINK				LED_S1_BLINK
#else
#define LED_S1_ON
#define LED_S2_ON
#define LED_S3_ON

#define LED_S1_OFF
#define LED_S2_OFF
#define LED_S3_OFF

#define LED_S1_BLINK
#define LED_S2_BLINK
#define LED_S3_BLINK

#define LED_CONNECTED_ON			LED_S3_ON
#define LED_READY_RUN_ON			LED_S2_ON
#define LED_ERROR_ON					LED_S1_ON

#define LED_CONNECTED_OFF			LED_S3_OFF
#define LED_READY_RUN_OFF			LED_S2_OFF
#define LED_ERROR_OFF					LED_S1_OFF    //LED_S1_OFF

#define LED_CONNECTED_BLINK		LED_S3_BLINK
#define LED_READY_RUN_BLINK		LED_S2_BLINK
#define LED_ERROR_BLINK				LED_S1_BLINK
#endif

#if 0
#define ALL_LED_OFF			Core_gpio_pattern |= 0x00000700;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define EXT_LED_CON_LOW			Core_gpio_pattern &= 0xffff7fff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define EXT_LED_CON_HIGH		Core_gpio_pattern |= 0x00008000;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define EXT_LED_CON_OFF			EXT_LED_CON_LOW
#define EXT_LED_CON_ON			EXT_LED_CON_HIGH
#endif

//=======================================================================================================================
// CoreGPIO input
// Switch1  -- GPIO_0
// Switch2  -- GPIO_1
// DoorErr  -- GPIO_2
// Remote start in -- GPIO_3
// Remote ready in -- GPIO_4
// LeakSen -- GPIO_5
// cpu_onoff_det -- GPIO_6
//=======================================================================================================================
/*
gpio_in = HAL_get_32bit_reg( this_gpio->base_addr, GPIO_IN );
#define HAL_get_32bit_reg(BASE_ADDR, REG_NAME) \
          (HW_get_32bit_reg( ((this_gpio->base_addr) + (GPIO_IN_REG_OFFSET)) ))
*/

//=======================================================================================================================
// CoreGPIO output
// LED_S1 - GPIO_8
// LED_S2 - GPIO_9
// LED_S3 - GPIO_10
// =======REMOTE Control
// READY_OUT - GPIO_11
// START_OUT - GPIO_12
// Cpu_Pwr_Con - GPIO_13
// remote start in은 polling mode 로 일정시간마다 검사한다. 
//   --- 인터럽트는 ads1259용으로 사용
//=======================================================================================================================

#if 0
#define REMOTE_READY_OUT_CONTACT_CLOSE		Core_gpio_pattern &= 0xfffff7ff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define REMOTE_READY_OUT_CONTACT_OPEN		Core_gpio_pattern |= 0x00000800;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define REMOTE_START_OUT_CONTACT_CLOSE		Core_gpio_pattern &= 0xffffefff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define REMOTE_START_OUT_CONTACT_OPEN		Core_gpio_pattern |= 0x00001000;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)

// 수정할 것 - 회로추가 - ready signal을 사용한다.
#define AUX_SIG_OUT_CONTACT_CLOSE		Core_gpio_pattern &= 0xfffff7ff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define AUX_SIG_OUT_CONTACT_OPEN		Core_gpio_pattern |= 0x00000800;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)

#define CPU_PWR_CON_LOW			Core_gpio_pattern &= 0xffffdfff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define CPU_PWR_CON_HIGH		Core_gpio_pattern |= 0x00002000;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)


#if BOARD_REV == 2
#define BUZZER_CON_OFF			Core_gpio_pattern &= 0xffffbfff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define BUZZER_CON_ON				Core_gpio_pattern |= 0x00004000;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#elif BOARD_REV == 3
	#if BUZZER_MODIFY
		//#else BOARD_REV == 4
		#define BUZZER_CON_ON			Core_gpio_pattern |= 0x00004000;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
		#define BUZZER_CON_OFF		Core_gpio_pattern &= 0xffffbfff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
	#else
		#define BUZZER_CON_ON			Core_gpio_pattern &= 0xffffbfff;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
		#define BUZZER_CON_OFF			Core_gpio_pattern |= 0x00004000;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
	#endif
#endif
#endif

#if 0
#define TEST_REMOTE_READY_TOGGLE		Core_gpio_pattern ^= 0x00000800;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#define TEST_REMOTE_START_TOGGLE		Core_gpio_pattern ^= 0x00001000;	*((uint32_t volatile *)(COREGPIO_APBBIF_0_OUT_REG)) = (Core_gpio_pattern)
#endif

//=======================================================================================================================
// PWM Output 
//=======================================================================================================================
// 설정 순서
// 1. set pwm_data
// 2. set peltier - heating & cooling 
// 3. peltier - enable (on/off)
//=======================================================================================================================
#if 0
#define PELTIER_1_CON_ON		pwnControl |=	0x00000001; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)
#define PELTIER_1_CON_OFF		pwnControl &=	0xfffffffe; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)

#define PELTIER_2_CON_ON		pwnControl |=	0x00000002; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)
#define PELTIER_2_CON_OFF		pwnControl &=	0xfffffffd; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)

//#define PEPELTIER_CON_ON		PELTIER_1_CON_ON; PELTIER_2_CON_ON
//#define PEPELTIER_CON_OFF		PELTIER_1_CON_OFF; PELTIER_2_CON_OFF
#define PELTIER_CON_ON			pwnControl |=	0x00000003; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)
#define PELTIER_CON_OFF			pwnControl &=	0xfffffffc; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)

#define PELTIER_1_SET_COOLING 	pwnControl |=	0x00000010; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)
#define PELTIER_1_SET_HEATING		pwnControl &=	0xffffffef; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)

#define PELTIER_2_SET_COOLING		pwnControl |=	0x00000020; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)
#define PELTIER_2_SET_HEATING 	pwnControl &=	0xffffffdf; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)

//#define PELTIER_SET_HEATING		PELTIER_1_SET_HEATING; PELTIER_2_SET_HEATING
//#define PELTIER_SET_COOLING		PELTIER_1_SET_COOLING; PELTIER_2_SET_COOLING
#define PELTIER_SET_COOLING 	pwnControl |=	0x00000030; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)
#define PELTIER_SET_HEATING		pwnControl &=	0xffffffcf; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)

#define PELTIER_1_DATA(X)		*((uint32_t volatile *)(PWM_CON_BASE_ADDR + S1_PWM1_OFFSET)) = (X)
#define PELTIER_2_DATA(X)		*((uint32_t volatile *)(PWM_CON_BASE_ADDR + S2_PWM2_OFFSET)) = (X)

#define PELTIER_DATA(X)			PELTIER_1_DATA(X); PELTIER_2_DATA(X)

#define HeaterPowerOn(X)		{ PELTIER_DATA(X);	PELTIER_SET_HEATING;	PELTIER_CON_ON; }
//#define HeaterPowerOff(X)		{ PELTIER_DATA(X);	PELTIER_SET_HEATING;	PELTIER_CON_OFF;}
#define HeaterPowerOff()		{ PELTIER_DATA(0);	PELTIER_SET_HEATING;	PELTIER_CON_OFF;}

///*FAN_1_DATA(X);*/ FAN_1_CON_OFF; }

#define CoolerPowerOn(X)		{ PELTIER_DATA(X);	PELTIER_SET_COOLING;	PELTIER_CON_ON;	}
#define CoolerPowerOff()		{ PELTIER_DATA(0);	PELTIER_SET_COOLING;	PELTIER_CON_OFF;}

#define FAN_1_CON_ON				pwnControl |=	0x00000004; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)
#define FAN_1_CON_OFF				pwnControl &=	0xfffffffb; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)

#define FAN_2_CON_ON				pwnControl |=	0x00000008; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)
#define FAN_2_CON_OFF				pwnControl &=	0xfffffff7; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)

#define FAN_1_DATA(X)		*((uint32_t volatile *)(PWM_CON_BASE_ADDR + S3_PWM3_OFFSET)) = (X)
#define FAN_2_DATA(X)		*((uint32_t volatile *)(PWM_CON_BASE_ADDR + S4_PWM4_OFFSET)) = (X)

#define FanConOn(X)					{ FAN_1_DATA(X); FAN_1_CON_ON; }
#define FanConOff()				{ FAN_1_DATA(0); FAN_1_CON_OFF; }

#define PUMP_CON_ON			pwnControl |=	0x00000008; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)
#define PUMP_CON_OFF		pwnControl &=	0xfffffff7; *((uint32_t volatile *)(PWM_CON_BASE_ADDR + S5_CTRL_OFFSET)) = (pwnControl)

#define PUMP_DATA(X)		*((uint32_t volatile *)(PWM_CON_BASE_ADDR + S4_PWM4_OFFSET)) = (X)

#define PumpConOn(X)					{ PUMP_DATA(X); PUMP_CON_ON; }
//#define PumpConOff()				{ PUMP_DATA(255); PUMP_CON_OFF; }
#define PumpConOff()				{ PUMP_DATA(0); PUMP_CON_OFF; }
#endif

#define PUMP_POWER_MAX				255
#define PUMP_POWER_MID				120 //150	// 200
#define PUMP_POWER_LOW				100	// 150
#define PUMP_POWER_MIN				0


#if SOC_TYPE_SF2
/*
#define POW_BTN_ENABLE_CMD(X)		*((uint32_t volatile *)(W0_POWER_BTN)) = (X)
#define POW_CON_5V_IN_CMD(X)		*((uint32_t volatile *)(W0_POWER_BTN)) = ((X)<<3)
// Power button 은 항상 Enable 상태로 
#define POW_CON_5V_CMD(X)		*((uint32_t volatile *)(W0_POWER_BTN)) = (((X)<<3) | 1)

#define POW_BTN_DET_CLR()		*((uint32_t volatile *)(T1_POW_BTN_DET_CLR)) = (1)
#define POW_BTN_TIME(X)		*((uint32_t volatile *)(W2_POWER_BTN_TIME)) = (X)



#define FLED_1_CONTROL(X)		*((uint32_t volatile *)(W6_FLED_COLOR_TIME1)) = (X)
#define FLED_2_CONTROL(X)		*((uint32_t volatile *)(W7_FLED_COLOR_TIME2)) = (X)
#define FLED_3_CONTROL(X)		*((uint32_t volatile *)(W8_FLED_COLOR_TIME3)) = (X)

#define FLED_CFG_LOAD_TOUCH()		*((uint32_t volatile *)(T5_FLED_CONFIG_LOAD)) = (1)
*/
#else		// SF
#define POW_BTN_ENABLE_CMD(X)		*((uint32_t volatile *)(W0_POWER_BTN)) = (X)
#define POW_CON_5V_IN_CMD(X)		*((uint32_t volatile *)(W0_POWER_BTN)) = ((X)<<3)
// Power button 은 항상 Enable 상태로 
#define POW_CON_5V_CMD(X)		*((uint32_t volatile *)(W0_POWER_BTN)) = (((X)<<3) | 1)

#define POW_BTN_DET_CLR()		*((uint32_t volatile *)(T1_POW_BTN_DET_CLR)) = (1)
#define POW_BTN_TIME(X)		*((uint32_t volatile *)(W2_POWER_BTN_TIME)) = (X)



#define FLED_1_CONTROL(X)		*((uint32_t volatile *)(W6_FLED_COLOR_TIME1)) = (X)
#define FLED_2_CONTROL(X)		*((uint32_t volatile *)(W7_FLED_COLOR_TIME2)) = (X)
#define FLED_3_CONTROL(X)		*((uint32_t volatile *)(W8_FLED_COLOR_TIME3)) = (X)

#define FLED_CFG_LOAD_TOUCH()		*((uint32_t volatile *)(T5_FLED_CONFIG_LOAD)) = (1)

#endif


void led_initialization();

void (*pLed_Control_Fn) (); // 함수포인터 선언
void led_Control();
void led_Control_Blank();


void InitialLED(void);
void PowerOff_LED(void);

void LED_Act_None();
void LED_Act_Initiaize();
void LED_Act_Ready();
void LED_Act_Run();
void LED_Act_Fault();
void LED_Act_Fault_Blink();
void LED_Act_Standby();
void LED_Act_Diagnostics();
void LED_Act_Adjust();

void LED_Act_Connect();
void LED_Act_NotConnect();







//void ledConnect(unsigned char cmd);
void ledReadyRun(unsigned char cmd);


void LedStateFunc();

void LedState_Gradient();
void LedState_Initializing();
void LedState_Readyinit();
void LedState_Halt();
void LedState_Diagnosis();
void LedState_Fault();
void LedState_Service();



void LED_ControlData(unsigned int data1, unsigned int data2, unsigned int data3);
//void LED_ControlState(unsigned int state);
void LED_ControlConnect(unsigned int state);

//void LED_Control_Error(unsigned int who);

/*
void LED_Control(unsigned int color1, unsigned int color2, unsigned int color3, unsigned int time);
void LED_Control_1(unsigned int color, unsigned int time);
void LED_Control_2(unsigned int color, unsigned int time);
void LED_Control_3(unsigned int color, unsigned int time);
void LED_Control_Off();
*/
















#endif	// #ifndef LED_TASK_H_

