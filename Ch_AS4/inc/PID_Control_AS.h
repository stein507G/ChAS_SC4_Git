 /**
  ******************************************************************************
  * @file    ADS1259_Ctrl.h
  * @date    07/22/2016
  * @brief   This file contains all the functions prototypes for the net_AS.c
  *          file.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADS1259_CTRL_H
#define __ADS1259_CTRL_H

#ifdef __cplusplus
 extern "C" {
#endif

#define ADS1259_SPI_INITCMD_CNT					11
#define ADS1259_RDATAC									0x10
#define ADS1259_SDATAC									0x11
#define ADS1259_SPI_DUMMY_DATA_8BIT			0x00

#define ADS1259_DATA_RATE_10SPS			0
#define ADS1259_DATA_RATE_16_6SPS		1
#define ADS1259_DATA_RATE_50SPS			2
#define ADS1259_DATA_RATE_60SPS			3
#define ADS1259_DATA_RATE_400SPS		4
#define ADS1259_DATA_RATE_1200SPS		5
#define ADS1259_DATA_RATE_3600SPS		6
#define ADS1259_DATA_RATE_14400SPS		7
//#define ADS1259_DATA_RATE_400SPS		1
//#define ADS1259_DATA_RATE_1200SPS		2
//#define ADS1259_DATA_RATE_3600SPS		3
#define ADS1259_DATA_RATE					ADS1259_DATA_RATE_10SPS
#if	ADS1259_DATA_RATE	== ADS1259_DATA_RATE_10SPS
	#define TEMP_CTRL_COUNT						5
#elif	ADS1259_DATA_RATE	== ADS1259_DATA_RATE_16_6SPS	
	#define TEMP_CTRL_COUNT						8
#elif	ADS1259_DATA_RATE	== ADS1259_DATA_RATE_50SPS	
	#define TEMP_CTRL_COUNT						25
#elif	ADS1259_DATA_RATE	== ADS1259_DATA_RATE_60SPS	
	#define TEMP_CTRL_COUNT						30
#elif	ADS1259_DATA_RATE	== ADS1259_DATA_RATE_400SPS
	#define GEN_200HZ_ADC_DATA_CNT		2
	#define TEMP_CTRL_COUNT						200
#elif	ADS1259_DATA_RATE == ADS1259_DATA_RATE_1200SPS
	#define GEN_200HZ_ADC_DATA_CNT		6
	#define TEMP_CTRL_COUNT						600	
#elif	ADS1259_DATA_RATE	== ADS1259_DATA_RATE_3600SPS
	#define GEN_200HZ_ADC_DATA_CNT		18
	#define TEMP_CTRL_COUNT						1800	
#endif

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


#define ADS1259_nRESET_PIN_LOW		
#define ADS1259_nRESET_PIN_HIGH		

#define ADS1259_nCS_PIN_LOW				
#define ADS1259_nCS_PIN_HIGH			

#define ADS1259_START_PIN_LOW			Mss_gpio_pattern &= 0xFF7FFFFF; MSS_GPIO_set_outputs( Mss_gpio_pattern )
#define ADS1259_START_PIN_HIGH		Mss_gpio_pattern |= 0x00800000; MSS_GPIO_set_outputs( Mss_gpio_pattern )

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


void ADS1259_Config_Init();
void ADS1259_read();
void ACE_TempRead();
void ACE_VoltRead();

void (*pPID_Control_Fn) (float , float , uint32_t); // 함수포인터 선언

void PID_Control_Init(float actValue ,float setValue, uint32_t setTempReset);
void PID_Control(float actValue ,float setValue, uint32_t setTempReset);
float pidCompute();

void pidAutoTunningInit(float actValue ,float setValue , uint32_t setTempReset);
void pidAutoTunnCooling(float actValue ,float setValue , uint32_t setTempReset);
void pidAutoTunning(float actValue ,float setValue , uint32_t setTempReset);

void PID_Function_Blank(float actValue ,float setValue ,uint32_t setTempReset);
void PID_Function_Init(float actValue ,float setValue ,uint32_t setTempReset);

void TempControlFunc();

void TempReadyCheck();

#ifdef __cplusplus
}
#endif
#endif //__ADS1259_CTRL_H

