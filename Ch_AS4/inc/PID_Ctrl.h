 /**
  ******************************************************************************
  * @file    ADS1259_Ctrl.h
  * @date    07/22/2016
  * @brief   This file contains all the functions prototypes for the net_AS.c
  *          file.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PID_CTRL_H
#define __PID_CTRL_H

#ifdef __cplusplus
 extern "C" {
#endif

#if 0


#define FLOATING_AVG				0
#define TOTAL_256						1	// 256번 더한 데이터

#if 1
#define ADC_MOVING_EVERAGE					16
#define ADC_MOVING_EVERAGE_CNT_CHECK		0x000f
#define ADC_MOVING_EVERAGE_DIV			4
#define ADC_MOVING_EVERAGE_DIV_FLOAT			16.0f
#else
#define ADC_MOVING_EVERAGE					128
#define ADC_MOVING_EVERAGE_CNT_CHECK		0x007f
#define ADC_MOVING_EVERAGE_DIV			7
#define ADC_MOVING_EVERAGE_DIV_FLOAT			128.0f
#endif

#define ADC_200HZ_MOVING_EVG				8
#define ADC_200HZ_MOVING_EVG_CNT_CHECK		0x0007
#define ADC_200HZ_MOVING_EVG_DIV		3
#define ADC_200HZ_MOVING_EVG_DIV_FLOAT		8.0f



/*

GPIO->GPIO_OUT = value;
MSS_GPIO_23;

            Mss_gpio_pattern = MSS_GPIO_get_outputs();
            Mss_gpio_pattern ^= 0xFFFFFFFF;
            MSS_GPIO_set_outputs( Mss_gpio_pattern );
*/


/* Includes ------------------------------------------------------------------*/

/**************************************************************************/
/* Driver Includes */
/**************************************************************************/


void (*pPID_Control_Fn) (float , float , uint32_t); // 함수포인터 선언

void PID_Control_Init(float actValue ,float setValue, uint32_t setTempReset);
void PID_Control(float actValue ,float setValue, uint32_t setTempReset);
float pidCompute();

void pidAutoTunningInit(float actValue ,float setValue , uint32_t setTempReset);
void pidAutoTunnCooling(float actValue ,float setValue , uint32_t setTempReset);
void pidAutoTunning(float actValue ,float setValue , uint32_t setTempReset);

void PID_Function_Blank(float actValue ,float setValue ,uint32_t setTempReset);
void PID_Function_Init(float actValue ,float setValue ,uint32_t setTempReset);
#endif

#ifdef __cplusplus
}
#endif

#if	AUTO_TUNNING_TARGET == HEATING_SYSTEM
#define AUTO_TUNNING_SET_TEMP	40.0		// Heating
#else
//#define AUTO_TUNNING_SET_TEMP	15.0		// Cooling
#define AUTO_TUNNING_SET_TEMP	6.0		// Cooling - 20191128
#endif

#define AUTO_TUNNING_ERRORSUM	386.0		// 
#define AUTO_TUNNING_FREE_SET_TIME	60.0 // 15.0 * 4	// sec(sampling)
//#define AUTO_TUNNING_FREE_SET_TIME	30.0 // 15.0 * 4	// sec(sampling)
#define AUTO_TUNNING_SUM_TIME	16	// sec(sampling)
#define AUTO_TUNNING_SAMPLING_TIME	0.256
#define AUTO_TUNNING_MAX_POWER	1022

#if MAIN_POWRER_15V
 #define FREE_SET_CAL_HEAT_TEMP		100.
 #define FREE_SET_CAL_COOL_TEMP		-4.0f //0. //20.
 #define FREE_SET_CAL_HEAT_FACTOR		0.08f // 0.1f // 0.06f //0.05
 #define FREE_SET_CAL_COOL_FACTOR		0.07f // 0.05f //0.04
#else
 #define FREE_SET_CAL_HEAT_TEMP		120.
 //#define FREE_SET_CAL_COOL_TEMP		10.
 #define FREE_SET_CAL_COOL_TEMP		0.
 #define FREE_SET_CAL_HEAT_FACTOR		0.1f // 0.06f //0.05
 //#define FREE_SET_CAL_COOL_FACTOR		0.03
 #define FREE_SET_CAL_COOL_FACTOR		0.07f // 0.05f //0.04
 //#define FREE_SET_CAL_FACTOR		0.03
#endif
//#define PID_COOLING_CONTROL_TEMP		25.
#define PID_AMBIENT_TEMP		25.	
#define PID_AMBIENT_TEMP_MAX	28.	// 히팅 만 사용
#define PID_AMBIENT_TEMP_MIN	22. // 쿨링만 사용

//#define PID_COOL_HEAT_CHANGE_CNT	10	// 빈번히 히팅 및 쿨링을 하지 않토록(릴레이사용자제)
#define PID_COOL_HEAT_CHANGE_CNT		0 //5	// 빈번히 히팅 및 쿨링을 하지 않토록(릴레이사용자제)	// 35도에서 약간 흔들림
//#define PID_COOL_HEAT_CHANGE_CNT	7	// 빈번히 히팅 및 쿨링을 하지 않토록(릴레이사용자제)

#define PID_FREE_HEATING_CONT	1
#define PID_FREE_COOLING_CONT	2
#define PID_HEATING_CONT		3
#define PID_COOL_HEAT_CONT		4
#define PID_COOLING_CONT		5

#if MAIN_POWRER_15V
/*
#define FAN_POWER_MAX				204		// 12V
#define FAN_POWER_MID				170 	// 10V
#define FAN_POWER_LOW				120		// 7V
#define FAN_POWER_MIN				0
*/
/*
// FAN 4710KL-04W-B49
#define FAN_POWER_MAX				220		// 11.5
#define FAN_POWER_MID				188		// 9.7
#define FAN_POWER_LOW				150
#define FAN_POWER_MIN				0
*/
// Oven Power(15V)  저항 삽입 - <<AFC1212D-F00>>
#define FAN_POWER_MAX				255		// full
#define FAN_POWER_MID				120		// 170
#define FAN_POWER_LOW				90
#define FAN_POWER_MIN				0

#else
#define FAN_POWER_MAX				255
#define FAN_POWER_MID				220 // 200
#define FAN_POWER_LOW				150
#define FAN_POWER_MIN				0
#endif

/*
#define FAN_POWER_10				255
#define FAN_POWER_9					240
#define FAN_POWER_8					230
#define FAN_POWER_7					220
#define FAN_POWER_6					210
#define FAN_POWER_5					200
#define FAN_POWER_4					190
#define FAN_POWER_3					180
#define FAN_POWER_2					170
#define FAN_POWER_1					160
#define FAN_POWER_0					150

#define AUTO_TUNNING_ERRSUM_MAX			POWER_MAX		// 
#define AUTO_TUNNING_ERRSUM_MIN			POWER_MIN		// 

#define AUTO_TUNNING_POWER_MAX	POWER_MAX
#define AUTO_TUNNING_POWER_MIN	POWER_MIN
*/

#define HEATING_FAN_OFF_CONTROL				0

#define PID_PI_CON				0		
#define PID_PID_CON				1

//#define PID_METHOD				PID_PID_CON
#define PID_METHOD				PID_PI_CON
#define PID_SAMPLING_TIME		0.2	

#if PID_METHOD == PID_PI_CON
	#define PID_KP_FACTOR		0.9f
	#define PID_PI_FACTOR		3.3f	
	#define PID_PD_FACTOR		0.0f		
#elif PID_METHOD == PID_PID_CON
	#define PID_KP_FACTOR		1.2f
	#define PID_PI_FACTOR		2.0f	
	#define PID_PD_FACTOR		0.5f		
#endif

#define USE_PID_FUNCTON2		1

#define USE_FREE_SET_TEMP		1 // 0


#define PID_K_PARA			0.303f //0.303f	//100도  //90도  0.254f
#define PID_A_PARA			255.0f
#define PID_Tau_PARA		982.0f //982.0f	//100도  //90도	844.5f
//#define PID_L_PARA			131.0f	//100도  //90도  39.5f - hunting
//#define PID_L_PARA			100.0f 	//100도  //90도  39.5f - hunting
//#define PID_L_PARA			80.0f 	//100도  //90도  39.5f - 낮은 온도 hunting  
#define PID_L_PARA			80.0f // 60.0f 	//100도  //90도  39.5f

#if PWM_CON_1024_SCALE
// 현재 단순히 4배만 
#define POWER_MAX						1023
#define POWER_MIN						-1023
#else
#define POWER_MAX						255
#define POWER_MIN						-255
#endif
#define HEATING_POWER_MAX		POWER_MAX
#define COOLING_POWER_MAX		POWER_MAX

#define PID_VALUE1_MAX	25.0f	
#define PID_VALUE1_MIN	-25.0f

#define PID_VALUE_MAX		255.0f
#define PID_VALUE_MIN		-255.0f


#define ERROR_SUM_RESET_RANGE	0.7






typedef struct _Pid_para_t
{
	float Kp;
	float Ts;
	float Ti;
	float Td;

	float Ki;
	float Kd;
} Pid_para_t;

typedef struct _Pid_con_t
{

} Pid_con_t;

void PID_Control_Init();

void pidParaSet_New(float setValue);
void pidParaSet_Grad(float setValue);

void tempControl(float actValue ,float setValue, unsigned char setTempReset);

void cooling( int pidValue );
void heating( int pidValue );
void heatingOff();
void coolingOff();
void fanOff();

//	void fanControl();
float pidCompute();


void TempControlFunc();

#endif //__PID_CTRL_H

