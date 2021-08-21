//==============================================================================
//==============================================================================

/**************************************************************************/
/* Standard Includes */
/**************************************************************************/

#include "config.h"

//#include <stdio.h>
//#include <stdlib.h>

/**************************************************************************/
/* Driver Includes */
/**************************************************************************/
//#include "./drivers/mss_ace/mss_ace.h"
//#include "mss_uart.h"
//#include "./drivers/mss_spi/mss_spi.h"
//#include "./drivers/mss_GPIO/mss_gpio.h"

// ryu
//#include "def_com.h"
//#include "define_AS.h"
//#include "debug_inform.h"


//#include "../common_inc/Def_stepCon.h"
//#include "../common_inc/protocolType_AS.h"
//#include "../common_inc/comType_AS.h"

#include "net_AS.h"
#include "PID_Control_AS.h"

//#include "main.h"
#include "led_task.h"


//float fVolt[NB_OF_ACE_CHANNEL_HANDLES];

/*
// Autotunning 실행 
#define AUTO_TUNNING_CONTROL		1

#define HEATING_SYSTEM					0
#define COOLING_SYSTEM					1
//#define AUTO_TUNNING_TARGET			HEATING_SYSTEM
#define AUTO_TUNNING_TARGET			COOLING_SYSTEM

#if	AUTO_TUNNING_TARGET == HEATING_SYSTEM
#define AUTO_TUNNING_SET_TEMP	40.0		// Heating
#else
#define AUTO_TUNNING_SET_TEMP	15.0		// Cooling
#endif
*/

#define AUTO_TUNNING_ERRORSUM	386.0		// 

//#define AUTO_TUNNING_FREE_SET_TIME	60.0 // 15.0 * 4	// sec(sampling)
#define AUTO_TUNNING_FREE_SET_TIME	30.0 // 15.0 * 4	// sec(sampling)
#define AUTO_TUNNING_SUM_TIME	16	// sec(sampling)

//#define AUTO_TUNNING_SAMPLING_TIME		0.5
#define AUTO_TUNNING_SAMPLING_TIME		0.2		// 20191127

#define POWER_MAX						255
#define POWER_MIN						-255
#define HEATING_POWER_MAX		POWER_MAX
#define COOLING_POWER_MAX		POWER_MAX

#define PELTIER_POWER_MAX		255
#define FAN_POWER_MAX				255
#define FAN_POWER_MID				200
#define FAN_POWER_LOW				150
#define FAN_POWER_MIN				0

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

typedef struct _Pid_para_t
{
	float Kp;
	float Ts;
	float Ti;
	float Td;

	float Ki;
	float Kd;
} Pid_para_t;

Pid_para_t pid;
Pid_para_t pid1;

#define AUTO_TUNNING_ERRSUM_MAX			POWER_MAX		// 
#define AUTO_TUNNING_ERRSUM_MIN			POWER_MIN		// 

#define AUTO_TUNNING_POWER_MAX	POWER_MAX
#define AUTO_TUNNING_POWER_MIN	POWER_MIN







#define PIDAUTO_POW_FULL					1
#define PIDAUTO_POW_OFF						2
#define PIDAUTO_POW_FULL_2ND			3
#define PIDAUTO_PID_PARA_COMPUTE	4
#define PIDAUTO_PID_CONTROL				5

#define PID_COOL_HEAT_CHANGE_CNT	5	// 빈번히 히팅 및 쿨링을 하지 않토록(릴레이사용자제)	// 35도에서 약간 흔들림

#define PID_FREE_HEATING_CONT	1
#define PID_FREE_COOLING_CONT	2
#define PID_HEATING_CONT			3
#define PID_COOL_HEAT_CONT		4
#define PID_COOLING_CONT			5

//#define FREE_SET_CAL_HEAT_TEMP		120.
#define FREE_SET_CAL_HEAT_TEMP		62.		// AutoSampler 최대온도 60도 
//#define FREE_SET_CAL_COOL_TEMP		10.
#define FREE_SET_CAL_COOL_TEMP		0.
#define FREE_SET_CAL_FACTOR		0.03
//#define FREE_SET_CAL_FACTOR		0.02
//#define FREE_SET_FACTOR		1.03

#define FREE_SET_CAL_HEAT_FACTOR		0.04
#define FREE_SET_CAL_COOL_FACTOR		0.03

	float pidValue=0.0;
	float pidValue1=0.0;
	float Error0,Error1,Error2,ErrorSumCoolInit[10];// ErrorSumDisp,
	float ErrorSum=0;
	float 	ErrorSumHeatInit[10]= {
		-140,//274;	// 0도 - 10도
		-40,//274;	// 10 
		0,	//274;	// 20
		40, //137/2;	// 30
		140, //252/2;	// 220/2	// 40
		240, // 386/2;		// 50 - 60 : 240
		340,//190;	//150	// 60
		440,//230;	//170	// 70
		540,//270;	//190	// 80
		640//310;	//210	// 90 - 100
	};

	
	float ErrorSumMax,ErrorSumMin;
	uint32_t bErrSumReset = NO;

float freeSettemp=100.0;
float tempDiff[AUTO_TUNNING_SUM_TIME] = {0}, tempDiffSum=0.0, tempDiffRatioMax=0.0;	// 최대로 상승되거나 하강하는 점을 찾기위해
float tempActOld=0.0;
float tempErrSumOld=0.0;

//int time=0, timeRatioMax=0;
int timeRatioMax=0;
int timeL0=0, timeL1=0, timeL2=0, timeLS=0, timeLF=0;


extern LCAS_STATE_t LcasState;
extern LCAS_DEV_STATE_t devState;
extern LCAS_RUN_SETUP_t runSetup;

extern LCAS_USED_TIME_t usedTime;
extern LCAS_DIAG_DATA_t diagData;
	
union utData utAdcData;

uint16_t uAdcDataOrgCnt = 0;
uint32_t uAdcDataOrg[ADC_MOVING_EVERAGE] = {0};	// 실제읽은 데이터를 이동평균을 위하여 저장한다.

uint32_t uAdcDataOrgMoveSum = 0;										// 실제읽은 데이터 합

//float gfACETemp;

#define HAL_Delay(X) 			RYU_Delay(X)

#if 0
void ADS1259_Config_Init()
{

	ADS1259_nRESET_PIN_HIGH;
	ADS1259_nCS_PIN_HIGH;
//	HAL_Delay(20);
	ADS1259_START_PIN_HIGH;

	ADS1259_nCS_PIN_LOW;		// SS
//	SPI_WriteByte(ADS1259_spi, ADS1259_SDATAC);	// This command cancels the RDATAC mode. RDATAC mode must be cancelled before the register write commands.
  MSS_SPI_set_slave_select( &g_mss_spi0, MSS_SPI_SLAVE_0 );
  MSS_SPI_transfer_frame( &g_mss_spi0, ADS1259_SDATAC );
  MSS_SPI_clear_slave_select( &g_mss_spi0, MSS_SPI_SLAVE_0 );
	ADS1259_nCS_PIN_HIGH;

//  while(__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_TXE) == RESET){}

//	HAL_Delay(1);
	ADS1259_nCS_PIN_LOW;

#if 0
	switch(SPI_WriteRead(ADS1259_spi, (uint8_t*)aTx_ADS1259_InitCmd, (uint8_t *)aRx_ADS1259_InitCmd, ADS1259_SPI_INITCMD_CNT))
	{
	case HAL_OK:
		/* Communication is completed_____________________________________________*/
		/* Compare the sent and received buffers */
//    if(Buffercmp((uint8_t*)aTx_ADS1259_InitCmd, (uint8_t*)aRx_ADS1259_InitCmd, size))
		{
			/* Transfer error in transmission process */
//      Error_Handler();
		}
		break;

	case HAL_TIMEOUT:
		/* A Timeout occurred______________________________________________________*/
		/* Call Timeout Handler */
//    Timeout_Error_Handler();
		break;

		/* An Error occurred_______________________________________________________*/
	case HAL_ERROR:
		/* Call Timeout Handler */
//    Error_Handler();
		break;

	default:
		break;
	}
#endif

  MSS_SPI_set_slave_select( &g_mss_spi0, MSS_SPI_SLAVE_0 );
  MSS_SPI_transfer_block
		(
		    &g_mss_spi0,                                /* this_spi */
				(uint8_t*)aTx_ADS1259_InitCmd,							//		    &master_tx_buffer[inc],                     /* cmd_buffer */
		    ADS1259_SPI_INITCMD_CNT,										//				COMMAND_BYTE_SIZE + NB_OF_TURNAROUND_BYTES, /* cmd_byte_size */
		    (uint8_t*)aRx_ADS1259_InitCmd,							//				master_rx_buffer,                           /* rd_buffer */
		    ADS1259_SPI_INITCMD_CNT											//				sizeof(master_rx_buffer)                    /* rd_byte_size */
		);

  MSS_SPI_clear_slave_select( &g_mss_spi0, MSS_SPI_SLAVE_0 );

	ADS1259_nCS_PIN_HIGH;

//	HAL_Delay(1);

	ADS1259_nCS_PIN_LOW;
//	SPI_WriteByte(ADS1259_spi, ADS1259_RDATAC);
  MSS_SPI_set_slave_select( &g_mss_spi0, MSS_SPI_SLAVE_0 );
  MSS_SPI_transfer_frame( &g_mss_spi0, ADS1259_RDATAC );
  MSS_SPI_clear_slave_select( &g_mss_spi0, MSS_SPI_SLAVE_0 );
	ADS1259_nCS_PIN_HIGH;

}

/*
#define TEMP_OFFSET_VOLT 0.40847485986781561114364594662428
#define TEMP_CONST_A		 449.3223458545971722580105412864
#define TEMP_CONST_B 		-2.091525140132184388856354053376
#define TEMP_CONST_C 		-199.3223458545971722580105412864
#define TEMP_CONST_C 		-14351.208901530996402576758972621
#define TEMP_CONST_B 		179999.99999999999999999999999992
*/

//#define TEMP_MULTIPLE 	72

/*
#define TEMP_CONST_A 		-1100.0
#define TEMP_CONST_B 		-180000.0
#define TEMP_CONST_C 		-150.58981008951727599765749184307
//  ohm = TEMP_CONST_A + TEMP_CONST_B / (Vo + TEMP_CONST_C)
*/

#if 0
// 2차식
#define TEMP_CONST_A 		2.28116E-14
#define TEMP_CONST_B 		6.00871E-06
#define TEMP_CONST_C 		-1.04984E+01
#else
// 1차식
#define TEMP_CONST_A		6.38445E-06
#define TEMP_CONST_B		-1.14115E+01

#define TEMP_CONST_C		-1.14115E+02
#define TEMP_CONST_D		-2.14115E+02

// 평균을 내지 않고 총합에서 바로 온도 계산
#define TEMP_CONST_A1		3.99028E-07	// = 6.38445E-06 / 16
#endif
#endif

#if 1
void pidAutoTunnCooling(float actValue ,float setValue ,uint32_t setTempReset)
{
	static uint32_t first = YES;
	static unsigned char ctrl=PIDAUTO_POW_FULL;
	static float freeSettemp=0.0;	//35.0;
	
	static int time=0;

	uint32_t	target = AUTO_TUNNING_TARGET;

//	gbPIDCtrlTick = NO;
	
	if(first == YES) {	
		tempActOld = actValue;
		first = NO;
	dp("========================================\n") ;
	}
	
	tempDiffSum -= tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)];
//	tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)] = actValue-tempActOld;	// Heating 
	tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)] = tempActOld-actValue;		// Cooling
	tempDiffSum += tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)];
//	printf("[%01d] t=[%04d] L0=[%04d] LS=[%04d] LF=[%04d]",ctrl,time,timeL0,timeLS,timeLF);
	dp("[%d] pid=[%4d] act=[%8d] set = [%8d] free=[%8d] max=[%8d] \n", ctrl,(int)pidValue,(int)(actValue*100),(int)(setValue*100),(int)(freeSettemp*100),(int)(tempDiffRatioMax*100));
//saveDatafile(actValue);

	switch(ctrl) {
		case PIDAUTO_POW_FULL:	// power full  ---> freeSetTemp계산 ---> freeSetTemp에서 파워오프 , timeL0 기록--->
			CoolerPowerOn(AUTO_TUNNING_POWER_MAX);
			FanConOn(FAN_POWER_MAX);
			if(tempDiffSum > tempDiffRatioMax) {
				tempDiffRatioMax = tempDiffSum;
				timeRatioMax = time;

//				freeSettemp = AUTO_TUNNING_SET_TEMP - tempSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;
//				freeSettemp = setValue - tempDiffSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;		// Heating 
				freeSettemp = setValue + tempDiffSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;		// Cooling
dp("actValue = [%8d] freeSettemp = [%8d] tempRatioMax = [%8d] \n",(int)(actValue*100),(int)(freeSettemp*100),(int)(tempDiffRatioMax*100));
			}

//			if(freeSettemp < actValue) {// Heating 
			if(freeSettemp > actValue) {	// Cooling
				CoolerPowerOff();
				FanConOn(FAN_POWER_MAX);				
				timeL0 = time;
	dp("========================================\n[%1d] act = [%8d] timeL0 = [%8d]\n",ctrl,(int)(actValue*100),(int)(timeL0*100)) ;
				ctrl = PIDAUTO_POW_OFF;

			}
			break;
		case PIDAUTO_POW_OFF:	// power off 후 오버슈트가 되었다가 다시 freeSetTemp까지 내려오면 timeL1기록 --->
				//	power full ---> freeSetTemp에서 파워오프 , timeL0 기록--->
//			if(freeSettemp > actValue) {// Heating 
			if(freeSettemp < actValue) {	// Cooling
				CoolerPowerOn(AUTO_TUNNING_POWER_MAX);
				FanConOn(FAN_POWER_MAX);

				timeL1 = time - timeL0;
				timeLS = timeL1/2;

//				pid1.Kp = 1.2/(tempRatioMax/AUTO_TUNNING_SUM_TIME*timeLS);
				pid1.Kp = 1.2/(tempDiffRatioMax/(AUTO_TUNNING_SUM_TIME*AUTO_TUNNING_SAMPLING_TIME)*timeLS) * AUTO_TUNNING_POWER_MAX;	// 1초간 최대상승 레이트는 tempRatioMax/(AUTO_TUNNING_SUM_TIME*pid.Ts)
				pid1.Ti = 2*timeLS ;
				pid1.Ki = pid1.Ts/pid1.Ti;
				pid1.Td = 0.5*timeLS ;
				pid1.Kd = pid1.Td /pid1.Ts;
				ctrl = PIDAUTO_POW_FULL_2ND;
#ifdef TEST_WRITE_FILE_OVEN
				fprintf(file, "actValue = [%08f] freeSettemp = [%08f] tempRatioMax = [%08f]  timeLS = [%04d] \n",actValue,freeSettemp,tempDiffRatioMax,timeLS) ;
				fprintf(file, "pid1.KP = [%08f]  pid1.Ti = [%08f] pid1.Td = [%08f]\n",pid1.Kp,pid1.Ti,pid1.Td) ;
#endif
				dp("[%d] actValue = [%8d] freeSettemp = [%8d] tempRatioMax = [%8d]  timeLS = [%4d] \n",ctrl,(int)(actValue*100),(int)(freeSettemp*100),(int)(tempDiffRatioMax*100),(int)(timeLS*100));
				dp("pid1.KP = [%8d]  pid1.Ti = [%8d] pid1.Td = [%8d]\n",(int)(pid1.Kp*100),(int)(pid1.Ti*100),(int)(pid1.Td*100)) ;

			}
			break;
		case PIDAUTO_POW_FULL_2ND:
//			if(freeSettemp < actValue) {// Heating 
			if(freeSettemp > actValue) {	// Cooling
				timeL2 = time - timeL0;
				timeLF = timeL2/4;

				tempDiffRatioMax = 0;
				timeRatioMax = 0;
				tempErrSumOld = 0;
				ctrl = PIDAUTO_PID_PARA_COMPUTE;
	dp("========================================\n[%1d] act = [%8d] timeL0 = [%8d]\n",ctrl,(int)(actValue*100),(int)(timeL0*100)) ;				
			}
			break;
		case PIDAUTO_PID_PARA_COMPUTE:
			if(tempDiffSum > tempDiffRatioMax) {
				tempDiffRatioMax = tempDiffSum;
				timeRatioMax = time;

//				freeSettemp = AUTO_TUNNING_SET_TEMP - tempSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;
//				freeSettemp = setValue - tempDiffSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;		// Heating 
				freeSettemp = setValue + tempDiffSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;		// Cooling
			}
//			if(freeSettemp < actValue) {// Heating 
			if(freeSettemp > actValue) {	// Cooling
 				pid.Kp = 1.2/(tempDiffRatioMax/(AUTO_TUNNING_SUM_TIME*AUTO_TUNNING_SAMPLING_TIME)*timeLF) * AUTO_TUNNING_POWER_MAX;	// 1초간 최대상승 레이트는 tempRatioMax/(AUTO_TUNNING_SUM_TIME*pid.Ts)
				pid.Ti = 2*timeLF ;
				pid.Ki = pid.Ts/pid.Ti ;
				pid.Td = 0.5*timeLF ;
				pid.Kd = pid.Td/pid.Ts ;

				ctrl = PIDAUTO_PID_CONTROL;
#ifdef TEST_WRITE_FILE_OVEN
				fprintf(file, "actValue = [%08f] freeSettemp = [%08f] tempRatioMax = [%08f]  timeLF = [%04d] \n",actValue,freeSettemp,tempDiffRatioMax,timeLF) ;
				fprintf(file, "pid.KP = [%08f]  pid.Ti = [%08f] pid.Td = [%08f]\n",pid.Kp,pid.Ti,pid.Td) ;
#endif
				dp("[%d] actValue = [%8d] freeSettemp = [%8d] tempRatioMax = [%8d]  timeLF = [%4d] \n",ctrl,(int)(actValue*100),(int)(freeSettemp*100),(int)(tempDiffRatioMax*100),(int)(timeLF*100));
				dp("pid.KP = [%8d]  pid.Ti = [%8d] pid.Td = [%8d]\n",(int)(pid.Kp*100),(int)(pid.Ti*100),(int)(pid.Td*100)) ;

			}
			break;
		case PIDAUTO_PID_CONTROL:
//			Error0 = AUTO_TUNNING_SET_TEMP - actValue;
			Error0 = setValue - actValue;
			ErrorSum += Error0;
//			ErrorSumDisp += Error0;
			dp("[%d] E0=[%8d] E1=[%8d] Esum=[%8d]",ctrl,(int)(Error0*100),(int)(Error1*100),(int)(ErrorSum*100));

#if 1
			if(ErrorSum > AUTO_TUNNING_ERRSUM_MAX) ErrorSum = AUTO_TUNNING_ERRSUM_MAX;
			else if(ErrorSum < AUTO_TUNNING_ERRSUM_MIN) ErrorSum = AUTO_TUNNING_ERRSUM_MIN;
			pidValue = pid.Kp * (	(Error0) +
									pid.Ki * ErrorSum +
									pid.Kd * (Error0-Error1)
								);
#else
			pidValue1 = pid.Kp * (	(Error0-Error1) +
									pid.Ki * Error0 +
									pid.Kd * (Error0-2*Error1+Error2)
								);
			if(pidValue1>50.0) pidValue1=50.0;
			else if(pidValue1<-50.0) pidValue1=-50.0;

			pidValue+=pidValue1;
#endif
			Error2=Error1;
			Error1=Error0;

#if AUTO_TUNNING_TARGET == COOLING_SYSTEM
// Cooling System
			if(pidValue < AUTO_TUNNING_POWER_MIN) {
				pidValue = AUTO_TUNNING_POWER_MIN;	//  하한값
				CoolerPowerOn(COOLING_POWER_MAX);
				FanConOn(COOLING_POWER_MAX);
			}
			else if(pidValue>0) {
				pidValue=0;
				CoolerPowerOff();
				FanConOff();
			}
			else {
				CoolerPowerOn((int)(-pidValue));
//				FanConOn((int)(-pidValue*0.4 + 153));
				FanConOn((int)(-pidValue*0.3 + 178.5));
			}
			
//			if(actValue > AUTO_TUNNING_SET_TEMP) {
/*
			if(actValue > (setValue +1.5)) {
				FanConOn(FAN_POWER_MAX);
			}
			else if(actValue > setValue) {
				FanConOn(FAN_POWER_MID);				
			}
			else {
				FanConOff();
			}
*/
//			CoolerPowerOn(-pidValue);
//			FanConOn(-pidValue);
			
#else		// HEATING_SYSTEM
// Heating System
			if(pidValue < -0.) {
				pidValue=0.;	//  하한값
			}
			else {
				if(pidValue>AUTO_TUNNING_POWER_MAX) {
					pidValue=AUTO_TUNNING_POWER_MAX;	// 200. 상한값
				}
			}

//			if(actValue > AUTO_TUNNING_SET_TEMP) {
			if(actValue > (setValue +1.5)) {
				FanConOn(FAN_POWER_MAX);
			}
			else if(actValue > setValue) {
				FanConOn(FAN_POWER_MID);				
			}
			else {
				FanConOff();
			}

			HeaterPowerOn((int)(pidValue));
			FanConOff();
			
#endif
		
			break;
	}				
//dp("================ctrl =[%d]\n",ctrl);
	time++;
	tempActOld = actValue;
	tempErrSumOld = tempDiffSum;
}
#endif

//=======================================================================================================
// Auto tunning result
//=======================================================================================================
#if 0
// 쿨링FAN도 펠티어와 같은 파워(PWM)로 동작
// set 15도 : 시작온도 33도
#define AUTO_TUNNING_FREE_SET_TIME	30.0 // 15.0 * 4	// sec(sampling)
#define AUTO_TUNNING_SUM_TIME	16	// sec(sampling)
#define AUTO_TUNNING_SAMPLING_TIME		0.5

[3] actValue = [    1701] freeSettemp = [    1697] tempRatioMax = [     105]  timeLS = [1400]
pid1.KP = [   16559]  pid1.Ti = [    2800] pid1.Td = [     700]

[4] pid=[   0] act=[    1607] set = [    1500] free=[    1609] max=[      58]
[5] actValue = [    1607] freeSettemp = [    1609] tempRatioMax = [      58]  timeLF = [1400]
pid.KP = [   29831]  pid.Ti = [    2800] pid.Td = [     700]

// 쿨링FAN도 펠티어와 같은 파워(PWM)로 동작
// set 15도 : 시작온도 38.6도
[3] actValue = [    1724] freeSettemp = [    1722] tempRatioMax = [     118]  timeLS = [1400]
pid1.KP = [   14767]  pid1.Ti = [    2800] pid1.Td = [     700]
uTemp = 0x00000571 temp =     1724

[5] actValue = [    1604] freeSettemp = [    1605] tempRatioMax = [      56]  timeLF = [1400]
pid.KP = [   31080]  pid.Ti = [    2800] pid.Td = [     700]
uTemp = 0x00000542 temp =     1604


// 20191128 - heating
pid1.KP = [     838]  pid1.Ti = [    4400] pid1.Td = [    1100]
[5] actValue = [    3653] freeSettemp = [    3652] tempRatioMax = [     185]  timeLF = [1800]
pid.KP = [    2937]  pid.Ti = [    3600] pid.Td = [     900]
#endif
//==========================================================

#if 1
void pidAutoTunning(float actValue ,float setValue,uint32_t setTempReset)
{
	static uint32_t first = YES;
	static unsigned char ctrl=PIDAUTO_POW_FULL;
	static float freeSettemp=100.0;	//35.0;
	
	static int time=0;

//	gbPIDCtrlTick = NO;
	
	if(first == YES) {	
		tempActOld = actValue;
		first = NO;
	dp("========================================\n") ;
	}
	
	tempDiffSum -= tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)];
	tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)] = actValue-tempActOld;
	tempDiffSum += tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)];
//	printf("[%01d] t=[%04d] L0=[%04d] LS=[%04d] LF=[%04d]",ctrl,time,timeL0,timeLS,timeLF);
	dp(" pid=[%4d] act=[%8d] set = [%8d] free=[%8d] max=[%8d] \n", (int)pidValue,(int)(actValue*100),(int)(setValue*100),(int)(freeSettemp*100),(int)(tempDiffRatioMax*100));
//saveDatafile(actValue);

	switch(ctrl) {
		case PIDAUTO_POW_FULL:	// power full  ---> freeSetTemp계산 ---> freeSetTemp에서 파워오프 , timeL0 기록--->
			HeaterPowerOn(AUTO_TUNNING_POWER_MAX);
			FanConOff();

			if(tempDiffSum > tempDiffRatioMax) {
				tempDiffRatioMax = tempDiffSum;
				timeRatioMax = time;

//				freeSettemp = AUTO_TUNNING_SET_TEMP - tempSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;
				freeSettemp = setValue - tempDiffSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;
dp("actValue = [%8d] freeSettemp = [%8d] tempRatioMax = [%8d] \n",(int)(actValue*100),(int)(freeSettemp*100),(int)(tempDiffRatioMax*100));
			}

			if(freeSettemp < actValue) {
				HeaterPowerOff();				
//				FanConOff();
				FanConOn(FAN_POWER_MAX);				
				timeL0 = time;
	dp("========================================\n[%1d] act = [%8d] timeL0 = [%8d]\n",ctrl,(int)(actValue*100),(int)(timeL0*100)) ;
				ctrl = PIDAUTO_POW_OFF;

#if 0
pid.Kp = 220.278946;   
	pid.Ti = 2*timeLF ;
pid.Ti = 50.000000;
	pid.Ki = pid.Ts/pid.Ti ;
	pid.Td = 0.5*timeLF ;
pid.Td = 13.500000;
	pid.Kd = pid.Td/pid.Ts ;
	ctrl = 5;
ErrorSum=AUTO_TUNNING_ERRORSUM/2.;
#endif
			}
			break;
		case PIDAUTO_POW_OFF:	// power off 후 오버슈트가 되었다가 다시 freeSetTemp까지 내려오면 timeL1기록 --->
				//	power full ---> freeSetTemp에서 파워오프 , timeL0 기록--->
			if(freeSettemp > actValue) {
				HeaterPowerOn(AUTO_TUNNING_POWER_MAX);
				FanConOff();

				timeL1 = time - timeL0;
				timeLS = timeL1/2;

//				pid1.Kp = 1.2/(tempRatioMax/AUTO_TUNNING_SUM_TIME*timeLS);
				pid1.Kp = 1.2/(tempDiffRatioMax/(AUTO_TUNNING_SUM_TIME*AUTO_TUNNING_SAMPLING_TIME)*timeLS) * AUTO_TUNNING_POWER_MAX;	// 1초간 최대상승 레이트는 tempRatioMax/(AUTO_TUNNING_SUM_TIME*pid.Ts)
				pid1.Ti = 2*timeLS ;
				pid1.Ki = pid1.Ts/pid1.Ti;
				pid1.Td = 0.5*timeLS ;
				pid1.Kd = pid1.Td /pid1.Ts;
				ctrl = PIDAUTO_POW_FULL_2ND;
#ifdef TEST_WRITE_FILE_OVEN
				fprintf(file, "actValue = [%08f] freeSettemp = [%08f] tempRatioMax = [%08f]  timeLS = [%04d] \n",actValue,freeSettemp,tempDiffRatioMax,timeLS) ;
				fprintf(file, "pid1.KP = [%08f]  pid1.Ti = [%08f] pid1.Td = [%08f]\n",pid1.Kp,pid1.Ti,pid1.Td) ;
#endif
				dp("[%d] actValue = [%8d] freeSettemp = [%8d] tempRatioMax = [%8d]  timeLS = [%4d] \n",ctrl,(int)(actValue*100),(int)(freeSettemp*100),(int)(tempDiffRatioMax*100),(int)(timeLS*100));
				dp("pid1.KP = [%8d]  pid1.Ti = [%8d] pid1.Td = [%8d]\n",(int)(pid1.Kp*100),(int)(pid1.Ti*100),(int)(pid1.Td*100)) ;

			}
			break;
		case PIDAUTO_POW_FULL_2ND:
			if(freeSettemp < actValue) {
				timeL2 = time - timeL0;
				timeLF = timeL2/4;

				tempDiffRatioMax = 0;
				timeRatioMax = 0;
				tempErrSumOld = 0;
				ctrl = PIDAUTO_PID_PARA_COMPUTE;
	dp("========================================\n[%1d] act = [%8d] timeL0 = [%8d]\n",ctrl,(int)(actValue*100),(int)(timeL0*100)) ;				
			}
			break;
		case PIDAUTO_PID_PARA_COMPUTE:
			if(tempDiffSum > tempDiffRatioMax) {
				tempDiffRatioMax = tempDiffSum;
				timeRatioMax = time;

//				freeSettemp = AUTO_TUNNING_SET_TEMP - tempSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;
				freeSettemp = setValue - tempDiffSum*AUTO_TUNNING_FREE_SET_TIME/AUTO_TUNNING_SUM_TIME;
			}
			if(freeSettemp < actValue) {
 				pid.Kp = 1.2/(tempDiffRatioMax/(AUTO_TUNNING_SUM_TIME*AUTO_TUNNING_SAMPLING_TIME)*timeLF) * AUTO_TUNNING_POWER_MAX;	// 1초간 최대상승 레이트는 tempRatioMax/(AUTO_TUNNING_SUM_TIME*pid.Ts)
				pid.Ti = 2*timeLF ;
				pid.Ki = pid.Ts/pid.Ti ;
				pid.Td = 0.5*timeLF ;
				pid.Kd = pid.Td/pid.Ts ;

				ctrl = PIDAUTO_PID_CONTROL;
#ifdef TEST_WRITE_FILE_OVEN
				fprintf(file, "actValue = [%08f] freeSettemp = [%08f] tempRatioMax = [%08f]  timeLF = [%04d] \n",actValue,freeSettemp,tempDiffRatioMax,timeLF) ;
				fprintf(file, "pid.KP = [%08f]  pid.Ti = [%08f] pid.Td = [%08f]\n",pid.Kp,pid.Ti,pid.Td) ;
#endif
				dp("[%d] actValue = [%8d] freeSettemp = [%8d] tempRatioMax = [%8d]  timeLF = [%4d] \n",ctrl,(int)(actValue*100),(int)(freeSettemp*100),(int)(tempDiffRatioMax*100),(int)(timeLF*100));
				dp("pid.KP = [%8d]  pid.Ti = [%8d] pid.Td = [%8d]\n",(int)(pid.Kp*100),(int)(pid.Ti*100),(int)(pid.Td*100)) ;

			}
			break;
		case PIDAUTO_PID_CONTROL:
//			Error0 = AUTO_TUNNING_SET_TEMP - actValue;
			Error0 = setValue - actValue;
			ErrorSum += Error0;
//			ErrorSumDisp += Error0;
			dp("[%d] E0=[%8d] E1=[%8d] Esum=[%8d]",ctrl,(int)(Error0*100),(int)(Error1*100),(int)(ErrorSum*100));

#if 1
			if(ErrorSum > AUTO_TUNNING_ERRSUM_MAX) ErrorSum = AUTO_TUNNING_ERRSUM_MAX;
			else if(ErrorSum < AUTO_TUNNING_ERRSUM_MIN) ErrorSum = AUTO_TUNNING_ERRSUM_MIN;
			pidValue = pid.Kp * (	(Error0) +
									pid.Ki * ErrorSum +
									pid.Kd * (Error0-Error1)
								);
#else
			pidValue1 = pid.Kp * (	(Error0-Error1) +
									pid.Ki * Error0 +
									pid.Kd * (Error0-2*Error1+Error2)
								);
			if(pidValue1>50.0) pidValue1=50.0;
			else if(pidValue1<-50.0) pidValue1=-50.0;

			pidValue+=pidValue1;
#endif
			Error2=Error1;
			Error1=Error0;

			if(pidValue < -0.) {
				pidValue=0.;	//  하한값
			}
			else {
				if(pidValue>AUTO_TUNNING_POWER_MAX) {
					pidValue=AUTO_TUNNING_POWER_MAX;	// 200. 상한값
				}
			}

//			if(actValue > AUTO_TUNNING_SET_TEMP) {
			if(actValue > (setValue +1.5)) {
				FanConOn(FAN_POWER_MAX);
			}
			else if(actValue > setValue) {
				FanConOn(FAN_POWER_MID);				
			}
			else {
				FanConOff();
			}

			HeaterPowerOn((int)(pidValue));
			FanConOff();
		
			break;
	}				
//dp("================ctrl =[%d]\n",ctrl);
	time++;
	tempActOld = actValue;
	tempErrSumOld = tempDiffSum;
}
#endif

//=======================================================================================================
// Auto tunning result
//=======================================================================================================
#if 0
// 쿨링의 경우 FAN만 사용 (Full Power)
// set 40도 : 시작온도 27도 상온
#define AUTO_TUNNING_FREE_SET_TIME	30.0 // 15.0 * 4	// sec(sampling)
#define AUTO_TUNNING_SUM_TIME	16	// sec(sampling)
#define AUTO_TUNNING_SAMPLING_TIME		0.5

[3] actValue = [    3595] freeSettemp = [    3596] tempRatioMax = [     215]  timeLS = [5900]
pid1.KP = [    1927]  pid1.Ti = [   11800] pid1.Td = [    2950]

pid=[   0] act=[    3743] set = [    4000] free=[    3739] max=[     139]
[5] actValue = [    3743] freeSettemp = [    3722] tempRatioMax = [     148]  timeLF = [3200]
pid.KP = [    5168]  pid.Ti = [    6400] pid.Td = [    1600]

// 쿨링의 경우 FAN만 사용 (Full Power)
// set 40도 : 시작온도 15도 
[3] actValue = [    3537] freeSettemp = [    3537] tempRatioMax = [     246]  timeLS = [2800]
pid1.KP = [    3546]  pid1.Ti = [    5600] pid1.Td = [    1400]

[5] actValue = [    3707] freeSettemp = [    3701] tempRatioMax = [     159]  timeLF = [1800]
pid.KP = [    8550]  pid.Ti = [    3600] pid.Td = [     900]
#endif
//=======================================================================================================

void pidAutoTunningInit(float actValue ,float setValue, uint32_t setTempReset)
{
//	gbPIDCtrlTick = NO;

	pid1.Ts = AUTO_TUNNING_SAMPLING_TIME;
	pid.Ts = AUTO_TUNNING_SAMPLING_TIME;

// Heating 40도 - 27도 상온
	pid1.Kp = 19.27;
	pid1.Ti = 118.0;
	pid1.Td = 29.5;
// set 40도 : 시작온도 15도 
	pid1.Kp = 35.46;
	pid1.Ti = 56.0;
	pid1.Td = 14.0;

/*
// Cooling 15도 - 33도
	pid1.Kp = 165.59;
	pid1.Ti = 28.0;
	pid1.Td = 7.0;
// Cooling 15도 - 38.5도
	pid1.Kp = 147.67;
	pid1.Ti = 28.0;
	pid1.Td = 7.0;
	
*/

	pid1.Ki = pid1.Ts/pid1.Ti ;
	pid1.Kd = pid1.Td/pid1.Ts ;

// Heating 40도
	pid.Kp = 51.68;
  pid.Ti = 64.0;
	pid.Td = 16.0;
// set 40도 : 시작온도 15도 
	pid.Kp = 85.50;
	pid.Ti = 36.0;
	pid.Td = 9.0;

// 20191128 - heating
//pid1.KP = [     838]  pid1.Ti = [    4400] pid1.Td = [    1100]
//[5] actValue = [    3653] freeSettemp = [    3652] tempRatioMax = [     185]  timeLF = [1800]
//	pid.KP = 29.37;
	pid.Kp = 20.37;
  pid.Ti = 36.00;
	pid.Td = 9.00;

/*
// Cooling 15도 - 33도
	pid.Kp = 298.31;
  pid.Ti = 28.0;
	pid.Td = 7.0;
// Cooling 15도 - 38.5도	
	pid.Kp = 310.80;
  pid.Ti = 28.0;
	pid.Td = 7.0;
	
*/
	pid.Ki = pid.Ts/pid.Ti ;
	pid.Kd = pid.Td/pid.Ts ;

#if	AUTO_TUNNING_TARGET == HEATING_SYSTEM
	pPID_Control_Fn = pidAutoTunning;
//	static float 
	freeSettemp=100.0;	//35.0;
#else
	pPID_Control_Fn = pidAutoTunnCooling;
//	static float 
	freeSettemp=0.0;	//35.0;
#endif
}

void PID_Control_Init(float actValue ,float setValue ,uint32_t setTempReset)
{
//	gbPIDCtrlTick = NO;
	
	pid1.Ts = AUTO_TUNNING_SAMPLING_TIME;
	pid.Ts = AUTO_TUNNING_SAMPLING_TIME;

// Heating 40도 - 27도 상온
	pid1.Kp = 19.27;
	pid1.Ti = 118.0;
	pid1.Td = 29.5;
// set 40도 : 시작온도 15도 
	pid1.Kp = 35.46;
	pid1.Ti = 56.0;
	pid1.Td = 14.0;

/*
// Cooling 15도 - 33도
	pid1.Kp = 165.59;
	pid1.Ti = 28.0;
	pid1.Td = 7.0;
// Cooling 15도 - 38.5도
	pid1.Kp = 147.67;
	pid1.Ti = 28.0;
	pid1.Td = 7.0;
	
*/

	pid1.Ki = pid1.Ts/pid1.Ti ;
	pid1.Kd = pid1.Td/pid1.Ts ;

// Heating 40도
	pid.Kp = 51.68;
  pid.Ti = 64.0;
	pid.Td = 16.0;
// set 40도 : 시작온도 15도 
	pid.Kp = 85.50;
	pid.Ti = 36.0;
	pid.Td = 9.0;

// 위의 중간값으로 테스트 진행(4도)
	pid.Kp = 68.59;	
	pid.Ti = 50.0;	
	pid.Td = 12.5;

// 다시 위의 중간값으로 테스트 진행(4도)
	pid.Kp = 77.45;
	pid.Ti = 43.0;	
	pid.Td = 10.75;	
/*
// Cooling 15도 - 33도
	pid.Kp = 298.31;
  pid.Ti = 28.0;
	pid.Td = 7.0;
// Cooling 15도 - 38.5도	
	pid.Kp = 310.80;
  pid.Ti = 28.0;
	pid.Td = 7.0;
	
*/
//=============================================================================================================
// 20191128 <<....xlsx>>
// Free Set 설정 : 0.03(heating)  0.03(Cooling)
	pid.Kp = 20.37;
  pid.Ti = 20.00;
	pid.Td = 5.00;
// ErrorSumMax : 255
// ErrorSumMin : -255
// 6도 설정안됨. (8.4도 까지)
// 15도 설정안됨. (15.24 까지) 
// 34도 ok // ErrorSumMax : 255

// 4도 - 5.3  // ErrorSumMax : 500
// 20191128 - heating
//pid1.KP = [     838]  pid1.Ti = [    4400] pid1.Td = [    1100]
//[5] actValue = [    3653] freeSettemp = [    3652] tempRatioMax = [     185]  timeLF = [1800]
//	pid.KP = 29.37;
//=============================================================================================================

//=============================================================================================================
// 20191203  <<PP...xlsx>>
// Free Set 설정 : 0.04(heating)  0.03(Cooling)
	pid.Kp = 20.37;
  pid.Ti = 15.00;
	pid.Td = 3.75;

//=============================================================================================================
// 20191203 오버슈트 <<PPP...xlsx>>
// Free Set 설정 : 0.04(heating)  0.03(Cooling)
	pid.Kp = 20.37;
  pid.Ti = 5.00;
	pid.Td = 1.250;

//=============================================================================================================
// 20191203 빨라졌음. <<P...xlsx>>
// Free Set 설정 : 0.04(heating)  0.03(Cooling)
	pid.Kp = 20.37;
  pid.Ti = 10.00;
	pid.Td = 2.50;

	pid.Ki = pid.Ts/pid.Ti ;
	pid.Kd = pid.Td/pid.Ts ;

// Temp Off => PID_Function_Blank 
// Temp On  => PID_Control
//	pPID_Control_Fn = PID_Control;

	pPID_Control_Fn = PID_Function_Blank;

}

#if 0
	Err= ref - set;

	Up = kp * Err;

	Ui = Ui + ki * Up + kc * satErr;

	Ud = kd * (Up - Up1);

	OutPreSat = Up + Ui + Ud;

	if(OutPreSat > OUT_MAX) Out = OUT_MAX;
	else if(OutPreSat < OUT_MIN) Out = OUT_MIN;
	else Out = OutPreSat;

	satErr = Out - OutPreSat;

	Up1 = Up;
#endif

#define ERROR_SUM_RESET_RANGE	0.7
#define ERROR_SUM_RESET_RANGE_MIN	-0.7
#define ERROR_SUM_RESET_RANGE_MAX	0.7
void PID_Control(float actValue ,float setValue, uint32_t setTempReset)
{
	static unsigned char ctrl=PID_FREE_HEATING_CONT;
//	static unsigned char action=0;	// 0:heating , 1: cooling
	static unsigned char coolingCnt=0;
	static unsigned char heatingCnt=0;

	static unsigned int usedTimehCnt = 0;	
	
	float diffValue;
//	gbPIDCtrlTick = NO;

static int col = NO;	
	static int ctriTime = 0;	// 제어시간 카운트
	static int dispTime = 0;	// 시리얼출력 디버깅용
	
//	if(++hCnt == 120) {	// 1분
	if(++usedTimehCnt == 300) {	// 1분	20191127
		usedTimehCnt = 0;
		usedTime.heatTime++;
	}

	if(setTempReset == TEMP_NEW_VALUE) {
		devState.setTempReset = TEMP_NOT_RESET;
		dp("setTempReset ========================================") ;		
 
// 20191128
#if 1
dp("bErrSumReset ===%d  %d  ========",runSetup.tempOnoff,devState.oldTempOnoff);					

		diffValue = setValue - actValue;
		if(diffValue >= 0.0f) {	// 범위를 설정하여야 할 듯하다. - 범위내에 들어오면 ErrorSum를 그대로 유지한다.
			if(diffValue > ERROR_SUM_RESET_RANGE_MAX) bErrSumReset = YES;
			else bErrSumReset = NO;
			ctrl=PID_FREE_HEATING_CONT;
			freeSettemp = setValue - (FREE_SET_CAL_HEAT_TEMP - setValue) * FREE_SET_CAL_HEAT_FACTOR;
//			freeSettemp = setValue - setValue * FREE_SET_CAL_FACTOR;
//			freeSettemp = setValue - (diffValue * FREE_SET_CAL_HEAT_FACTOR);
			tempActOld = actValue;
		}
		else {
			if(diffValue < ERROR_SUM_RESET_RANGE_MIN) bErrSumReset = YES;
			else bErrSumReset = NO;			
			ctrl=PID_FREE_COOLING_CONT;
			freeSettemp = setValue + ((setValue + FREE_SET_CAL_COOL_TEMP) * FREE_SET_CAL_COOL_FACTOR);
//			freeSettemp = setValue + (setValue) * FREE_SET_CAL_FACTOR;
//			freeSettemp = setValue - (diffValue * FREE_SET_CAL_COOL_FACTOR);
			tempActOld = actValue;
		}

		if(runSetup.tempOnoff != devState.oldTempOnoff) {
			devState.oldTempOnoff = runSetup.tempOnoff;
			bErrSumReset = YES;
dp("bErrSumReset = YES ========================================") ;					
		}		

		// 20191129
		if(bErrSumReset == YES)	{
				ctriTime = 0;
	//		ErrorSum = 0.0;
			if(setValue >= 60)	{				ErrorSumMax = 1000.;			ErrorSumMin = -1000.;
				ErrorSum = 330;		}
			else if(setValue >= 55)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;
				ErrorSum = 250;		}
			else if(setValue >= 50)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = 200;		}
			else if(setValue >= 45)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = 160;		}
			else if(setValue >= 40)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = 120;		}
			else if(setValue >= 35)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = 95;		}
			else if(setValue >= 30)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = 60;		}
			else if(setValue >= 25)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = 28;		}		
			else if(setValue >= 20)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = -65;	}
			else if(setValue >= 15)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = -220;	}		
			else if(setValue >= 10)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = -400;	}
			else if(setValue >= 5)	{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.;			
				ErrorSum = -510;	}		
			else										{		ErrorSumMax = 1000.;			ErrorSumMin = -1000.; // -500.; // 255.;
				ErrorSum = -610;
			}
		}

#else
		ctrl=PID_COOL_HEAT_CONT;
#endif

/*
#if 0
		if(setValue > 90)		{ErrorSumMax = 255.; ErrorSumMin = 0.;}
		else if(setValue > 80)	{ErrorSumMax = 255.; ErrorSumMin = -20.;}
		else if(setValue > 70)	{ErrorSumMax = 255.; ErrorSumMin = -40.;}
		else if(setValue > 60)	{ErrorSumMax = 150.; ErrorSumMin = -60.;}
		else if(setValue > 50)	{ErrorSumMax = 150.; ErrorSumMin = -80.;}
		else if(setValue > 40)	{ErrorSumMax = 100.; ErrorSumMin = -100.;}				
		else if(setValue > 30)	{ErrorSumMax = 100.; ErrorSumMin = -150.;}		
		else if(setValue > 20)	{ErrorSumMax = 100.; ErrorSumMin = -200.;}
		else if(setValue > 10)	{ErrorSumMax = 100.; ErrorSumMin = -255.;}
		else										{ErrorSumMax = 100.; ErrorSumMin = -255.;}
#else
// 20191128
		ErrorSumMax = 1000.; // 500.; // 255.;
		ErrorSumMin = -1000.; // -500.; // 255.;
#endif		
*/

// ErrorSum 안정
// pid.Kp = 51.68;  pid.Ti = 64.0;	pid.Td = 16.0;
// 온도: 시작=>끝 : ErrorSum pidValue
// 38 : 33도=>38.81 : 58.49
// 25 : 38도=>24.29 : (-13.48)
// 15 : 25도=>14.74 : (-155.30)
// 10 : 15도=>9.99 : (-255.00) (-103)
//XXXX 4 : 10도=> 5.01: (-255.00) (-170)
//	pid.Kp = 85.50;	pid.Ti = 36.0;	pid.Td = 9.0;
// 4 : 11도=> 3.34: (-150.00) (-180)
// 위 PID의 중간값으로 테스트
//	pid.Kp = 68.59;	pid.Ti = 50.0;	pid.Td = 12.5;
//XXXX 4 : 16도=> 4.25: (-255.00) (-190)
//	pid.Kp = 77.45;	pid.Ti = 43.0;	pid.Td = 10.75;
// 4 : 12도=>3.79 : (-223.00) (-202)
// 35 : 4도=>35.5 : (26.22) (20)				<= EsumMax=100
// 60 : 35도=>60.32 : (76.02) (68)				<= EsumMax=150
// 70 : 60도=>70.57 : (110) (95)				<= EsumMax=200
// 60 : 70도=>58.48 : (72) (60)				<= EsumMax=-150
// 70 : 57.5도=>70.25 : (110) (100)				<= EsumMax=150
// 35 : 67도=>34.25 : (16) (15)				<= EsumMax=-150
// 27 : 35도=>26.01 : (16) (15)				<= EsumMax=-200
// 35 : 27도=>35.55 : (16) (15)				<= EsumMax=100
	}
	else if(setTempReset == TEMP_PROG_MODE) 
	{
		ctrl=PID_COOL_HEAT_CONT;
#if 1
		if(setValue > 90)		{ErrorSumMax = 255.; ErrorSumMin = -100.;}
		else if(setValue > 80)	{ErrorSumMax = 255.; ErrorSumMin = -100.;}
		else if(setValue > 70)	{ErrorSumMax = 255.; ErrorSumMin = -100.;}
		else if(setValue > 60)	{ErrorSumMax = 255.; ErrorSumMin = -150.;}
		else if(setValue > 50)	{ErrorSumMax = 255.; ErrorSumMin = -200.;}
		else if(setValue > 20)	{ErrorSumMax = 255.; ErrorSumMin = -255.;}
		else if(setValue > 10)	{ErrorSumMax = 200.; ErrorSumMin = -255.;}
		else										{ErrorSumMax = 100.; ErrorSumMin = -255.;}
#endif		
				
	}
	
/*	
	tempDiffSum -= tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)];	// 누적에러를 일정 시간 만큼만 적용한다.
	tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)]=actValue-tempActOld;
	tempDiffSum += tempDiff[time & (AUTO_TUNNING_SUM_TIME-1)];
*/

#if 1 //-- TEST
if(++dispTime == 5) { // 1초
dispTime = 0;
ctriTime++;
dp("ct= %4d S-[%1d] act= %5d set= %5d freeSettemp= %5d pid= %6d ErrSum= %6d\n"
,ctriTime,ctrl,(int)(actValue*1000),(int)(setValue*100),(int)(freeSettemp*100),(int)pidValue,(int)(ErrorSum*10));
}
#endif
//	dp("S[%1d] kp = [%8d] Ti = [%6d] Td = [%6d] Ts = [%d]\n",ctrl,(int)(pid.Kp),(int)(pid.Ti),(int)(pid.Td),(int)(pid.Ts*100)) ;
//	dp("S[%1d] E0 = [%8d] E1 = [%6d] ES = [%d]\n",ctrl,(int)(Error0*100),(int)(Error1*100),(int)(ErrorSum*100)) ;
	
	switch(ctrl) {
		case PID_FREE_HEATING_CONT:	// full power 로 freeSetTemp까지 올린다.freeSetTemp에서 PID 제어 시작
			HeaterPowerOn(AUTO_TUNNING_POWER_MAX);
			FanConOff();
			if(freeSettemp < actValue) {
				HeaterPowerOff();				
				FanConOn(FAN_POWER_MAX);				
				
				ctrl=PID_COOL_HEAT_CONT;
#if 0					
				if(bErrSumReset == YES) {// 온도가 에러 범위내면 실행하지 않는다.
					ErrorSum = setValue*10 -260;
				}
#endif				
			}
			break;
		case PID_FREE_COOLING_CONT:	// full power 로 freeSetTemp까지 쿨링한다.freeSetTemp에서 PID 제어 시작
			CoolerPowerOn(AUTO_TUNNING_POWER_MAX);
			FanConOn(FAN_POWER_MAX);

			if(freeSettemp > actValue ) {
				CoolerPowerOff();
				FanConOn(FAN_POWER_MAX);				
//	printf("===PID_FREE_COOLING_CONT==[%1d] act = [%8.4f] timeL0 = [%8.4f]\n",ctrl,actValue,timeL0) ;
//				ctrl = 5;
#if 0
				if(setValue < PID_AMBIENT_TEMP_MIN) { // 쿨링만 사용
					ctrl=PID_COOLING_CONT;
				}
				else  if(setValue > PID_AMBIENT_TEMP_MAX) { // 히팅만 사용
					ctrl=PID_HEATING_CONT;
				}
				else ctrl=PID_COOL_HEAT_CONT;
#else
				ctrl=PID_COOL_HEAT_CONT;
#endif
//			}
#if 0
				if(bErrSumReset == YES) {// 온도가 에러 범위내면 실행하지 않는다.
					ErrorSum = (setValue - 26) * 30;
				}
#endif				
			}
			break;			
		case PID_HEATING_CONT:
			Error0 = setValue - actValue;
			ErrorSum += Error0;
//			ErrorSumDisp += Error0;

			pidValue=pidCompute();

			if(pidValue < -0.) {
				pidValue=0.;	//  하한값
			}
			else {
				if(pidValue>255.) {
					pidValue=255.;	// 200. 상한값
				}
			}

			HeaterPowerOn(pidValue);
			if(actValue > (setValue +1.5)) {	FanConOn(FAN_POWER_MAX);}
			else if(actValue > setValue) {		FanConOn(FAN_POWER_MID);}
			else {														FanConOff();}

			break;
		case PID_COOL_HEAT_CONT:
			Error0 = setValue - actValue;
			ErrorSum += Error0;
//			ErrorSumDisp += Error0;
			pidValue=pidCompute();
// fast cooling 사용여부...

			if(pidValue < -0.) {	// 쿨링 
				if(pidValue < -255.) pidValue = -255.;
				heatingCnt=0;
				if(++coolingCnt > PID_COOL_HEAT_CHANGE_CNT) {	// 펠티어 쿨링을 사용하기 위한변수 - 빈번히 히팅 및 쿨링을 하지 않토록(릴레이사용자제)
					coolingCnt = PID_COOL_HEAT_CHANGE_CNT;
					
					CoolerPowerOn((int)(-pidValue));
#if 0
					FanConOn((int)(-pidValue*0.3 + 178.5));
//					FanConOn(240);
#else
/*
					if(col == 1) {
						col = 0;
						FanConOn(255);
					}
					else {
						col = 1;
						FanConOn(0);
					}
*/					
#endif
				}
				else {	// 릴레이는 그대로 두고 히팅 및 쿨링 파워를 0으로 함.
					CoolerPowerOff();
//					FanConOn((int)(-pidValue*0.3 + 178.5));
					FanConOn(COOLING_POWER_MAX);
				}
			}
			else {
				if(pidValue>255.) pidValue=255.;	// 200. 상한값
				coolingCnt = 0;
				if(++heatingCnt >10) { 
					heatingCnt=10;
					HeaterPowerOn( (int)(pidValue) );
				}
				else {	// 릴레이는 그대로 두고 히팅 및 쿨링 파워를 0으로 함.
					HeaterPowerOff();
				}

				if(actValue > (setValue +1.5)) {	FanConOn(FAN_POWER_MAX);}
//				else if(actValue > setValue) {		FanConOn(FAN_POWER_MID);}
				else if(actValue > (setValue + 0.7)) {		FanConOn(FAN_POWER_MID);}
				else {														FanConOff();}
			}
			break;			
		case PID_COOLING_CONT:
			Error0 = setValue - actValue;
			ErrorSum += Error0;

			pidValue=pidCompute();
// fast cooling 사용여부...

			if(pidValue >= -0.) {	// 쿨링 
				pidValue=0.;	//  상한값
				FanConOff();
			}
			else {
				if(pidValue < -255.) pidValue = -255.;
				FanConOn(FAN_POWER_MAX);
				FanConOn((int)(-pidValue) );				
			}

			CoolerPowerOn( (int)(-pidValue) );
			break;			
	}
}

float pidCompute()
{
	if(ErrorSum > ErrorSumMax ) {
		ErrorSum = ErrorSumMax;
	}
	else if(ErrorSum < ErrorSumMin) {
		ErrorSum = ErrorSumMin;
	}
	pidValue = pid.Kp * (	(Error0) +
							pid.Ki * ErrorSum +
							pid.Kd * (Error0-Error1)
						);
	Error2=Error1;
	Error1=Error0;
	return pidValue;
}

void PID_Function_Blank(float actValue ,float setValue ,uint32_t setTempReset)
{
}




void TempControlFunc()	 // 5hz
{
#if 0
//					pidOven.pidAutoTunning(baseaddrOven, gOVEN_STATE.fTemperature, gOVEN_STATE.fSetTemperature);
						if(gOVEN_STATE.btState == OVEN_STATE_GRADIENT_MODE)
						{
// Gradient table이 잇으면 						
#ifdef GRADIENT_CURVE
							ovenStt.setTempReset = TEMP_PROG_MODE;
							tempControl(gOVEN_STATE.fTemperature, ovenStt.fSetTemperature, ovenStt.setTempReset);
#else
							tempControl(gOVEN_STATE.fTemperature, ovenStt.fSetTemperature, ovenStt.setTempReset);
							ovenStt.setTempReset = TEMP_NOT_RESET;
#endif
						}
						else
						{
							tempControl(gOVEN_STATE.fTemperature, ovenStt.fSetTemperature, ovenStt.setTempReset);
							ovenStt.setTempReset = TEMP_NOT_RESET;//NO;	// 온도설정값이 변경되었는가?  -- 온도가 변경될대만
						}
#endif



// RYU_SF2
	#if AUTO_TUNNING_CONTROL
		pPID_Control_Fn(LcasState.fTemp, AUTO_TUNNING_SET_TEMP, devState.setTempReset);
	#else
		pPID_Control_Fn(LcasState.fTemp, runSetup.temp, devState.setTempReset);
	#endif



						
}


void TempReadyCheck()
{
// RYU_SF2
	if(LcasState.fTemp >= devState.tempReadyUnder && LcasState.fTemp <= devState.tempReadyOver) {
		LcasState.tempReady = YES;
	}
	else {
		LcasState.tempReady = NO;
	}
}























#if 0
// pPID_Control_Fn 맨 처음 초기화 부분 
void PID_Function_Init(float actValue ,float setValue,uint32_t setTempReset)
{
	static uint32_t i = 0;
//	gbPIDCtrlTick = NO;
	
	if(++i == 20) {
		i = 0;
#if AUTO_TUNNING_CONTROL
		pPID_Control_Fn = pidAutoTunningInit;
#else
		pPID_Control_Fn = PID_Control_Init;
#endif
	}
}
#endif

#if 0

int a(int aa)
{
	dp("aa=%d\n",aa);
	return aa;
}

int b(int bb)
{
	dp("bb=%d\n",bb);	
	return bb;
}

int c(int cc)
{
	dp("cc=%d\n",cc);	
	return cc;
}

int (*p[3])(int);// = {a, b , c}; //함수포인터배열을민들어함수주소저장

//선택한 메뉴에따라배열첨자 이용해함수호출
	void (*p) (char *); // 함수포인터 선언
	char a[] = "pointer";
	p = test; // 함수 포인터 에 tes 함수 주소 대입
	p(a); // 함수포인터 를이용해함수호출
	int (*p[3])(int) = {a, b , c}; //함수포인터배열을민들어함수주소저장

	
	//선택한 메뉴에따라배열첨자 이용해함수호출
	z = p[i-1](4);

p[0] = b;
p[1] = c;
p[2] = a;

p[0](111);
p[1](100);
p[2](200);

	
#endif	
