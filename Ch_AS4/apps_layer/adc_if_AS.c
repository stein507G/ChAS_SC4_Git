#include "config.h"

// ADC Clock source
// FPGA : 16.471Mhz
// Use external clock source to change the frequency.

#if 0
#include "protocol_Macro.h"
#include "protocol_CMD_Oven.h"
#include "protocolType_oven.h"
#include "protocolType_HPLC.h"
#endif

#include "adc_if_AS.h"
#include "util.h"

#include "fpga_ChAS.h"


ADC_ST adcif;
uint32_t ConfigEN;
uint8_t ConfigDat[32];


extern LCAS_DEV_STATE_t devState;		// 통신프로토콜에 없는 것
extern LCAS_DIAG_DATA_t diagData;


#if TEMP_CAL_DATA_FORMAT_DOUBLE
double GfTempConstA;
double GfTempConstB;
double GfTempConstC;
#else
float GfTempConstA;
float GfTempConstB;
float GfTempConstC;
#endif

//extern LCAS_DIAG_DATA_t diagData;
extern LCAS_STATE_t LcasState;
extern LCAS_RUN_SETUP_t runSetup;

//LCAS_TEMP_CAL_t tempCalData;


void adc_init(void)
{
	adcif.State = ADCST_POR_READY;
	adc_Reset();
	adcif.intr  = 0;
	adcif.cnt = 0;
	adcif.chkcnt = 0;
	ConfigEN = 0;
	adcif.procen = 0;
	adcif.Dbgen = 0;

// ConfigDat[0x0B] = 0x68; ConfigEN |= (1<<0x0B);	// internal reference enable  == 2.442 V
// Chrozen LC Oven / AS : 외부 레퍼런스 사용 
	ConfigDat[0x0B] = 0x60; ConfigEN |= (1<<0x0B);		// external reference		== 2.5V
	ConfigDat[0x0C] = 0x3E; ConfigEN |= (1<<0x0C);

// ==========================================================================================
// 1kHz --> 1k/5 => 200hz 
//	ConfigDat[0x0D] = 0x02; ConfigEN |= (1<<0x0D);		// 001 : fICLK = fCLKIN / 2
//	ConfigDat[0x0E] = 0x20; ConfigEN |= (1<<0x0E);		// 001 : fMOD = fICLK / 2   , 0000 : fDATA = fMOD / 4096

// ==========================================================================================
// 50Hz --> 50/5 => 10hz 
	ConfigDat[0x0D] = 0x08; ConfigEN |= (1<<0x0D);		// 100 : fICLK = fCLKIN / 8 (default)
	ConfigDat[0x0E] = 0xA0; ConfigEN |= (1<<0x0E);		// 101 : fMOD = fICLK / 10   , 0000 : fDATA = fMOD / 4096

	ConfigDat[0x0F] = 0x0F; ConfigEN |= (1<<0x0F);
	ConfigDat[0x11] = 0x00; ConfigEN |= (1<<0x11);
	ConfigDat[0x12] = 0x00; ConfigEN |= (1<<0x12);
	ConfigDat[0x13] = 0x00; ConfigEN |= (1<<0x13);
	ConfigDat[0x14] = 0x00; ConfigEN |= (1<<0x14);
// 

}

uint8_t adc_ProcInterval(void)
{
// every 1mSec
//	if(adcif.tick1ms)
	{
		adcif.tick1ms = 0;
		adcif.cnt++;
		if(adcif.cnt > 10)
		{
			adcif.cnt = 0;
			adcif.procen = 1;
		}
	}
	return adcif.procen;
}

//================================================
//       CH1    CH2        CH3    CH4
//------------------------------------------------
// sel0  Oven		Ext_Temp   V3.3   D_err 
// Sel1  Oven		Ext_Temp   V5     L_err   
// sel2  Oven		Ext_Temp   NV5 XX T1_err
// Sel3  Oven		Ext_Temp   V12    T2_err

// 50Hz의 데이터를 
// 5개 저장한 후에 인터럽트 발생(10Hz)시킨다. 
// 현재 5개중 1,2,3은 버린고 4,5번은 활용한다.
// -- 1,2,3,4를 버리자. ...

float minPwr[8]	=		{0   , 3.135, 4.75, 1.425, 2.375, 14.7/*11.16*/, 4.75, -6.3};
float maxPwr[8]	=		{17  , 3.465, 5.25, 1.575, 2.625, 15.3/*12.84*/, 5.25, -4.7};
float referencePwr[8]=	{1.50, 3.3,   5.0,  1.5,   2.5,   12.0, 5.0, -5.0};
float factorPwr[8] =		{2.0 , 3.0,   3.0,  2.0,   2.0,   11.0, 3.0, -3.96};	// - , 1/3 , 1/3 , 1/2 , 1/2 , 1/3 , 1/11


uint32_t adc_Pow[4];
uint32_t adc_Sensor[4];
float errVolt[4];	

void ADC_reading(void)
{
	int i;
	static int k;
	
	uint32_t fnNo;
	uint8_t mux_Ch;
	uint8_t mux_Ch4;

// Temp
	uint32_t ad;
	static uint32_t uAdcSum = TEMP_ADC_SUM;	// 1000*16
	uint32_t uAdcAve = 1000;	// 1000*16	
	static uint32_t uTmp[16] = {1000,1000,1000,1000,1000,1000,1000,1000,
		1000,1000,1000,1000,1000,1000,1000,1000};
	static uint32_t array = 0, cnt = 0;

// Ext Temp
	uint32_t extAD;
	static uint32_t extADSum = TEMP_ADC_SUM;	// 1000*16
	uint32_t extADAve = 1000;	// 1000*16	
	static uint32_t uExtTmp[16] = {1000,1000,1000,1000,1000,1000,1000,1000,
		1000,1000,1000,1000,1000,1000,1000,1000};

//	uint32_t adc_Pow[4];
//	uint32_t adc_Sensor[4];

	float oven_Volt;
	float ext_Temp;	

	static uint32_t readValid = 0;


int ret = OK;

	float sysVolt;
	unsigned char sysVoltCheck;


	//	if(adcif.State == ADCST_RUN) {
	if(adcif.intr)
	{
		adcif.intr =0;

		ad = FPGA_READ_WORD(0x310C);
		mux_Ch = (ad >> 24) & 0x03;
//	mux_Ch4 = mux_Ch & 0x03;

		ad = ad & 0x00ffffff;	// ch1
		extAD = FPGA_READ_WORD(0x310D);	// ch2
		adc_Pow[mux_Ch] = FPGA_READ_WORD(0x310E);			// ch3
		adc_Sensor[mux_Ch] = FPGA_READ_WORD(0x310F);		// ch4

// oven & tray temp
		uAdcSum -= uTmp[array];
		uTmp[array] = ad;
		uAdcSum += uTmp[array];

#if 1
		uAdcAve = uAdcSum >> TEMP_AVE_DIV;
		oven_Volt = uAdcAve * DIGIT_TO_VOLT_FACTOR;
#else
		oven_Volt = uAdcSum * DIGIT_TO_VOLT_FACTOR;
#endif
		
		//gOVEN_STATE.fTemperature = oven_Volt * (GfTempConstA * oven_Volt + GfTempConstB) + GfTempConstC;
		LcasState.fTemp = oven_Volt * (GfTempConstA * oven_Volt + GfTempConstB) + GfTempConstC;

// ext temp
		extADSum -= uExtTmp[array];
		uExtTmp[array] = extAD;
		extADSum += uExtTmp[array];
//		extADAve = extADSum >> TEMP_AVE_DIV;

		ext_Temp = extADSum * SUM_DIGIT_TO_EXT_TEMP_FACTOR;

//printf("ad=%08X extTemp=%08X %8.4f %8.4f %8.3f\n", ad, extAD, ext_Temp, oven_Volt, gOVEN_STATE.fTemperature);

		switch(mux_Ch) {
			case 0:
				sysVolt = diagData.V3_3Dvalue = adc_Pow[mux_Ch] * POW_V33_DIGIT_TO_VOLT_FACTOR;
				
				if(sysVolt > minPwr[1] && sysVolt < maxPwr[1]) {
					sysVoltCheck = OK;
				}
				else {
					sysVoltCheck = ERROR;
			// 여기에 에러처리를 삽입하라.
				}

				diagData.V3_3Dcheck = sysVoltCheck;
				break;
			case 1:
				sysVolt = diagData.V5Dvalue = adc_Pow[mux_Ch] * POW_V5_DIGIT_TO_VOLT_FACTOR;
				
				if(sysVolt > minPwr[2] && sysVolt < maxPwr[2]) {
					sysVoltCheck = OK;
				}
				else {
					sysVoltCheck = ERROR;
			// 여기에 에러처리를 삽입하라.
				}

				diagData.V5Dcheck = sysVoltCheck;
				break;
			case 2:
				if(devState.adReadValid == NO) {	// 온도 에러 체크 시작점 
					if(++readValid == 10) {
						readValid = 10;
						devState.adReadValid = YES;
					}
				}
				break;
			case 3:
				sysVolt = diagData.V12Dvalue = adc_Pow[mux_Ch] * POW_V12_DIGIT_TO_VOLT_FACTOR;

				if(sysVolt > minPwr[5] && sysVolt < maxPwr[5]) {
					sysVoltCheck = OK;
				}
				else {
					sysVoltCheck = ERROR;
			// 여기에 에러처리를 삽입하라.
				}

				diagData.V12Dvalue = sysVoltCheck;

devState.powerCheck = YES;
				break;
			}

devState.powerState = ret;

			if(++array == TEMP_SUM_TIME) {
				array = 0;

//printf("ad=%08X extTemp=%08X %8.4f %8.4f %8.3f\n", ad, extAD, ext_Temp, oven_Volt, gOVEN_STATE.fTemperature);			
//printf("Power v33=%8.4f v50=%8.4f v12=%8.4f\n", ovenDiag.sysVolt[1], ovenDiag.sysVolt[2], ovenDiag.sysVolt[5]);
//printf("Error D_err=%8.4f L_err=%8.4f T1_err=%8.4f T2_err=%8.4f\n", errVolt[0],errVolt[1],errVolt[2],errVolt[3]);
			}


static int act = 0;
static int readyCheck = 0;
#if 0
			if(ovenStt.btHeating)	{
//				if(++act == 1) {
				if(++act == 2) {	// 5Hz ?
					act = 0;
					TempControlFunc();
				}
				else if(++readyCheck == 5) {		// 1Hz 
					TempReadyCheck();
					readyCheck = 0;
				}
			}
			else {
				ovenStt.tempReady = NOT_READY;		// 온도 안정되었는가 ? 임시 값
				gOVEN_STATE.btTempReady = NOT_READY;
				ledReadyRun(LED_OFF);				
			}
#else

			if(++act == 2) {	// 5Hz ?
				act = 0;
			// RYU_SF2

#if AUTO_TUNNING_CONTROL
				pPID_Control_Fn(LcasState.fTemp, AUTO_TUNNING_SET_TEMP, devState.setTempReset);
#else
				pPID_Control_Fn(LcasState.fTemp, runSetup.temp, devState.setTempReset);
#endif
			}
			else if(++readyCheck == 5) {		// 1Hz 
				TempReadyCheck();
				readyCheck = 0;
			}
#endif

		}

}

void adc_process(void)
{
	uint16_t RVal; // Received value
	uint16_t CVal; // Check value
	
#if 0	
	if(adcif.State == ADCST_RUN)
	{
		if(adcif.intr)
		{
			adcif.intr =0;
			adc_GetData();
		}
	}
	else 
#endif		

	if(adc_ProcInterval())
	{
		adcif.procen = 0;

		//----------------------------------------------
		if(adcif.State == ADCST_POR_READY)
		{
			adc_Set();

			if(adc_CmdNull()==0xFF04)
			{
				adcif.chkcnt++;
				if(adcif.chkcnt > 5)
				{
					adcif.chkcnt = 0;
					adcif.State = ADCST_UNLOCK;
				}
			}
			else
				adcif.chkcnt = 0;
		}
		//----------------------------------------------
		else if(adcif.State == ADCST_UNLOCK)
		{
			if((adc_CmdUnlock()&0x1FFF)==0x0655)
			{
				adcif.chkcnt = 0;
				adcif.i =0;
				adcif.State = ADCST_CONFIG_REGS;
			}
			else
				adcif.chkcnt = 0;
		}
		//----------------------------------------------
		else if(adcif.State == ADCST_CONFIG_REGS)
		{
			if(((1<<adcif.i)&ConfigEN) > 0)
			{
				CVal = adcif.i;
				CVal = (CVal<<8) + ConfigDat[adcif.i];
				RVal = adc_CmdWriteReg(adcif.i, ConfigDat[adcif.i]);

				if((RVal&0x1FFF)==CVal)
				{
					adcif.chkcnt = 0;
					adcif.i++;
				}
				else
					adcif.chkcnt = 0;
			}
			else
			{
				adcif.i++;
				if(adcif.i > 31)
				{
					adcif.State = ADCST_ENABLE;
				}
				adcif.chkcnt = 0;
			}
		}
		else if(adcif.State == ADCST_ENABLE)
		{
			if((adc_CmdEnable()&0x1FFF)==0x0F0F)
			{
				adcif.chkcnt = 0;
				adcif.i =0;
				adcif.State = ADCST_WAKEUP;
			}
			else
				adcif.chkcnt = 0;
		}
		else if(adcif.State == ADCST_WAKEUP)
		{
			if((adc_CmdWakeup()&0x1FFF)==0x0033)
			{
				adcif.chkcnt = 0;
				adcif.i =0;
				adcif.State = ADCST_LOCKREG;
			}
			else
				adcif.chkcnt = 0;
		}
		else if(adcif.State == ADCST_LOCKREG)
		{
			if((adc_CmdLock()&0x1FFF)==0x0555)
			{
				adcif.chkcnt = 0;
				adcif.i =0;
iprintf("ADC initialization completed ^^!\r\n");
				adc_InterruptEnable();
				adcif.State = ADCST_RUN;

				pfAdcProcess = ADC_reading;
			}
			else
				adcif.chkcnt = 0;
		}
		else
		{
			adcif.State = ADCST_POR_READY;
		}
	}

}

void adc_process_blank(void){}

void adc_Reset(void)
{
	FPGA_WRITE_WORD(0x3010, 0x12345678);
}

void adc_Set(void)
{
	FPGA_WRITE_WORD(0x3010, 0x87654321);
}

void adc_InterruptEnable(void)
{
	FPGA_WRITE_WORD(0x3003, 1);
}

uint32_t adc_CmdNull(void)
{
	FPGA_WRITE_WORD(0x3001, 0x0);
	return FPGA_READ_WORD(0x3002);
}

uint32_t adc_CmdUnlock(void)
{
	FPGA_WRITE_WORD(0x3001, 0x0655);
	return FPGA_READ_WORD(0x3002);
}

uint32_t adc_CmdEnable(void)
{
	FPGA_WRITE_WORD(0x3001, 0x4F0F); // address 0xF, write 0x0F ( 4 channels)
	return FPGA_READ_WORD(0x3002);
}

uint32_t adc_CmdWakeup(void)
{
	FPGA_WRITE_WORD(0x3001, 0x0033);
	return FPGA_READ_WORD(0x3002);
}

uint32_t adc_CmdLock(void)
{
	FPGA_WRITE_WORD(0x3001, 0x0555);
	return FPGA_READ_WORD(0x3002);
}

uint32_t adc_CmdReadReg(uint32_t addr)
{
	FPGA_WRITE_WORD(0x3001, 0x2000 + ((addr <<8)&0x1F00));
	return FPGA_READ_WORD(0x3002);
}

uint32_t adc_CmdWriteReg(uint32_t addr, uint8_t data)
{
	FPGA_WRITE_WORD(0x3001, 0x4000 + ((addr <<8)&0x1F00) + data);
	return FPGA_READ_WORD(0x3002);
}

void adc_GetData(void)
{
	uint8_t i;
	static int k = 0;

	for(i=0;i<20;i++)
//	for(i=8;i<20;i++)
	{
		adcif.Dat[i] = FPGA_READ_WORD(0x3100 + i);
	}

if(k==0	|| k==1 || k==2) ShowMemoryWord(adcif.Dat,20);
if(++k == 16) {
	k = 0;
//ShowMemoryWord(adcif.Dat,20);	
}
//ShowMemoryWord(adcif.Dat,8);	
/*
	if(adcif.Dbgen)
	{
		adcif.Dbgen =0;
		ShowMemoryWord(adcif.Dat,20);
	}
*/

}





